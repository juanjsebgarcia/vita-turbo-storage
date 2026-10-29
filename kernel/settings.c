/*
 * settings.c - load, validate, apply and save Turbo Storage's settings.
 *
 * The file is tiny and versioned:
 *   magic 'TSCF' | version | count | count x int32 (enum ts_key order) | check
 * Keys are only ever appended, so a file from an older version (fewer keys)
 * still loads: its keys are read and the newer ones take their defaults.
 * A missing, short or corrupt file means defaults. Each value is validated on
 * its own, so one bad field falls back to its default without losing the rest.
 *
 * ts_settings_set() runs in the caller's syscall context, so it only updates
 * memory (and the staging switches) and marks the settings dirty; the 1 Hz
 * worker (main.c) does the file write from its own kernel thread.
 */
#include <psp2kern/kernel/sysclib.h>   /* memcpy/memset                        */
#include <psp2kern/io/fcntl.h>
#include <psp2kern/io/stat.h>

#include "settings.h"
#include "klog.h"
#include "stage.h"

#define TS_CFG_MAGIC    0x46435354u   /* 'TSCF'                                */
#define TS_CFG_VERSION  1u

typedef struct TsSettingsFile {
	uint32_t magic;
	uint32_t version;
	uint32_t count;
	int32_t  values[TS_KEY_COUNT];
	uint32_t check;
} TsSettingsFile;

volatile TsSettings g_ts_settings;
static volatile int g_dirty = 0;
static int          g_save_fails = 0;   /* retry a failed save a few times */

static const int32_t g_defaults[TS_KEY_COUNT] = {
	TS_DEF_SD2VITA, TS_DEF_INTERNAL, TS_DEF_VERIFY, TS_DEF_HUD, TS_DEF_HUD_POS,
	TS_DEF_DEVMODE, TS_DEF_MEMCARD, TS_DEF_HUD_GAME, TS_DEF_HUD_RECENT,
};

static uint32_t file_check(const TsSettingsFile *f)
{
	uint32_t c = f->magic ^ (f->version << 8) ^ (f->count << 16);
	for (uint32_t i = 0; i < f->count && i < TS_KEY_COUNT; i++)
		c = (c << 5 | c >> 27) ^ (uint32_t)f->values[i];
	return c;
}

void ts_settings_init(void)
{
	TsSettings s;
	for (int k = 0; k < TS_KEY_COUNT; k++)
		*ts_setting_field(&s, k) = g_defaults[k];

	TsSettingsFile f;
	memset(&f, 0, sizeof(f));
	SceUID fd = ksceIoOpen(TS_SETTINGS_PATH, SCE_O_RDONLY, 0);
	if (fd < 0) {
		TS_KLOGI("settings: no saved file (0x%08x), using defaults", (unsigned)fd);
	} else {
		int n = ksceIoRead(fd, &f, sizeof(f));
		ksceIoClose(fd);
		/* A file with fewer keys is shorter: its check word sits where the
		 * first missing value would be. */
		uint32_t cnt = f.count;
		int ok = f.magic == TS_CFG_MAGIC && f.version == TS_CFG_VERSION &&
		         cnt >= 1 && cnt <= TS_KEY_COUNT &&
		         n == (int)(3 * sizeof(uint32_t) + (cnt + 1) * sizeof(int32_t));
		if (ok) {
			uint32_t stored = cnt == TS_KEY_COUNT ? f.check : (uint32_t)f.values[cnt];
			ok = stored == file_check(&f);
		}
		if (!ok) {
			TS_KLOGW("settings: saved file unusable (n=%d), using defaults", n);
		} else {
			for (int k = 0; k < (int)cnt; k++) {
				if (ts_setting_valid(k, f.values[k]))
					*ts_setting_field(&s, k) = f.values[k];
				else
					TS_KLOGW("settings: key %d value %d out of range, default %d",
					         k, f.values[k], g_defaults[k]);
			}
		}
	}
	memcpy((void *)&g_ts_settings, &s, sizeof(s));
	TS_KLOGI("settings: sd2vita=%d internal=%d memcard=%d verify=%d hud=%d pos=%d devmode=%d",
	         s.sd2vita, s.internal, s.memcard, s.verify, s.hud, s.hud_pos, s.devmode);
	ts_stage_apply(&s);
}

int ts_settings_set(int key, int value)
{
	if (!ts_setting_valid(key, value))
		return TS_ERR_ARG;
	int32_t *f = ts_setting_field((TsSettings *)&g_ts_settings, key);
	if (!f)
		return TS_ERR_ARG;
	if (*f != value) {
		TsSettings now;

		*(volatile int32_t *)f = value;
		g_save_fails = 0;
		g_dirty = 1;
		ts_settings_copy(&now);
		ts_stage_apply(&now);              /* takes effect on the next transfer */
	}
	return TS_OK;
}

void ts_settings_copy(TsSettings *out)
{
	memcpy(out, (const void *)&g_ts_settings, sizeof(*out));
}

void ts_settings_flush(void)
{
	if (!g_dirty)
		return;
	g_dirty = 0;

	TsSettingsFile f;
	TsSettings s;
	ts_settings_copy(&s);
	memset(&f, 0, sizeof(f));
	f.magic = TS_CFG_MAGIC;
	f.version = TS_CFG_VERSION;
	f.count = TS_KEY_COUNT;
	for (int k = 0; k < TS_KEY_COUNT; k++)
		f.values[k] = *ts_setting_field(&s, k);
	f.check = file_check(&f);

	ksceIoMkdir("ur0:data", 0777);
	ksceIoMkdir(TS_DATA_DIR, 0777);
	SceUID fd = ksceIoOpen(TS_SETTINGS_PATH,
	                       SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
	int w = fd >= 0 ? ksceIoWrite(fd, &f, sizeof(f)) : fd;
	if (fd >= 0)
		ksceIoClose(fd);
	if (w != (int)sizeof(f)) {
		if (g_save_fails++ < 5) {
			TS_KLOGW("settings: save failed (0x%08x), will retry", (unsigned)w);
			g_dirty = 1;
		} else {
			TS_KLOGE("settings: save failed (0x%08x), giving up until the next change",
			         (unsigned)w);
		}
		return;
	}
	g_save_fails = 0;
	TS_KLOGI("settings: saved sd2vita=%d internal=%d memcard=%d verify=%d hud=%d pos=%d devmode=%d",
	         s.sd2vita, s.internal, s.memcard, s.verify, s.hud, s.hud_pos, s.devmode);
}
