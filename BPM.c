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

#include "opts_aux.h"
#include "CONTEXT_utils.h"
#include "utils.h"
#include "BP.h"
#include "BPM.h"
#include "dmalloc.h"
#include "bitfields.h"
#include "process.h"
#include "bp_address.h"
#include "fmt_utils.h"

struct BPM *create_BPM(unsigned width, enum BPM_type t)
{
    struct BPM *rt=DCALLOC (struct BPM, 1, "BPM");

    rt->width=width;
    rt->t=t;

    return rt;
};

void BPM_free(struct BPM *o)
{
    DFREE (o);
};

void BPM_ToString(struct BPM *bpm, strbuf *out)
{
    strbuf_addf (out, "BPM. width=%d, ", bpm->width);
    if (bpm->t==BPM_type_RW)
        strbuf_addstr (out, "type=RW");
    else
        strbuf_addstr (out, "type=W");
    strbuf_addstr (out, "\n");
};

void BPM_set_or_update_DRx_breakpoint(struct BPM *bpm, address a, unsigned DRx_no, CONTEXT *ctx)
{
    unsigned DR7_idx;

    SET_BIT (ctx->Dr7, REG_1<<(DRx_no*2));

    switch (DRx_no)
    {
        case 0: ctx->Dr0=a;
                DR7_idx=16;
                break;
        case 1: ctx->Dr1=a;
                DR7_idx=20;
                break;
        case 2: ctx->Dr2=a;
                DR7_idx=24;
                break;
        case 3: ctx->Dr3=a;
                DR7_idx=28;
                break;
        default:
                oassert (0);
                break;
    };

    if (bpm->t==BPM_type_W) // writting
    {
        SET_BIT (ctx->Dr7, REG_1<<DR7_idx);
        REMOVE_BIT (ctx->Dr7, REG_1<<(DR7_idx+1));
    }
    else if (bpm->t==BPM_type_RW) // reading/writting
    {
        SET_BIT (ctx->Dr7, REG_1<<DR7_idx);
        SET_BIT (ctx->Dr7, REG_1<<(DR7_idx+1));
    }
    else
    {
        oassert (0);
    };

    switch (bpm->width)
    {
        case 1:
            REMOVE_BIT (ctx->Dr7, REG_1<<(DR7_idx+2));
            REMOVE_BIT (ctx->Dr7, REG_1<<(DR7_idx+3));
            break;
        case 2:
            SET_BIT (ctx->Dr7, REG_1<<(DR7_idx+2));
            REMOVE_BIT (ctx->Dr7, REG_1<<(DR7_idx+3));
            break;
        case 4:
            SET_BIT (ctx->Dr7, REG_1<<(DR7_idx+2));
            SET_BIT (ctx->Dr7, REG_1<<(DR7_idx+3));
            break;
        case 8:
#ifdef _WIN64
            REMOVE_BIT (ctx->Dr7, REG_1<<(DR7_idx+2));
            SET_BIT (ctx->Dr7, REG_1<<(DR7_idx+3));
#else
            oassert (!"64-bit memory breakpoints can't be implemented in win32 version");
#endif
            break;
        default:
            oassert (0);
            break;
    };
};

void handle_BPM(struct process *p, struct thread *t, int bp_no, CONTEXT *ctx, struct MemoryCache *mc)
{
    struct BP *bp=breakpoints[bp_no];
    struct BPM *bpm=bp->u.bpm;

    address a=bp->a->abs_address;
    bool b;
    REG val;
    const char* v_type;

    switch (bpm->width)
    {
        case 1:
            {
                v_type="BYTE"; 
                byte tmp;
                b=MC_ReadByte (mc, a, &tmp);
                oassert(b);
                val=tmp;
            };
            break;

        case 2:
            {
                v_type="WORD"; 
                wyde tmp;
                b=MC_ReadWyde (mc, a, &tmp);
                oassert(b);
                val=tmp;
            };
            break;

        case 4:
            {
                v_type="DWORD"; 
                DWORD tmp;
                b=MC_ReadTetrabyte (mc, a, &tmp);
                oassert(b);
                val=tmp;
            };
            break;

        case 8:
            {
#ifdef _WIN64
                v_type="QWORD"; 
                DWORD64 tmp;
                b=MC_ReadOctabyte (mc, a, &tmp);
                oassert(b);
                val=tmp;
#else
                oassert (0);
#endif
            };
            break;

        default:
            oassert(0);
            fatal_error();
    };

    strbuf sb_sym=STRBUF_INIT, sb_bp_adr=STRBUF_INIT;
    address PC=CONTEXT_get_PC(ctx);
    process_get_sym (p, PC, true, true, &sb_sym);
    address_to_string(bp->a, &sb_bp_adr);

    dump_PID_if_need(p); dump_TID_if_need(p, t);
    L ("(%d) some code at %s (0x" PRI_ADR_HEX ") %s %s variable at %s (it contain 0x" PRI_REG_HEX 
            " after execution of that instruction)\n",
            bp_no, sb_sym.buf, PC, bpm->t==BPM_type_RW ? "reading or writting" : "writting", 
            v_type, sb_bp_adr.buf, val);

    //MC_WriteOctabyte(mc, bp->a->abs_address, 0); // can be option?
    
    strbuf_deinit(&sb_bp_adr);
    strbuf_deinit(&sb_sym);
};

/* vim: set expandtab ts=4 sw=4 : */
