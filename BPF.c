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
#include "rand.h"

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
    if (b->set_present)
    {
        printf ("set_width=%d set_arg_n=%d set_ofs=0x" PRI_REG_HEX " set_val=0x" PRI_REG_HEX "\n", b->set_width, b->set_arg_n, b->set_ofs, b->set_val);
    };
    // NOTE: args_n, arg_types, ret_type, this_type not dumped
};

void BPF_free(BPF* o)
{
    if (o->when_called_from_address)
        bp_address_free(o->when_called_from_address);
    if (o->when_called_from_func)
        bp_address_free(o->when_called_from_func);
    DFREE (o->arg_types);
    DFREE (o);
};

static void dump_QString (address a, MemoryCache *mc)
{
    /*
       struct Data {
       QBasicAtomicInt ref;
       int alloc, size;
       ushort *data; // QT5: put that after the bit field to fill alignment gap; don't use sizeof any more then
       ushort clean : 1;
       ushort simpletext : 1;
       ushort righttoleft : 1;
       ushort asciiCache : 1;
       ushort capacity : 1;
       ushort reserved : 11;
    // ### Qt5: try to ensure that "array" is aligned to 16 bytes on both 32- and 64-bit
    ushort array[1];
    };
    */
    REG r2;
    L ("QString: ");
    if (MC_ReadREG(mc, a+sizeof(REG)+sizeof(tetrabyte)*2, &r2))
    {
        strbuf sb=STRBUF_INIT;
        if (MC_GetString (mc, r2, true, &sb))
            L ("(data=\"%s\")", sb.buf);
        else
        {
            BYTE buf[4];
            if (MC_ReadBuffer (mc, r2, 4, buf))
                L ("(data=(0x%x 0x%x 0x%x 0x%x ... ))", buf[0], buf[1], buf[2], buf[3]);
            else
                L ("(data=(read error))");
        };

        strbuf_deinit(&sb);
    }
    else
        L ("(object read error)");
};

// function to be separated?
// return true if hexadecimal number dumped
static bool BPF_dump_arg (MemoryCache *mc, REG arg, bool unicode, function_type ft)
{
    bool rt=false;
    switch (ft)
    {
        case TY_UNKNOWN:
            {
                strbuf sb=STRBUF_INIT;

                if (MC_GetString(mc, arg, unicode, &sb))
                    L ("\"%s\"", sb.buf);
                else
                {
                    L ("0x" PRI_REG_HEX, arg);
                    rt=true;
                };

                strbuf_deinit (&sb);
            };
            break;

        case TY_QSTRING:
            dump_QString(arg, mc);
            break;

        case TY_PTR: // get sym?
        case TY_REG:
        case TY_INT:
            L ("0x" PRI_REG_HEX, arg);
            break;

        case TY_VOID:
        case TY_UNINTERESTING:
            break;

        default:
            printf ("ft=%d\n", ft);
            assert(!"not implemented");
            break;
    };
    return rt;
};

static void BPF_dump_args (MemoryCache *mc, REG* args, unsigned args_n, bool unicode, function_type *arg_types)
{
    for (unsigned i=0; i<args_n; i++)
    {
        BPF_dump_arg (mc, args[i], unicode, arg_types ? arg_types[i] : TY_UNKNOWN);
        if ((i+1) != args_n)
            L (", ");
    };
};

static void load_args(thread *t, CONTEXT *ctx, MemoryCache *mc, unsigned bp_no, unsigned args)
{
    address SP=CONTEXT_get_SP(ctx);
    BP_thread_specific_dynamic_info *di=&t->BP_dynamic_info[bp_no];
    di->BPF_args=DMALLOC(REG, args, "REG");

#ifdef _WIN64
    for (unsigned a=0; a<args; a++)
    {
        switch (a)
        {
            case 0: t->_BPF_args[bp_no][a]=ctx->Rcx;
                    break;
            case 1: t->_BPF_args[bp_no][a]=ctx->Rdx;
                    break;
            case 2: t->_BPF_args[bp_no][a]=ctx->R8;
                    break;
            case 3: t->_BPF_args[bp_no][a]=ctx->R9;
                    break;
            default:
                    if (MC_ReadOctabyte (mc, SP+(a+1+4)*sizeof(REG), &t->BPF_args[a])==false)
                        goto read_failed;
                    break;
        };
    };
#else
    if (MC_ReadBuffer(mc, SP+REG_SIZE, args*REG_SIZE, (BYTE*)di->BPF_args)==false)
        goto read_failed;
#endif
    return;

read_failed:
    DFREE (di->BPF_args);
    di->BPF_args=NULL;
};

