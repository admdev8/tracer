#include <assert.h>

#include "BPF.h"
#include "bp_address.h"
#include "dmalloc.h"
#include "BP.h"
#include "process.h"
#include "thread.h"
#include "utils.h"
#include "opts.h"

void dump_BPF(BPF *b)
{
    printf ("BPF. options: ");
    if (b->unicode)
        printf ("unicode ");
    if (b->skip)
        printf ("skip ");
    if (b->skip_stdcall)
        printf ("skip_stdcall ");
    if (b->trace)
        printf ("trace ");
    if (b->trace_cc)
        printf ("trace_cc ");
    if (b->rt)
    {
        printf ("rt: ");
        obj_dump (b->rt);
        printf (" ");
    };
    
    if (b->rt_probability!=1)
        printf ("rt_probability: %f ", b->rt_probability);
    
    if (b->args)
        printf ("args: %d ", b->args);
    
    if (b->dump_args)
        printf ("dump_args: %d ", b->dump_args);
    
    if (b->pause)
        printf ("pause: %d ", b->pause);

    printf ("\n");
    if (b->when_called_from_address)
    {
        printf ("when_called_from_address: ");
        dump_address (b->when_called_from_address);
        printf ("\n");
    };
    if (b->when_called_from_func)
    {
        printf ("when_called_from_func: ");
        dump_address (b->when_called_from_func);
        printf ("\n");
    };
};

void BPF_free(BPF* o)
{
    if (o->rt)
        obj_free (o->rt);
    if (o->when_called_from_address)
        bp_address_free(o->when_called_from_address);
    if (o->when_called_from_func)
        bp_address_free(o->when_called_from_func);
    DFREE (o);
};

static void BPF_dump_arg (MemoryCache *mc, REG arg, bool unicode)
{
    strbuf sb=STRBUF_INIT;

    if (MC_GetString(mc, arg, unicode, &sb))
        L ("\"%s\"", sb.buf);
    else
        L ("0x" PRI_REG_HEX, arg);

    strbuf_deinit (&sb);
};

static void BPF_dump_args (MemoryCache *mc, REG* args, unsigned args_n, bool unicode)
{
    for (unsigned i=0; i<args_n; i++)
    {
        BPF_dump_arg (mc, args[i], unicode);
        if ((i+1) != args_n)
            L (", ");
    };
};
    
static void load_args(thread *t, address SP, MemoryCache *mc, unsigned args)
{
    t->BPF_args=DMALLOC(REG, args, "REG");
    if (MC_ReadBuffer(mc, SP+REG_SIZE, args*REG_SIZE, (BYTE*)t->BPF_args))
        return;

    DFREE (t->BPF_args);
    t->BPF_args=NULL;
};

void handle_BPF(process *p, thread *t, int DRx_no /* -1 for OEP */, CONTEXT *ctx, MemoryCache *mc)
{
    BP *bp;
    
    if (DRx_no==-1)
        bp=OEP_breakpoint;
    else
        bp=DRx_breakpoints[DRx_no];

    BPF *bpf=bp->u.bpf;
    bp_address *bp_a=bp->a;
    BPF_state* cur_state=&t->BPF_states[DRx_no];
    strbuf sb_address=STRBUF_INIT;

    address_to_string(bp_a, &sb_address);
    
    if (*cur_state==BPF_state_default)
    {
        // do function begin things
        address ret_adr;
        bool got_ret_adr=false;
        strbuf sb=STRBUF_INIT;

        if (MC_ReadREG(mc, CONTEXT_get_SP(ctx), &ret_adr))
        {
            L ("ret_adr=0x" PRI_ADR_HEX "\n", ret_adr);
            // set current DRx to return
            CONTEXT_setDRx_and_DR7 (ctx, DRx_no, ret_adr);
            *cur_state=BPF_state_at_return;
            process_get_sym (p, ret_adr, &sb);
            got_ret_adr=true;
        }
        else
            L ("Cannot read a register at SP, so, BPF return will not be handled\n");

        load_args(t, CONTEXT_get_SP(ctx), mc, bpf->args);
        
        dump_PID_if_need(p); dump_TID_if_need(p, t);
        L ("(%d) %s (", DRx_no, sb_address.buf);
        BPF_dump_args (mc, t->BPF_args, bpf->args, bpf->unicode);
        L (")");
        if (got_ret_adr)
            L (" (called from %s (0x" PRI_ADR_HEX "))", sb.buf, ret_adr);
        L ("\n");
        
        strbuf_deinit(&sb);

        if (dash_s)
            dump_stack_EBP_frame (p, t, ctx, mc);

    }
    else if (*cur_state==BPF_state_at_return)
    {
        // do function finish things
        REG accum=CONTEXT_get_Accum (ctx);

        dump_PID_if_need(p); dump_TID_if_need(p, t);
        L ("(%d) %s () -> " PRI_SIZE_T_DEC " (0x" PRI_REG_HEX ")\n", DRx_no, sb_address.buf, accum, accum);
        
        DFREE(t->BPF_args); t->BPF_args=NULL;
        // set back current DRx to begin
        CONTEXT_setDRx_and_DR7 (ctx, DRx_no, bp_a->abs_address);
        *cur_state=BPF_state_default;
    }
    else
    {
        assert(!"invalid *cur_state");
    };

    strbuf_deinit(&sb_address);
};
