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

#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

#include "bitfields.h"
#include "dmalloc.h"
#include "logging.h"
#include "porg_utils.h"
#include "opts.h"
#include "thread.h"
#include "module.h"
#include "rbtree.h"
#include "process.h"
#include "tracer.h"
#include "X86_emu.h"
#include "stuff.h"
#include "BPF.h"
#include "BPX.h"
#include "BPM.h"
#include "x86.h"
#include "utils.h"
#include "bp_address.h"
#include "one_time_INT3_BP.h"

bool detaching=false;

void detach()
{
    L ("Detaching...\n");
    detaching=true;
};

void handle_BP(process *p, thread *t, int bp_no, CONTEXT *ctx, MemoryCache *mc)
{
    if (cycle_c_debug)
        L ("%s(bp_no=%d) begin\n", __func__, bp_no);

    BP* bp=breakpoints[bp_no];

    switch (bp->t)
    {
        case BP_type_BPF:
            handle_BPF(p, t, bp_no, ctx, mc);
            break;
        case BP_type_BPX:
            handle_BPX(p, t, bp_no, ctx, mc);
            break;
        case BP_type_BPM:
            handle_BPM(p, t, bp_no, ctx, mc);
            break;
        default:
            assert(0);
    };
    if (cycle_c_debug)
        L ("%s() end. TF=%s\n", __func__, IS_SET(ctx->EFlags, FLAG_TF) ? "true" : "false");
};

static bool handle_OEP_breakpoint (process *p, thread *t, MemoryCache *mc)
{
    if (cycle_c_debug)
        L ("%s() begin\n", __func__);

    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_ALL;
    BOOL B;
    B=GetThreadContext (t->THDL, &ctx); assert (B!=FALSE);

    CONTEXT_decrement_PC(&ctx);
    address PC=CONTEXT_get_PC(&ctx);

    B=SetThreadContext (t->THDL, &ctx); assert (B!=FALSE);

    bool b=MC_WriteByte (mc, PC, p->executable_module->saved_OEP_byte);
    assert (b && "cannot restore original byte at OEP");

    if (load_filename)
        p->we_are_loading_and_OEP_was_executed=true;

    set_or_update_all_DRx_breakpoints(p); // for all threads! only DRx breakpoints set/updated!

    if (cycle_c_debug)
        L ("%s() end\n", __func__);

    return true;
};

void handle_Bx (process *p, thread *t, CONTEXT *ctx, MemoryCache *mc)
{
    if (cycle_c_debug)
        L ("%s() begin\n", __func__);

    bool bp_handled_in_SS_mode[4]={ false, false, false, false };

    if (IS_SET(ctx->Dr6, FLAG_DR6_BS) || ctx->Dr6==0) // DR6=0 sometimes observed while tracing
    {
        bool tracing_handled=false;

        for (unsigned b=0; b<4; b++)
            if (t->BP_dynamic_info[b].tracing)
            {
                handle_BP(p, t, b, ctx, mc);
                tracing_handled=bp_handled_in_SS_mode[b]=true;
            }

        if (tracing_handled==false && cycle_c_debug)
            L ("[!] BS flag in DR6 (or DR6 is zero), but no breakpoint in tracing mode\n");
    };

    if (IS_SET(ctx->Dr6, FLAG_DR6_B0) && bp_handled_in_SS_mode[0]==false)
    {
        assert (breakpoints[0]);
        handle_BP(p, t, 0, ctx, mc);
    };

    if (IS_SET(ctx->Dr6, FLAG_DR6_B1) && bp_handled_in_SS_mode[0]==false)
    {
        assert (breakpoints[1]);
        handle_BP(p, t, 1, ctx, mc);
    };

    if (IS_SET(ctx->Dr6, FLAG_DR6_B2) && bp_handled_in_SS_mode[0]==false)
    {
        assert (breakpoints[2]);
        handle_BP(p, t, 2, ctx, mc);
    };

    if (IS_SET(ctx->Dr6, FLAG_DR6_B3) && bp_handled_in_SS_mode[0]==false)
    {
        assert (breakpoints[3]);
        handle_BP(p, t, 3, ctx, mc);
    };
    
    clear_TF(ctx);
    for (unsigned b=0; b<4; b++)
        if (t->BP_dynamic_info[b].tracing)
        {
            if (cycle_c_debug)
                L ("%s() setting TF because bp #%d is in tracing mode\n", __func__, b);
            set_TF(ctx);
        };

    if (cycle_c_debug)
        L ("%s() end. TF=%s\n", __func__, IS_SET(ctx->EFlags, FLAG_TF) ? "true" : "false");
};