static void handle_set (BPF* bpf, unsigned bp_no, process *p, thread *t, CONTEXT *ctx, MemoryCache *mc, unsigned args)
{
    BP_thread_specific_dynamic_info *di=&t->BP_dynamic_info[bp_no];
    if (bpf->set_arg_n+1 > args)
    {
        L ("Can't work with arg_%d: it wasn't loaded, change 'args' options in BPF!\n", bpf->set_arg_n);
        return;
    };

    address adr=di->BPF_args[bpf->set_arg_n] + bpf->set_ofs;
    bool succ=MC_WriteValue(mc, adr, bpf->set_width, bpf->set_val);

    L ("0x" PRI_REG_HEX " %swritten at address 0x" PRI_ADR_HEX " (arg_%d+0x" PRI_REG_HEX ")\n", 
            bpf->set_val, succ ? "" : "cannot be ", adr, bpf->set_arg_n, bpf->set_ofs);
};

static void dump_bufs_if_need(thread *t, MemoryCache *mc, unsigned size, unsigned args_n, REG* args, unsigned bp_no)
{
    BP_thread_specific_dynamic_info *di=&t->BP_dynamic_info[bp_no];
    assert (di->BPF_buffers_at_start==NULL);
    di->BPF_buffers_at_start=(REG**)DCALLOC(REG*, args_n, "REG*");
    di->BPF_buffers_at_start_cnt=args_n;

    for (unsigned i=0; i<args_n; i++)
    {
        BYTE *buf=DMALLOC(BYTE, size, "buf");
        if (MC_ReadBuffer (mc, args[i], size, buf))
        {
            L ("Argument %d/%d\n", i+1, args_n);
            L_print_buf_ofs (buf, size, args[i]);
            di->BPF_buffers_at_start[i]=buf;
        }
        else
            DFREE(buf);
    };
};

static void dump_args_diff_if_need(thread *t, MemoryCache *mc, unsigned size, unsigned args_n, REG* args, unsigned bp_no)
{
    BP_thread_specific_dynamic_info *di=&t->BP_dynamic_info[bp_no];
    assert (di->BPF_buffers_at_start);

    for (unsigned i=0; i<args_n; i++)
    {
        if (di->BPF_buffers_at_start[i]==NULL)
            continue;

        BYTE *buf2=DMALLOC(BYTE, size, "buf");
        bool b=MC_ReadBuffer (mc, args[i], size, buf2);
        assert (b);
        if (memcmp (di->BPF_buffers_at_start[i], buf2, size)!=0)
        {
            L ("Argument %d/%d difference\n", i+1, args_n);
            L_print_bufs_diff (di->BPF_buffers_at_start[i], buf2, size);
        };
        DFREE(buf2);
        DFREE(di->BPF_buffers_at_start[i]);
    };
    DFREE (di->BPF_buffers_at_start);
    di->BPF_buffers_at_start=NULL;
};
        
static void dump_object_info_if_needed(BPF *bpf, MemoryCache *mc, CONTEXT *ctx)
{
    if (bpf->this_type==TY_UNKNOWN || bpf->this_type==TY_UNINTERESTING)
        return;

    L ("this=");
    REG r;
    if (MC_ReadREG(mc, CONTEXT_get_xCX(ctx), &r))
        BPF_dump_arg(mc, r, bpf->unicode, bpf->this_type);
    else
        L ("(read error)");
    L ("\n");
};

