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
#include "datatypes.h"
#include "rbtree.h"

typedef struct _process process;
typedef struct _thread thread;
typedef struct _BP BP;
typedef struct _MemoryCache MemoryCache;
typedef struct _Da Da;

void dump_PID_if_need(process *p);
void dump_TID_if_need(process *p, thread *t);
void set_or_update_DRx_for_thread(thread *t, BP *bp, unsigned DRx_no);
void set_or_update_DRx_breakpoint(BP *bp, CONTEXT *ctx, unsigned DRx_no);
void set_or_update_all_DRx_breakpoints(process *p);
Da* MC_disas(address a, MemoryCache *mc);
void dump_buf_as_array_of_strings(MemoryCache *mc, address a, size_t size);
