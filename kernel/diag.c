/*
 * diag.c - one-off boot diagnostics, so a bug report's kernel.log says which
 * console it came from and which other kernel plugins share it.
 *
 * Every function here is resolved at run time and skipped if it can't be:
 * the module manager's kernel library changed its NIDs in 3.63
 * (3.60-3.61: 0xC445FA63, 3.63+: 0x92C9FFC2), and importing it directly would
 * stop the whole module loading on the other half of the range. Nothing here
 * runs on the I/O path.
 */
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysclib.h>
#include <taihen.h>

#include "diag.h"
#include "klog.h"

int module_get_export_func(SceUID pid, const char *modname, uint32_t libnid,
                           uint32_t funcnid, uintptr_t *func);

#define MAX_MODS  256

static int (*g_get_list)(SceUID pid, int flags1, int flags2, SceUID *modids, SceSize *num);
static int (*g_get_info)(SceUID pid, SceUID modid, SceKernelModuleInfo *info);
static int    g_resolved;
static SceUID g_seen[MAX_MODS];
static int    g_nseen;

static uintptr_t export_func(const char *mod, uint32_t lib, uint32_t nid)
{
	uintptr_t f = 0;
	return module_get_export_func(KERNEL_PID, mod, lib, nid, &f) >= 0 ? f : 0;
}

static void resolve(void)
{
	static const struct { uint32_t lib, list, info; } k_mgr[] = {
		{ 0x92C9FFC2, 0xB72C75A4, 0xDAA90093 },   /* 3.63-3.74 */
		{ 0xC445FA63, 0x97CF7B4E, 0xD269F915 },   /* 3.60-3.61 */
	};

	if (g_resolved)
		return;
	g_resolved = 1;
	for (unsigned i = 0; i < sizeof(k_mgr) / sizeof(k_mgr[0]) && !g_get_list; i++) {
		uintptr_t l = export_func("SceKernelModulemgr", k_mgr[i].lib, k_mgr[i].list);
		uintptr_t n = export_func("SceKernelModulemgr", k_mgr[i].lib, k_mgr[i].info);
		if (l && n) {
			g_get_list = (void *)l;
			g_get_info = (void *)n;
		}
	}
}

void ts_diag_console(void)
{
	/* SceSblAIMgrForDriver (in SceSysmem), SceSysconForDriver */
	int (*product)(void) = (void *)export_func("SceSysmem", 0xFD00C69A, 0x14345161);
	int (*sub)(void)     = (void *)export_func("SceSysmem", 0xFD00C69A, 0xB33CEC8F);
	int (*hw)(void)      = (void *)export_func("SceSyscon", 0x60A35F64, 0xCBD6D8BC);

	TS_KLOGI("console: product 0x%x sub 0x%x hw 0x%08x",
	         product ? (unsigned)product() : 0u, sub ? (unsigned)sub() : 0u,
	         hw ? (unsigned)hw() : 0u);
}

static int system_path(const char *p)
{
	/* Sony's own: the boot image and the system partitions. */
	return !p[0] || !memcmp(p, "os0:", 4) || !memcmp(p, "vs0:", 4) || !memcmp(p, "sd0:", 4) ||
	       !memcmp(p, "bootfs:", 7);
}

void ts_diag_modules(const char *when)
{
	static SceUID ids[MAX_MODS];
	static SceKernelModuleInfo info;   /* 0x1B8 bytes: off the kernel stack */
	SceSize num = MAX_MODS;
	int first = g_nseen == 0, sys = 0, shown = 0;

	resolve();
	if (!g_get_list) {
		TS_KLOGW("modules (%s): module list unavailable", when);
		return;
	}
	/* One block: the log normally closes the file after every line (so a crash
	 * loses nothing), which made a long list cost seconds at boot. The block is
	 * flushed when sync mode is restored, before the install starts. */
	int was_sync = ts_klog_sync(0);
	int r = g_get_list(KERNEL_PID, 0x7FFFFFFF, 1, ids, &num);
	if (r < 0) {
		TS_KLOGW("modules (%s): list failed (0x%08x)", when, (unsigned)r);
		ts_klog_sync(was_sync);
		return;
	}
	for (SceSize i = 0; i < num && i < MAX_MODS; i++) {
		int seen = 0;
		for (int j = 0; j < g_nseen && !seen; j++)
			seen = g_seen[j] == ids[i];
		if (seen)
			continue;
		if (g_nseen < MAX_MODS)
			g_seen[g_nseen++] = ids[i];
		memset(&info, 0, sizeof(info));
		info.size = sizeof(info);
		if (g_get_info(KERNEL_PID, ids[i], &info) < 0)
			continue;
		info.module_name[sizeof(info.module_name) - 1] = 0;
		info.path[sizeof(info.path) - 1] = 0;
		if (system_path(info.path)) {
			sys++;
			continue;
		}
		TS_KLOGI("modules (%s): %s %s", when, info.module_name, info.path);
		shown++;
	}
	TS_KLOGI("modules (%s): %u %s, %d plugin(s), %d system", when, (unsigned)num,
	         first ? "loaded" : "now loaded", shown, sys);
	ts_klog_sync(was_sync);   /* back on: closes the file, so the block is on disk */
}
