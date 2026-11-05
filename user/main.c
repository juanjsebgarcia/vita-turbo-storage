/*
 * main.c - storageturbo.suprx, the SceShell module (*main).
 *
 * It does three small things: tells the kernel module the boot reached the
 * Home screen (which clears the boot guard), registers SceShell as a system
 * screen for the HUD, and runs the developer command server (user/dispatch.c),
 * which has no thread of its own until Developer mode is switched on.
 *
 * The Settings page is a separate module, storageturbo_settings.suprx, listed
 * under *NPXS10015, so SceShell carries none of its code, text or buffers. A
 * config.txt from before v0.4.0 lists this module under *NPXS10015 instead: in
 * the Settings app it only loads the page module (unless config.txt already
 * did) and returns.
 *
 * Everything that matters - the storage hooks, the HUD, saving settings - lives
 * in the kernel module. The kernel library SceStorageTurbo is a strong import:
 * if the kernel module is not loaded, taiHEN refuses this module.
 */
#include <psp2/kernel/modulemgr.h>
#include <taihen.h>

#include "../include/storageturbo.h"
#include "../include/storageturbo_kernel_api.h"
#include "../settings_app/tslog.h"
#include "dispatch.h"

/* Settings app only: the page module this one loaded, for an older config.txt. */
#define TS_SETTINGS_MODULE "ur0:tai/storageturbo_settings.suprx"
static SceUID g_settings_mod = -1;

static int in_shell(void)
{
	tai_module_info_t ti;
	ti.size = sizeof(ti);
	return taiGetModuleInfo("SceShell", &ti) >= 0;
}

int module_start(SceSize argc, const void *args);
/* Entry symbol for the linker; taiHEN itself enters through module_start. */
int _start(SceSize argc, const void *args) __attribute__ ((weak, alias("module_start")));

/* The firmware the console really runs, for bug reports. A version spoof
 * changes what the system reports, but not the Home screen itself, and each
 * firmware ships its own SceShell build. Nothing is gated on it. */
static void log_shell_firmware(void)
{
	static const struct { unsigned nid; const char *ver; } builds[] = {
		{ 0x0552F692, "3.60" }, { 0x532155E5, "3.61" }, { 0xBB4B0A3E, "3.63" },
		{ 0x5549BF1F, "3.65" }, { 0x34B4D82E, "3.67" }, { 0x12DAC0F3, "3.68" },
		{ 0x0703C828, "3.69" }, { 0x2053B5A5, "3.70" }, { 0xF476E785, "3.71" },
		{ 0x939FFBE9, "3.72" }, { 0x734D476A, "3.73" }, { 0x51CB6207, "3.74" },
	};
	tai_module_info_t ti;
	ti.size = sizeof(ti);
	if (taiGetModuleInfo("SceShell", &ti) < 0)
		return;
	for (unsigned i = 0; i < sizeof(builds) / sizeof(builds[0]); i++)
		if (builds[i].nid == (unsigned)ti.module_nid) {
			TS_LOGI("firmware: home screen is the %s build", builds[i].ver);
			return;
		}
	TS_LOGI("firmware: home screen build 0x%08x not known", (unsigned)ti.module_nid);
}

int module_start(SceSize argc, const void *args)
{
	(void)argc; (void)args;

	if (!in_shell()) {
		tai_module_info_t ti;
		ti.size = sizeof(ti);
		if (taiGetModuleInfo("storageturbo_settings", &ti) < 0) {
			int st = 0;
			g_settings_mod = sceKernelLoadStartModule(TS_SETTINGS_MODULE, 0, NULL, 0, NULL, &st);
			if (g_settings_mod < 0)
				TS_LOGI("[SET] %s not loaded (0x%08x) - copy it to ur0:tai/ for the "
				        "Settings page", TS_SETTINGS_MODULE, (unsigned)g_settings_mod);
		}
		return SCE_KERNEL_START_SUCCESS;
	}

	TsState st;
	int r = tsGetState2(&st, sizeof(st));
	TS_LOGI("=== storageturbo user v%s (shell) === kernel ping=0x%08x state rc=%d disabled=%d",
	        TS_BUILD_STR, (unsigned)tsSelfTestPing(), r,
	        r == TS_OK ? (int)st.disabled : -1);

	log_shell_firmware();

	/* The HUD draws over the Home screen even with In-game HUD off. */
	tsHudRegisterProc(TS_HUD_PROC_SHELL);
	tsShellUp();                  /* the boot reached SceShell: clears the guard */
	ts_dispatch_start();          /* one small idle thread; returns immediately */
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
	(void)argc; (void)args;
	if (!in_shell()) {
		if (g_settings_mod >= 0)
			sceKernelStopUnloadModule(g_settings_mod, 0, NULL, 0, NULL, NULL);
		return SCE_KERNEL_STOP_SUCCESS;
	}
	ts_dispatch_stop();
	return SCE_KERNEL_STOP_SUCCESS;
}
