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

void dump_PID_if_need(struct process *p);
void dump_TID_if_need(struct process *p, struct thread *t);
void set_or_update_DRx_for_thread(struct thread *t, struct BP *bp, unsigned DRx_no);
void set_or_update_DRx_breakpoint(struct BP *bp, CONTEXT *ctx, unsigned DRx_no);
void set_or_update_all_DRx_breakpoints(struct process *p);
bool MC_disas(address a, struct MemoryCache *mc, struct Da* out);
void dump_buf_as_array_of_strings(struct MemoryCache *mc, address a, size_t size);
bool read_REG_from_stack (struct MemoryCache *mc, CONTEXT *ctx, int idx, REG * out);
bool read_argument_from_stack (struct MemoryCache *mc, CONTEXT *ctx, unsigned arg, REG * out);
void print_symbol_if_possible (struct process *p, struct MemoryCache *mc, address a, char *name);
void print_symbols_in_buf_if_possible (struct process *p, struct MemoryCache *mc, byte *buf, size_t s, char *name);
void print_symbols_in_intersection_of_bufs (struct process *p, struct MemoryCache *mc, 
        byte *buf1, byte* buf2, char *buf1name, char *buf2name, size_t bufsize);

/* vim: set expandtab ts=4 sw=4 : */