DWORD handle_EXCEPTION_BREAKPOINT(DEBUG_EVENT *de)
{
    DWORD PID=de->dwProcessId, TID=de->dwThreadId;
    EXCEPTION_DEBUG_INFO *e=&de->u.Exception;
    EXCEPTION_RECORD *er=&e->ExceptionRecord;
    address adr=(address)er->ExceptionAddress;
    process *p=find_process(PID);
    thread *t=find_thread(PID, TID);
    DWORD rt=DBG_EXCEPTION_NOT_HANDLED; // default;
    strbuf tmp=STRBUF_INIT;
    
    process_get_sym (p, adr, true, true, &tmp);

    if (cycle_c_debug)
        L ("EXCEPTION_BREAKPOINT %s (0x" PRI_ADR_HEX ")\n", tmp.buf, adr);

    MemoryCache *mc=MC_MemoryCache_ctor (p->PHDL, false);

    if (stricmp(tmp.buf, "ntdll.dll!DbgBreakPoint")==0)
    {
        if (cycle_c_debug)
            L ("We handle this\n");
        rt=DBG_CONTINUE;
    };

    // we are loading and is it OEP?
    if (rt==DBG_EXCEPTION_NOT_HANDLED && load_filename && (adr==p->executable_module->OEP))
    {
        handle_OEP_breakpoint (p, t, mc); // we shouldn't pass ctx here
        rt=DBG_CONTINUE; // handled
    };
    
    if (rt==DBG_EXCEPTION_NOT_HANDLED) // still not handled
    {
        CONTEXT ctx;
        ctx.ContextFlags = CONTEXT_ALL;
        BOOL b;
        b=GetThreadContext (t->THDL, &ctx); assert (b!=FALSE);

        if (check_for_onetime_INT3_BP(p, t, adr, mc, tmp.buf, &ctx))
            rt=DBG_CONTINUE; // handled

        b=SetThreadContext (t->THDL, &ctx); assert (b!=FALSE);
    };

    if (rt==DBG_EXCEPTION_NOT_HANDLED)
        L ("Warning: unknown (to us) INT3 breakpoint at %s (0x" PRI_ADR_HEX ")\n", tmp.buf, adr);

    MC_Flush (mc);
    MC_MemoryCache_dtor (mc, true);
    strbuf_deinit(&tmp);
    return rt;
};

DWORD handle_EXCEPTION_DEBUG_INFO(DEBUG_EVENT *de)
{
    DWORD PID=de->dwProcessId, TID=de->dwThreadId;
    EXCEPTION_DEBUG_INFO *e=&de->u.Exception;
    EXCEPTION_RECORD *er=&e->ExceptionRecord;
    address adr=(address)er->ExceptionAddress;
    process *p=find_process(PID);
    thread *t=find_thread(PID, TID);
    DWORD rt=DBG_EXCEPTION_NOT_HANDLED; // default;

    switch (er->ExceptionCode)
    {
        case EXCEPTION_SINGLE_STEP:
            {
                strbuf tmp=STRBUF_INIT;
                process_get_sym (p, adr, true, true, &tmp);
                CONTEXT ctx;
                ctx.ContextFlags = CONTEXT_ALL;
                BOOL B=GetThreadContext (t->THDL, &ctx); assert (B!=FALSE);           
                MemoryCache *mc=MC_MemoryCache_ctor (p->PHDL, false);

                if (cycle_c_debug)
                {
                    L ("EXCEPTION_SINGLE_STEP %s (0x" PRI_ADR_HEX ") DR6=", tmp.buf, adr); 
                    dump_DR6 (&cur_fds, ctx.Dr6); L(" (0x%x)\n", ctx.Dr6);
                    //L ("DR7="); dump_DR7 (&cur_fds, ctx.Dr7); L("\n");
                    //L ("DR0=0x" PRI_REG_HEX "\n", ctx.Dr0);
                };

                handle_Bx (p, t, &ctx, mc);

                MC_Flush (mc);
                MC_MemoryCache_dtor (mc, false);
                B=SetThreadContext (t->THDL, &ctx); assert (B!=FALSE);
                strbuf_deinit(&tmp);
                rt=DBG_CONTINUE; // handled
            };
            break;

        case EXCEPTION_BREAKPOINT:
            rt=handle_EXCEPTION_BREAKPOINT(de);
            break;

        case EXCEPTION_ACCESS_VIOLATION:
            {
                strbuf sb_sym=STRBUF_INIT;
                address a=(address)de->u.Exception.ExceptionRecord.ExceptionAddress;
                process_get_sym(p, a, true, true, &sb_sym);
                L ("EXCEPTION_ACCESS_VIOLATION at %s (0x" PRI_ADR_HEX ") ExceptionInformation[0]=%d\n",
                        sb_sym.buf,
                        a,
                        de->u.Exception.ExceptionRecord.ExceptionInformation[0]);
                strbuf_deinit(&sb_sym);
                CONTEXT ctx;
                ctx.ContextFlags = CONTEXT_ALL;
                DWORD tmpd;
                tmpd=GetThreadContext (t->THDL, &ctx); assert (tmpd!=FALSE);           

                dump_CONTEXT (&cur_fds, &ctx, dump_fpu, false /* dump_DRx */, dump_xmm);
            };
            break;

        default:
            if (cycle_c_debug)
            {
                strbuf sb_sym=STRBUF_INIT;
                address a=(address)er->ExceptionAddress;
                process_get_sym(p, a, true, true, &sb_sym);
                L ("unknown ExceptionCode: %ld (0x%08X) at %s (0x" PRI_ADR_HEX ")\n", 
                        er->ExceptionCode, er->ExceptionCode, sb_sym.buf, adr);
                strbuf_deinit(&sb_sym);
            };
            break;
    };
    return rt;
};

