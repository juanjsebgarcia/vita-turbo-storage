/*
 * hud.c - draw the Turbo Storage HUD over the live framebuffer.
 *
 * Technique (the standard taiHEN overlay idiom, as in Wi-Fi Turbo): hook
 * SceDisplay's set-framebuffer funnel. The caller hands us the framebuffer
 * base, pitch and size every frame; we let the original run, then blit our
 * text into the same buffer before the display controller scans it out.
 *
 * Strictly read + overdraw: no storage state is touched here. The text comes
 * from the 1 Hz worker (main.c) through ts_hud_set_text(), double-buffered so
 * the frame hook never reads a half-written line.
 *
 * Position: one of the four corners, computed from the framebuffer's own width
 * and height each frame, so it lands correctly in titles that render at a
 * lower resolution. Below 720 px wide the font is drawn at 1x instead of 2x.
 *
 * Drawing is guarded by a plausibility check on pitch and size and is simply
 * skipped if the buffer looks wrong, rather than risk a bad write.
 */
#include <psp2kern/kernel/sysclib.h>   /* memcpy/memset                        */
#include <psp2kern/display.h>
#include <psp2kern/kernel/threadmgr.h>   /* ksceKernelGetProcessId              */
#include <taihen.h>

#include "hud.h"
#include "font8x8.h"
#include "klog.h"
#include "settings.h"                   /* g_ts_settings.hud_game             */
#include "../include/storageturbo.h"

#define HUD_FG         0xFFFFFFFFu /* opaque white (ABGR8888)                 */
#define HUD_SHADOW     0xFF000000u /* opaque black drop shadow                */

static SceUID         g_hook = -1;
static tai_hook_ref_t g_ref;

typedef struct {
	char lines[TS_HUD_LINES][TS_HUD_LINE_MAX];
	int  n;
	int  pos;
	int  maxlen;   /* longest line, for right/bottom alignment              */
} HudText;

/* Double-buffered: the worker fills the back copy, then flips g_front. */
static HudText      g_text[2];
static volatile int g_front = 0;

/* Diagnostics, written by the frame hook and logged by the worker
 * (ts_hud_diag_poll): the last frame seen and why it was or wasn't drawn. */
enum { HD_DRAWN = 0, HD_NO_TEXT, HD_BAD_SIZE, HD_BAD_FORMAT, HD_IN_GAME, HD_COUNT };

/* Each pid is one aligned word, written once by its process, so the hook
 * reads it unlocked. */
static volatile SceUID g_sys_pid[TS_HUD_PROC_COUNT] = { -1, -1 };
static volatile uint32_t g_d_fmt, g_d_w, g_d_h, g_d_why;
static volatile uint32_t g_d_n[HD_COUNT];

/* ---- framebuffer text blitter ------------------------------------------ */

static inline void put_px(uint32_t *fb, int pitch, int w, int h,
                          int x, int y, uint32_t color)
{
	if (x < 0 || y < 0 || x >= w || y >= h)
		return;
	fb[y * pitch + x] = color;
}

static void draw_char(uint32_t *fb, int pitch, int w, int h, int scale,
                      int ox, int oy, char ch, uint32_t color)
{
	if ((unsigned char)ch < TS_FONT_FIRST || (unsigned char)ch > TS_FONT_LAST)
		return;
	const unsigned char *g = ts_font8x8[(unsigned char)ch - TS_FONT_FIRST];
	for (int row = 0; row < 8; row++) {
		unsigned char bits = g[row];
		for (int col = 0; col < 8; col++) {
			if (!(bits & (1u << col)))
				continue;
			for (int sy = 0; sy < scale; sy++)
				for (int sx = 0; sx < scale; sx++)
					put_px(fb, pitch, w, h,
					       ox + col * scale + sx,
					       oy + row * scale + sy, color);
		}
	}
}

static void draw_text(uint32_t *fb, int pitch, int w, int h, int scale,
                      int ox, int oy, const char *s)
{
	for (int x = ox; *s; s++, x += 8 * scale) {
		/* 1px drop shadow for legibility over any background. */
		draw_char(fb, pitch, w, h, scale, x + 1, oy + 1, *s, HUD_SHADOW);
		draw_char(fb, pitch, w, h, scale, x, oy, *s, HUD_FG);
	}
}

