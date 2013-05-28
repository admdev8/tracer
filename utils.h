#pragma once

#include <windows.h>

typedef struct _process process;
typedef struct _thread thread;
typedef struct _BP BP;

void dump_PID_if_need(process *p);
void dump_TID_if_need(process *p, thread *t);
void set_or_update_DRx_breakpoint(BP *bp, CONTEXT *ctx, unsigned DRx_no);
void set_or_update_all_DRx_breakpoints(process *p);
