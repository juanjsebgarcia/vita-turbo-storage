/*
 * fg.h - what is in front, for "Pause in Vita games" (kernel/fg.c).
 */
#ifndef STORAGETURBO_FG_H
#define STORAGETURBO_FG_H

#include "../include/storageturbo.h"

void ts_fg_init(void);            /* resolve the lookups (module_start)        */
void ts_fg_enable(int on);        /* the "Pause in Vita games" setting         */

/* Display hook, in the context of the process presenting the frame: loads
 * and stores only; the tick classifies. */
void ts_fg_frame(void);

/* I/O hooks: 1 if this transfer must stay stock because a game is in front
 * or the calling process is a game. Cheap: a pid lookup in a small cache. */
int  ts_fg_io_hold(void);

/* 1 while the setting is on and a game was in front in the last moments. */
int  ts_fg_paused(void);

/* The app in front (TS_FG_*) and its title id (12 bytes, may be ""). */
void ts_fg_get(int *cls, char title[12]);

void ts_fg_tick(void);            /* worker, 1 Hz: classify, refresh, log       */

#endif /* STORAGETURBO_FG_H */
