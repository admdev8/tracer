#pragma once

#include <windows.h>
#include "address.h"
#include "BPF.h"
#include "BPX.h"

typedef struct _process process;

typedef struct _BP_thread_specific_dynamic_info
{
    // BPF-related info
    BPF_state BPF_states; // for each DRx breakpoint. 0..3 - DR0-3
    REG* BPF_args;
    address ret_adr; // sometimes present
    BYTE** BPF_buffers_at_start; // need for dump_args option
    unsigned BPF_buffers_at_start_cnt;
    unsigned tracing_CALLs_executed; // 0 is default, 1 after first CALL, etc
    
    // BPX-related info
    BPX_state BPX_states; // for each DRx breakpoint. 0..3 - DR0-3

    // BPX and BPF related info
    bool tracing;

} BP_thread_specific_dynamic_info;

typedef struct _thread
{
    DWORD TID;
    HANDLE THDL;
    address start;

    BP_thread_specific_dynamic_info BP_dynamic_info[4];
} thread;

void thread_free (thread *t);
thread *find_thread (DWORD PID, DWORD TID);
void add_thread (process *p, DWORD TID, HANDLE THDL, address start);
void dump_stack (process *p, thread *t, CONTEXT * ctx, MemoryCache *mem);
