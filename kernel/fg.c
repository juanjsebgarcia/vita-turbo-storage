/*
 * fg.c - "Pause in Vita games": tell the I/O hooks when a game is in front.
 *
 * Why: the faster path leaves the data it copies in the CPU cache, and a game
 * may hand freshly loaded data (textures) straight to the GPU. Games get stock
 * storage while this is on; the Home screen, Settings, the Store, VitaShell
 * and other homebrew keep the faster path.
 *
 * Two signals, both a process id:
 *   - the process presenting frames (the display hook in hud.c calls
 *     ts_fg_frame, which only records; the 1 Hz ts_fg_tick classifies and
 *     refreshes the hold): whatever is in front;
 *   - the process issuing a transfer (the I/O hooks call ts_fg_io_hold): a
 *     game's own loads, including those before its first frame.
 * A process is classified once and cached:
 *   - the PSP emulator (Sony's own check, or Adrenaline's title id): game;
 *   - a retail Vita title id (PCSA-PCSH, and the cartridge prefixes): game;
 *   - NPXS... and SceShell ("main"): system; anything else: app.
 * While a game was seen in the last TS_FG_HOLD_US, every transfer stays stock,
 * whichever thread issues it (a game's reads may run on system threads), and
 * the quick menu drawn over a game does not resume the faster path.
 *
 * Both lookups are resolved at run time; without them a process counts as an
 * app, so the faster path stays on (the behaviour before this setting).
 */
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/threadmgr.h>
#include <taihen.h>

#include "fg.h"
#include "klog.h"

int module_get_export_func(SceUID pid, const char *modname, uint32_t libnid,
                           uint32_t funcnid, uintptr_t *func);

#define TS_FG_HOLD_US   3000000u   /* the 1 Hz tick refreshes it: two ticks of
                                    * margin                                    */
#define TS_FG_RETRY_US  1000000u   /* re-ask about a process whose title was unknown */
#define CACHE           8

static int (*g_is_psp)(SceUID pid);
static int (*g_title_of)(SceUID pid, char *title, SceSize len);

static volatile int      g_on;
static volatile int      g_game_any;     /* a game has been seen this boot        */
static uint64_t          g_game_seen;    /* when, ksceKernelGetSystemTimeWide; only
                                          * through __atomic (a plain 64-bit access
                                          * can tear between cores)                */
static volatile SceUID   g_fg_pid;       /* last process to present a frame; 0 none.
                                          * A pid is a UID: any value but 0, the
                                          * high bit may be set (never test < 0) */
static volatile SceUID   g_fg_miss;      /* presented a frame, not yet classified */
static volatile unsigned g_game_frames;  /* frames presented by a game (hook)     */

/* pid -> class. A writer clears the pid, fills the entry, then sets the pid;
 * a reader re-reads the pid after the rest, so it never uses a half-written
 * entry. A process whose title could not be read is classified provisionally
 * and asked about again after TS_FG_RETRY_US. */
static volatile SceUID g_c_pid[CACHE] = { [0 ... CACHE - 1] = -1 };
static volatile int    g_c_cls[CACHE];
static volatile int    g_c_prov[CACHE];  /* provisional: title unknown           */
static uint64_t        g_c_when[CACHE];  /* when classified                       */
static char            g_c_title[CACHE][12];
static volatile unsigned g_c_next;

void ts_fg_init(void)
{
	uintptr_t f = 0;

	/* SceSblACMgrForDriver: ksceSblACMgrIsPspEmu */
	if (module_get_export_func(KERNEL_PID, "SceSblACMgr", 0x9AD8E213, 0xFD00C72A, &f) >= 0)
		g_is_psp = (int (*)(SceUID))f;
	f = 0;
	/* SceSysrootForKernel: ksceKernelSysrootGetProcessTitleId */
	if (module_get_export_func(KERNEL_PID, "SceSysmem", 0x3691DA45, 0xEC3124A3, &f) >= 0)
		g_title_of = (int (*)(SceUID, char *, SceSize))f;
	TS_KLOGI("fg: psp check %s, title lookup %s", g_is_psp ? "ok" : "missing",
	         g_title_of ? "ok" : "missing");
}

void ts_fg_enable(int on)
{
	g_on = on ? 1 : 0;
}

static int pfx(const char *s, const char *p)
{
	while (*p)
		if (*s++ != *p++)
			return 0;
	return 1;
}

/* Retail Vita title ids: a known 4-letter prefix and 5 digits. */
static int retail_vita(const char *t)
{
	static const char k_pfx[][5] = {
		"PCSA", "PCSB", "PCSC", "PCSD", "PCSE", "PCSF", "PCSG", "PCSH",
		"VCJS", "VCAS", "VCKS", "VLJS", "VLJM", "VLAS", "VLKS",
	};

	for (int i = 4; i < 9; i++)
		if (t[i] < '0' || t[i] > '9')
			return 0;
	if (t[9])
		return 0;
	for (unsigned i = 0; i < sizeof(k_pfx) / sizeof(k_pfx[0]); i++)
		if (pfx(t, k_pfx[i]))
			return 1;
	return 0;
}

/* The class of `pid`; *prov set when its title could not be read (so the
 * answer may change: a game must not stay an app for good). */