static void is_it_known_function (const char *fn_name, BPF* bpf)
{
    if (BPF_c_debug)
        L ("%s(fn_name=%s)\n", __func__, fn_name);

    // demangler should be here!
    if (strstr(fn_name, "?information@QMessageBox@@SAHPEAVQWidget@@AEBVQString@@1111HH@Z"))
    {
        bpf->args_n=8;
        bpf->arg_types=DCALLOC(function_type, bpf->args_n, "function_type");
        bpf->arg_types[0]=TY_PTR; // QWidget
        bpf->arg_types[1]=bpf->arg_types[2]=bpf->arg_types[3]=bpf->arg_types[4]=bpf->arg_types[5]=TY_QSTRING;
        bpf->arg_types[6]=bpf->arg_types[7]=TY_INT;
        bpf->ret_type=bpf->this_type=TY_UNINTERESTING;
        bpf->known_function=Fuzzy_True;
        if (BPF_c_debug)
            L ("%s() - True\n", __func__);
        return;
    };

    if (strstr(fn_name, "?1QString@@QEAA@XZ")) // in fact: "??1QString@@QEAA@XZ" 
    {
        bpf->this_type=TY_QSTRING;
        bpf->known_function=Fuzzy_True;
        if (BPF_c_debug)
            L ("%s() - True\n", __func__);
        return;
    };
    bpf->known_function=Fuzzy_False;
    if (BPF_c_debug)
        L ("%s() - False\n", __func__);
};

// return - should this call skipped?
static bool handle_when_called_from_func(process *p, thread *t, unsigned bp_no, BPF *bpf, bool got_ret_adr)
{
    BP_thread_specific_dynamic_info *di=&t->BP_dynamic_info[bp_no];
    assert (bpf->when_called_from_func);

    if (bpf->when_called_from_func->resolved==false || got_ret_adr==false)
        return true;

    if (bpf->when_called_from_func_next_func_adr_present==false)
    {
        // be sure (once), we work with symbol start
        if (process_sym_exist_at (p, bpf->when_called_from_func->abs_address)==NULL)
            die ("The address set in when_called_from_func should be symbol start!\n");

        address a=process_get_next_sym_address_after (p, bpf->when_called_from_func->abs_address);
        if (a)
        {
            bpf->when_called_from_func_next_func_adr=a;
            if (BPF_c_debug)
                L ("(case 1) bpf->when_called_from_func_next_func_adr=0x" PRI_ADR_HEX "\n", bpf->when_called_from_func_next_func_adr);
        }
        else
        {
            bpf->when_called_from_func_next_func_adr=get_module_end(find_module_for_address (p, bpf->when_called_from_func->abs_address));
            if (BPF_c_debug)
                L ("(case 2) bpf->when_called_from_func_next_func_adr=0x" PRI_ADR_HEX "\n", bpf->when_called_from_func_next_func_adr);
        };
        bpf->when_called_from_func_next_func_adr_present=true;
    };

    if (di->ret_adr>=bpf->when_called_from_func->abs_address && 
            di->ret_adr<bpf->when_called_from_func_next_func_adr)
        return false;

    return true;
};

