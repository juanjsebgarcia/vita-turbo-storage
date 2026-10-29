/*
 * storageturbo_kernel_api.h - the "SceStorageTurbo" syscall library the kernel
 * module publishes for the Settings page (or any other user module) to import.
 *
 * Why a kernel module at all: the storage I/O path runs in the kernel, and the
 * kernel module also owns the settings, so they apply from boot without any user
 * module running.
 *
 * These functions cross the user->kernel boundary: pointer arguments are copied
 * out with ksceKernelMemcpyKernelToUser inside the kernel, and every key id and
 * value is validated there. Keep the structs small and POD.
 *
 * Every function here must also be listed in storageturbo_kernel.yml and
 * defined in kernel/main.c.
 */
#ifndef STORAGETURBO_KERNEL_API_H
#define STORAGETURBO_KERNEL_API_H

#include "storageturbo.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Snapshot of everything the kernel module knows, for the Settings page and
 * diagnostics. */
typedef struct TsState {
	uint32_t   size;             /* sizeof(TsState), set by the kernel        */
	uint32_t   magic;            /* TS_STATE_MAGIC                            */
	TsSettings settings;         /* current (applied) settings                */
	int32_t    disabled;         /* TS_DIS_*: why nothing is active           */
	uint32_t   fingerprint;      /* storage driver module fingerprint         */
	int32_t    verify_ready;     /* 1 once the verify buffer is allocated     */
	uint32_t   uptime_s;         /* seconds since the kernel module started   */
	int32_t    memcard_slow;     /* memory card slow mode: 1 on, 0 off, -1 n/a */
	TsDevState dev[TS_DEV_COUNT];
} TsState;

/* TsState.disabled */
#define TS_DIS_NONE      0   /* active                                          */
#define TS_DIS_FIRMWARE  1   /* unknown firmware: nothing installed, all stock  */
#define TS_DIS_HOOKS     2   /* hooks could not be installed: all stock         */
#define TS_DIS_SAFE      3   /* last start never finished: safe mode, all stock */

#define TS_STATE_MAGIC  0x54535354u   /* 'TSST' */

/* Copy the current state to *out (user pointer). Returns TS_OK or
 * TS_ERR_ARG. */
int tsGetState(TsState *out);

/* Apply one setting (enum ts_key) and save it. Returns TS_OK, or TS_ERR_ARG
 * for an unknown key or an out-of-range value (nothing changes then). */
int tsSetSetting(int key, int value);

/* Round-trip cookie: proves the syscall path reaches the kernel module.
 * Always returns TS_SELFTEST_MAGIC; changes nothing. 'TST1'. */
#define TS_SELFTEST_MAGIC  0x54535431u
int tsSelfTestPing(void);

/* Register the calling process as one of the system screens the HUD draws over
 * (TS_HUD_PROC_*): the user module calls it at start in SceShell and in the
 * Settings app. Returns TS_OK or TS_ERR_ARG. */
int tsHudRegisterProc(int role);

/* Safe mode (TsState.disabled == TS_DIS_SAFE: the last start didn't finish).
 * TS_SAFE_QUERY returns 1 while in safe mode, 2 once left (restart pending),
 * else 0. TS_SAFE_LEAVE re-arms the boot guard so the next start installs
 * everything again; it takes effect after a restart. */
#define TS_SAFE_QUERY  0
#define TS_SAFE_LEAVE  1
int tsSafeMode(int op);

/* SceShell has started (the user module calls this from SceShell): the boot
 * got as far as the Home screen, so a few seconds later the boot guard is
 * cleared - a quick restart after that is not mistaken for a crash. */
int tsShellUp(void);

#ifdef __cplusplus
}
#endif

#endif /* STORAGETURBO_KERNEL_API_H */
