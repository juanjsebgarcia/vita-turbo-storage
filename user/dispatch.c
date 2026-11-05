/*
 * dispatch.c - the developer command server (TCP :1340), SceShell instance only.
 *
 * Same listener as Wi-Fi Turbo's :1339 surface: it only listens while
 * Developer mode is on (a Turbo Storage setting, off by default), so a normal
 * console has no open port. One command per line, one reply per command:
 *
 *   help                      this list
 *   version                   plugin version
 *   ping                      round-trip to the kernel module
 *   status                    settings, per-device status and counters
 *   set <key> <value>         enabled|sd2vita|memcard|game_pause|
 *                             verify|devmode|hud_game|hud_recent 0/1,
 *                             hud 0-2 (off/compact/extended), hud_pos 0-3
 *
 * BOOT-SAFE: module_start only starts a small watch thread; it waits ~3 s
 * before anything else (touching the network during early SceShell boot can
 * wedge the whole boot), then polls the kernel settings every 5 s. The
 * server thread, with its larger stack, exists only while Developer mode is
 * on, so a normal console costs SceShell one 4 KiB stack.
 */
#include <psp2/kernel/threadmgr.h>
#include <psp2/net/net.h>
#include <stdarg.h>
#include <string.h>

#include "../include/storageturbo.h"
#include "../include/storageturbo_kernel_api.h"
#include "dispatch.h"
#include "../settings_app/tslog.h"

#define TS_DEFERRED_BRINGUP_DELAY_US  (3 * 1000 * 1000)

static const char *const k_keys[TS_KEY_COUNT] = {
	"sd2vita", "-retired1", "verify", "hud", "hud_pos", "devmode", "memcard", "hud_game", "hud_recent",
	"enabled", "game_pause",
};
static const char *const k_status[] = { "off", "active", "fallback", "no-buffer", "unavailable",
                                        "paused" };
static const char *const k_dev[TS_DEV_COUNT] = { "sd2vita", "memcard" };
static const char *const k_fg[] = { "system", "app", "Vita game", "PSP emulator" };

static volatile int g_run = 0;
static SceUID g_thread = -1;          /* server: only while Developer mode is on */
static volatile int g_thread_done;    /* the server thread has returned          */
static volatile int g_srv = -1;
static SceUID g_watch = -1;

/* Tiny formatter (no libc printf in a taiHEN user module): %s %d %u %x with an
 * optional zero-pad width, enough for the replies below. Returns chars written. */
static int put(char *p, char *end, const char *fmt, ...)
{
	char *o = p;
	va_list ap;

	if (p >= end)
		return 0;
	va_start(ap, fmt);
	for (; *fmt && o < end - 1; fmt++) {
		if (*fmt != '%') {
			*o++ = *fmt;
			continue;
		}
		int zero = 0, width = 0;
		fmt++;
		if (*fmt == '0')
			zero = 1, fmt++;
		while (*fmt >= '0' && *fmt <= '9')
			width = width * 10 + (*fmt++ - '0');
		if (*fmt == 's') {
			const char *a = va_arg(ap, const char *);
			while (a && *a && o < end - 1)
				*o++ = *a++;
		} else if (*fmt == 'd' || *fmt == 'u' || *fmt == 'x') {
			unsigned v = va_arg(ap, unsigned);
			char t[12];
			int k = 0, neg = 0, base = *fmt == 'x' ? 16 : 10;

			if (*fmt == 'd' && (int)v < 0)
				neg = 1, v = -v;
			do { t[k++] = "0123456789abcdef"[v % base]; v /= base; } while (v && k < 11);
			while (k < width && k < 11)
				t[k++] = zero ? '0' : ' ';
			if (neg && o < end - 1)
				*o++ = '-';
			while (k && o < end - 1)
				*o++ = t[--k];
		} else if (*fmt == '%') {
			*o++ = '%';
		} else {
			break;
		}
	}
	va_end(ap);
	*o = 0;
	return (int)(o - p);
}

static int parse_int(const char *s, int *v)
{
	int n = 0, any = 0;

	while (*s == ' ')
		s++;
	while (*s >= '0' && *s <= '9') {
		n = n * 10 + (*s++ - '0');
		any = 1;
	}
	*v = n;
	return any;
}

