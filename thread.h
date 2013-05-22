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
    BPF_state BPF_states[4]; // for each DRx breakpoint
} thread;

void thread_free (thread *t);
thread *find_thread (DWORD PID, DWORD TID);
void add_thread (process *p, DWORD TID, HANDLE THDL, address start);

