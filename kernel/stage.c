/*
 * stage.c - the faster storage path ("staging").
 *
 * Why it is faster: before and after every transfer, the system's storage driver
 * cleans and invalidates the CPU data cache over the whole caller buffer, and
 * that work is not overlapped with the transfer. On the SD2Vita it costs ~13 us
 * of every ~34 us 512-byte block. The driver skips it for non-cacheable memory.
 *
 * What this does: for a large transfer on an enabled device, call the system's
 * own, unmodified sector read/write with a physically contiguous non-cacheable
 * staging buffer in place of the caller's buffer, and copy between the two with
 * the CPU. The driver still issues every command, handles every error and runs
 * the DMA; only the buffer it is handed changes.
 *
 * Hooked exports (SceSdifForDriver), all pass-through unless a transfer
 * qualifies:
 *   ksceSdifReadSectorSd / ksceSdifWriteSectorSd    -> SD2Vita (port 1, SD card)
 * and (SceMsifForDriver):
 *   ksceMsifReadSector / ksceMsifWriteSector        -> the memory card (one slot)
 * Never touched: small transfers (< 32 KiB: file-system metadata), transfers
 * into or out of non-cacheable memory (the driver does no cache work there, so
 * the standard path is already the fast one), other ports
 * (the Wi-Fi chip is on its own port and never calls these), USB storage,
 * game cards, and the console's built-in storage, which also holds the
 * system's own partitions (its MMC exports are not hooked at all).
 *
 * Safety rules:
 *   - one staging buffer per device; if it is busy the transfer goes stock;
 *   - nothing is cached or deferred: a write has reached the card before the
 *     call returns, exactly as stock;
 *   - any error on the faster path switches that device back to stock for the
 *     session and finishes the same request the stock way;
 *   - with "Pause in Vita games" on, nothing takes the faster path while a game
 *     is in front or for a game's own transfers (fg.c);
 *   - verify mode re-reads every faster transfer the stock way and compares;
 *     on a mismatch the caller gets stock data (reads) or the chunk is
 *     rewritten the stock way (writes), and the device falls back.
 *
 * HOT PATH: no logging here, only counters (the 1 Hz worker logs).
 */
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/threadmgr.h>
#include <taihen.h>

#include "stage.h"
#include "klog.h"
#include "fg.h"

#define LIB_SDIF       0x96D306FA    /* SceSdifForDriver                       */
#define NID_READ_SD    0xB9593652    /* ksceSdifReadSectorSd                   */
#define NID_WRITE_SD   0xE0781171    /* ksceSdifWriteSectorSd                  */
#define LIB_MSIF       0xB706084A    /* SceMsifForDriver                       */
#define NID_READ_MS    0x58654AA3    /* ksceMsifReadSector(sector, buf, count) */
#define NID_WRITE_MS   0x329035EF    /* ksceMsifWriteSector                    */

/* The largest request the storage stack issues is 512 sectors (256 KiB);
 * anything larger is moved in pieces. */
#define STAGE_SECTORS  512
#define STAGE_BYTES    (STAGE_SECTORS * 512)
#define MIN_SECTORS    64            /* 32 KiB: smaller transfers stay stock   */

/* Kernel non-cacheable RW memory, physically contiguous (attr below): the same
 * kind the system uses for its own USB storage I/O buffer. */
#define MEMBLOCK_KERNEL_NC_RW  0x10208006

void *ksceSdifGetSdContextGlobal(int port);
void ts_boot_mark(const char *stage);   /* main.c: names the bring-up step */

typedef struct Dev {
	int           port;          /* SDIF port, or -1: the memory card          */
	SceUID        uid;
	void         *buf;           /* NC staging buffer                          */
	volatile int  busy;
	volatile int  enabled;       /* from settings                              */
	volatile int  fallback;      /* error/mismatch seen: stock for the session */
	int           hooked;        /* its driver's hooks are in: may stage      */
	int           alloc_failed;  /* buffer allocation failed: retried when the
	                              * storage's own switch goes off and on      */
	uint64_t      rd_bytes, wr_bytes;   /* moved on the faster path            */
	uint64_t      all_rd, all_wr;       /* every transfer on this device (HUD) */
	TsDevState    st;            /* counters (status filled in by stage_get)   */
} Dev;