void ts_dispatch_exec(const char *line, char *out, int cap)
{
	char *p = out, *end = out + cap;

	out[0] = 0;
	if (!strcmp(line, "help")) {
		put(p, end, "commands: help | version | ping | status | set <key> <value>\n"
		            "keys: sd2vita memcard verify devmode hud_game hud_recent (0/1), hud (0-2), hud_pos (0-3)");
	} else if (!strcmp(line, "version")) {
		put(p, end, "turbo-storage %s", TS_BUILD_STR);
	} else if (!strcmp(line, "ping")) {
		int r = tsSelfTestPing();
		put(p, end, "ping %s (0x%08x)", r == (int)TS_SELFTEST_MAGIC ? "ok" : "FAILED", (unsigned)r);
	} else if (!strcmp(line, "status")) {
		TsState st;

		if (tsGetState2(&st, sizeof(st)) != TS_OK || st.magic != TS_STATE_MAGIC) {
			put(p, end, "status: kernel module not answering");
			return;
		}
		p += put(p, end, "turbo-storage %s disabled=%d fingerprint=0x%08x uptime=%us verify_ready=%d\n",
		         TS_BUILD_STR, (int)st.disabled, (unsigned)st.fingerprint,
		         (unsigned)st.uptime_s, (int)st.verify_ready);
		p += put(p, end, "settings: enabled=%d game_pause=%d sd2vita=%d memcard=%d verify=%d hud=%d hud_pos=%d hud_game=%d hud_recent=%d devmode=%d\n",
		         (int)st.settings.enabled, (int)st.settings.game_pause,
		         (int)st.settings.sd2vita, (int)st.settings.memcard,
		         (int)st.settings.verify, (int)st.settings.hud, (int)st.settings.hud_pos, (int)st.settings.hud_game, (int)st.settings.hud_recent,
		         (int)st.settings.devmode);
		p += put(p, end, "memcard slow mode: %s\n", st.memcard_slow == 1 ? "ON" :
		         st.memcard_slow == 0 ? "off" : "unknown");
		p += put(p, end, "in front: %s (%s)%s\n", st.fg_title[0] ? st.fg_title : "-",
		         k_fg[st.fg_class & 3], st.paused ? " - storage stock" : "");
		for (int i = 0; i < TS_DEV_COUNT; i++) {
			const TsDevState *d = &st.dev[i];
			int s = d->status >= 0 && d->status <= TS_ST_PAUSED ? d->status : TS_ST_UNAVAIL;

			p += put(p, end, "%s: %s reads=%u (%u MiB) writes=%u (%u MiB) busy=%u held=%u nc=%u errors=%u "
			         "verified=%u mismatches=%u\n", k_dev[i], k_status[s], (unsigned)d->reads,
			         (unsigned)d->read_mib, (unsigned)d->writes, (unsigned)d->write_mib,
			         (unsigned)d->passthru, (unsigned)d->held, (unsigned)d->uncached, (unsigned)d->errors, (unsigned)d->verified,
			         (unsigned)d->mismatches);
		}
		if (p > out && p[-1] == '\n')
			p[-1] = 0;
	} else if (!strncmp(line, "set ", 4)) {
		const char *k = line + 4;
		int key = -1, v;

		for (int i = 0; i < TS_KEY_COUNT; i++) {
			if (i == TS_KEY_RETIRED1)
				continue;   /* removed setting: not addressable */
			int n = (int)strlen(k_keys[i]);
			if (!strncmp(k, k_keys[i], n) && k[n] == ' ') {
				key = i;
				k += n;
				break;
			}
		}
		if (key < 0 || !parse_int(k, &v)) {
			put(p, end, "set: usage: set <key> <value> (see help)");
			return;
		}
		int r = tsSetSetting(key, v);
		put(p, end, "set %s = %d -> %s", k_keys[key], v, r == TS_OK ? "ok" : "rejected");
		TS_LOGI("[DEV] set %s = %d -> %d", k_keys[key], v, r);
	} else {
		put(p, end, "unknown command (try: help)");
	}
}

static int devmode_on(void)
{
	TsState st;
	return tsGetState2(&st, sizeof(st)) == TS_OK && st.magic == TS_STATE_MAGIC && st.settings.devmode;
}

