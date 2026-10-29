/*
 * hud.h - storage overlay, kernel side.
 *
 * Drawn by hooking SceDisplay's set-framebuffer path, so it appears over
 * whatever is on screen in any app. It draws only text the 1 Hz worker hands
 * it (ts_hud_set_text); the frame hook itself formats nothing.
 */
#ifndef STORAGETURBO_HUD_H
#define STORAGETURBO_HUD_H

#define TS_HUD_LINES     6
#define TS_HUD_LINE_MAX  48   /* chars incl. NUL; ASCII only               */

int  ts_hud_init(void);   /* install the set-framebuffer hook              */
void ts_hud_fini(void);
int  ts_hud_installed(void);
void ts_hud_diag_poll(unsigned secs);   /* worker: log why frames are (not) drawn */

/* Replace the text (n lines, n = 0 hides the HUD) and the screen corner
 * (TS_POS_*). Worker thread only. */
void ts_hud_set_text(const char lines[][TS_HUD_LINE_MAX], int n, int pos);

/* Record the calling process (syscall context) as a system screen
 * (TS_HUD_PROC_*); with the in-game HUD off, only those get the overlay. */
int  ts_hud_register_proc(int role);

#endif /* STORAGETURBO_HUD_H */
