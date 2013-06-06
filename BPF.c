#include <assert.h>

#include "BPF.h"
#include "bp_address.h"
#include "dmalloc.h"
#include "BP.h"
#include "process.h"
#include "thread.h"
#include "utils.h"
#include "opts.h"
#include "CONTEXT_utils.h"
#include "bitfields.h"
#include "x86.h"
#include "symbol.h"
#include "cc.h"
#include "stuff.h"

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
    if (b->cc)
        printf ("cc ");
    printf ("rt: " PRI_REG_HEX " ", b->rt);

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

static void load_args(thread *t, CONTEXT *ctx, MemoryCache *mc, unsigned args)
{
    address SP=CONTEXT_get_SP(ctx);
    t->BPF_args=DMALLOC(REG, args, "REG");

#ifdef _WIN64
    for (unsigned a=0; a<args; a++)
    {
        switch (a)
        {
            case 0: t->BPF_args[a]=ctx->Rcx;
                    break;
            case 1: t->BPF_args[a]=ctx->Rdx;
                    break;
            case 2: t->BPF_args[a]=ctx->R8;
                    break;
            case 3: t->BPF_args[a]=ctx->R9;
                    break;
            default:
                    if (MC_ReadOctabyte (mc, SP+(a+1+4)*sizeof(REG), &t->BPF_args[a])==false)
                        goto read_failed;
                    break;
        };
    };
#else
    if (MC_ReadBuffer(mc, SP+REG_SIZE, args*REG_SIZE, (BYTE*)t->BPF_args)==false)
        goto read_failed;
#endif
    return;

read_failed:
    DFREE (t->BPF_args);
    t->BPF_args=NULL;
};

static void dump_args_if_need(thread *t, MemoryCache *mc, unsigned size, unsigned args_n, REG* args)
{
    assert (t->BPF_buffers_at_start==NULL);
    t->BPF_buffers_at_start=(REG**)DCALLOC(REG*, args_n, "REG*");
    t->BPF_buffers_at_start_cnt=args_n;

    for (unsigned i=0; i<args_n; i++)
    {
        BYTE *buf=DMALLOC(BYTE, size, "buf");
        if (MC_ReadBuffer (mc, args[i], size, buf))
        {
            L ("Argument %d/%d\n", i+1, args_n);
            L_print_buf_ofs (buf, size, args[i]);
            t->BPF_buffers_at_start[i]=buf;
        }
        else
            DFREE(buf);
    };
};

static void dump_args_diff_if_need(thread *t, MemoryCache *mc, unsigned size, unsigned args_n, REG* args)
{
    assert (t->BPF_buffers_at_start);

    for (unsigned i=0; i<args_n; i++)
    {
        if (t->BPF_buffers_at_start[i]==NULL)
            continue;

        BYTE *buf2=DMALLOC(BYTE, size, "buf");
        bool b=MC_ReadBuffer (mc, args[i], size, buf2);
        assert (b);
        if (memcmp (t->BPF_buffers_at_start[i], buf2, size)!=0)
        {
            L ("Argument %d/%d difference\n", i+1, args_n);
            L_print_bufs_diff (t->BPF_buffers_at_start[i], buf2, size);
        };
        DFREE(buf2);
        DFREE(t->BPF_buffers_at_start[i]);
    };
    DFREE (t->BPF_buffers_at_start);
    t->BPF_buffers_at_start=NULL;
};