static void render(uint32_t *fb, int pitch, int w, int h)
{
	const HudText *t = &g_text[g_front];
	if (t->n <= 0)
		return;

	int scale  = 2;   /* 16 px glyphs at any resolution, as Wi-Fi Turbo's HUD */
	int margin = 12 * scale;
	int line_h = 8 * scale + 2 * scale;
	int box_w  = t->maxlen * 8 * scale;
	int box_h  = t->n * line_h;

	int right  = t->pos == TS_POS_TOP_RIGHT || t->pos == TS_POS_BOTTOM_RIGHT;
	int bottom = t->pos == TS_POS_BOTTOM_LEFT || t->pos == TS_POS_BOTTOM_RIGHT;
	int x = right ? w - margin - box_w : margin;
	/* The system status bar is ~30 px tall at 960x544; keep clear of it. */
	int y = bottom ? h - margin - box_h : margin + (scale == 2 ? 24 : 12);
	if (x < 0) x = 0;
	if (y < 0) y = 0;

	for (int i = 0; i < t->n; i++, y += line_h)
		draw_text(fb, pitch, w, h, scale, x, y, t->lines[i]);
}

/* ---- the set-framebuffer hook ------------------------------------------- */

/* pParam->base is a user-space CDRAM VA. Writing it from this kernel hook
 * faults under the default ARM domain permissions, so open the Domain Access
 * Control Register for the duration of the blit, then restore it - the
 * standard Vita kernel-overlay technique. */
static inline uint32_t hud_dacr_open(void)
{
	uint32_t prev;
	__asm__ volatile("mrc p15, 0, %0, c3, c0, 0" : "=r"(prev));
	__asm__ volatile("mcr p15, 0, %0, c3, c0, 0" :: "r"(0xFFFFFFFFu));
	__asm__ volatile("isb");
	return prev;
}
static inline void hud_dacr_restore(uint32_t prev)
{
	__asm__ volatile("mcr p15, 0, %0, c3, c0, 0" :: "r"(prev));
	__asm__ volatile("isb");
}

/* ksceDisplaySetFrameBufInternal(head, index, pParam, sync) - the kernel
 * funnel every set-framebuffer call passes through, so the overlay is
 * system-wide. Draw only for the main panel (head 0). */
static int hook_set_framebuf(int head, int index,
                             const SceDisplayFrameBuf *pParam, int sync)
{
	int ret = TAI_CONTINUE(int, g_ref, head, index, pParam, sync);

	if (head != 0 || !pParam || !pParam->base)
		return ret;
	int w = (int)pParam->width, h = (int)pParam->height,
	    pitch = (int)pParam->pitch;
	g_d_fmt = pParam->pixelformat; g_d_w = w; g_d_h = h;

	if (g_text[g_front].n <= 0) {
		g_d_why = HD_NO_TEXT; g_d_n[HD_NO_TEXT]++;
		return ret;
	}

	/* Plausibility guard: only draw into a sane 32bpp linear buffer. */
	if (w <= 0 || h <= 0 || pitch < w || w > 1920 || h > 1088) {
		g_d_why = HD_BAD_SIZE; g_d_n[HD_BAD_SIZE]++;
		return ret;
	}
	if (pParam->pixelformat != SCE_DISPLAY_PIXELFORMAT_A8B8G8R8) {
		g_d_why = HD_BAD_FORMAT; g_d_n[HD_BAD_FORMAT]++;
		return ret;
	}
	/* In-game HUD off: only frames set by a registered system process (this
	 * runs in the context of the process setting the frame). */
	if (!g_ts_settings.hud_game) {
		SceUID pid = ksceKernelGetProcessId();
		if (pid != g_sys_pid[TS_HUD_PROC_SHELL] && pid != g_sys_pid[TS_HUD_PROC_SETTINGS]) {
			g_d_why = HD_IN_GAME; g_d_n[HD_IN_GAME]++;
			return ret;
		}
	}
	g_d_why = HD_DRAWN; g_d_n[HD_DRAWN]++;

	uint32_t prev_dacr = hud_dacr_open();
	render((uint32_t *)pParam->base, pitch, w, h);
	hud_dacr_restore(prev_dacr);
	return ret;
}