void save_OEP_byte_and_set_INT3_breakpoint (MemoryCache *mc, module *m)
{
    bool b;

    if (cycle_c_debug)
        L ("Setting INT3 at OEP\n");

    b=MC_ReadByte (mc, m->OEP, &m->saved_OEP_byte);
    assert(b && "can't read byte at breakpoint start");
    b=MC_WriteByte (mc, m->OEP, 0xCC);
    assert(b && "can't write 0xCC byte at breakpoint start");
};

void handle_CREATE_PROCESS_DEBUG_EVENT(DEBUG_EVENT *de)
{
    CREATE_PROCESS_DEBUG_INFO *i=&de->u.CreateProcessInfo;
    DWORD PID=de->dwProcessId, TID=de->dwThreadId;

    if (cycle_c_debug)
        L ("%s() begin\n", __func__);

    process* p=process_init (PID, i->hProcess, i->hFile, i->lpBaseOfImage);
    
    add_thread (p, TID, i->hThread, (address)i->lpStartAddress);
    rbtree_insert(processes, (void*)PID, p);
    
    MemoryCache *mc=MC_MemoryCache_ctor (p->PHDL, false);

    p->executable_module=add_module(p, (address)i->lpBaseOfImage, p->file_handle, mc);
    L ("PID=%d|New process %s\n", PID, get_module_name (p->executable_module));

    if (load_filename)
        save_OEP_byte_and_set_INT3_breakpoint (mc, p->executable_module);
    else
    {
        // we are attaching?
        // there are may be present breakpoints with absolute addresses, so set them
        set_or_update_all_DRx_breakpoints(p);
    };

    MC_Flush (mc);
    MC_MemoryCache_dtor (mc, false);

    if (cycle_c_debug)
        L ("%s() end\n", __func__);
};

void handle_CREATE_THREAD_DEBUG_EVENT (DEBUG_EVENT *de)
{
    CREATE_THREAD_DEBUG_INFO *i=&de->u.CreateThread;
    process *p=find_process(de->dwProcessId);

    if (cycle_c_debug)
        L ("CREATE_THREAD_DEBUG_EVENT\n");

    add_thread (p, de->dwThreadId, i->hThread, (address)i->lpStartAddress);
    set_or_update_all_DRx_breakpoints(p); // overkill...
};

void handle_LOAD_DLL_DEBUG_EVENT (DEBUG_EVENT *de)
{
    LOAD_DLL_DEBUG_INFO *i=&de->u.LoadDll;
    DWORD PID=de->dwProcessId;
    process *p=find_process(PID);
    strbuf sb=STRBUF_INIT;

    bool b=GetFileNameFromHandle(de->u.LoadDll.hFile, &sb);
    assert (b);

    if (cycle_c_debug)
        L ("%s() LOAD_DLL_DEBUG_EVENT: %s 0x%x\n", __func__, sb.buf, i->lpBaseOfDll);
    strbuf_deinit (&sb);
    
    MemoryCache *mc=MC_MemoryCache_ctor (p->PHDL, false);

    add_module (p, (address)i->lpBaseOfDll, i->hFile, mc);

    if (cycle_c_debug)
        L ("%s() MC_Flush() to be called\n", __func__);

    MC_Flush (mc);
    MC_MemoryCache_dtor (mc, false);

    if (cycle_c_debug)
        L ("%s() exiting\n", __func__);
};

void handle_UNLOAD_DLL_DEBUG_EVENT (DEBUG_EVENT *de)
{
    UNLOAD_DLL_DEBUG_INFO *i=&de->u.UnloadDll;
    DWORD PID=de->dwProcessId;
    process *p=find_process(PID);

    if (cycle_c_debug)
        L ("UNLOAD_DLL_DEBUG_EVENT 0x%x\n", i->lpBaseOfDll);

    remove_module (p, (address)i->lpBaseOfDll);
};