static bool handle_begin(process *p, thread *t, BP *bp, int bp_no, CONTEXT *ctx, MemoryCache *mc)
{
    // do function begin things
    BPF *bpf=bp->u.bpf;
    bp_address *bp_a=bp->a;
    strbuf sb=STRBUF_INIT, sb_address=STRBUF_INIT;
    address_to_string(bp_a, &sb_address);
    bool got_ret_adr=MC_ReadREG(mc, CONTEXT_get_SP(ctx), &t->ret_adr);
    bool rt;

    if (got_ret_adr)
        process_get_sym (p, t->ret_adr, true, &sb);
    else    
        L ("Cannot read a register at SP, so, BPF return will not be handled\n");

    load_args(t, ctx, mc, bpf->args);

    dump_PID_if_need(p); dump_TID_if_need(p, t);
    L ("(%d) %s (", bp_no, sb_address.buf);
    BPF_dump_args (mc, t->BPF_args, bpf->args, bpf->unicode);
    L (")");
    if (got_ret_adr)
        L (" (called from %s (0x" PRI_ADR_HEX "))", sb.buf, t->ret_adr);
    L ("\n");

    if (bpf->dump_args)
        dump_args_if_need(t, mc, bpf->dump_args, bpf->args, t->BPF_args);

    if (dash_s)
        dump_stack (p, t, ctx, mc);

    if (got_ret_adr)
    {
        if (bpf->skip)
        {
            L ("(%d) Skipping execution of this function\n", bp_no);
            CONTEXT_set_PC(ctx, t->ret_adr);
            CONTEXT_set_SP(ctx, CONTEXT_get_SP(ctx)+sizeof(REG));
            rt=false;
        }
        else
        {
            // set current DRx to return
            CONTEXT_setDRx_and_DR7 (ctx, bp_no, t->ret_adr); // set breakpoint at return
            rt=true;
        }
    }
    else
        rt=false;

    strbuf_deinit(&sb);
    strbuf_deinit(&sb_address);
    return rt;
};

static void handle_finish(process *p, thread *t, BP *bp, int bp_no, CONTEXT *ctx, MemoryCache *mc)
{
    bp_address *bp_a=bp->a;
    BPF *bpf=bp->u.bpf;
    strbuf sb_address=STRBUF_INIT;
    address_to_string(bp_a, &sb_address);
    // do function finish things
    REG accum=CONTEXT_get_Accum (ctx);

    dump_PID_if_need(p); dump_TID_if_need(p, t);
    L ("(%d) %s () -> " PRI_SIZE_T_DEC " (0x" PRI_REG_HEX ")\n", bp_no, sb_address.buf, accum, accum);

    if (bpf->dump_args)
        dump_args_diff_if_need(t, mc, bpf->dump_args, bpf->args, t->BPF_args);

    DFREE(t->BPF_args); t->BPF_args=NULL;

    if (bpf->rt_probability!=0.0 && bpf->rt_probability!=1.0)
    {
        assert (!"not implemented");
    };

    dump_PID_if_need(p); dump_TID_if_need(p, t);
    if (bpf->rt_probability==1.0)
    {
        L ("(%d) Modifying %s register to 0x%x\n", bp_no, get_AX_register_name(), bpf->rt);
        CONTEXT_set_Accum(ctx, bpf->rt);
    };

    // set back current DRx to begin
    assert(bp_no<4); // be sure we work only with DRx breakpoints
    CONTEXT_setDRx_and_DR7 (ctx, bp_no, bp_a->abs_address);
    strbuf_deinit(&sb_address);
};

bool tracing_dbg=false;