int ts_hud_init(void)
{
	memset(g_text, 0, sizeof(g_text));

	/* NIDs from the VitaSDK db (vita-headers db/360/SceDisplay.yml), present
	 * unchanged in 3.65 and 3.68:
	 *   SceDisplayForDriver library    = 0x9FED47AC
	 *   ksceDisplaySetFrameBufInternal = 0x16466675 */
	g_hook = taiHookFunctionExportForKernel(KERNEL_PID, &g_ref,
	                                        "SceDisplay",
	                                        0x9FED47AC, /* SceDisplayForDriver  */
	                                        0x16466675, /* ...SetFrameBufInternal */
	                                        hook_set_framebuf);
	if (g_hook < 0) {
		TS_KLOGW("hud: set-framebuffer hook failed (0x%08x)", (unsigned)g_hook);
		return TS_ERR_RESOLVE;
	}
	TS_KLOGI("hud: set-framebuffer hook installed (uid=0x%08x)", (unsigned)g_hook);
	return TS_OK;
}

int ts_hud_register_proc(int role)
{
	if (role < 0 || role >= TS_HUD_PROC_COUNT)
		return TS_ERR_ARG;
	SceUID pid = ksceKernelGetProcessId();
	g_sys_pid[role] = pid;
	TS_KLOGI("hud: system process %d = pid 0x%08x", role, (unsigned)pid);
	return TS_OK;
}

void ts_hud_fini(void)
{
	g_text[0].n = g_text[1].n = 0;
	if (g_hook >= 0) {
		taiHookReleaseForKernel(g_hook, g_ref);
		g_hook = -1;
	}
}

/* Called once a second by the worker: log when the frame situation changes,
 * and a frame-count summary every 30 s. */
void ts_hud_diag_poll(unsigned secs)
{
	static uint32_t last_fmt = 0xFFFFFFFFu, last_w, last_h, last_why = 0xFFFFFFFFu;
	if (g_d_fmt != last_fmt || g_d_w != last_w || g_d_h != last_h || g_d_why != last_why) {
		static const char *const why[HD_COUNT] = { "drawn", "no text (off)",
		                                           "odd size", "pixel format",
		                                           "in-game (In-game HUD off)" };
		TS_KLOGI("hud: frame %ux%u fmt 0x%08x -> %s", (unsigned)g_d_w, (unsigned)g_d_h,
		         (unsigned)g_d_fmt, g_d_why < HD_COUNT ? why[g_d_why] : "?");
		last_fmt = g_d_fmt; last_w = g_d_w; last_h = g_d_h; last_why = g_d_why;
	}
	if (secs % 30 == 0)
		TS_KLOGI("hud: frames drawn=%u off=%u size=%u format=%u in-game=%u",
		         (unsigned)g_d_n[HD_DRAWN], (unsigned)g_d_n[HD_NO_TEXT],
		         (unsigned)g_d_n[HD_BAD_SIZE], (unsigned)g_d_n[HD_BAD_FORMAT],
		         (unsigned)g_d_n[HD_IN_GAME]);
}

int ts_hud_installed(void)
{
	return g_hook >= 0;
}

void ts_hud_set_text(const char lines[][TS_HUD_LINE_MAX], int n, int pos)
{
	int back = g_front ^ 1;
	HudText *t = &g_text[back];
	if (n < 0) n = 0;
	if (n > TS_HUD_LINES) n = TS_HUD_LINES;
	t->maxlen = 0;
	for (int i = 0; i < n; i++) {
		int len = 0;
		while (len < TS_HUD_LINE_MAX - 1 && lines[i][len]) {
			t->lines[i][len] = lines[i][len];
			len++;
		}
		t->lines[i][len] = 0;
		if (len > t->maxlen)
			t->maxlen = len;
	}
	t->pos = pos;
	t->n = n;
	g_front = back;
}
