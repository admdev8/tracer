#include <assert.h>

#include "BPF.h"
#include "BP.h"
#include "process.h"
#include "thread.h"
#include "opts.h"

static void BPF_dump_arg (MemoryCache *mc, REG arg)
{
    L ("0x" PRI_REG_HEX, arg);
};

static void BPF_dump_args (MemoryCache *mc, REG* args, unsigned args_n)
{
    for (unsigned i=0; i<args_n; i++)
    {
        BPF_dump_arg (mc, args[i]);
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
        load_args(t, CONTEXT_get_SP(ctx), mc, bpf->args);
        L ("(%d) %s (", DRx_no, sb_address.buf);
        BPF_dump_args (mc, t->BPF_args, bpf->args);
        L (")\n");
        // set current DRx to return
        address ret_adr;
        
        if (MC_ReadREG(mc, CONTEXT_get_SP(ctx), &ret_adr))
        {
            L ("ret_adr=0x" PRI_ADR_HEX "\n", ret_adr);
            CONTEXT_setDRx_and_DR7 (ctx, DRx_no, ret_adr);
            *cur_state=BPF_state_at_return;
        }
        else
            L ("Cannot read a register at SP, so, BPF return will not be handled\n");
    }
    else if (*cur_state==BPF_state_at_return)
    {
        // do function finish things
        L ("(%d) %s (...) -> ...\n", DRx_no, sb_address.buf);
        DFREE(t->BPF_args); t->BPF_args=NULL;
        // set back current DRx to begin
        CONTEXT_setDRx_and_DR7 (ctx, DRx_no, bp_a->abs_address);
        *cur_state=BPF_state_default;
    }
    else
    {
        assert(0);
    };

    strbuf_deinit(&sb_address);
};
