#include <assert.h>

#include "BPF.h"
#include "BP.h"
#include "process.h"
#include "thread.h"

void handle_BPF(BP *bp, process *p, thread *t, int DRx_no /* -1 for OEP */, CONTEXT *ctx, MemoryCache *mc)
{
    //BPF *bpf=bp->u.bpf;
    bp_address *bp_a=bp->a;
    BPF_state* cur_state=&t->BPF_states[DRx_no];
    strbuf sb_address=STRBUF_INIT;

    address_to_string(bp_a, &sb_address);
    
    if (*cur_state==BPF_state_default)
    {
        // do function begin things
        L ("(%d) %s (...)\n", DRx_no, sb_address.buf);
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