void handle_EXIT_THREAD_DEBUG_EVENT(DEBUG_EVENT *de)
{
    DWORD PID=de->dwProcessId, TID=de->dwThreadId;
    process *p;
    thread *t=find_thread(PID, TID);

    p=find_process(PID);

    if (cycle_c_debug)
        L ("EXIT_THREAD_DEBUG_EVENT\n");

    thread_free (t);
    rbtree_delete (p->threads, (void*)TID);
};

void handle_EXIT_PROCESS_DEBUG_EVENT(DEBUG_EVENT *de)
{
    EXIT_PROCESS_DEBUG_INFO *i=&de->u.ExitProcess;
    DWORD PID=de->dwProcessId;
    process *p=find_process(PID);

    L ("PID=%d|Process %s exited. ExitCode=%d (0x%x)\n", PID, get_module_name(p->executable_module), 
            i->dwExitCode, i->dwExitCode);

    process_free (p);
    rbtree_delete (processes, (void*)PID);
};

DWORD handle_OUTPUT_DEBUG_STRING_EVENT(DEBUG_EVENT *de)
{
    DWORD PID=de->dwProcessId;
    process *p=find_process(PID);

    char *buf=DMALLOC(char, de->u.DebugString.nDebugStringLength, "char");
    assert (de->u.DebugString.fUnicode==0); // TODO
    MemoryCache* mc=MC_MemoryCache_ctor(p->PHDL, true);
    bool b=MC_ReadBuffer(mc, (address)de->u.DebugString.lpDebugStringData, de->u.DebugString.nDebugStringLength, (BYTE*)buf);
    assert (b);

    strbuf tmp=STRBUF_INIT;
    strbuf_cvt_to_C_string (buf, &tmp, false);

    if (cycle_c_debug)
        L ("OUTPUT_DEBUG_STRING_EVENT: [%s]\n", tmp.buf);

    strbuf_deinit (&tmp);
    DFREE (buf);
    MC_MemoryCache_dtor(mc, false);
    return DBG_CONTINUE;
};

DWORD handle_debug_event (DEBUG_EVENT *de)
{
    if (cycle_c_debug)
        L ("%s() begin\n", __func__);

    switch (de->dwDebugEventCode)
    {
        case EXCEPTION_DEBUG_EVENT:
            return handle_EXCEPTION_DEBUG_INFO (de);
            break;

        case CREATE_PROCESS_DEBUG_EVENT:
            handle_CREATE_PROCESS_DEBUG_EVENT(de);
            break;

        case CREATE_THREAD_DEBUG_EVENT:
            handle_CREATE_THREAD_DEBUG_EVENT(de);
            break;

        case EXIT_THREAD_DEBUG_EVENT:
            handle_EXIT_THREAD_DEBUG_EVENT(de);
            break;

        case EXIT_PROCESS_DEBUG_EVENT:
            handle_EXIT_PROCESS_DEBUG_EVENT(de);
            break;

        case LOAD_DLL_DEBUG_EVENT:
            handle_LOAD_DLL_DEBUG_EVENT (de);
            break;

        case UNLOAD_DLL_DEBUG_EVENT:
            handle_UNLOAD_DLL_DEBUG_EVENT (de);
            break;

        case OUTPUT_DEBUG_STRING_EVENT:
            return handle_OUTPUT_DEBUG_STRING_EVENT(de);

        case RIP_EVENT:
            L ("RIP_EVENT\n");
            break;

        default:
            assert(!"unknown dwDebugEventCode");
            break;
    };
    if (cycle_c_debug)
        L ("%s() end\n", __func__);
    return DBG_EXCEPTION_NOT_HANDLED; // default
};

void cycle()
{
    DEBUG_EVENT de;
    DWORD ContinueStatus;

    if (cycle_c_debug)
        L ("%s() begin\n", __func__);

    while (detaching==false)
        while (WaitForDebugEvent (&de, 1000)) // 1000ms. INFINITE shouldn't be here!
        {
            ContinueStatus=handle_debug_event(&de);
            if (rbtree_empty(processes))
            {
                if (cycle_c_debug)
                    L ("%s() no more processes to receive events from\n", __func__);
                goto exit; 
            };
            ContinueDebugEvent (de.dwProcessId, de.dwThreadId, ContinueStatus);
            if (detaching)
                clean_all_DRx();
        };

exit:
    if (cycle_c_debug)
        L ("%s() end\n", __func__);
};

/* vim: set expandtab ts=4 sw=4 : */
