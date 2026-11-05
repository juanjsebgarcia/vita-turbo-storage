/*
 * hud_i18n.h - HUD word translations (kernel side, ASCII only).
 *
 * The HUD draws with an 8x8 ASCII font, so every translation is plain ASCII
 * with accents dropped. Languages whose script the font cannot draw
 * (Japanese, Russian, Korean, Chinese) show English.
 *
 * Words are upper case, like the other Turbo HUDs. Only words are translated; units and abbreviations (R, W, MB/s, MiB,
 * SD2Vita) stay the same everywhere. Keep entries short: a HUD line is narrow.
 *
 * lang is the Vita system-language index (/CONFIG/SYSTEM/language).
 */
#ifndef STORAGETURBO_HUD_I18N_H
#define STORAGETURBO_HUD_I18N_H

enum hud_tr_tok {
	HTR_ON = 0,       /* the faster path is in use             */
	HTR_OFF,          /* the faster path is switched off       */
	HTR_STANDARD,     /* back to standard speed (after error)  */
	HTR_ERROR,        /* marks a storage that hit an error     */
	HTR_UNAVAILABLE,  /* cannot run (firmware, memory...)      */
	HTR_ERR,          /* short "errors" counter label          */
	HTR_VERIFY,       /* verify mode is on                     */
	HTR_MEMCARD,      /* the memory card's name                */
	HTR_SLOW,         /* memory card slow mode is on           */
	HTR_PAUSED,       /* stock for now: a game is in front     */
	HTR_COUNT
};

/* ASCII word for `tok` in language `lang`; English for non-Latin or unknown
 * languages. Never returns NULL. */
const char *hud_tr(int lang, int tok);

#endif /* STORAGETURBO_HUD_I18N_H */
