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

#define NOTICE_OP1 1<<WORKOUT_OP1
#define NOTICE_OP2 1<<WORKOUT_OP2
#define NOTICE_OP3 1<<WORKOUT_OP3
#define NOTICE_AX 1<<WORKOUT_AX
#define NOTICE_CX 1<<WORKOUT_CX
#define NOTICE_DX 1<<WORKOUT_DX
#define NOTICE_PF 1<<6
#define NOTICE_SF 1<<7
#define NOTICE_AF 1<<8
#define NOTICE_ZF 1<<9
#define NOTICE_OF 1<<10
#define NOTICE_CF 1<<11

typedef struct _op_info
{
    // value is unused in both trees, so these are kind of sets

    rbtree *values; // key may be REG or 8-byte double. 
    
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
    op_info *op[6]; // OP1/2/3/AX/CX/DX
    enum value_t op_t[6]; // type for OP1/2/3/AX/CX/DX
    octabyte executed; // how many times we've been here?
    char *comment; // (one) comment about this PC
    wyde flags; // FLAG_xF_CAN_BE_xxxx
} PC_info;

void cc_dump_and_free(module *m); // for module m
void handle_cc(Da* da, process *p, thread *t, CONTEXT *ctx, MemoryCache *mc, bool CALL_to_be_skipped);

