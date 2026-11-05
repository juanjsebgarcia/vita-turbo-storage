/*
 * stage.h - the faster storage path (kernel/stage.c).
 */
#ifndef STORAGETURBO_STAGE_H
#define STORAGETURBO_STAGE_H

#include "../include/storageturbo.h"

/* Allocate the per-device staging buffers and install the I/O hooks. All hooks
 * or none: on failure everything is released and storage stays stock. */
int  ts_stage_init(void);
void ts_stage_fini(void);

/* Apply settings (master and per-device on/off, verify mode, "Pause in Vita
 * games"). Safe from any context. */
void ts_stage_apply(const TsSettings *s);

/* Called once a second by the worker (allocates the verify buffer on demand). */
void ts_stage_worker_tick(void);

int  ts_stage_verify_ready(void);

/* Memory card slow mode: 1 on, 0 off, -1 unknown. Read only. */
int  ts_stage_memcard_slow(void);

/* Bytes read/written on `dev` since boot, faster path or not (for the HUD). */
void ts_stage_bytes(int dev, uint64_t *rd, uint64_t *wr);

/* Live per-device status and counters. */
void ts_stage_get(TsDevState out[TS_DEV_COUNT], int plugin_active);

#endif /* STORAGETURBO_STAGE_H */
