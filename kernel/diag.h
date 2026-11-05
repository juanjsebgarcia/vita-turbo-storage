/*
 * diag.h - one-off boot diagnostics for bug reports (kernel/diag.c).
 */
#ifndef STORAGETURBO_DIAG_H
#define STORAGETURBO_DIAG_H

/* Console model codes, once at start. */
void ts_diag_console(void);

/* Kernel modules not from the system partitions. The first call lists every
 * one; later calls list only those loaded since. */
void ts_diag_modules(const char *when);

#endif /* STORAGETURBO_DIAG_H */