static bool handle_begin(process *p, thread *t, BP *bp, int bp_no, CONTEXT *ctx, MemoryCache *mc)
{
    // do function begin things
    BP_thread_specific_dynamic_info *di=&t->BP_dynamic_info[bp_no];
    BPF *bpf=bp->u.bpf;
    bp_address *bp_a=bp->a;
    strbuf sb=STRBUF_INIT, sb_address=STRBUF_INIT;
    address_to_string(bp_a, &sb_address); // sb_address in form module!regexp
    address SP=CONTEXT_get_SP(ctx);
    bool got_ret_adr=MC_ReadREG(mc, SP, &di->ret_adr);
    if (got_ret_adr)
        di->SP_at_ret_adr=SP;

    bool rt;

    if (bpf->when_called_from_func && handle_when_called_from_func(p, t, bp_no, bpf, got_ret_adr))
    {
        rt=false;
        goto exit;
    };

    if (got_ret_adr)
        process_get_sym (p, di->ret_adr, true, true, &sb);
    else
        L ("Cannot read a register at SP, so, BPF return will not be handled\n");

    if (bpf->known_function==Fuzzy_Undefined)
    {
        strbuf sb_symbol_name=STRBUF_INIT;
        
        // FIXME: all symbols at $bp_a->abs_address$ should be enumerated!
        is_it_known_function(sb_address.buf, bpf);

        if (bpf->known_function==Fuzzy_True)
            L ("This is known (to us) function\n");
        strbuf_deinit(&sb_symbol_name);
    };

    unsigned args=bpf->known_function==Fuzzy_True ? bpf->args_n : bpf->args;

    load_args(t, ctx, mc, bp_no, args);

    dump_PID_if_need(p); dump_TID_if_need(p, t);
    L ("(%d) %s (", bp_no, sb_address.buf);
    BPF_dump_args (mc, di->BPF_args, args, bpf->unicode, bpf->arg_types);
    L (")");
    if (got_ret_adr)
        L (" (called from %s (0x" PRI_ADR_HEX "))", sb.buf, di->ret_adr);
    L ("\n");

    dump_object_info_if_needed(bpf, mc, ctx);

    if (bpf->dump_args)
        dump_bufs_if_need(t, mc, args, bpf->args, di->BPF_args, bp_no);

    if (dash_s)
        dump_stack (p, t, ctx, mc);
    
    if (bpf->pause)
    {
        L ("Sleeping for %d milliseconds...\n", bpf->pause);
        Sleep (bpf->pause);
    };

    if (bpf->set_present)
        handle_set(bpf, bp_no, p, t, ctx, mc, args);

    if (got_ret_adr)
    {
        if (bpf->skip)
        {
            L ("(%d) Skipping execution of this function\n", bp_no);
            CONTEXT_set_PC(ctx, di->ret_adr);
            CONTEXT_set_SP(ctx, CONTEXT_get_SP(ctx)+sizeof(REG));
            rt=false;
        }
        else
        {
            // set current DRx to return
            CONTEXT_setDRx_and_DR7 (ctx, bp_no, di->ret_adr); // set breakpoint at return
            rt=true;
        }
    }
    else
        rt=false;
exit:
    strbuf_deinit(&sb);
    strbuf_deinit(&sb_address);
    return rt;
};

static void handle_finish(process *p, thread *t, BP *bp, int bp_no, CONTEXT *ctx, MemoryCache *mc)
{
    BP_thread_specific_dynamic_info *di=&t->BP_dynamic_info[bp_no];
    bp_address *bp_a=bp->a;
    BPF *bpf=bp->u.bpf;
    strbuf sb_address=STRBUF_INIT;
    address_to_string(bp_a, &sb_address);
    // do function finish things
    REG accum=CONTEXT_get_Accum (ctx);

    // dump all info in one line:
    dump_PID_if_need(p); dump_TID_if_need(p, t);
    L ("(%d) %s () -> ", bp_no, sb_address.buf);

    if (BPF_dump_arg(mc, accum, bpf->unicode, bpf->ret_type)==false)
        L (" (0x" PRI_REG_HEX ")", accum);

    if (dump_fpu && IS_SET(ctx->FloatSave.TagWord, 1)) // ST0 present?
        L (", ST0=%.1f", (double)*(long double*)&ctx->FloatSave.RegisterArea[0]);
    L ("\n");

    if (bpf->dump_args)
        dump_args_diff_if_need(t, mc, bpf->dump_args, bpf->args, di->BPF_args, bp_no);

    DFREE(di->BPF_args); di->BPF_args=NULL;

    bool modify_AX=false;

    if (bpf->rt_probability!=0.0 && bpf->rt_probability!=1.0)
        if (rand_double()<bpf->rt_probability)
            modify_AX=true;

    if (modify_AX || bpf->rt_probability==1.0)
    {
        dump_PID_if_need(p); dump_TID_if_need(p, t);
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
    address PC, SP;
    BP *bp=breakpoints[bp_no];
    BPF *bpf=bp->u.bpf;
    BP_thread_specific_dynamic_info *di=&t->BP_dynamic_info[bp_no];

    if (tracing_dbg)
    {
        PC=CONTEXT_get_PC(ctx);
        strbuf sb=STRBUF_INIT;
        process_get_sym(p, PC, true, true, &sb);

        L ("%s() PC=%s (0x%x)\n", __func__, sb.buf, PC);
        strbuf_deinit (&sb);
    };

    do // emu cycle
    {
        PC=CONTEXT_get_PC(ctx);
        SP=CONTEXT_get_SP(ctx);
        // (to be implemented in future): check other BPF/BPX breakpoints. handle them if need.

        // finished? PC==ret_adr and SP==SP_at_ret_adr? return 2
        // check SP as well, this we need for tracing recursive functions correctly
        if (PC==di->ret_adr && (SP==(di->SP_at_ret_adr)+sizeof(REG)))
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
                REG ret_adr;
                //L ("symbol to be skipped\n");
                if (MC_ReadREG(mc, CONTEXT_get_SP(ctx), &ret_adr)==false)
                {
                    assert(0);
                };
                //clear_TF(ctx);
                CONTEXT_setDRx_and_DR7 (ctx, bp_no, ret_adr);
                return 3;
            };
        };

        Da* da=MC_disas(PC, mc);

        if (da==NULL)
        {
            strbuf sb=STRBUF_INIT;
            process_get_sym(p, PC, true, true, &sb);

            L_once ("%s() disassemble failed for PC=%s (0x" PRI_ADR_HEX ")\n", __func__, sb.buf, PC);
            strbuf_deinit (&sb);
        }

        if (da && da->ins_code==I_CALL)
        {
            if (limit_trace_nestedness && di->tracing_CALLs_executed+1 >= limit_trace_nestedness)
            {
                // this call is to be skipped!
                if (BPF_c_debug)
                    L ("t->tracing_CALLs_executed=%d, limit_trace_nestedness=%d, skipping this CALL!\n",
                            di->tracing_CALLs_executed, limit_trace_nestedness);
                address new_adr=CONTEXT_get_PC(ctx) + da->len;
                //clear_TF(ctx);
                CONTEXT_setDRx_and_DR7 (ctx, bp_no, new_adr);
                if (bpf->cc)                            // redundant
                    handle_cc(da, p, t, ctx, mc, true); // redundant
                Da_free(da);
                return 3;
            }
            else
                di->tracing_CALLs_executed++;
        };

        if (da && da->ins_code==I_RETN && di->tracing_CALLs_executed>0)
            di->tracing_CALLs_executed--;

        if (bpf->cc)
            handle_cc(da, p, t, ctx, mc, false);
        Da_free(da);

    } while (emulated);

    // continue?
    return 1;
};

