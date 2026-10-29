/*
 * storageturbo_i18n.h - text for the "Turbo Storage" Settings page.
 *
 * All 20 Vita system languages. English (US/UK) is authoritative; the others
 * are machine translations (see tools/gen_i18n.py). The language follows the
 * system setting and falls back to English (US).
 * C99, no char16_t: UTF-16 output uses plain unsigned short.
 */
#ifndef STORAGETURBO_I18N_H
#define STORAGETURBO_I18N_H

enum ts_i18n_id {
	TS_I18N_TITLE = 0,          /* "Turbo Storage"                                */
	TS_I18N_ROW_SD2VITA,        /* row titles                                     */
	TS_I18N_ROW_INTERNAL,
	TS_I18N_ROW_HUD,
	TS_I18N_ROW_HUD_POS,
	TS_I18N_ROW_DEVMODE,
	TS_I18N_ROW_MEMCARD,
	TS_I18N_ROW_HUD_GAME,
	TS_I18N_ROW_HUD_RECENT,
	TS_I18N_HINT_SD2VITA,       /* help text, same order as the rows              */
	TS_I18N_HINT_INTERNAL,
	TS_I18N_HINT_HUD,
	TS_I18N_HINT_HUD_POS,
	TS_I18N_HINT_DEVMODE,
	TS_I18N_HINT_MEMCARD,
	TS_I18N_HINT_HUD_GAME,
	TS_I18N_HINT_HUD_RECENT,
	TS_I18N_ITEM_HUD_OFF,       /* HUD list items, in TS_HUD_* order              */
	TS_I18N_ITEM_HUD_COMPACT,
	TS_I18N_ITEM_HUD_EXTENDED,
	TS_I18N_ITEM_POS_TL,        /* position items, in TS_POS_* order              */
	TS_I18N_ITEM_POS_TR,
	TS_I18N_ITEM_POS_BL,
	TS_I18N_ITEM_POS_BR,
	TS_I18N_ST_FALLBACK,        /* added to a storage's help when it isn't working */
	TS_I18N_ST_NO_BUFFER,
	TS_I18N_ST_FIRMWARE,
	TS_I18N_ST_HOOKS,
	TS_I18N_ST_SAFE,
	TS_I18N_ST_NO_KERNEL,
	TS_I18N_ST_SLOW,
	TS_I18N_COUNT
};

/* Vita system-language indices (the raw /CONFIG/SYSTEM/language value). */
enum ts_lang_id {
	TS_LANG_JAPANESE = 0, TS_LANG_ENGLISH_US, TS_LANG_FRENCH, TS_LANG_SPANISH,
	TS_LANG_GERMAN, TS_LANG_ITALIAN, TS_LANG_DUTCH, TS_LANG_PORTUGUESE_PT,
	TS_LANG_RUSSIAN, TS_LANG_KOREAN, TS_LANG_CHINESE_T, TS_LANG_CHINESE_S,
	TS_LANG_FINNISH, TS_LANG_SWEDISH, TS_LANG_DANISH, TS_LANG_NORWEGIAN,
	TS_LANG_POLISH, TS_LANG_PORTUGUESE_BR, TS_LANG_ENGLISH_GB, TS_LANG_TURKISH,
	TS_LANG_COUNT
};

int ts_i18n_lang(void);
const char *ts_i18n_for(int id, int lang);
const char *ts_i18n(int id);   /* in the current system language */

/* UTF-8 -> UTF-16 (NUL-terminated, truncated to cap units). Returns the length. */
int ts_i18n_to_u16(const char *utf8, unsigned short *out, int cap);

#endif /* STORAGETURBO_I18N_H */
