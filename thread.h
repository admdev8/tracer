/*
 *  _______                      
 * |__   __|                     
 *    | |_ __ __ _  ___ ___ _ __ 
 *    | | '__/ _` |/ __/ _ \ '__|
 *    | | | | (_| | (_|  __/ |   
 *    |_|_|  \__,_|\___\___|_|   
 *
 * Written by Dennis Yurichev <dennis(a)yurichev.com>, 2013
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/3.0/.
 *
 */

#pragma once

#include <windows.h>
#include "BPF.h"
#include "BPX.h"

struct BP_thread_specific_dynamic_info
{
    // BPF-related info
    enum BPF_state BPF_states; // for each DRx breakpoint. 0..3 - DR0-3
    REG* BPF_args;
    address ret_adr, SP_at_ret_adr; // sometimes present
    BYTE** BPF_buffers_at_start; // need for dump_args option
    unsigned BPF_buffers_at_start_cnt;
    unsigned tracing_CALLs_executed; // 0 is default, 1 after first CALL, etc
    
    // BPX-related info
    enum BPX_state BPX_states; // for each DRx breakpoint. 0..3 - DR0-3

    // BPX and BPF related info
    bool tracing;
};

struct thread
{
    DWORD TID;
    HANDLE THDL;
    address start;
    address TIB;

    struct BP_thread_specific_dynamic_info BP_dynamic_info[4];

    // for CPU emulator
    bool last_emulated_present;
    struct Da *last_emulated_ins;
    struct MemoryCache *last_emulated_MC;
    CONTEXT *last_emulated_ctx;
};

void thread_free (struct thread *t);
struct thread *find_thread (DWORD PID, DWORD TID);
void add_thread (struct process *p, DWORD TID, HANDLE THDL, address start, address TIB);
void dump_stack (struct process *p, struct thread *t, CONTEXT * ctx, struct MemoryCache *mem);

/* vim: set expandtab ts=4 sw=4 : */