void handle_BPF(process *p, thread *t, int bp_no, CONTEXT *ctx, MemoryCache *mc)
{
    if (BPF_c_debug)
        L ("%s() begin\n", __func__);
    BP *bp=breakpoints[bp_no];
    BP_thread_specific_dynamic_info *di=&t->BP_dynamic_info[bp_no];
    BPF_state* state=&di->BPF_states;
    BPF *bpf=bp->u.bpf;

    switch (*state)
    {
        case BPF_state_default:
            if (handle_begin(p, t, bp, bp_no, ctx, mc)==false)
                // got no return address or function to be skipped or when_called_from_func isn't we need - exit
                goto switch_to_default;

            if (bpf->trace)
            {
                // begin tracing
                //set_TF(ctx);
                *state=BPF_state_tracing_inside_function;
                di->tracing=true;
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
            CONTEXT_setDRx_and_DR7 (ctx, bp_no, di->ret_adr); // return DRx back
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
            if (BPF_c_debug)
                L ("handle_tracing() -> 1\n");
            //set_TF (ctx);
            di->tracing=true;
            break;
        case 2: // finished
            if (BPF_c_debug)
                L ("handle_tracing() -> 2\n");
            di->tracing=false;
            //clear_TF(ctx);
            REMOVE_BIT(ctx->Dr6, FLAG_DR6_BS);
            goto handle_finish_and_switch_to_default;
        case 3: // need to skip something
            if (BPF_c_debug)
                L ("handle_tracing() -> 3\n");
            di->tracing=false;
            //clear_TF(ctx);
            *state=BPF_state_tracing_skipping;
            break;
    };
    goto exit;

handle_finish_and_switch_to_default:
    handle_finish(p, t, bp, bp_no, ctx, mc);
switch_to_default:
    *state=BPF_state_default;

exit:
    if (BPF_c_debug)
        L ("%s() end. TF=%s\n", __func__, IS_SET(ctx->EFlags, FLAG_TF) ? "true" : "false");
};