static int g_stage_ok;   /* hooks installed: buffers may be allocated */

static Dev g_dev[TS_DEV_COUNT] = {
	[TS_DEV_SD2VITA]  = { .port = 1, .uid = -1, .hooked = 1 },
	[TS_DEV_MEMCARD]  = { .port = -1, .uid = -1, .hooked = 1 },
};

/* Verify mode: one shared cached buffer for the stock re-read. */
static volatile int g_verify;
static void        *g_shadow;
static SceUID       g_shadow_uid = -1;
static volatile int g_shadow_busy;
static volatile int g_shadow_failed;   /* allocation failed: no retry every tick */

/* Returned when a hook runs before taiHEN has stored its ref (see ref_of):
 * a failed transfer (SCE_ERROR_ERRNO_EIO), never a jump through NULL. */
#define TS_IO_ERROR  ((int)0x80010005)

/* SceSysmemForDriver ksceKernelGetMemBlockMemtypeByAddr: the memory type at an
 * address of the current address space (0x8000 non-cacheable, CDRAM included;
 * 0x0800 device), or < 0. Resolved at run time; without it everything counts
 * as cached, as before. */
#define MEMTYPE_NC      0x8000
#define MEMTYPE_DEVICE  0x0800
static int (*g_memtype_of)(const void *addr);

enum { H_RD_SD, H_WR_SD, H_RD_MS, H_WR_MS, H_COUNT };
#define H_SDIF_COUNT H_RD_MS          /* the SceSdif hooks come first          */
static volatile tai_hook_ref_t g_ref[H_COUNT];
static SceUID g_hook[H_COUNT] = { -1, -1, -1, -1 };

/* Calling the original: SceSdif takes the card context first, SceMsif has a
 * single slot and no context. `part` is NULL for the memory card. */
typedef int (*io_fn)(tai_hook_ref_t ref, void *part, uint32_t sector, void *buf, uint32_t n);

static int io_sdif(tai_hook_ref_t ref, void *part, uint32_t sector, void *buf, uint32_t n)
{
	return TAI_CONTINUE(int, ref, part, sector, buf, n);
}

static int io_msif(tai_hook_ref_t ref, void *part, uint32_t sector, void *buf, uint32_t n)
{
	(void)part;
	return TAI_CONTINUE(int, ref, sector, buf, n);
}

/* taiHEN can make a hook live a moment before it stores the ref. A call that
 * lands in that window waits for it (bounded) instead of sleeping blindly; if
 * it is still not there, the hook fails the transfer (TS_IO_ERROR). */
static tai_hook_ref_t ref_of(int h)
{
	for (int i = 0; i < 2000 && g_ref[h] == 0; i++)
		ksceKernelDelayThread(10);
	return g_ref[h];
}

static int on_port(void *part, int port)
{
	if (port < 0)
		return 1;              /* the memory card hooks only see the card      */
	/* A card context's first word is its controller context. */
	return part && *(void **)part == ksceSdifGetSdContextGlobal(port);
}

int module_get_export_func(SceUID pid, const char *modname, uint32_t libnid,
                           uint32_t funcnid, uintptr_t *func);

static int alloc_buf(unsigned type, int phycont, SceUID *uid, void **base)
{
	SceKernelAllocMemBlockKernelOpt opt;

	memset(&opt, 0, sizeof(opt));
	opt.size = sizeof(opt);
	if (phycont)
		opt.attr = SCE_KERNEL_ALLOC_MEMBLOCK_ATTR_PHYCONT;
	*uid = ksceKernelAllocMemBlock("SceStorageTurbo", type, STAGE_BYTES, &opt);
	if (*uid < 0)
		return *uid;
	if (ksceKernelGetMemBlockBase(*uid, base) < 0) {
		ksceKernelFreeMemBlock(*uid);
		*uid = -1;
		return TS_ERR_NOMEM;
	}
	return TS_OK;
}

