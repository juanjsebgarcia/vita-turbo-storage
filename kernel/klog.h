/*
 * klog.h - kernel-side logger for the SceStorageTurbo kernel module.
 *
 * Primary sink is ksceDebugPrintf with a "[storageturbo]" tag (visible over a
 * kernel debug channel). In addition every line is appended to
 * ur0:data/turbostorage/kernel.log via ksceIo*, best-effort and
 * failure-tolerant. When the file passes TS_KLOG_MAX_BYTES it is renamed to
 * kernel.old.log and a fresh one is started, so an overnight run keeps about
 * twice that much history.
 *
 * Crash-safe mode (the default from ts_klog_init): the file is closed after
 * every line, so nothing is lost if the console goes down. main.c turns it off
 * once the boot is known good; warnings and errors are always written this way.
 *
 * HOT-PATH RULE: the storage I/O hooks (kernel/stage.c) must NOT call
 * these. They only bump counters; the 1 Hz worker (kernel/main.c) does the
 * logging.
 *
 * The self-contained formatter means no libc (%s %d %i %u %x %X %c %p %%,
 * optional zero-pad width).
 */
#ifndef STORAGETURBO_KLOG_H
#define STORAGETURBO_KLOG_H

#include <stdint.h>

#include "../include/storageturbo.h"

#define TS_KLOG_ERR    0
#define TS_KLOG_WARN   1
#define TS_KLOG_INFO   2
#define TS_KLOG_DEBUG  3
#define TS_KLOG_TRACE  4

#ifndef TS_KLOG_DEFAULT_LEVEL
#define TS_KLOG_DEFAULT_LEVEL  TS_KLOG_DEBUG
#endif

#define TS_KLOG_DIR       TS_DATA_DIR
#define TS_KLOG_PATH      TS_KLOG_DIR "/kernel.log"
#define TS_KLOG_PATH_OLD  TS_KLOG_DIR "/kernel.old.log"
#define TS_KLOG_MAX_BYTES (128 * 1024)

void ts_klog_init(void);
void ts_klog_fini(void);
int  ts_klog_sync(int on);   /* 1: close the file after every line; returns the previous setting */

void ts_klog(int level, const char *fmt, ...);

#define TS_KLOGE(...) ts_klog(TS_KLOG_ERR,   __VA_ARGS__)
#define TS_KLOGW(...) ts_klog(TS_KLOG_WARN,  __VA_ARGS__)
#define TS_KLOGI(...) ts_klog(TS_KLOG_INFO,  __VA_ARGS__)
#define TS_KLOGD(...) ts_klog(TS_KLOG_DEBUG, __VA_ARGS__)
#define TS_KLOGT(...) ts_klog(TS_KLOG_TRACE, __VA_ARGS__)

#endif /* STORAGETURBO_KLOG_H */
