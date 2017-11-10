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
#include "CONTEXT_utils.h"
#include "memorycache.h"
#include "x86_register.h"

typedef struct _BP BP;
typedef struct _process process;
typedef struct _thread thread;

enum BPX_option_type
{
    BPX_option_DUMP,
    BPX_option_SET,
    BPX_option_COPY
};

#define BPX_DUMP_DEFAULT 0x10

typedef struct _BPX_option
{
    enum BPX_option_type t;

    bp_address *a; // if NULL, see reg
    enum X86_register reg;

    // in case of DUMP or SET
    REG size_or_value;
    double float_value; // in case if register is FPU register
    
    // in case of COPY
    byte *copy_string; // may be NULL if absent
    unsigned copy_string_len;
    struct _BPX_option *next;
} BPX_option;

typedef struct _BPX
{
    BPX_option* opts; // may be NULL if options absent
} BPX;

typedef enum _BPX_state
{
    BPX_state_default=0,
    BPX_state_skipping_first_instruction    
} BPX_state;

void BPX_option_free(BPX_option *);
void BPX_ToString(BPX *b, strbuf *out);
void BPX_option_ToString(BPX_option *b, strbuf *out);
void BPX_free(BPX *);
BPX* create_BPX(BPX_option *opts);

void handle_BPX(process *p, thread *t, int DRx_no, CONTEXT *ctx, MemoryCache *mc);

/* vim: set expandtab ts=4 sw=4 : */
