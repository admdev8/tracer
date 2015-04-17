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
#include "oassert.h"

#include "dmalloc.h"
#include "BP.h"
#include "strbuf.h"
#include "stuff.h"
#include "opts_aux.h"
#include "bp_address.h"
#include "BPF.h"
#include "BPX.h"
#include "BPM.h"
#include "x86.h"
#include "bitfields.h"
#include "thread.h"
#include "ostrings.h"
#include "fmt_utils.h"

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
            oassert(0);
            fatal_error();
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
            oassert(0);
            fatal_error();
    };
};

void dump_BP (BP* b)
{
    strbuf sb=STRBUF_INIT;
    BP_ToString(b, &sb);
    strbuf_puts (&sb);
    strbuf_deinit (&sb);
};

void handle_BP(process *p, thread *t, int bp_no, CONTEXT *ctx, MemoryCache *mc)
{
    if (cycle_c_debug)
        L ("%s(bp_no=%d) begin\n", __func__, bp_no);

    BP* bp=breakpoints[bp_no];

    switch (bp->t)
    {
        case BP_type_BPF:
            handle_BPF(p, t, bp_no, ctx, mc);
            break;
        case BP_type_BPX:
            handle_BPX(p, t, bp_no, ctx, mc);
            break;
        case BP_type_BPM:
            handle_BPM(p, t, bp_no, ctx, mc);
            break;
        default:
            oassert(0);
            fatal_error();
    };
    if (cycle_c_debug)
        L ("%s() end. TF=%s, PC=0x" PRI_ADR_HEX "\n", 
                __func__, bool_to_string(IS_SET(ctx->EFlags, FLAG_TF)), CONTEXT_get_PC(ctx));
};

void handle_Bx (process *p, thread *t, CONTEXT *ctx, MemoryCache *mc)
{
    if (cycle_c_debug)
        L ("%s() begin\n", __func__);

    bool bp_handled_in_SS_mode[4]={ false, false, false, false };

    if (IS_SET(ctx->Dr6, FLAG_DR6_BS) || ctx->Dr6==0) // DR6=0 sometimes observed while tracing
    {
        bool tracing_handled=false;

        for (unsigned b=0; b<4; b++)
            if (t->BP_dynamic_info[b].tracing)
            {
                handle_BP(p, t, b, ctx, mc);
                tracing_handled=bp_handled_in_SS_mode[b]=true;
            }

        if (tracing_handled==false && cycle_c_debug)
            L ("[!] BS flag in DR6 (or DR6 is zero), but no breakpoint in tracing mode\n");
    };

    if (IS_SET(ctx->Dr6, FLAG_DR6_B0) && bp_handled_in_SS_mode[0]==false)
    {
        oassert (breakpoints[0]);
        handle_BP(p, t, 0, ctx, mc);
    };

    if (IS_SET(ctx->Dr6, FLAG_DR6_B1) && bp_handled_in_SS_mode[0]==false)
    {
        oassert (breakpoints[1]);
        handle_BP(p, t, 1, ctx, mc);
    };

    if (IS_SET(ctx->Dr6, FLAG_DR6_B2) && bp_handled_in_SS_mode[0]==false)
    {
        oassert (breakpoints[2]);
        handle_BP(p, t, 2, ctx, mc);
    };

    if (IS_SET(ctx->Dr6, FLAG_DR6_B3) && bp_handled_in_SS_mode[0]==false)
    {
        oassert (breakpoints[3]);
        handle_BP(p, t, 3, ctx, mc);
    };
    
    clear_TF(ctx);
    for (unsigned b=0; b<4; b++)
        if (t->BP_dynamic_info[b].tracing)
        {
            if (cycle_c_debug)
                L ("%s() setting TF because bp #%d is in tracing mode\n", __func__, b);
            set_TF(ctx);
        };

    if (cycle_c_debug)
        L ("%s() end. TF=%s, PC=0x" PRI_ADR_HEX "\n", 
                __func__, bool_to_string(IS_SET(ctx->EFlags, FLAG_TF)), CONTEXT_get_PC(ctx));
};


/* vim: set expandtab ts=4 sw=4 : */