static int server_thread(SceSize args, void *argp)
{
	(void)args; (void)argp;

	/* Self-healing listener, as in Wi-Fi Turbo: a dead socket is rebuilt rather
	 * than spun on. Returns once Developer mode is off. */
	while (g_run && devmode_on()) {
		int srv = sceNetSocket("tsdev", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
		if (srv >= 0) {
			SceNetSockaddrIn addr;
			memset(&addr, 0, sizeof(addr));
			addr.sin_family = SCE_NET_AF_INET;
			addr.sin_port = sceNetHtons(TS_DEV_PORT);
			addr.sin_addr.s_addr = sceNetHtonl(SCE_NET_INADDR_ANY);
			if (sceNetBind(srv, (SceNetSockaddr *)&addr, sizeof(addr)) < 0) {
				TS_LOGI("[DEV] bind :%d failed - retrying", TS_DEV_PORT);
				sceNetSocketClose(srv);
				srv = -1;
			} else {
				sceNetListen(srv, 1);
				g_srv = srv;
				TS_LOGI("[DEV] listening on TCP :%d", TS_DEV_PORT);
			}
		}
		if (srv < 0) {
			sceKernelDelayThread(2 * 1000 * 1000);
			continue;
		}

		int accept_fails = 0;
		while (g_run) {
			int cli = sceNetAccept(srv, NULL, NULL);
			if (cli < 0 && !devmode_on())
				break;
			if (cli < 0) {
				if (++accept_fails >= 25) {
					TS_LOGI("[DEV] accept failing (0x%08x) - rebuilding listener", (unsigned)cli);
					break;
				}
				sceKernelDelayThread(200 * 1000);
				continue;
			}
			accept_fails = 0;
			char line[160];
			char reply[1024];
			for (;;) {
				int n = sceNetRecv(cli, line, sizeof(line) - 1, 0);
				if (n <= 0)
					break;
				line[n] = 0;
				while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r'))
					line[--n] = 0;
				ts_dispatch_exec(line, reply, sizeof(reply));
				int rl = (int)strlen(reply);
				reply[rl++] = '\n';
				sceNetSend(cli, reply, rl, 0);
				if (!devmode_on())
					break;
			}
			sceNetSocketClose(cli);
			if (!devmode_on())
				break;
		}
		g_srv = -1;
		sceNetSocketClose(srv);
		TS_LOGI("[DEV] listener closed");
	}
	g_thread_done = 1;
	return 0;
}

static void server_reap(void)
{
	if (g_thread < 0)
		return;
	sceKernelWaitThreadEnd(g_thread, NULL, NULL);
	sceKernelDeleteThread(g_thread);
	g_thread = -1;
}

/* Developer mode is switched in the Settings app, a different process, so
 * nothing there can reach this socket. Watch the setting every 5 s: start
 * the server when it goes on; when it goes off, close the listener (that wakes
 * the blocked accept() and the server returns, as Wi-Fi Turbo closes its
 * socket) and reap the thread. */
static int watch_thread(SceSize args, void *argp)
{
	(void)args; (void)argp;
	sceKernelDelayThread(TS_DEFERRED_BRINGUP_DELAY_US);
	while (g_run) {
		int on = devmode_on();
		if (g_thread >= 0 && g_thread_done)
			server_reap();
		if (on && g_thread < 0) {
			g_thread_done = 0;
			g_thread = sceKernelCreateThread("TsDevServer", server_thread, 0x40, 0x4000,
			                                 0, 0, NULL);
			if (g_thread >= 0 && sceKernelStartThread(g_thread, 0, NULL) < 0) {
				sceKernelDeleteThread(g_thread);
				g_thread = -1;
			}
		}
		int srv = g_srv;
		if (srv >= 0 && !on)
			sceNetSocketClose(srv);
		for (int i = 0; i < 5 && g_run; i++)     /* every 5 s; stops within 1 s */
			sceKernelDelayThread(1000 * 1000);
	}
	return 0;
}

void ts_dispatch_start(void)
{
	if (g_watch >= 0)
		return;
	g_run = 1;
	g_watch = sceKernelCreateThread("TsDevWatch", watch_thread, 0x40, 0x1000, 0, 0, NULL);
	if (g_watch >= 0 && sceKernelStartThread(g_watch, 0, NULL) < 0) {
		sceKernelDeleteThread(g_watch);
		g_watch = -1;
	}
	if (g_watch < 0)
		g_run = 0;
}

void ts_dispatch_stop(void)
{
	g_run = 0;
	if (g_srv >= 0)
		sceNetSocketClose(g_srv);   /* unblocks accept() */
	if (g_watch >= 0) {
		sceKernelWaitThreadEnd(g_watch, NULL, NULL);
		sceKernelDeleteThread(g_watch);
		g_watch = -1;
	}
	server_reap();
}
