/*
 * main.c - storageturbo.suprx, the user module.
 *
 * The ONE storageturbo.suprx is listed under both *main (SceShell) and
 * *NPXS10015 (the Settings app) in config.txt, as in Wi-Fi Turbo. Branch on the
 * process:
 *   - Settings app: install only the Settings page hooks
 *     (settings_app/storageturbo_settings.c);
 *   - SceShell: start the developer command server (user/dispatch.c), which is
 *     deferred and only listens while Developer mode is on.
 * Everything that matters - the storage hooks, the HUD, saving settings - lives
 * in the kernel module, so neither instance has anything to undo but its own
 * hooks or thread.
 *
 * The kernel library SceStorageTurbo is a strong import: if the kernel module
 * is not loaded, taiHEN refuses this module and nothing appears.
 */
#include <psp2/kernel/modulemgr.h>
#include <taihen.h>

#include "../include/storageturbo.h"
#include "../include/storageturbo_kernel_api.h"
#include "../settings_app/settings_ui.h"
#include "../settings_app/tslog.h"
#include "dispatch.h"

static int in_shell(void)
{
	tai_module_info_t ti;
	ti.size = sizeof(ti);
	return taiGetModuleInfo("SceShell", &ti) >= 0;
}

int module_start(SceSize argc, const void *args);
/* Entry symbol for the linker; taiHEN itself enters through module_start. */
int _start(SceSize argc, const void *args) __attribute__ ((weak, alias("module_start")));

int module_start(SceSize argc, const void *args)
{
	(void)argc; (void)args;

	int ping = tsSelfTestPing();
	TsState st;
	int r = tsGetState(&st);
	TS_LOGI("=== storageturbo user v%s (%s) === kernel ping=0x%08x state rc=%d disabled=%d",
	        TS_VERSION_STR, in_shell() ? "shell" : "settings", (unsigned)ping, r,
	        r == TS_OK ? (int)st.disabled : -1);

	/* The HUD draws over these system screens even with In-game HUD off. */
	tsHudRegisterProc(in_shell() ? TS_HUD_PROC_SHELL : TS_HUD_PROC_SETTINGS);

	if (in_shell()) {
		tsShellUp();                  /* the boot reached SceShell: clears the guard */
		ts_dispatch_start();          /* deferred; returns immediately */
	}
	else
		ts_settings_hooks_install();
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
	(void)argc; (void)args;
	if (in_shell())
		ts_dispatch_stop();
	else
		ts_settings_hooks_release();
	return SCE_KERNEL_STOP_SUCCESS;
}
