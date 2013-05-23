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
    BPF_state BPF_states[4]; // for each DRx breakpoint
    REG* BPF_args;
} thread;

void thread_free (thread *t);
thread *find_thread (DWORD PID, DWORD TID);
void add_thread (process *p, DWORD TID, HANDLE THDL, address start);
void dump_stack_EBP_frame (process *p, thread *t, CONTEXT * ctx, MemoryCache *mem);