static int is_uncached(const void *a)
{
	int m = g_memtype_of(a);
	return m == MEMTYPE_NC || m == MEMTYPE_DEVICE;
}

/* Non-cacheable at both ends: anything unknown counts as cached. */
static int caller_uncached(const void *buf, uint32_t nsec)
{
	return g_memtype_of && is_uncached(buf) &&
	       is_uncached((const uint8_t *)buf + nsec * 512 - 1);
}

/* Take the device buffer for one request, or 0 if the request stays stock. */
static int take(Dev *d, int mine, const void *buf, uint32_t nsec)
{
	if (!mine || !d->enabled || d->fallback || !d->buf || nsec < MIN_SECTORS)
		return 0;
	if (caller_uncached(buf, nsec)) {
		d->st.uncached++;
		return 0;
	}
	if (ts_fg_io_hold()) {
		d->st.held++;
		return 0;
	}
	if (__sync_lock_test_and_set(&d->busy, 1)) {
		d->st.passthru++;
		return 0;
	}
	/* Again with the buffer held: ts_stage_fini clears `enabled`, then waits
	 * for `busy` to drop, so a request that passed the check above just before
	 * must not go on to use the buffer. */
	__sync_synchronize();
	if (!d->enabled || d->fallback) {
		__sync_lock_release(&d->busy);
		return 0;
	}
	return 1;
}

static int take_shadow(void)
{
	return g_verify && g_shadow && !__sync_lock_test_and_set(&g_shadow_busy, 1);
}

static int stage_read(Dev *d, io_fn io, tai_hook_ref_t ref, void *part, uint32_t sector,
                      void *buf, uint32_t nsec)
{
	uint8_t *dst = buf;
	uint32_t left = nsec;
	int verify, ret = 0, mine = on_port(part, d->port);

	if (mine)
		d->all_rd += (uint64_t)nsec * 512;
	if (!take(d, mine, buf, nsec))
		return io(ref, part, sector, buf, nsec);
	verify = take_shadow();
	while (left && !d->fallback) {
		uint32_t n = left > STAGE_SECTORS ? STAGE_SECTORS : left, bytes = n * 512;

		ret = io(ref, part, sector, d->buf, n);
		if (ret < 0)
			break;
		memcpy(dst, d->buf, bytes);
		if (verify && io(ref, part, sector, g_shadow, n) >= 0) {
			d->st.verified++;
			if (memcmp(dst, g_shadow, bytes) != 0) {
				d->st.mismatches++;
				d->fallback = 1;
				memcpy(dst, g_shadow, bytes);   /* the caller gets stock data */
			}
		}
		d->rd_bytes += bytes;
		dst += bytes;
		sector += n;
		left -= n;
	}
	if (verify)
		__sync_lock_release(&g_shadow_busy);
	__sync_lock_release(&d->busy);
	if (ret < 0) {
		d->st.errors++;
		d->fallback = 1;
	} else {
		d->st.reads++;
	}
	/* Anything not done on the faster path (error, mismatch) is done stock. */
	return left ? io(ref, part, sector, dst, left) : ret;
}

