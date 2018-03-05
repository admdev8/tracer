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

#include "oassert.h"
#include "bp_address.h"
#include "dmalloc.h"
#include "BPX.h"
#include "thread.h"
#include "opts_aux.h"
#include "SEH.h"
#include "utils.h"
#include "X86_register_helpers.h"
#include "process.h"
#include "fmt_utils.h"

void BPX_option_free(struct BPX_option *o)
{
    bp_address_free (o->a);
    DFREE (o->copy_string);
    DFREE (o);
};

void BPX_free(struct BPX *o)
{
    if (o->opts)
    {
        struct BPX_option *t=o->opts, *t_next=o->opts;
        for (;t_next;t=t_next)
        {
            t_next=t->next;
            BPX_option_free(t);
        };
    };
    DFREE (o);
};

void BPX_ToString(struct BPX *b, strbuf *out)
{
    strbuf_addstr (out, "BPX.");
    if (b->opts)
    {
        strbuf_addstr (out, " options: ");
        for (struct BPX_option *o=b->opts; o; o=o->next)
            BPX_option_ToString(o, out);
    };
    strbuf_addstr (out, "\n");
};

static void BPX_DUMP_option_address_ToString(struct BPX_option *b, strbuf *out)
{
    if (b->a)
        address_to_string(b->a, out);
    else
        strbuf_addstr (out, X86_register_ToString(b->reg));
};

void BPX_option_ToString(struct BPX_option *b, strbuf *out)
{
    switch (b->t)
    {
        case BPX_option_DUMP:
            strbuf_addstr (out, "[DUMP ");
            BPX_DUMP_option_address_ToString (b, out);
            strbuf_addf (out, " size: %d]", b->size_or_value);
            break;

        case BPX_option_SET:
            oassert (b->a==NULL); // must be always register
            strbuf_addf (out, "[SET reg:%s ", X86_register_ToString(b->reg));
            if (X86_register_is_STx(b->reg))
                strbuf_addf (out, "float_value:%f]", b->float_value);
            else
                strbuf_addf (out, "value:%d]", b->size_or_value);
            break;

        case BPX_option_COPY:
            strbuf_addstr (out, "[COPY ");
            if (b->a)
                address_to_string(b->a, out);
            else
                strbuf_addf (out, "reg:%s", X86_register_ToString(b->reg));
            oassert(b->copy_string);
            strbuf_addstr (out, "[");
            for (int i=0; i<b->copy_string_len; i++)
                strbuf_addf (out, "0x%02X ", b->copy_string[i]);
            strbuf_addstr (out, "]");
            break;

        default:
            oassert(0);
            fatal_error();
    };
};

struct BPX* create_BPX(struct BPX_option *opts)
{
    struct BPX* rt=DCALLOC (struct BPX, 1, "BPX");
    rt->opts=opts;

    return rt;
};

static bool BPX_get_address_for_DUMP_SET_COPY (struct BPX_option *o, CONTEXT *ctx, address *out)
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

static void handle_BPX_option (struct process *p, struct thread *t, CONTEXT *ctx, struct MemoryCache *mc, struct BPX_option *o, unsigned bp_no)
{
    switch (o->t)
    {
        case BPX_option_DUMP:
            {
                address a;
                if (BPX_get_address_for_DUMP_SET_COPY(o, ctx, &a))
                {
                    size_t size=o->size_or_value;

                    byte* buf=DMALLOC (byte, size, "buf");

                    if (MC_ReadBuffer (mc, a, size, buf))
                    {
                        L_print_buf_ofs (buf, size, a);

                        strbuf sb=STRBUF_INIT;
                        BPX_DUMP_option_address_ToString (o, &sb);
                        print_symbols_in_buf_if_possible (p, mc, buf, size, sb.buf);
                        strbuf_deinit(&sb);
                    }
                    else
                        L ("(%d) Can't read buffer of size %d at address 0x" PRI_ADR_HEX "\n", 
                                bp_no, size, a);

                    DFREE (buf);
                };
            };
            break;

        case BPX_option_SET:
            oassert (o->a==NULL); // only reg allowed (yet)
            obj val;
            obj_REG2_and_set_type (X86_register_get_type(o->reg), o->size_or_value, o->float_value, &val);
            if (X86_register_is_STx(o->reg))
                L ("Setting %s register to %f\n", X86_register_ToString(o->reg), o->float_value);
            else
                L ("Setting %s register to 0x" PRI_REG_HEX "\n", X86_register_ToString(o->reg), o->size_or_value);
            X86_register_set_value(o->reg, ctx, &val);
            break;

        case BPX_option_COPY:
            {
                address a;
                if (BPX_get_address_for_DUMP_SET_COPY(o, ctx, &a))
                {
                    if (MC_WriteBuffer (mc, a, o->copy_string_len, o->copy_string))
                        L ("(%d) C-string copied to memory at 0x" PRI_ADR_HEX "\n", bp_no, a);
                    else
                        L ("(%d) Can't write C-string to memory at 0x" PRI_ADR_HEX "\n", bp_no, a);

                };
            };
            break;
        default:
            oassert(0);
            fatal_error();
            break;
    };

    if (o->next)
        handle_BPX_option(p, t, ctx, mc, o->next, bp_no);
};

