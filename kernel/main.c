/*
 * main.c - SceStorageTurbo kernel module entry point.
 *
 * Responsibilities:
 *   - bring up the logger (klog.c);
 *   - check the storage driver is one this build knows (3.60-3.74); on any
 *     other firmware install nothing at all;
 *   - the crash-loop boot guard: if the last start never finished, install
 *     nothing (safe mode);
 *   - allocate the staging buffers and install the I/O hooks (stage.c), then
 *     load and apply the saved settings (settings.c);
 *   - run the 1 Hz worker: saves settings, allocates the verify buffer on
 *     demand, feeds the HUD (hud.c, installed only once the HUD is first
 *     switched on), logs status changes, re-arms the boot guard;
 *   - publish the user-callable "SceStorageTurbo" syscall library (see
 *     include/storageturbo_kernel_api.h and storageturbo_kernel.yml).
 *
 * The exported functions are the user->kernel boundary. The state is copied out
 * with ksceKernelMemcpyKernelToUser, which fails cleanly on a bad pointer; key
 * ids and values are validated before anything changes. They do no file I/O and
 * no logging themselves - the worker saves and logs within a second.
 */
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>    /* ksceKernelMemcpy{User,Kernel} macros */
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/kernel/threadmgr.h>
#include <taihen.h>

#include "../include/storageturbo.h"
#include "../include/storageturbo_kernel_api.h"
#include "klog.h"
#include "settings.h"
#include "stage.h"
#include "hud.h"
#include "hud_i18n.h"

#define TS_GUARD_PATH  TS_DATA_DIR "/boot_guard"
#define TS_GUARD_SECS  30
#define TS_GUARD_SHELL_SECS  5   /* after SceShell starts, clear the guard this soon */

/* Storage driver (SceSdif) module fingerprints of the firmwares this build
 * supports. Its code is identical across all of them; anything else stays
 * stock. */
static const uint32_t k_known_fw[] = {
	0x2e7c52f7,   /* 3.60 */
	0x707d9ddd,   /* 3.61 */
	0x10046e19,   /* 3.63 */
	0x285023ed,   /* 3.65 */
	0x555b752a,   /* 3.67 */
	0x65b8a859,   /* 3.68 */
	0x6a9a4d5b,   /* 3.69 */
	0xc39462fe,   /* 3.70 */
	0x70e63db7,   /* 3.71 */
	0x22158017,   /* 3.72 */
	0x74b301ef,   /* 3.73 */
	0xabd91b5b,   /* 3.74 */
};

static int      g_disabled = TS_DIS_NONE;
static uint32_t g_fingerprint;
static SceUID   g_worker = -1;
static volatile int g_run = 1;
static volatile uint32_t g_uptime;

static char g_guard_txt[48];   /* last content read, for the log */
static volatile int      g_guard_clear;   /* this start's guard is already back at 0 */
static volatile uint32_t g_shell_up;      /* worker second SceShell reported up, +1  */
static volatile int      g_safe_left;     /* safe mode left from Settings this boot  */

/* First byte of the guard file, or 0 if there is none. */
static char guard_read(void)
{
	memset(g_guard_txt, 0, sizeof(g_guard_txt));
	SceUID fd = ksceIoOpen(TS_GUARD_PATH, SCE_O_RDONLY, 0);
	if (fd >= 0) {
		ksceIoRead(fd, g_guard_txt, sizeof(g_guard_txt) - 1);
		ksceIoClose(fd);
	}
	for (int i = 0; g_guard_txt[i]; i++)
		if (g_guard_txt[i] < ' ' || g_guard_txt[i] > '~')
			g_guard_txt[i] = '.';
	return g_guard_txt[0];
}

