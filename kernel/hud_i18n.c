/*
 * hud_i18n.c - ASCII HUD word translations (see hud_i18n.h).
 *
 * Machine-quality, upper-case ASCII only (accents dropped so the 8x8 font can draw
 * them), short to fit a HUD line. Non-Latin-script languages (Japanese,
 * Russian, Korean, Chinese) fall back to English; corrections from native
 * speakers are welcome. Row index = Vita system-language id.
 * Column order: ON, OFF, STANDARD, ERROR, UNAVAILABLE, ERR, VERIFY, MEMCARD, SLOW, PAUSED.
 */
#include "hud_i18n.h"

#define TS_HUD_LANG_COUNT 20

#define EN { "ON", "OFF", "STANDARD", "ERROR", "UNAVAILABLE", "ERR", "VERIFY", "MEMORY CARD", "SLOW", "PAUSED" }

static const char *const T[TS_HUD_LANG_COUNT][HTR_COUNT] = {
/* 0  JA (non-Latin -> EN) */ EN,
/* 1  EN-US              */ EN,
/* 2  FR                 */ { "ACTIVE", "DESACTIVE", "STANDARD", "ERREUR", "INDISPONIBLE", "ERR", "VERIF", "CARTE MEMOIRE", "LENT" , "EN PAUSE" },
/* 3  ES                 */ { "ACTIVADO", "DESACTIVADO", "ESTANDAR", "ERROR", "NO DISPONIBLE", "ERR", "VERIF", "TARJ. MEMORIA", "LENTO" , "EN PAUSA" },
/* 4  DE                 */ { "AN", "AUS", "NORMAL", "FEHLER", "NICHT VERFUEGBAR", "FEHL", "PRUEF", "SPEICHERKARTE", "LANGSAM" , "PAUSIERT" },
/* 5  IT                 */ { "ATTIVO", "DISATTIVATO", "STANDARD", "ERRORE", "NON DISPONIBILE", "ERR", "VERIFICA", "SCHEDA MEMORIA", "LENTO" , "IN PAUSA" },
/* 6  NL                 */ { "AAN", "UIT", "STANDAARD", "FOUT", "NIET BESCHIKBAAR", "FOUT", "CONTROLE", "GEHEUGENKAART", "TRAAG" , "GEPAUZEERD" },
/* 7  PT-PT              */ { "LIGADO", "DESLIGADO", "PADRAO", "ERRO", "INDISPONIVEL", "ERR", "VERIF", "CARTAO MEMORIA", "LENTO" , "EM PAUSA" },
/* 8  RU (non-Latin->EN) */ EN,
/* 9  KO (non-Latin->EN) */ EN,
/* 10 ZH-T (non-Latin)   */ EN,
/* 11 ZH-S (non-Latin)   */ EN,
/* 12 FI                 */ { "PAALLA", "POIS", "VAKIO", "VIRHE", "EI KAYTETTAVISSA", "VIRH", "TARKISTUS", "MUISTIKORTTI", "HIDAS" , "TAUOLLA" },
/* 13 SV                 */ { "PA", "AV", "STANDARD", "FEL", "EJ TILLGANGLIG", "FEL", "KONTROLL", "MINNESKORT", "LANGSAM" , "PAUSAD" },
/* 14 DA                 */ { "TIL", "FRA", "STANDARD", "FEJL", "UTILGAENGELIG", "FEJL", "KONTROL", "HUK.-KORT", "LANGSOM" , "PA PAUSE" },
/* 15 NO                 */ { "PA", "AV", "STANDARD", "FEIL", "UTILGJENGELIG", "FEIL", "KONTROLL", "MINNEKORT", "TREG" , "PAUSET" },
/* 16 PL                 */ { "WL.", "WYL.", "STANDARD", "BLAD", "NIEDOSTEPNE", "BL.", "WERYF.", "KARTA PAMIECI", "WOLNO" , "WSTRZYMANO" },
/* 17 PT-BR              */ { "LIGADO", "DESLIGADO", "PADRAO", "ERRO", "INDISPONIVEL", "ERR", "VERIF", "CARTAO MEMORIA", "LENTO" , "PAUSADO" },
/* 18 EN-GB              */ EN,
/* 19 TR                 */ { "ACIK", "KAPALI", "STANDART", "HATA", "KULLANILAMAZ", "HATA", "DOGRULAMA", "HAFIZA KARTI", "YAVAS" , "DURAKLATILDI" },
};

const char *hud_tr(int lang, int tok)
{
	if (tok < 0 || tok >= HTR_COUNT)
		return "";
	if (lang < 0 || lang >= TS_HUD_LANG_COUNT)
		lang = 1;                      /* English (US) fallback */
	return T[lang][tok];
}
