/*
 * storageturbo_qm.c - the Turbo Storage on/off row in the PS button quick menu.
 *
 * An optional client plugin for QuickMenuReborn: QuickMenuReborn (under *main)
 * loads every .suprx in ur0:QuickMenuReborn/ (or ux0:) into SceShell, and
 * rebuilds the widgets registered here each time the quick menu opens.
 *
 * One row, laid out like Sony's: icon, "Turbo Storage", and a check box for
 * the master switch (TS_KEY_ENABLED). It goes through tsSetSetting, the same
 * syscall the Settings page uses, so a change here applies at once and is
 * saved exactly like one made in Settings. Each storage's own switch is kept.
 *
 * Links SceStorageTurbo strongly: without the kernel module this plugin fails
 * to load and the quick menu simply shows nothing extra.
 */
#include <psp2/kernel/modulemgr.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/types.h>
#include <string.h>

#include "qmr.h"
#include "qm_icon.h"

#include "../include/storageturbo.h"
#include "../include/storageturbo_kernel_api.h"

/* QuickMenuReborn keeps every client's widgets in one list keyed by these ids,
 * so they carry our prefix. */
#define ID_SEPARATOR   "storageturbo_qm_sep"
#define ID_PLANE       "storageturbo_qm_plane"
#define ID_TEXT        "storageturbo_qm_text"
#define ID_ICON        "storageturbo_qm_icon"
#define ID_ICON_TEX    "storageturbo_qm_icon_tex"
#define ID_BOX         "storageturbo_qm_box"

/* QuickMenuReborn loads textures from files, so the icon built into this
 * plugin is written out once (and again if it ever changes). */
#define ICON_PATH      TS_DATA_DIR "/qm_icon.png"

/* The product name, the same in every language, so this module carries no
 * translations (they would cost SceShell memory). Set from the row's own
 * OnLoad: while the menu is open QuickMenuReborn applies a label to the live
 * widget at once, so a row that isn't built yet must not be touched. */
static void on_label_load(const char *id)
{
	QuickMenuRebornSetWidgetLabel(id, "Turbo Storage");
}

/* The menu is being built: show the current state (it may have changed in
 * Settings since the menu was last open).
 *
 * The check box can't be set directly: QuickMenuRebornSetCheckBoxState passes
 * the state in the wrong argument and always unticks it. QuickMenuReborn's
 * default recall sets it correctly from its saved-state file, so store the
 * current value there before the box is built - this runs as the OnLoad of
 * its parent plane, which is created first. */
static void on_plane_load(const char *id)
{
	(void)id;
	TsState st;
	if (tsGetState2(&st, sizeof(st)) == TS_OK && st.magic == TS_STATE_MAGIC)
		QuickMenuRebornSaveCheckBoxState(ID_BOX, st.settings.enabled ? 1 : 0);
}

static void on_toggle(const char *id, SceInt32 hash, SceInt32 event, void *user)
{
	(void)id; (void)hash; (void)event; (void)user;
	int on = QuickMenuRebornGetCheckboxValue(ID_BOX) > 0;   /* live widget: ticked */
	if (tsSetSetting(TS_KEY_ENABLED, on) == TS_OK)
		return;
	/* Refused (safe mode, unsupported firmware): the live box can't be set
	 * (see on_plane_load). on_plane_load stores the real state each time the
	 * menu is built; this covers a build where it could not read the state. */
	TsState st;
	if (tsGetState2(&st, sizeof(st)) == TS_OK && st.magic == TS_STATE_MAGIC)
		QuickMenuRebornSaveCheckBoxState(ID_BOX, st.settings.enabled ? 1 : 0);
}

/* Write the built-in icon to ICON_PATH unless it is already there, intact. */
static int icon_file_ready(void)
{
	SceIoStat st;
	if (sceIoGetstat(ICON_PATH, &st) >= 0 && st.st_size == (SceOff)sizeof(k_qm_icon_png))
		return 1;
	sceIoMkdir("ur0:data", 0777);
	sceIoMkdir(TS_DATA_DIR, 0777);
	SceUID fd = sceIoOpen(ICON_PATH, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
	if (fd < 0)
		return 0;
	int n = sceIoWrite(fd, k_qm_icon_png, sizeof(k_qm_icon_png));
	sceIoClose(fd);
	return n == (int)sizeof(k_qm_icon_png);
}

int module_start(SceSize argc, const void *args);
int _start(SceSize argc, const void *args) __attribute__ ((weak, alias("module_start")));

int module_start(SceSize argc, const void *args)
{
	(void)argc; (void)args;

	QuickMenuRebornSeparator(ID_SEPARATOR, QMR_SEPARATOR_HEIGHT);

	/* Icon and label left, check box right, laid out like Sony's rows (icon
	 * centred at x 118, text from x 161 on screen). The text style centres
	 * text in its box, so the box sits right of the icon. */
	QuickMenuRebornRegisterWidget(ID_PLANE, NULL, QMR_PLANE);
	QuickMenuRebornSetWidgetSize(ID_PLANE, QMR_ROW_WIDTH, 80, 0, 0);
	QuickMenuRebornSetWidgetColor(ID_PLANE, 1, 1, 1, 0);   /* invisible container */
	QuickMenuRebornAssignOnLoadHandler(on_plane_load, ID_PLANE);
	if (icon_file_ready() && QuickMenuRebornRegisterTexture(ID_ICON_TEX, ICON_PATH) >= 0) {
		QuickMenuRebornRegisterWidget(ID_ICON, ID_PLANE, QMR_PLANE);
		QuickMenuRebornSetWidgetSize(ID_ICON, 48, 48, 0, 0);
		QuickMenuRebornSetWidgetColor(ID_ICON, 1, 1, 1, 1);
		QuickMenuRebornSetWidgetPosition(ID_ICON, -362, 0, 0, 0);
		QuickMenuRebornSetWidgetTexture(ID_ICON, ID_ICON_TEX);
	}
	QuickMenuRebornRegisterWidget(ID_TEXT, ID_PLANE, QMR_TEXT);
	QuickMenuRebornSetWidgetSize(ID_TEXT, 640, 75, 0, 0);
	QuickMenuRebornSetWidgetColor(ID_TEXT, 1, 1, 1, 1);
	QuickMenuRebornSetWidgetPosition(ID_TEXT, -152, 0, 0, 0);
	QuickMenuRebornAssignOnLoadHandler(on_label_load, ID_TEXT);
	QuickMenuRebornRegisterWidget(ID_BOX, ID_PLANE, QMR_CHECK_BOX);
	QuickMenuRebornSetWidgetSize(ID_BOX, 48, 48, 0, 0);
	QuickMenuRebornSetWidgetColor(ID_BOX, 1, 1, 1, 1);
	QuickMenuRebornSetWidgetPosition(ID_BOX, 350, 0, 0, 0);
	QuickMenuRebornAssignDefaultCheckBoxRecall(ID_BOX);
	QuickMenuRebornRegisterEventHanlder(ID_BOX, QMR_EVENT_RELEASE, on_toggle, NULL);

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
	(void)argc; (void)args;
	QuickMenuRebornUnregisterWidget(ID_BOX);
	QuickMenuRebornUnregisterWidget(ID_TEXT);
	QuickMenuRebornUnregisterWidget(ID_ICON);
	QuickMenuRebornUnregisterTexture(ID_ICON_TEX);
	QuickMenuRebornUnregisterWidget(ID_PLANE);
	QuickMenuRebornRemoveSeparator(ID_SEPARATOR);
	return SCE_KERNEL_STOP_SUCCESS;
}
