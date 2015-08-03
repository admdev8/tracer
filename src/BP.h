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

#include "x86_disas.h"
#include "X86_register.h"
#include "datatypes.h"
#include "regex.h"
#include "lisp.h"

typedef struct _bp_address bp_address;
typedef struct _BPF BPF;
typedef struct _BPX BPX;
typedef struct _BPM BPM;
typedef struct _process process;
typedef struct _thread thread;
typedef struct _MemoryCache MemoryCache;

enum BP_type
{
    BP_type_BPM,
    BP_type_BPX,
    BP_type_BPF
};

typedef struct _BP
{
    enum BP_type t;
    Da* ins; // disassembled instruction at the place of INT3

    bp_address *a;
    union
    {
        BPM* bpm;
        BPX* bpx;
        BPF* bpf;
        void *p;
    } u;
    struct _BP *next;
} BP;

BP* create_BP (enum BP_type t, bp_address* a, void* p);
void BP_ToString (BP* b, strbuf* out);
void dump_BP (BP* b);
void BP_free(BP*);
void handle_BP(process *p, thread *t, int bp_no, CONTEXT *ctx, MemoryCache *mc);
void handle_Bx (process *p, thread *t, CONTEXT *ctx, MemoryCache *mc);

/* vim: set expandtab ts=4 sw=4 : */
