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

enum BPM_type
{
    BPM_type_W,
    BPM_type_RW
};

struct BPM
{
    unsigned width; // in bytes!
    enum BPM_type t;
};

struct BPM *create_BPM(unsigned width, enum BPM_type t);
void BPM_ToString(struct BPM *bpm, strbuf *out);
void BPM_free(struct BPM *);
void handle_BPM(struct process *p, struct thread *t, int bp_no, CONTEXT *ctx, struct MemoryCache *mc);
void BPM_set_or_update_DRx_breakpoint(struct BPM *b, address a, unsigned DRx_no, CONTEXT *ctx);

/* vim: set expandtab ts=4 sw=4 : */
