/*
 * settings_main.c - module entry of storageturbo_settings.suprx, the Turbo
 * Storage page in the Settings app (NPXS10015).
 *
 * The page used to be compiled into storageturbo.suprx, which SceShell loads
 * too: its code, its text in 20 languages and its page buffers then took
 * SceShell memory it never used. Now config.txt loads this module under
 * *NPXS10015. An older config.txt that still lists storageturbo.suprx there
 * keeps working: that module loads this one (user/main.c). If both happen,
 * the second copy finds the first and leaves.
 */
#include <psp2/kernel/modulemgr.h>
#include <taihen.h>

#include "../include/storageturbo.h"
#include "../include/storageturbo_kernel_api.h"
#include "settings_ui.h"
#include "tslog.h"

#define START_NO_RESIDENT 1   /* SCE_KERNEL_START_NO_RESIDENT: unload me */

int module_start(SceSize argc, const void *args);
/* Entry symbol for the linker; taiHEN itself enters through module_start. */
int _start(SceSize argc, const void *args) __attribute__ ((weak, alias("module_start")));

int module_start(SceSize argc, const void *args)
{
	(void)argc; (void)args;
	tai_module_info_t ti;
	ti.size = sizeof(ti);
	if (taiGetModuleInfo("storageturbo_settings", &ti) >= 0 &&
	    ti.modid != sceKernelGetModuleIdByAddr((void *)module_start))
		return START_NO_RESIDENT;   /* already loaded in this process */

	TsState st;
	int r = tsGetState2(&st, sizeof(st));
	TS_LOGI("=== storageturbo settings v%s === kernel ping=0x%08x state rc=%d disabled=%d",
	        TS_BUILD_STR, (unsigned)tsSelfTestPing(), r,
	        r == TS_OK ? (int)st.disabled : -1);

	/* The HUD draws over the Settings app even with In-game HUD off. */
	tsHudRegisterProc(TS_HUD_PROC_SETTINGS);
	ts_settings_hooks_install();
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
	(void)argc; (void)args;
	ts_settings_hooks_release();
	return SCE_KERNEL_STOP_SUCCESS;
}