static void guard_write(char c, const char *who)
{
	char buf[64];
	int n = 0;
	buf[n++] = c;
	n += snprintf(buf + n, sizeof(buf) - n, " up=%u by=%s\n",
	              (unsigned)ksceKernelGetSystemTimeLow(), who);
	SceUID fd = ksceIoOpen(TS_GUARD_PATH, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
	if (fd >= 0) {
		ksceIoWrite(fd, buf, n);
		ksceIoClose(fd);
	}
}

static int firmware_known(void)
{
	tai_module_info_t info;

	memset(&info, 0, sizeof(info));
	info.size = sizeof(info);
	if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSdif", &info) < 0)
		return 0;
	g_fingerprint = info.module_nid;
	for (unsigned i = 0; i < sizeof(k_known_fw) / sizeof(k_known_fw[0]); i++)
		if (k_known_fw[i] == g_fingerprint)
			return 1;
	return 0;
}

/* ---- 1 Hz worker ------------------------------------------------------------ */

static const char *const k_dev_short[TS_DEV_COUNT] = { "SD2V", "INT ", "MC  " };   /* compact */
static const char *const k_dev_log[TS_DEV_COUNT] = { "sd2vita", "internal", "memcard" };
/* HUD order: the removable storages first. */
static const int k_hud_order[TS_DEV_COUNT] = { TS_DEV_SD2VITA, TS_DEV_MEMCARD, TS_DEV_INTERNAL };

int module_get_export_func(SceUID pid, const char *modname, uint32_t libnid,
                           uint32_t funcnid, uintptr_t *func);

/* System language for the HUD words, from the registry; English until known. */
static int g_lang = 1;

static void read_lang(void)
{
	static int (*get_int)(const char *, const char *, int *) = 0;
	static int tried = 0;
	if (!tried) {
		uintptr_t f = 0;
		tried = 1;
		if (module_get_export_func(KERNEL_PID, "SceRegistryMgr", 0xB2223AEB,
		                           0x16DDF3DC, &f) >= 0)
			get_int = (int (*)(const char *, const char *, int *))f;
	}
	int v = 1;
	if (get_int && get_int("/CONFIG/SYSTEM", "language", &v) >= 0)
		g_lang = v;
}

/* "12.3" from bytes per second. */
static void mbps(char *o, int n, uint64_t bytes)
{
	uint32_t t = (uint32_t)((bytes * 10) >> 20);   /* tenths of a MiB/s */
	snprintf(o, n, "%u.%u", (unsigned)(t / 10), (unsigned)(t % 10));
}

static void hud_feed(const TsDevState *now)
{
	static uint64_t last_rd[TS_DEV_COUNT], last_wr[TS_DEV_COUNT];
	static char L[TS_HUD_LINES][TS_HUD_LINE_MAX];
	static int hud_failed;           /* tried once and failed: do not retry */
	TsSettings s;
	int n = 0;

	ts_settings_copy(&s);
	if (s.hud != TS_HUD_OFF)
		read_lang();
	if (s.hud != TS_HUD_OFF && !ts_hud_installed() && !hud_failed && ts_hud_init() != TS_OK) {
		hud_failed = 1;
		TS_KLOGW("hud: could not install - HUD unavailable");
	}
	if (!ts_hud_installed())
		s.hud = TS_HUD_OFF;
	/* One line per storage that has seen any I/O since boot (an SD2Vita line
	 * only appears with one fitted): live speed of all its transfers, whether
	 * or not they took the faster path. Extended adds totals and state. With
	 * "recent activity only", a storage idle for TS_HUD_RECENT_SECS drops off
	 * (one in error stays), and with nothing in use the HUD hides. Called once
	 * a second, so idle[] counts seconds. */
	int slow = ts_stage_memcard_slow() == 1;

	for (int o = 0; o < TS_DEV_COUNT; o++) {
		static uint64_t seen[TS_DEV_COUNT];
		static uint32_t idle[TS_DEV_COUNT];   /* seconds since the last read/write */
		int i = k_hud_order[o];
		uint64_t rd, wr;
		char r[12], w[12];

		ts_stage_bytes(i, &rd, &wr);
		mbps(r, sizeof(r), rd - last_rd[i]);
		mbps(w, sizeof(w), wr - last_wr[i]);
		if (rd != last_rd[i] || wr != last_wr[i])
			idle[i] = 0;
		else if (idle[i] < TS_HUD_RECENT_SECS)
			idle[i]++;
		last_rd[i] = rd;
		last_wr[i] = wr;
		seen[i] |= rd | wr;
		if (s.hud == TS_HUD_OFF || !seen[i])
			continue;
		if (s.hud_recent && idle[i] >= TS_HUD_RECENT_SECS && now[i].status != TS_ST_FALLBACK)
			continue;
		const char *name = s.hud == TS_HUD_COMPACT ? k_dev_short[i]
		                 : i == TS_DEV_INTERNAL ? hud_tr(g_lang, HTR_INTERNAL)
		                 : i == TS_DEV_MEMCARD  ? hud_tr(g_lang, HTR_MEMCARD) : "SD2VITA";
		const char *flag = now[i].status == TS_ST_FALLBACK ? hud_tr(g_lang, HTR_ERROR)
		                 : i == TS_DEV_MEMCARD && slow     ? hud_tr(g_lang, HTR_SLOW) : "";
		snprintf(L[n++], TS_HUD_LINE_MAX, "%.14s R %.6s W %.6s MB/s%s%.12s", name, r, w,
		         flag[0] ? " " : "", flag);
		if (s.hud != TS_HUD_EXTENDED)
			continue;
		const char *state =
			now[i].status == TS_ST_ACTIVE   ? hud_tr(g_lang, HTR_ON) :
			now[i].status == TS_ST_OFF      ? hud_tr(g_lang, HTR_OFF) :
			now[i].status == TS_ST_FALLBACK ? hud_tr(g_lang, HTR_STANDARD) :
			                                  hud_tr(g_lang, HTR_UNAVAILABLE);
		snprintf(L[n++], TS_HUD_LINE_MAX, "  %u/%u MiB %.10s %.6s %u%s%.10s",
		         (unsigned)(rd >> 20), (unsigned)(wr >> 20), state,
		         hud_tr(g_lang, HTR_ERR), (unsigned)now[i].errors,
		         s.verify ? " " : "", s.verify ? hud_tr(g_lang, HTR_VERIFY) : "");
	}
	if (ts_hud_installed())
		ts_hud_set_text(L, s.hud == TS_HUD_OFF ? 0 : n, s.hud_pos);
}


static const char *status_name(int st)
{
	switch (st) {
	case TS_ST_OFF:       return "off";
	case TS_ST_ACTIVE:    return "active";
	case TS_ST_FALLBACK:  return "FALLBACK (stock after an error)";
	case TS_ST_NO_BUFFER: return "no buffer (stock)";
	default:              return "unavailable";
	}
}

static void log_status(const TsDevState *d, int i)
{
	TS_KLOGI("dev %s: %s reads=%u (%u MiB) writes=%u (%u MiB) busy=%u errors=%u "
	         "verified=%u mismatches=%u", k_dev_log[i],
	         status_name(d->status), (unsigned)d->reads, (unsigned)d->read_mib,
	         (unsigned)d->writes, (unsigned)d->write_mib, (unsigned)d->passthru,
	         (unsigned)d->errors, (unsigned)d->verified, (unsigned)d->mismatches);
}

static int worker(SceSize args, void *argp)
{
	TsDevState prev[TS_DEV_COUNT];

	(void)args; (void)argp;
	memset(prev, 0, sizeof(prev));
	while (g_run) {
		TsDevState now[TS_DEV_COUNT];

		ksceKernelDelayThread(1000 * 1000);
		g_uptime++;
		ts_settings_flush();
		ts_stage_worker_tick();
		{
			static int last_slow = -2;
			int sl = ts_stage_memcard_slow();
			if (sl != last_slow) {
				TS_KLOGI("memory card slow mode: %s",
				         sl == 1 ? "ON (Sony throttles memory card transfers)" :
				         sl == 0 ? "off" : "unknown");
				last_slow = sl;
			}
		}
		/* The boot reached the Home screen (SceShell up), or the plugin has run
		 * for TS_GUARD_SECS: this start didn't bring the console down. */
		if (!g_guard_clear && g_shell_up && g_uptime >= g_shell_up - 1 + TS_GUARD_SHELL_SECS) {
			guard_write('0', "shell");
			g_guard_clear = 1;
			TS_KLOGI("SceShell up for %us - boot_guard set to 0", (unsigned)TS_GUARD_SHELL_SECS);
		}
		if (!g_guard_clear && g_uptime >= TS_GUARD_SECS) {
			guard_write('0', "worker");
			g_guard_clear = 1;
			TS_KLOGI("stable for %us - boot_guard set to 0", (unsigned)TS_GUARD_SECS);
		}
		/* Log a device on any status change, an error or a mismatch at once,
		 * and its counters every five minutes while it is busy. */
		ts_stage_get(now, 1);
		hud_feed(now);
		for (int i = 0; i < TS_DEV_COUNT; i++) {
			int changed = now[i].status != prev[i].status ||
			              now[i].errors != prev[i].errors ||
			              now[i].mismatches != prev[i].mismatches;
			int busy = now[i].reads != prev[i].reads || now[i].writes != prev[i].writes;

			if (changed || (busy && g_uptime % 300 == 0)) {
				log_status(&now[i], i);
				prev[i] = now[i];
			}
		}
	}
	return 0;
}

/* ---- exported SceStorageTurbo API (user-callable) --------------------------- */

int tsGetState(TsState *out)
{
	TsState k;
	if (!out)
		return TS_ERR_ARG;
	memset(&k, 0, sizeof(k));
	k.size = sizeof(k);
	k.magic = TS_STATE_MAGIC;
	ts_settings_copy(&k.settings);
	k.disabled = g_disabled;
	k.fingerprint = g_fingerprint;
	k.verify_ready = ts_stage_verify_ready();
	k.uptime_s = g_uptime;
	k.memcard_slow = ts_stage_memcard_slow();
	ts_stage_get(k.dev, g_disabled == TS_DIS_NONE);
	if (ksceKernelMemcpyKernelToUser(out, &k, sizeof(k)) < 0)
		return TS_ERR_ARG;
	return TS_OK;
}

int tsSetSetting(int key, int value)
{
	if (g_disabled != TS_DIS_NONE)
		return TS_ERR_NOT_READY;
	if (!ts_setting_valid(key, value))
		return TS_ERR_ARG;
	return ts_settings_set(key, value);   /* applies now; worker saves + logs */
}

int tsSelfTestPing(void)
{
	return (int)TS_SELFTEST_MAGIC;
}

/* Safe mode has no worker thread, and a syscall does no file I/O (see
 * settings.c), so leaving it writes the guard from a one-shot kernel thread. */
static int safe_leave_thread(SceSize args, void *argp)
{
	(void)args; (void)argp;
	guard_write('0', "settings");
	TS_KLOGI("safe mode left from Settings - everything installs at the next start");
	return ksceKernelExitDeleteThread(0);
}

int tsSafeMode(int op)
{
	if (g_disabled != TS_DIS_SAFE)
		return op == TS_SAFE_QUERY ? 0 : TS_ERR_ARG;
	if (op == TS_SAFE_LEAVE && !g_safe_left) {
		SceUID t = ksceKernelCreateThread("SceStorageTurboSafe", safe_leave_thread,
		                                  0x80, 0x2000, 0, 0, NULL);
		if (t < 0 || ksceKernelStartThread(t, 0, NULL) < 0)
			return TS_ERR_NOT_READY;
		g_safe_left = 1;
	}
	return g_safe_left ? 2 : 1;
}

int tsShellUp(void)
{
	if (!g_shell_up)
		g_shell_up = g_uptime + 1;
	return TS_OK;
}

/* Works before the HUD is installed too: only records the pid. */
int tsHudRegisterProc(int role)
{
	return ts_hud_register_proc(role);
}

/* ---- module lifecycle ------------------------------------------------------- */

int module_start(SceSize argc, const void *args);
/* Entry symbol for the linker; taiHEN itself enters through module_start. */
int _start(SceSize argc, const void *args) __attribute__ ((weak, alias("module_start")));

int module_start(SceSize argc, const void *args)
{
	(void)argc; (void)args;

	ts_klog_init();   /* FIRST, so the whole bring-up is captured (makes the dir) */
	TS_KLOGI("=== SceStorageTurbo kernel v%s start ===", TS_VERSION_STR);

	if (!firmware_known()) {
		g_disabled = TS_DIS_FIRMWARE;
		TS_KLOGW("storage driver fingerprint 0x%08x not supported - nothing installed",
		         (unsigned)g_fingerprint);
		return SCE_KERNEL_START_SUCCESS;
	}
	TS_KLOGI("storage driver fingerprint 0x%08x: supported", (unsigned)g_fingerprint);

	/* Crash-loop guard: the file holds '1' from here until the worker has run
	 * for TS_GUARD_SECS, then '0'. Finding '1' at start means the last start
	 * brought the console down, so this boot installs nothing. It is only ever
	 * overwritten, never deleted (a delete can be lost if the console is reset
	 * before the storage flushes). */
	if (guard_read() == '1') {
		g_disabled = TS_DIS_SAFE;
		TS_KLOGW("safe mode: the last start didn't finish (boot_guard [%s]), so "
		         "nothing is installed this boot. To leave it, untick Safe mode in "
		         "Settings > System > Turbo Storage and restart (or write 0 to "
		         TS_GUARD_PATH ")", g_guard_txt);
		return SCE_KERNEL_START_SUCCESS;
	}
	TS_KLOGI("boot_guard was [%s], setting 1", g_guard_txt);
	guard_write('1', "start");

	if (ts_stage_init() != TS_OK) {
		g_disabled = TS_DIS_HOOKS;
		TS_KLOGW("storage hooks NOT installed: storage stays stock");
	}
	ts_settings_init();   /* defaults, then the saved file; applies to stage.c */

	g_worker = ksceKernelCreateThread("SceStorageTurboWorker", worker, 0x80, 0x2000, 0, 0, NULL);
	if (g_worker >= 0)
		ksceKernelStartThread(g_worker, 0, NULL);
	else
		TS_KLOGW("worker thread not started (0x%08x): settings will not be saved",
		         (unsigned)g_worker);

	TS_KLOGI("kernel start complete");
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
	(void)argc; (void)args;
	TS_KLOGI("=== SceStorageTurbo kernel stop ===");
	if (g_disabled == TS_DIS_FIRMWARE || g_disabled == TS_DIS_SAFE) {
		ts_klog_fini();
		return SCE_KERNEL_STOP_SUCCESS;
	}
	g_run = 0;
	if (g_worker >= 0)
		ksceKernelWaitThreadEnd(g_worker, NULL, NULL);
	ts_hud_fini();
	ts_stage_fini();
	ts_settings_flush();
	ts_klog_fini();
	return SCE_KERNEL_STOP_SUCCESS;
}
