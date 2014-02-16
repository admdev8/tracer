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
#include "ostrings.h"
#include "oassert.h"
#include "datatypes.h"
#include "logging.h"
#include "utils.h"
#include "tracer.h"
#include "process.h"
#include "thread.h"
#include "BP.h"
#include "BPM.h"
#include "bp_address.h"
#include "opts.h"
#include "bitfields.h"
#include "x86.h"
#include "stuff.h"
#include "module.h"

void dump_PID_if_need(process *p)
{
   if (rbtree_count(processes)>1)
       L ("PID=%d|", p->PID);
};

void dump_TID_if_need(process *p, thread *t)
{
   if (rbtree_count(p->threads)>1)
       L ("TID=%d|", t->TID);
};

// FIXME: find another place for this function
void set_or_update_DRx_breakpoint(BP *bp, CONTEXT *ctx, unsigned DRx_no)
{
    oassert (bp->a->resolved);
    if (utils_c_debug)
    {
        strbuf sb=STRBUF_INIT;
        address_to_string(bp->a, &sb);
        L ("%s(): begin. setting DRx-breakpoint %d for %s at 0x" PRI_ADR_HEX "\n", 
                __func__, DRx_no, sb.buf, bp->a->abs_address);
        strbuf_deinit (&sb);
    };

    if (bp->t==BP_type_BPF || bp->t==BP_type_BPX)
        CONTEXT_setDRx_and_DR7 (ctx, DRx_no, bp->a->abs_address);
    else if (bp->t==BP_type_BPM)
        BPM_set_or_update_DRx_breakpoint(bp->u.bpm, bp->a->abs_address, DRx_no, ctx);
    else
    {
        oassert(0);
        fatal_error();
    };
    if (utils_c_debug)
        L ("%s() end\n", __func__);
};

void set_or_update_DRx_for_thread(thread *t, BP *bp, unsigned DRx_no)
{
    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_ALL;
    DWORD rt;
    rt=GetThreadContext (t->THDL, &ctx); oassert (rt!=FALSE);      

    if (utils_c_debug)
        L ("%s() going to call set_or_update_DRx_breakpoint for TID %d\n", __func__, t->TID);
    set_or_update_DRx_breakpoint(bp, &ctx, DRx_no);

    rt=SetThreadContext (t->THDL, &ctx); oassert (rt!=FALSE);
};

void set_or_update_all_DRx_breakpoints(process *p)
{
    if (utils_c_debug)
        L ("%s() begin\n", __func__);

    // enum all breakpoints, pick out a->resolved ones
    for (unsigned DRx_no=0; DRx_no<4; DRx_no++)
    {
        BP *bp=breakpoints[DRx_no];
        if (bp)
        {
            //L ("%s() DRx_breakpoints[%d]=0x%p\n", __func__, DRx_no, bp);
        }
        else
        {
            if (utils_c_debug)
                L ("%s() breakpoints[%d]==NULL\n", __func__, DRx_no);
            continue;
        };

        //dump_BP (bp);

        if (bp->a->resolved==false)
        {
            if (utils_c_debug)
                L ("%s() breakpoints[%d]->a->resolved==false\n", __func__, DRx_no);
            continue;
        };

        if (load_filename && p->we_are_loading_and_OEP_was_executed==false)
        {
            if (utils_c_debug)
                L ("%s() p->we_are_loading_and_OEP_was_executed==false\n", __func__);
            continue;
        };
        for (struct rbtree_node_t *_t=rbtree_minimum(p->threads); _t; _t=rbtree_succ(_t))
        {
            thread *t=(thread*)(_t->value);
            set_or_update_DRx_for_thread (t, bp, DRx_no);
        };
    };
    if (utils_c_debug)
        L ("%s() end\n", __func__);
};

bool MC_disas(address a, MemoryCache *mc, Da* out)
{
    return Da_Da_callbacks (Fuzzy_Undefined, a, 
            (callback_read_byte)MC_ReadByte, 
            (callback_read_word)MC_ReadWyde, 
            (callback_read_dword)MC_ReadTetrabyte, 
            (callback_read_oword)MC_ReadOctabyte, 
            (void *)mc, out);
};

void dump_buf_as_array_of_strings(MemoryCache *mc, address a, size_t size)
{
    strbuf sb=STRBUF_INIT;
    BYTE *buf=DMALLOC (BYTE, size, "BYTE*");
    if (MC_ReadBuffer (mc, a, size, buf)==false)
        goto exit;
    for (unsigned i=0; i<size; i+=sizeof(REG))
    {
        address a2=*(REG*)&buf[i];
        if (MC_get_any_string(mc, a2, &sb))
        {
            L ("0x" PRI_ADR_HEX "+0x%x: ptr to %s\n", a, i, sb.buf);
            strbuf_reinit(&sb, 0);
        };
    };
exit:
    DFREE (buf);
    strbuf_deinit(&sb);
};

// idx may be negative
bool read_REG_from_stack (MemoryCache *mc, CONTEXT *ctx, int idx, REG * out)
{
    address SP=CONTEXT_get_SP(ctx);
    return MC_ReadREG (mc, SP + idx*sizeof(REG), out);
};

bool read_argument_from_stack (MemoryCache *mc, CONTEXT *ctx, unsigned arg, REG * out)
{
    return read_REG_from_stack (mc, ctx, arg+1, out);
};

void print_symbol_if_possible (process *p, MemoryCache *mc, address a, char *name)
{
    module *m=find_module_for_address (p, a);
    if (m==NULL)
        return;

    strbuf sb=STRBUF_INIT;
    module_get_sym (m, a, true /*add_module_name*/, true /*add_offset*/, &sb);
    L ("%s: %s\n", name, sb.buf);
    strbuf_deinit (&sb);
};

void print_symbols_in_buf_if_possible (process *p, MemoryCache *mc, byte *buf, size_t s, char *name)
{
    for (size_t i=0; i*sizeof(address)<s; i++)
    {
        strbuf tmp=STRBUF_INIT;
        strbuf_addf(&tmp, "%s+0x%X", name, i*sizeof(address));
        print_symbol_if_possible (p, mc, ((REG*)buf)[i], tmp.buf);
        strbuf_deinit (&tmp);
    };
};

void print_symbols_in_intersection_of_bufs (process *p, MemoryCache *mc, 
        byte *buf1, byte* buf2, char *buf1name, char *buf2name, size_t bufsize)
{
    for (size_t i=0; (i*sizeof(address))<bufsize; i++)
    {
        REG val_in_buf1=((REG*)buf1)[i];
        REG val_in_buf2=((REG*)buf2)[i];
        if (val_in_buf1==val_in_buf2)
            continue;
        strbuf tmp1=STRBUF_INIT, tmp2=STRBUF_INIT;
        strbuf_addf(&tmp1, "%s+0x%X", buf1name, i*sizeof(address));
        strbuf_addf(&tmp2, "%s+0x%X", buf2name, i*sizeof(address));
        print_symbol_if_possible (p, mc, val_in_buf1, tmp1.buf);
        print_symbol_if_possible (p, mc, val_in_buf2, tmp2.buf);
        strbuf_deinit (&tmp1);
        strbuf_deinit (&tmp2);
    };
};

/* vim: set expandtab ts=4 sw=4 : */
