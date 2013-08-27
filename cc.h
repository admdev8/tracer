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
#include "memorycache.h"
#include "x86_disas.h"
#include "lisp.h"

typedef struct _process process;
typedef struct _thread thread;
typedef struct _module module;
typedef struct _Da Da;

// order matters!
#define WORKOUT_OP1 0
#define WORKOUT_OP2 1
#define WORKOUT_OP3 2
#define WORKOUT_AX 3
#define WORKOUT_CX 4
#define WORKOUT_DX 5
#define WORKOUT_ST0 6
#define WORKOUT_PF 7
#define WORKOUT_SF 8
#define WORKOUT_AF 9
#define WORKOUT_ZF 10
#define WORKOUT_OF 11
#define WORKOUT_CF 12

#define NOTICE_OP1 1<<WORKOUT_OP1
#define NOTICE_OP2 1<<WORKOUT_OP2
#define NOTICE_OP3 1<<WORKOUT_OP3
#define NOTICE_AX 1<<WORKOUT_AX
#define NOTICE_CX 1<<WORKOUT_CX
#define NOTICE_DX 1<<WORKOUT_DX
#define NOTICE_ST0 1<<WORKOUT_ST0
#define NOTICE_PF 1<<WORKOUT_PF
#define NOTICE_SF 1<<WORKOUT_SF
#define NOTICE_AF 1<<WORKOUT_AF
#define NOTICE_ZF 1<<WORKOUT_ZF
#define NOTICE_OF 1<<WORKOUT_OF
#define NOTICE_CF 1<<WORKOUT_CF

// FIXME: lisp object could be here!
typedef struct _op_info
{
    // value is unused in both trees, so these are kind of sets

    rbtree *values; // key may be REG or 8-byte double. 

    rbtree *FPU_values; // in double (64-bit) form

    // set of strings "can be ptr to ASCII string '<...>'"
    rbtree *ptr_to_string_set;

    // ? set of (other comment) strings
    // ? name of op
} op_info;

#define FLAG_PF_CAN_BE_TRUE 1<<0
#define FLAG_SF_CAN_BE_TRUE 1<<1
#define FLAG_AF_CAN_BE_TRUE 1<<2
#define FLAG_ZF_CAN_BE_TRUE 1<<3
#define FLAG_OF_CAN_BE_TRUE 1<<4
#define FLAG_CF_CAN_BE_TRUE 1<<5
#define FLAG_PF_CAN_BE_FALSE 1<<6
#define FLAG_SF_CAN_BE_FALSE 1<<7
#define FLAG_AF_CAN_BE_FALSE 1<<8
#define FLAG_ZF_CAN_BE_FALSE 1<<9
#define FLAG_OF_CAN_BE_FALSE 1<<10
#define FLAG_CF_CAN_BE_FALSE 1<<11

typedef struct _PC_info
{
    Da *da;
    op_info *op[7]; // OP1/2/3/AX/CX/DX/ST0
    enum obj_type op_t[7]; // type for OP1/2/3/AX/CX/DX/ST0. use this from lisp.h
    octabyte executed; // how many times we've been here?
    char *comment; // (one) comment about this PC
    wyde flags; // FLAG_xF_CAN_BE_(TRUE|FALSE)
} PC_info;

void cc_dump_and_free(module *m); // for module m
void handle_cc(Da* da, process *p, thread *t, CONTEXT *ctx, MemoryCache *mc, 
        bool CALL_to_be_skipped_due_to_module, bool CALL_to_be_skipped_due_to_trace_limit);

// can be called from cc_tests.c
void construct_common_string(strbuf *out, address a, PC_info *info);
void free_PC_info (PC_info *i);

/* vim: set expandtab ts=4 sw=4 : */
