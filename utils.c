#include <assert.h>

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

// temporary in this file!
void set_or_update_DRx_breakpoint(BP *bp, CONTEXT *ctx, unsigned DRx_no)
{
    assert (bp->a->resolved);
    if (utils_c_debug)
    {
        strbuf sb=STRBUF_INIT;
        address_to_string(bp->a, &sb);
        L ("%s(): begin. setting DRx-breakpoint %d for %s\n", __func__, DRx_no, sb.buf);
        strbuf_deinit (&sb);
    };

    if (bp->t==BP_type_BPF || bp->t==BP_type_BPX)
        CONTEXT_setDRx_and_DR7 (ctx, DRx_no, bp->a->abs_address);
    else if (bp->t==BP_type_BPM)
        BPM_set_or_update_DRx_breakpoint(bp->u.bpm, bp->a->abs_address, DRx_no, ctx);
    else
    {
        assert(0);
    };
    if (utils_c_debug)
        L ("%s() end\n", __func__);
};

void set_or_update_DRx_for_thread(thread *t, BP *bp, unsigned DRx_no)
{
    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_ALL;
    DWORD rt;
    rt=GetThreadContext (t->THDL, &ctx); assert (rt!=FALSE);      

    if (utils_c_debug)
        L ("%s() going to call set_or_update_DRx_breakpoint for TID %d\n", __func__, t->TID);
    set_or_update_DRx_breakpoint(bp, &ctx, DRx_no);

    rt=SetThreadContext (t->THDL, &ctx); assert (rt!=FALSE);
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

Da* MC_disas(address a, MemoryCache *mc)
{
    return Da_Da_callbacks (Fuzzy_Undefined, a, 
            (callback_read_byte)MC_ReadByte, 
            (callback_read_word)MC_ReadWyde, 
            (callback_read_dword)MC_ReadTetrabyte, 
            (callback_read_oword)MC_ReadOctabyte, 
            (void *)mc);
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

