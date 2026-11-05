/*
 * storageturbo.h - shared constants and types for Turbo Storage (PS Vita
 * storage plugin).
 *
 * Firmware 3.60-3.74 CEX, taiHEN/HENkaku. Both modules include this header, so
 * every struct here that crosses the syscall boundary is POD and must be built
 * from this one definition.
 *
 * Layout of the project:
 *   kernel/        storageturbo_kernel.skprx - the storage I/O hooks (staging),
 *                  settings persistence, the 1 Hz worker, the boot guard, and
 *                  the user-callable "SceStorageTurbo" syscall library.
 *   user/          storageturbo.suprx in SceShell (*main): boot guard, HUD
 *                  registration, the developer command server (TCP :1340, only
 *                  while Developer mode is on). Kept a few KiB on purpose.
 *   settings_app/  storageturbo_settings.suprx in the Settings app
 *                  (*NPXS10015): the Turbo Storage page. Both talk to the
 *                  kernel module through the syscalls.
 *   quickmenu/     storageturbo_qm.suprx, optional: the on/off row in the PS
 *                  button quick menu (a QuickMenuReborn client).
 */
#ifndef STORAGETURBO_H
#define STORAGETURBO_H

#include <stdint.h>

/* Release version. The .yml module versions carry major.minor (0.1); bump all
 * four together (kernel, SceShell, Settings, quick menu). Logged in each
 * module's init banner so a log pins the build. */
#define TS_VERSION_STR    "0.4.0"

/* Version and the git commit it was built from (set by the build; "-dirty"
 * means uncommitted changes), e.g. "0.4.0 (a1b2c3d)". Logged at every
 * start, so a log names the exact build. */
#ifndef TS_BUILD_ID
#define TS_BUILD_ID       "unknown"
#endif
#define TS_BUILD_STR      TS_VERSION_STR " (" TS_BUILD_ID ")"

/* Everything the plugin writes lives here (created on first use). */
#define TS_DATA_DIR       "ur0:data/turbostorage"

/* Error codes returned across the whole plugin. */
#define TS_OK              0
#define TS_ERR_NOT_READY  -1
#define TS_ERR_RESOLVE    -2   /* module/export not found                    */
#define TS_ERR_MISMATCH   -3   /* unknown firmware: nothing installed        */
#define TS_ERR_ARG        -4   /* bad pointer, key id or value               */
#define TS_ERR_IO         -5
#define TS_ERR_NOMEM      -6   /* staging buffer could not be allocated      */

/* ------------------------------------------------------------------------- */
/* Devices                                                                   */
/* ------------------------------------------------------------------------- */

enum ts_dev {
	TS_DEV_SD2VITA = 0,   /* microSD in the game-card slot (SD2Vita adapter) */
	TS_DEV_MEMCARD,       /* the official PS Vita memory card                */
	/* The console's built-in storage is never touched: it also holds the
	 * system's own partitions (removed in 0.4.0). */
	TS_DEV_COUNT
};

/* ------------------------------------------------------------------------- */
/* Settings                                                                  */
/* ------------------------------------------------------------------------- */

/* Key ids for tsSetSetting(). The order is also the field order of TsSettings
 * and the registry key table in the Settings page. */
enum ts_key {
	TS_KEY_SD2VITA = 0,    /* 0/1: faster SD2Vita reads and writes           */
	TS_KEY_RETIRED1,       /* was "faster internal storage" (removed in 0.4.0):
	                        * the slot stays so saved files keep their layout;
	                        * only 0 is accepted                              */
	TS_KEY_VERIFY,         /* 0/1: check every faster transfer (diagnostic)  */
	TS_KEY_HUD,            /* TS_HUD_*                                       */
	TS_KEY_HUD_POS,        /* TS_POS_*                                       */
	TS_KEY_DEVMODE,        /* 0/1: developer command server on TCP :1340     */
	TS_KEY_MEMCARD,        /* 0/1: faster memory card (added after 0.1.0)    */
	TS_KEY_HUD_GAME,       /* 0/1: HUD also over games and apps (after 0.2.0) */
	TS_KEY_HUD_RECENT,     /* 0/1: only storages used in the last 30 s       */
	TS_KEY_ENABLED,        /* 0/1: master switch over all storages (0.4.0)   */
	TS_KEY_GAME_PAUSE,     /* 0/1: stock while a Vita game or PSP title is in front */
	TS_KEY_COUNT
};

#define TS_HUD_OFF        0
#define TS_HUD_COMPACT    1
#define TS_HUD_EXTENDED   2

#define TS_POS_TOP_LEFT     0
#define TS_POS_TOP_RIGHT    1
#define TS_POS_BOTTOM_LEFT  2
#define TS_POS_BOTTOM_RIGHT 3

#define TS_DEV_PORT       1340   /* developer command server (turbo-wifi: 1339) */

