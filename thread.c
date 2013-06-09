#include <assert.h>

#include "thread.h"
#include "dmalloc.h"
#include "rbtree.h"
#include "logging.h"

#include "process.h"
#include "bolt_stuff.h"

bool thread_c_debug=false;

void thread_free (thread *t)
{
    if (thread_c_debug)
        L ("%s() begin\n", __func__);
    DFREE (t->BPF_args);
    if (t->BPF_buffers_at_start)
    {
        for (unsigned i=0; i<t->BPF_buffers_at_start_cnt; i++)
            DFREE(t->BPF_buffers_at_start[i]);
        DFREE(t->BPF_buffers_at_start);
    };
    DFREE (t);
};

thread *find_thread (DWORD PID, DWORD TID)
{
    process *p=find_process(PID);
    thread *t=(thread*)rbtree_lookup (p->threads, (void*)TID);
    assert (t!=NULL && "TID not found in threads table");
    return t;
};

void add_thread (process *p, DWORD TID, HANDLE THDL, address start)
{
    thread *t=DCALLOC (thread, 1, "thread");

    if (thread_c_debug)
        L ("%s() begin\n", __func__);

    t->TID=TID;
    t->THDL=THDL;
    t->start=start;
    assert (rbtree_lookup(p->threads, (void*)TID)==NULL && "this TID is already in table");
    rbtree_insert (p->threads, (void*)TID, t);
    
    if (thread_c_debug)
        L ("%s() end\n", __func__);
};

static void dump_stack_not_using_EBP (process *p, thread *t, CONTEXT *ctx, MemoryCache *mc)
{
    HANDLE THDL=t->THDL;
    address stack_top=TIB_get_stack_top (THDL, mc);
    address stack_bottom=TIB_get_stack_bottom (THDL, mc);
    //strbuf sb=STRBUF_INIT;
    address SP_at_start=CONTEXT_get_SP(ctx);

    L ("Call stack:\n");
 
    L ("SP_at_start=0x%x, stack_top=0x%x, stack_bottom=0x%x\n", SP_at_start, stack_top, stack_bottom);

    for (address a=SP_at_start; a<stack_top; a=a+sizeof(REG))
    {
        REG r;
        if (MC_ReadREG(mc, a, &r))
        {
            //L ("r=0x" PRI_REG_HEX "\n", r);
            if (adr_in_executable_section(p, r))
            {
                strbuf sb=STRBUF_INIT;
                process_get_sym (p, r, true, &sb);
                L ("(SP+0x%x) return address=0x" PRI_ADR_HEX " (%s)\n", a-SP_at_start, r, sb.buf);
                strbuf_deinit(&sb);
            };
        };
    };
};

static void dump_stack_EBP_frame (process *p, thread *t, CONTEXT * ctx, MemoryCache *mem)
{
    HANDLE THDL=t->THDL;
    address stack_top=TIB_get_stack_top (THDL, mem);
    address stack_bottom=TIB_get_stack_bottom (THDL, mem);
    strbuf sb=STRBUF_INIT;

    L ("Call stack:\n");

    address next_BP=CONTEXT_get_BP (ctx);

    if (TIB_is_ptr_in_stack_limits (THDL, next_BP, mem)==false)
    {
        L ("Current frame pointer %s (0x" PRI_ADR_HEX ") is not within current stack limits (top=0x" PRI_ADR_HEX ", bottom=0x" PRI_ADR_HEX ")\n", 
                get_BP_register_name(),
                CONTEXT_get_BP (ctx), stack_top, stack_bottom);
        goto exit;
    };

    while (next_BP<=stack_top && next_BP>=stack_bottom)
    {
        REG tmp;
        bool b=MC_ReadREG(mem, next_BP, &tmp);
        assert (b);
        address ret_adr;
        b=MC_ReadREG(mem, next_BP+sizeof(REG), &ret_adr);
        assert (b);

        if (ret_adr==0)
            break;

        strbuf_reinit(&sb, 0);
        process_get_sym (p, ret_adr, true, &sb);

        L ("return address=0x" PRI_ADR_HEX " (%s)\n", ret_adr, sb.buf);

        if (next_BP==tmp)
            break;
        
        next_BP=tmp;
    };

exit:
    strbuf_deinit (&sb);
};

void dump_stack (process *p, thread *t, CONTEXT * ctx, MemoryCache *mem)
{
#ifdef _WIN64
    dump_stack_not_using_EBP (p, t, ctx, mem);
#else
    dump_stack_EBP_frame (p, t, ctx, mem);
#endif
};