static int stage_write(Dev *d, io_fn io, tai_hook_ref_t ref, tai_hook_ref_t rd_ref,
                       void *part, uint32_t sector, void *buf, uint32_t nsec)
{
	const uint8_t *src = buf;
	uint32_t left = nsec;
	int verify, ret = 0, mine = on_port(part, d->port);

	if (mine)
		d->all_wr += (uint64_t)nsec * 512;
	if (!take(d, mine, buf, nsec))
		return io(ref, part, sector, buf, nsec);
	verify = take_shadow();
	if (verify && !rd_ref) {            /* no read hook yet: cannot verify this one */
		__sync_lock_release(&g_shadow_busy);
		verify = 0;
	}
	while (left && !d->fallback) {
		uint32_t n = left > STAGE_SECTORS ? STAGE_SECTORS : left, bytes = n * 512;

		memcpy(d->buf, src, bytes);
		ret = io(ref, part, sector, d->buf, n);
		if (ret < 0)
			break;
		if (verify && io(rd_ref, part, sector, g_shadow, n) >= 0) {
			d->st.verified++;
			if (memcmp(g_shadow, src, bytes) != 0) {
				d->st.mismatches++;
				d->fallback = 1;
				/* rewrite this chunk the stock way, from the caller's data */
				ret = io(ref, part, sector, (void *)src, n);
				if (ret < 0)
					break;
			}
		}
		d->wr_bytes += bytes;
		src += bytes;
		sector += n;
		left -= n;
	}
	if (verify)
		__sync_lock_release(&g_shadow_busy);
	__sync_lock_release(&d->busy);
	if (ret < 0) {
		d->st.errors++;
		d->fallback = 1;
	} else {
		d->st.writes++;
	}
	return left ? io(ref, part, sector, (void *)src, left) : ret;
}

static int h_rd_sd(void *part, uint32_t sector, void *buf, uint32_t nsec)
{
	tai_hook_ref_t r = ref_of(H_RD_SD);

	return r ? stage_read(&g_dev[TS_DEV_SD2VITA], io_sdif, r, part, sector, buf, nsec) : TS_IO_ERROR;
}

static int h_wr_sd(void *part, uint32_t sector, void *buf, uint32_t nsec)
{
	tai_hook_ref_t r = ref_of(H_WR_SD);

	return r ? stage_write(&g_dev[TS_DEV_SD2VITA], io_sdif, r, g_ref[H_RD_SD], part,
	                       sector, buf, nsec) : TS_IO_ERROR;
}

static int h_rd_ms(uint32_t sector, void *buf, uint32_t nsec)
{
	tai_hook_ref_t r = ref_of(H_RD_MS);

	return r ? stage_read(&g_dev[TS_DEV_MEMCARD], io_msif, r, NULL, sector, buf, nsec) : TS_IO_ERROR;
}

static int h_wr_ms(uint32_t sector, void *buf, uint32_t nsec)
{
	tai_hook_ref_t r = ref_of(H_WR_MS);

	return r ? stage_write(&g_dev[TS_DEV_MEMCARD], io_msif, r, g_ref[H_RD_MS], NULL,
	                       sector, buf, nsec) : TS_IO_ERROR;
}

