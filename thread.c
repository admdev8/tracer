#include <assert.h>

#include "thread.h"
#include "dmalloc.h"
#include "rbtree.h"
#include "logging.h"

#include "process.h"
#include "bolt_stuff.h"

bool thread_c_debug=true;

void thread_free (thread *t)
{
    if (thread_c_debug)
        L ("%s() begin\n", __func__);
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

void dump_stack_EBP_frame (process *p, thread *t, CONTEXT * ctx, MemoryCache *mem)
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
#ifdef _WIN64
                "RBP",
#else
                "EBP",
#endif
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
        process_get_sym (p, ret_adr, &sb);

        L ("return address=0x" PRI_ADR_HEX " (%s)\n", ret_adr, sb.buf);

        if (next_BP==tmp)
            break;
        
        next_BP=tmp;
    };

exit:
    strbuf_deinit (&sb);
};