static void dump_register_and_symbol (struct process *p, enum X86_register_t reg, CONTEXT *ctx)
{
        strbuf sb=STRBUF_INIT;
        REG val=CONTEXT_get_reg(ctx, reg);
        process_get_sym(p, val, true, true, &sb);
        L ("%s=%s\n", X86_register_ToString(reg), sb.buf);
        strbuf_deinit (&sb);
};

static void dump_symbols_for_all_registers (struct process *p, CONTEXT *ctx)
{
#ifdef _WIN64
    dump_register_and_symbol (p, R_RAX, ctx);
    dump_register_and_symbol (p, R_RBX, ctx);
    dump_register_and_symbol (p, R_RCX, ctx);
    dump_register_and_symbol (p, R_RDX, ctx);
    dump_register_and_symbol (p, R_RSI, ctx);
    dump_register_and_symbol (p, R_RDI, ctx);
    dump_register_and_symbol (p, R_R8,  ctx);
    dump_register_and_symbol (p, R_R9,  ctx);
    dump_register_and_symbol (p, R_R10, ctx);
    dump_register_and_symbol (p, R_R11, ctx);
    dump_register_and_symbol (p, R_R12, ctx);
    dump_register_and_symbol (p, R_R13, ctx);
    dump_register_and_symbol (p, R_R14, ctx);
    dump_register_and_symbol (p, R_R15, ctx);
#else    
    dump_register_and_symbol (p, R_EAX, ctx);
    dump_register_and_symbol (p, R_EBX, ctx);
    dump_register_and_symbol (p, R_ECX, ctx);
    dump_register_and_symbol (p, R_EDX, ctx);
    dump_register_and_symbol (p, R_ESI, ctx);
    dump_register_and_symbol (p, R_EDI, ctx);
#endif    
};

static void handle_BPX_default_state(unsigned bp_no, struct BP *bp, struct process *p, struct thread *t, int DRx_no, CONTEXT *ctx, struct MemoryCache *mc)
{
    struct BPX *bpx=bp->u.bpx;
    if (verbose>0)
        L ("%s() begin\n", __func__);
    struct bp_address *bp_a=bp->a;
    strbuf sb_address=STRBUF_INIT;
    address_to_string(bp_a, &sb_address);

    dump_PID_if_need(p); dump_TID_if_need(p, t);
    L ("(%d) %s\n", DRx_no, sb_address.buf);
    dump_CONTEXT (&cur_fds, ctx, dump_fpu, false /*DRx?*/, dump_xmm);
    dump_symbols_for_all_registers(p, ctx);

    if (bpx->opts)
        handle_BPX_option (p, t, ctx, mc, bpx->opts, bp_no);

    if (dump_seh)
        dump_SEH_chain (&cur_fds, p, t, ctx, mc);

    // remove DRx
    CONTEXT_clear_bp_in_DR7 (ctx, DRx_no);
    // turn on TF
    //set_TF (ctx);
    t->BP_dynamic_info[DRx_no].tracing=true;
    strbuf_deinit(&sb_address);
    if (verbose>0)
        L ("%s() end\n", __func__);
}

static void handle_BPX_skipping_first_instruction(struct BP *bp, struct process *p, struct thread *t, int DRx_no, CONTEXT *ctx, struct MemoryCache *mc)
{
    if (verbose>0)
        L ("%s() begin\n", __func__);
    // set DRx back
    set_or_update_DRx_breakpoint(bp, ctx, DRx_no);
    t->BP_dynamic_info[DRx_no].tracing=false;
    //clear_TF (ctx);
    if (verbose>0)
        L ("%s() end\n", __func__);
};

void handle_BPX(struct process *p, struct thread *t, int DRx_no, CONTEXT *ctx, struct MemoryCache *mc)
{
    if (verbose>0)
        L ("%s() begin\n", __func__);
    struct BP *bp=breakpoints[DRx_no];
    enum BPX_state* state=&t->BP_dynamic_info[DRx_no].BPX_states;
   
    if (*state==BPX_state_default)
    {
        handle_BPX_default_state(DRx_no, bp, p, t, DRx_no, ctx, mc);
        *state=BPX_state_skipping_first_instruction;
    }
    else if (*state==BPX_state_skipping_first_instruction)
    {
        handle_BPX_skipping_first_instruction(bp, p, t, DRx_no, ctx, mc);
        *state=BPX_state_default;
    }
    else
    {
        fatal_error();
    };
    if (verbose>0)
        L ("%s() end\n", __func__);
};

/* vim: set expandtab ts=4 sw=4 : */