int ts_stage_init(void)
{
	static const struct { int h; uint32_t nid; void *fn; const char *name; } k_hooks[H_COUNT] = {
		{ H_RD_SD,  NID_READ_SD,   h_rd_sd,  "hook sd read"      },
		{ H_WR_SD,  NID_WRITE_SD,  h_wr_sd,  "hook sd write"     },
		{ H_RD_MS,  NID_READ_MS,   h_rd_ms,  "hook memcard read" },
		{ H_WR_MS,  NID_WRITE_MS,  h_wr_ms,  "hook memcard write" },
	};

	uintptr_t mt = 0;
	if (module_get_export_func(KERNEL_PID, "SceSysmem", 0x6F25E18A, 0xF3BBE2E1, &mt) >= 0)
		g_memtype_of = (int (*)(const void *))mt;
	TS_KLOGI("stage: memory type lookup %s", g_memtype_of ? "ok" :
	         "missing - non-cacheable buffers are staged too");

	/* Staging buffers are not allocated here: the worker allocates one only
	 * for a storage that is switched on (ts_stage_worker_tick), so a default
	 * install (every storage off) holds no contiguous memory. */
	ts_boot_mark("hooks");
	for (int i = 0; i < H_SDIF_COUNT; i++) {
		g_hook[k_hooks[i].h] = taiHookFunctionExportForKernel(KERNEL_PID,
			(tai_hook_ref_t *)&g_ref[k_hooks[i].h], "SceSdif", LIB_SDIF, k_hooks[i].nid,
			k_hooks[i].fn);
		if (g_hook[k_hooks[i].h] < 0) {
			TS_KLOGE("stage: hook %d failed (0x%08x) - releasing all", i,
			         (unsigned)g_hook[k_hooks[i].h]);
			ts_stage_fini();
			return TS_ERR_RESOLVE;
		}
	}
	/* The memory card is optional: if its hooks can't be installed, only the
	 * memory card stays stock (its buffer is released). */
	for (int i = H_SDIF_COUNT; i < H_COUNT; i++) {
		g_hook[k_hooks[i].h] = taiHookFunctionExportForKernel(KERNEL_PID,
			(tai_hook_ref_t *)&g_ref[k_hooks[i].h], "SceMsif", LIB_MSIF, k_hooks[i].nid,
			k_hooks[i].fn);
		if (g_hook[k_hooks[i].h] < 0) {
			TS_KLOGW("stage: memory card hook %d failed (0x%08x) - memory card stays stock",
			         i, (unsigned)g_hook[k_hooks[i].h]);
			for (int j = H_SDIF_COUNT; j < H_COUNT; j++)
				if (g_hook[j] >= 0) {
					taiHookReleaseForKernel(g_hook[j], g_ref[j]);
					g_hook[j] = -1;
				}
			g_dev[TS_DEV_MEMCARD].hooked = 0;   /* never gets a buffer */
			break;
		}
	}
	g_stage_ok = 1;
	TS_KLOGI("stage: hooks installed (memory card %s); buffers as storages are switched on",
	         g_dev[TS_DEV_MEMCARD].hooked ? "too" : "not hooked");
	return TS_OK;
}

void ts_stage_fini(void)
{
	/* Stop taking the faster path, then wait for any transfer in flight. */
	g_stage_ok = 0;   /* no new buffers */
	for (int i = 0; i < TS_DEV_COUNT; i++)
		g_dev[i].enabled = 0;
	for (int i = 0; i < TS_DEV_COUNT; i++)
		while (g_dev[i].busy)
			ksceKernelDelayThread(1000);
	for (int i = H_COUNT - 1; i >= 0; i--)
		if (g_hook[i] >= 0) {
			taiHookReleaseForKernel(g_hook[i], g_ref[i]);
			g_hook[i] = -1;
		}
	for (int i = 0; i < TS_DEV_COUNT; i++)
		if (g_dev[i].uid >= 0) {
			ksceKernelFreeMemBlock(g_dev[i].uid);
			g_dev[i].uid = -1;
			g_dev[i].buf = NULL;
		}
	if (g_shadow_uid >= 0) {
		ksceKernelFreeMemBlock(g_shadow_uid);
		g_shadow_uid = -1;
		g_shadow = NULL;
	}
}

/* Allocate i's staging buffer if it has none (worker only, so it can't race
 * ts_stage_fini: module_stop joins the worker first). Only ever NULL -> buffer
 * while the module runs, so a transfer that saw a buffer keeps a valid one.
 * The uid is in place before the buffer is. */
static void ensure_buf(int i)
{
	Dev *d = &g_dev[i];
	SceUID uid;
	void *base;

	if (d->buf || d->alloc_failed || !d->hooked || !g_stage_ok)
		return;
	int r = alloc_buf(MEMBLOCK_KERNEL_NC_RW, 1, &uid, &base);
	if (r < 0) {
		d->alloc_failed = 1;
		TS_KLOGW("stage: dev %d staging buffer not allocated (0x%08x) - stock; "
		         "switch it off and on to retry", i, (unsigned)r);
		return;
	}
	d->uid = uid;
	__sync_synchronize();
	d->buf = base;
	TS_KLOGI("stage: dev %d staging buffer %u KiB at %p", i,
	         (unsigned)(STAGE_BYTES >> 10), base);
}

