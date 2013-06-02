#pragma once

#include <windows.h>
#include "address.h"
#include "BPF.h"

typedef struct _process process;

typedef struct _thread
{
    DWORD TID;
    HANDLE THDL;
    address start;

    // BPF-related info
    BPF_state BPF_states[5]; // for each DRx breakpoint. 0..3 - DR0-3, 4th - OEP bp
    REG* BPF_args;
    address ret_adr; // sometimes present
    BYTE** BPF_buffers_at_start; // need for dump_args option
    unsigned BPF_buffers_at_start_cnt;

    bool tracing;
    unsigned tracing_bp; // in case of tracing
    //int tracing_CALLs_executed; // 0 is default, 1 after first CALL, etc
} thread;

void thread_free (thread *t);
thread *find_thread (DWORD PID, DWORD TID);
void add_thread (process *p, DWORD TID, HANDLE THDL, address start);
void dump_stack_EBP_frame (process *p, thread *t, CONTEXT * ctx, MemoryCache *mem);
