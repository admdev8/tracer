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

#include "x86_disas.h"
#include "x86_register.h"
#include "datatypes.h"
#include "regex.h"
#include "lisp.h"

enum BP_type
{
    BP_type_BPM,
    BP_type_BPX,
    BP_type_BPF
};

struct BP
{
    enum BP_type t;
    struct Da* ins; // disassembled instruction at the place of INT3

    struct bp_address *a;
    union
    {
        struct BPM* bpm;
        struct BPX* bpx;
        struct BPF* bpf;
        void *p;
    } u;
    struct BP *next;
};

struct BP* create_BP (enum BP_type t, struct bp_address* a, void* p);
void BP_ToString (struct BP* b, strbuf* out);
void dump_BP (struct BP* b);
void BP_free(struct BP*);
void handle_BP (struct process *p, struct thread *t, int bp_no, CONTEXT *ctx, struct MemoryCache *mc);
void handle_Bx (struct process *p, struct thread *t, CONTEXT *ctx, struct MemoryCache *mc);

/* vim: set expandtab ts=4 sw=4 : */