void ts_stage_apply(const TsSettings *s)
{
	static int32_t own_prev[TS_DEV_COUNT];   /* the storage's own switch, last apply */
	const int32_t own[TS_DEV_COUNT] = { s->sd2vita, s->memcard };

	for (int i = 0; i < TS_DEV_COUNT; i++) {
		/* Switching the storage's own row back on is a fresh start; the master
		 * switch (also in the quick menu) is not, so it can't re-arm a storage
		 * that fell back after an error or a mismatch. */
		if (own[i] && !own_prev[i]) {
			g_dev[i].fallback = 0;
			g_dev[i].alloc_failed = 0;
		}
		own_prev[i] = own[i] ? 1 : 0;
		g_dev[i].enabled = s->enabled && own[i] ? 1 : 0;
	}
	g_verify = s->verify ? 1 : 0;
	if (!g_verify)
		g_shadow_failed = 0;      /* turning verify off and on again retries the buffer */
	ts_fg_enable(s->game_pause);
}

void ts_stage_worker_tick(void)
{
	/* Staging buffers: allocated here, in the worker (a kernel thread), never
	 * in the thread of the process that changed a setting, for a storage that
	 * is on and has none. Until then its transfers stay stock (take() checks
	 * the buffer). At boot that is the first tick, about 1 s after start. */
	for (int i = 0; i < TS_DEV_COUNT; i++)
		if (g_dev[i].enabled)
			ensure_buf(i);

	/* The verify buffer is only allocated once verify mode is first turned on. */
	if (g_verify && !g_shadow && g_shadow_uid < 0 && !g_shadow_failed) {
		int r = alloc_buf(SCE_KERNEL_MEMBLOCK_TYPE_KERNEL_RW, 0, &g_shadow_uid, &g_shadow);

		if (r < 0) {
			g_shadow = NULL;
			g_shadow_uid = -1;
			g_shadow_failed = 1;    /* do not retry every second */
			TS_KLOGW("stage: verify buffer not allocated (0x%08x)", (unsigned)r);
		} else {
			TS_KLOGI("stage: verify buffer allocated");
		}
	}
}

void ts_stage_bytes(int dev, uint64_t *rd, uint64_t *wr)
{
	*rd = g_dev[dev].all_rd;
	*wr = g_dev[dev].all_wr;
}

int ts_stage_verify_ready(void)
{
	return g_shadow != NULL;
}

void ts_stage_get(TsDevState out[TS_DEV_COUNT], int plugin_active)
{
	int paused = ts_fg_paused();

	for (int i = 0; i < TS_DEV_COUNT; i++) {
		const Dev *d = &g_dev[i];

		out[i] = d->st;
		out[i].read_mib = (uint32_t)(d->rd_bytes >> 20);
		out[i].write_mib = (uint32_t)(d->wr_bytes >> 20);
		out[i].status = !plugin_active ? TS_ST_UNAVAIL :
		                !d->enabled    ? TS_ST_OFF :
		                !d->buf        ? TS_ST_NO_BUFFER :
		                d->fallback    ? TS_ST_FALLBACK :
		                paused         ? TS_ST_PAUSED : TS_ST_ACTIVE;
	}
}

/* Memory card slow mode (Sony's own throttle: it sleeps after every memory
 * card transfer). Read only; Turbo Storage never changes it. 1 on, 0 off,
 * -1 if the driver's getter isn't available. */

int ts_stage_memcard_slow(void)
{
	static int (*get)(void);
	static int tried;

	if (!tried) {
		uintptr_t f = 0;
		tried = 1;
		if (module_get_export_func(KERNEL_PID, "SceMsif", LIB_MSIF, 0x491E25B5, &f) >= 0)
			get = (int (*)(void))f;   /* ksceMsifGetSlowCardModeState */
	}
	return get ? (get() ? 1 : 0) : -1;
}
