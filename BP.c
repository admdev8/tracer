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

#include <stdbool.h>
#include <assert.h>

#include "dmalloc.h"
#include "BP.h"
#include "strbuf.h"
#include "stuff.h"
#include "opts.h"
#include "bp_address.h"
#include "BPF.h"
#include "BPX.h"
#include "BPM.h"

void BP_free(BP* b)
{
    if (b==NULL)
        return; // free(NULL) behaviour
    bp_address_free(b->a);
    switch (b->t)
    {
        case BP_type_BPM:
            BPM_free(b->u.bpm);
            break;
        case BP_type_BPX:
            BPX_free(b->u.bpx);
            break;
        case BP_type_BPF:
            BPF_free(b->u.bpf);
            break;
        default:
            assert(0);
    };
    DFREE(b->ins);
    DFREE(b);
};

BP* create_BP (enum BP_type t, bp_address* a, void* p)
{
    BP* rt=DCALLOC(BP, 1, "BP");
    rt->t=t;
    rt->a=a;
    rt->u.p=p;
    
    return rt;
};

void BP_ToString (BP* b, strbuf* out)
{
    if (b->a)
    {
        strbuf_addstr (out, "bp_address=");
        address_to_string (b->a, out);
        strbuf_addstr (out, ". ");
    };

    switch (b->t)
    {
        case BP_type_BPM:
            BPM_ToString(b->u.bpm, out);
            break;
        case BP_type_BPX:
            BPX_ToString(b->u.bpx, out);
            break;
        case BP_type_BPF:
            BPF_ToString(b->u.bpf, out);
            break;
        default:
            assert(0);
    };
};

void dump_BP (BP* b)
{
    strbuf sb=STRBUF_INIT;
    BP_ToString(b, &sb);
    strbuf_puts (&sb);
    strbuf_deinit (&sb);
};

/* vim: set expandtab ts=4 sw=4 : */