static int classify(SceUID pid, char title[12], int *prov)
{
	memset(title, 0, 12);
	*prov = 0;
	if (pid == KERNEL_PID)
		return TS_FG_SYSTEM;
	if (g_title_of && g_title_of(pid, title, 12) < 0)
		memset(title, 0, 12);
	title[11] = 0;
	if (g_is_psp && g_is_psp(pid) > 0)
		return TS_FG_PSP;
	if (pfx(title, "PSPEMUCFW") || pfx(title, "NPXS10028"))
		return TS_FG_PSP;
	if (retail_vita(title))
		return TS_FG_GAME;
	if (pfx(title, "NPXS") || pfx(title, "main"))
		return TS_FG_SYSTEM;
	*prov = !title[0];
	return TS_FG_APP;
}

/* Index of pid's entry, with its class, provisional flag, time and title as
 * they were when the entry was published; -1 if not cached. */
static int cache_find(SceUID pid, int *cls, int *prov, uint64_t *when, char title[12])
{
	for (int i = 0; i < CACHE; i++)
		if (g_c_pid[i] == pid) {
			__sync_synchronize();
			int c = g_c_cls[i], pv = g_c_prov[i];
			uint64_t w = g_c_when[i];
			char t[12];
			memcpy(t, g_c_title[i], sizeof(t));
			__sync_synchronize();
			if (g_c_pid[i] == pid) {
				*cls = c;
				if (prov)
					*prov = pv;
				if (when)
					*when = w;
				if (title)
					memcpy(title, t, sizeof(t));
				return i;
			}
		}
	return -1;
}

static int class_of(SceUID pid)
{
	int c, prov, i;
	uint64_t when, now = ksceKernelGetSystemTimeWide();

	i = cache_find(pid, &c, &prov, &when, NULL);
	if (i >= 0 && !(prov && now - when >= TS_FG_RETRY_US))
		return c;

	char title[12];
	c = classify(pid, title, &prov);
	if (i < 0)
		i = (int)(__sync_fetch_and_add(&g_c_next, 1) % CACHE);
	g_c_pid[i] = -1;
	__sync_synchronize();
	g_c_cls[i] = c;
	g_c_prov[i] = prov;
	g_c_when[i] = now;
	memcpy(g_c_title[i], title, sizeof(title));
	__sync_synchronize();
	g_c_pid[i] = pid;
	return c;
}

static void game_seen(void)
{
	__atomic_store_n(&g_game_seen, ksceKernelGetSystemTimeWide(), __ATOMIC_RELAXED);
	g_game_any = 1;
}

/* Called for every frame on every display, from the display hook: loads and
 * stores only, no locks and no I/O. A cache hit that is a game
 * bumps g_game_frames; a miss is left in g_fg_miss for the 1 Hz tick to
 * classify, since asking the system about a process may take locks. The class
 * is looked up each time rather than remembered with the pid, so two processes
 * presenting at once (a game and the quick menu) can't leave a mismatched pair
 * behind. */
void ts_fg_frame(void)
{
	if (!g_on)
		return;
	SceUID pid = ksceKernelGetProcessId();
	int c;

	if (cache_find(pid, &c, NULL, NULL, NULL) < 0)
		g_fg_miss = pid;
	else if (c >= TS_FG_GAME)
		g_game_frames++;
	g_fg_pid = pid;
}

int ts_fg_paused(void)
{
	return g_on && g_game_any &&
	       ksceKernelGetSystemTimeWide() - __atomic_load_n(&g_game_seen, __ATOMIC_RELAXED)
	           < TS_FG_HOLD_US;
}

int ts_fg_io_hold(void)
{
	if (!g_on)
		return 0;
	SceUID pid = ksceKernelGetProcessId();
	if (pid != KERNEL_PID && class_of(pid) >= TS_FG_GAME) {
		game_seen();
		return 1;
	}
	return ts_fg_paused();
}

static void fg_of(SceUID pid, int *cls, char title[12])
{
	memset(title, 0, 12);
	*cls = TS_FG_SYSTEM;
	if (pid)
		cache_find(pid, cls, NULL, NULL, title);
	title[11] = 0;
}

void ts_fg_get(int *cls, char title[12])
{
	fg_of(g_fg_pid, cls, title);
}

void ts_fg_tick(void)
{
	static const char *const k_name[] = { "system", "app", "Vita game", "PSP emulator" };
	static SceUID last_pid[2];   /* two, so an overlay alternating with
	                                           * a game doesn't log every second   */
	static int last_paused = -1;
	static unsigned last_frames;
	SceUID pid = g_fg_pid, miss = g_fg_miss;
	unsigned frames = g_game_frames;

	/* The display hook only records: classify here, outside it. */
	if (g_on) {
		if (miss) {
			g_fg_miss = 0;
			if (class_of(miss) >= TS_FG_GAME)
				game_seen();
		}
		if (pid && pid != miss && class_of(pid) >= TS_FG_GAME)
			game_seen();
		if (frames != last_frames)
			game_seen();
	}
	last_frames = frames;

	int paused = ts_fg_paused();

	if (pid && pid != last_pid[0] && pid != last_pid[1]) {
		int cls;
		char title[12];

		fg_of(pid, &cls, title);
		TS_KLOGI("fg: in front pid 0x%08x %s (%s)", (unsigned)pid,
		         title[0] ? title : "-", k_name[cls & 3]);
		last_pid[1] = last_pid[0];
		last_pid[0] = pid;
	}
	if (paused != last_paused) {
		if (last_paused >= 0 || paused)
			TS_KLOGI("fg: %s", paused ? "game in front - storage stock" :
			                            "no game in front - faster path as set");
		last_paused = paused;
	}
}