static int handle_tracing(int bp_no, process *p, thread *t, CONTEXT *ctx, MemoryCache *mc)
{
    bool emulated=false;
    address PC;
    BP *bp=breakpoints[bp_no];
    BPF *bpf=bp->u.bpf;

    if (tracing_dbg)
    {
        PC=CONTEXT_get_PC(ctx);
        strbuf sb=STRBUF_INIT;
        process_get_sym(p, PC, true, &sb);

        L ("%s() PC=%s (0x%x)\n", __func__, sb.buf, PC);
        strbuf_deinit (&sb);
    };

    do // emu cycle
    {
        PC=CONTEXT_get_PC(ctx);
        // (to be implemented in future): check other BPF/BPX breakpoints. handle them if need.

        // finished? PC==ret_adr? return 2
        if (PC==t->ret_adr)
            return 2;

        // need to skip something? are we at the start of function to skip? is SYSCALL here? depth-level reached?
        // set drx, return 3
        DWORD tmp;
        if (MC_ReadTetrabyte (mc, PC, &tmp) && tmp==0xC015FF64) // 64 FF 15 C0 00 00 00 <call large dword ptr fs:0C0h>
        {
            L ("syscall to be skipped\n");
            CONTEXT_setDRx_and_DR7 (ctx, bp_no, PC+7);
            return 3;
        };

        // is there symbol?
        module *m=find_module_for_address (p, PC);
        symbol *s=process_sym_exist_at(p, PC);
        if (s)
        {
            // should it be skipped?
            if (symbol_skip_on_tracing(m, s))
            {
                DWORD ret_adr;
                L ("symbol to be skipped\n");
                if (MC_ReadREG(mc, CONTEXT_get_SP(ctx), &ret_adr)==false)
                {
                    assert(0);
                };
                CONTEXT_setDRx_and_DR7 (ctx, bp_no, ret_adr);
                return 3;
            };
        };
        /*
           if (ins==I_CALL)
           t->tracing_CALLs_executed++;
           if (ins==I_RETN)
           t->tracing_CALLs_executed--;
           */
        // handle all here
        {
            Da* da=MC_disas(PC, mc);

            //Da_DumpString (&cur_fds, da);
            //die("");

            if (bpf->cc)
                handle_cc(da, p, t, ctx, mc);

            Da_free(da);
        };

    } while (emulated);

    // continue?
    return 1;
};

void handle_BPF(process *p, thread *t, int bp_no, CONTEXT *ctx, MemoryCache *mc)
{
    L ("%s() begin\n", __func__);
    BP *bp=breakpoints[bp_no];
    BPF_state* state=&t->BPF_states[bp_no];
    BPF *bpf=bp->u.bpf;

    switch (*state)
    {
        case BPF_state_default:
            if (handle_begin(p, t, bp, bp_no, ctx, mc)==false)
                goto handle_finish_and_switch_to_default; // got no return address or function to be skipped, exit

            if (bpf->trace)
            {
                // begin tracing
                set_TF(ctx);
                *state=BPF_state_tracing_inside_function;
                t->tracing=true; t->tracing_bp=bp_no;
                goto call_handle_tracing_etc;
            }
            else
                *state=BPF_state_at_return;
            break;

        case BPF_state_at_return:
            goto handle_finish_and_switch_to_default;

        case BPF_state_tracing_inside_function:
            goto call_handle_tracing_etc;

        case BPF_state_tracing_skipping:
            CONTEXT_setDRx_and_DR7 (ctx, bp_no, t->ret_adr); // return DRx back
            *state=BPF_state_tracing_inside_function;
            goto call_handle_tracing_etc;

        default:
            assert(!"invalid *state");
            break; // TMCH
    };
    goto exit;

call_handle_tracing_etc:
    switch (handle_tracing(bp_no, p, t, ctx, mc))
    {
        case 1: // go on
            L ("handle_tracing() -> 1\n");
            set_TF (ctx);
            break;
        case 2: // finished
            L ("handle_tracing() -> 2\n");
            t->tracing=false;
            clear_TF(ctx);
            REMOVE_BIT(ctx->Dr6, FLAG_DR6_BS);
            goto handle_finish_and_switch_to_default;
        case 3: // need to skip something
            L ("handle_tracing() -> 3\n");
            clear_TF(ctx);
            *state=BPF_state_tracing_skipping;
            break;
    };
    goto exit;

handle_finish_and_switch_to_default:
    handle_finish(p, t, bp, bp_no, ctx, mc);
    *state=BPF_state_default;

exit:
    L ("%s() end. TF=%s\n", __func__, IS_SET(ctx->EFlags, FLAG_TF) ? "true" : "false");
};