/* Defaults: every storage starts off; the user opts in. */
#define TS_DEF_SD2VITA   0   /* opt-in: see README, "use it when you need it" */
#define TS_DEF_RETIRED1  0
#define TS_DEF_VERIFY    0
#define TS_DEF_HUD       TS_HUD_OFF
#define TS_DEF_HUD_POS   TS_POS_TOP_RIGHT
#define TS_DEF_DEVMODE   0
#define TS_DEF_MEMCARD   0
#define TS_DEF_HUD_GAME  1   /* on: the HUD is most useful while copying in an app */
#define TS_DEF_HUD_RECENT 1  /* on: idle storages drop off, an idle HUD hides  */
#define TS_DEF_ENABLED    1  /* on: each storage's own switch still decides */
#define TS_DEF_GAME_PAUSE 1  /* on: games get stock storage (see README)      */
#define TS_HUD_RECENT_SECS 30

/* System processes the HUD draws over when the in-game HUD is off: each one
 * registers itself with tsHudRegisterProc() as it starts. Frames from any other
 * process (a game or app) are left alone. */
#define TS_HUD_PROC_SHELL     0   /* SceShell: Home screen, LiveArea, quick menu */
#define TS_HUD_PROC_SETTINGS  1   /* the Settings app (NPXS10015)               */
#define TS_HUD_PROC_COUNT     2

/* One int per key, in enum ts_key order. */
typedef struct TsSettings {
	int32_t sd2vita;
	int32_t retired1;   /* TS_KEY_RETIRED1: always 0 */
	int32_t verify;
	int32_t hud;
	int32_t hud_pos;
	int32_t devmode;
	int32_t memcard;
	int32_t hud_game;
	int32_t hud_recent;
	int32_t enabled;
	int32_t game_pause;
} TsSettings;

/* 1 if `v` is a legal value for `key`, else 0. Shared so the kernel's
 * validation and the Settings page agree. */
static inline int ts_setting_valid(int key, int v)
{
	switch (key) {
	case TS_KEY_RETIRED1:
		return v == 0;
	case TS_KEY_SD2VITA:
	case TS_KEY_VERIFY:
	case TS_KEY_DEVMODE:
	case TS_KEY_MEMCARD:
	case TS_KEY_HUD_GAME:
	case TS_KEY_HUD_RECENT:
	case TS_KEY_ENABLED:
	case TS_KEY_GAME_PAUSE:
		return v == 0 || v == 1;
	case TS_KEY_HUD:
		return v >= TS_HUD_OFF && v <= TS_HUD_EXTENDED;
	case TS_KEY_HUD_POS:
		return v >= TS_POS_TOP_LEFT && v <= TS_POS_BOTTOM_RIGHT;
	default:
		return 0;
	}
}

static inline int32_t *ts_setting_field(TsSettings *s, int key)
{
	if (key < 0 || key >= TS_KEY_COUNT)
		return 0;
	return &((int32_t *)s)[key];   /* the struct is TS_KEY_COUNT int32s       */
}

/* ------------------------------------------------------------------------- */
/* Live status                                                               */
/* ------------------------------------------------------------------------- */

/* TsDevState.status: what a device is doing right now. */
#define TS_ST_OFF        0   /* turned off in Settings                         */
#define TS_ST_ACTIVE     1   /* on: large transfers take the faster path       */
#define TS_ST_FALLBACK   2   /* switched itself back to stock after an error   */
#define TS_ST_NO_BUFFER  3   /* its staging buffer could not be allocated      */
#define TS_ST_UNAVAIL    4   /* plugin inactive (see TsState.disabled)         */
#define TS_ST_PAUSED     5   /* on, but a game is in front: stock for now      */

/* What is in front (TsState.fg_class), as "Pause in Vita games" sees it. */
#define TS_FG_SYSTEM     0   /* Home screen, Settings, other system apps       */
#define TS_FG_APP        1   /* homebrew and other non-retail apps             */
#define TS_FG_GAME       2   /* a retail Vita title                            */
#define TS_FG_PSP        3   /* the PSP emulator (PSP titles, Adrenaline)      */

typedef struct TsDevState {
	int32_t  status;          /* TS_ST_*                                       */
	uint32_t reads;           /* transfers that took the faster path           */
	uint32_t writes;
	uint32_t read_mib;        /* data moved on the faster path, MiB            */
	uint32_t write_mib;
	uint32_t passthru;        /* large transfers left stock (buffer busy)      */
	uint32_t held;            /* large transfers left stock (a game in front)  */
	uint32_t uncached;        /* large transfers left stock (non-cacheable buffer) */
	uint32_t errors;          /* transfers that failed on the faster path      */
	uint32_t verified;        /* transfers checked in verify mode              */
	uint32_t mismatches;      /* checks that failed (caller got stock data)    */
} TsDevState;

#endif /* STORAGETURBO_H */
