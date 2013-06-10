#include <assert.h>

#include "bp_address.h"
#include "dmalloc.h"
#include "BPX.h"
#include "thread.h"
#include "opts.h"
#include "utils.h"

void BPX_option_free(BPX_option *o)
{
    bp_address_free (o->a);
    DFREE (o->copy_string);
    DFREE (o);
};

void BPX_free(BPX *o)
{
    if (o->opts)
    {
        BPX_option *t=o->opts, *t_next=o->opts;
        for (;t_next;t=t_next)
        {
            t_next=t->next;
            BPX_option_free(t);
        };
    };
    DFREE (o);
};

void dump_BPX(BPX *b)
{
    printf ("BPX.");
    if (b->opts)
    {
        printf (" options: ");
        for (BPX_option *o=b->opts; o; o=o->next)
            dump_BPX_option(o);
    };
    printf ("\n");
};

void dump_BPX_option(BPX_option *b)
{
    switch (b->t)
    {
        case BPX_option_DUMP:
            printf ("[DUMP ");
            if (b->a)
                dump_address(b->a);
            else
                printf ("reg:%s", X86_register_ToString(b->reg));
            printf (" size: %d]", b->size_or_value);
            break;

        case BPX_option_SET:
            assert (b->a==NULL); // must be always register
            printf("[SET reg:%s value:%d]", X86_register_ToString(b->reg), b->size_or_value);
            break;

        case BPX_option_COPY:
            printf ("[COPY ");
            if (b->a)
                dump_address(b->a);
            else
                printf ("reg:%s", X86_register_ToString(b->reg));
            assert(b->copy_string);
            printf ("[");
            for (int i=0; i<b->copy_string_len; i++)
                printf ("0x%02X ", b->copy_string[i]);
            printf ("]");
            break;

        default:
            assert(0);
    };
};

BPX* create_BPX(BPX_option *opts)
{
    BPX* rt=DCALLOC (BPX, 1, "BPX");
    rt->opts=opts;

    return rt;
};

static bool BPX_get_address_for_DUMP_SET_COPY (BPX_option *o, CONTEXT *ctx, address *out)
{
    if (o->a)
        if (o->a->resolved==false)
        {
            strbuf sb=STRBUF_INIT;
            address_to_string(o->a, &sb);
            L ("address %s (from BPX option) is still not resolved\n", sb.buf);
            strbuf_deinit(&sb);
            return false;
        }
        else
            *out=o->a->abs_address;
    else
        *out=CONTEXT_get_reg(ctx, o->reg);

    return true;
};

static void handle_BPX_option (process *p, thread *t, CONTEXT *ctx, MemoryCache *mc, BPX_option *o)
{
    switch (o->t)
    {
        case BPX_option_DUMP:
            {
                address a;
                if (BPX_get_address_for_DUMP_SET_COPY(o, ctx, &a))
                {
                    if (MC_L_print_buf_in_mem_ofs (mc, a, o->size_or_value, a)==false)
                        L ("Can't read buffer of size %d at address 0x" PRI_ADR_HEX "\n", o->size_or_value, a);
                };
            };
            break;

        case BPX_option_SET:
        case BPX_option_COPY:
            assert (!"not implemented");
            break;
        default:
            assert(0);
            break;
    };

    if (o->next)
        handle_BPX_option(p, t, ctx, mc, o->next);
};

static void handle_BPX_default_state(BP *bp, process *p, thread *t, int DRx_no, CONTEXT *ctx, MemoryCache *mc)
{
    BPX *bpx=bp->u.bpx;
    if (bpx_c_debug)
        L ("%s() begin\n", __func__);
    bp_address *bp_a=bp->a;
    strbuf sb_address=STRBUF_INIT;
    address_to_string(bp_a, &sb_address);

    dump_PID_if_need(p); dump_TID_if_need(p, t);
    L ("(%d) %s\n", DRx_no, sb_address.buf);
    dump_CONTEXT (&cur_fds, &ctx, false, false, false);

    if (bpx->opts)
        handle_BPX_option (p, t, ctx, mc, bpx->opts);

    // remove DRx
    CONTEXT_clear_bp_in_DR7 (ctx, DRx_no);
    // turn on TF
    set_TF (ctx);
    t->tracing=true; t->tracing_bp=DRx_no;
    strbuf_deinit(&sb_address);
    if (bpx_c_debug)
        L ("%s() end\n", __func__);
}

static void handle_BPX_skipping_first_instruction(BP *bp, process *p, thread *t, int DRx_no, CONTEXT *ctx, MemoryCache *mc)
{
    if (bpx_c_debug)
        L ("%s() begin\n", __func__);
    // set DRx back
    set_or_update_DRx_breakpoint(bp, ctx, DRx_no);
    t->tracing=false;
    if (bpx_c_debug)
        L ("%s() end\n", __func__);
};

void handle_BPX(process *p, thread *t, int DRx_no, CONTEXT *ctx, MemoryCache *mc)
{
    if (bpx_c_debug)
        L ("%s() begin\n", __func__);
    BP *bp=breakpoints[DRx_no];
    BPX_state* state=&t->BPX_states[DRx_no];
   
    if (*state==BPX_state_default)
    {
        handle_BPX_default_state(bp, p, t, DRx_no, ctx, mc);
        *state=BPX_state_skipping_first_instruction;
    }
    else if (*state==BPX_state_skipping_first_instruction)
    {
        handle_BPX_skipping_first_instruction(bp, p, t, DRx_no, ctx, mc);
        *state=BPX_state_default;
    }
    else
    {
        assert(0);
    };
    if (bpx_c_debug)
        L ("%s() end\n", __func__);
};
