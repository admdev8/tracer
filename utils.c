#include <assert.h>

#include "logging.h"
#include "utils.h"
#include "tracer.h"
#include "process.h"
#include "thread.h"
#include "BP.h"
#include "bp_address.h"
#include "opts.h"
#include "bitfields.h"
#include "x86.h"

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

// temporary here!
void set_or_update_DRx_breakpoint(BP *bp, CONTEXT *ctx, unsigned DRx_no)
{
    //if (tracer_c_debug)
    {
        strbuf sb=STRBUF_INIT;
        address_to_string(bp->a, &sb);
        L ("%s(): setting DRx-breakpoint %d for %s\n", __func__, DRx_no, sb.buf);
        strbuf_deinit (&sb);
    };

    CONTEXT_setDRx_and_DR7 (ctx, DRx_no, bp->a->abs_address);
};

void set_or_update_all_DRx_breakpoints(process *p)
{
    //if (tracer_c_debug)
        L ("%s() begin\n", __func__);

    // enum all breakpoints, pick out a->resolved ones
    for (unsigned DRx_no=0; DRx_no<4; DRx_no++)
    {
        BP *bp=breakpoints[DRx_no];
        if (bp)
            L ("%s() DRx_breakpoints[%d]=0x%p\n", __func__, DRx_no, bp);
        else
            continue;

        //dump_BP (bp);

        if (bp->a->resolved==false)
            continue;

        if (p->we_are_loading_and_OEP_was_executed==false)
            continue;
        for (struct rbtree_node_t *_t=rbtree_minimum(p->threads); _t; _t=rbtree_succ(_t))
        {
            thread *t=(thread*)(_t->value);
            CONTEXT ctx;
            ctx.ContextFlags = CONTEXT_ALL;
            DWORD rt;
            rt=GetThreadContext (t->THDL, &ctx); assert (rt!=FALSE);      

            set_or_update_DRx_breakpoint(bp, &ctx, DRx_no);

            rt=SetThreadContext (t->THDL, &ctx); assert (rt!=FALSE);
        };
    };
};
