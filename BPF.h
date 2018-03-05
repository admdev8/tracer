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

#include <stdbool.h>

#include "fuzzybool.h"
#include "CONTEXT_utils.h"
#include "memorycache.h"
#include "lisp.h"

enum BPF_state
{
    BPF_state_default=0,
    BPF_state_at_return,
    BPF_state_tracing_inside_function,
    BPF_state_tracing_skipping
};

enum function_type
{
    TY_UNKNOWN=0,
    TY_VOID,
    TY_UNINTERESTING,
    TY_REG,
    TY_INT,
    TY_PTR,
    TY_TETRABYTE,
    TY_PTR_TO_DOUBLE,
    TY_QSTRING,
    TY_PTR_TO_QSTRING
};

struct BPF
{
    bool unicode, microsoft_fastcall, borland_fastcall, skip, skip_stdcall, trace, cc;
    // these params may be NULL
    bool rt_present;
    REG rt;
    double rt_probability;
    bool rt_probability_present;
    unsigned args, dump_args, pause /* milliseconds */;
    struct bp_address *when_called_from_address, *when_called_from_func;
    // variables filled by is_it_known_function
    TrueFalseUndefined known_function;
    enum function_type ret_type, this_type;
    enum function_type *arg_types;
    // variables filled if when_called_from_func
    bool when_called_from_func_next_func_adr_present;
    address when_called_from_func_next_func_adr;
    // ... if SET option is present
    bool set_present;
    unsigned set_width;
    unsigned set_arg_n;
    REG set_ofs, set_val;
};

void BPF_free(struct BPF*);
void BPF_ToString(struct BPF *b, strbuf *out);
void handle_BPF(struct process *p, struct thread *t, int DRx_no, CONTEXT *ctx, struct MemoryCache *mc);
void handle_BPF_INT3(struct process *p, struct thread *t, int DRx_no, CONTEXT *ctx, struct MemoryCache *mc);

/* vim: set expandtab ts=4 sw=4 : */
