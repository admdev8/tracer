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
#include "oassert.h"
#include <stdio.h>
#include <stdbool.h>

#include "bitfields.h"
#include "dmalloc.h"
#include "logging.h"
#include "porg_utils.h"
#include "opts_aux.h"
#include "thread.h"
#include "module.h"
#include "rbtree.h"
#include "process.h"
#include "tracer.h"
#include "X86_emu.h"
#include "stuff.h"
#include "BP.h"
#include "BPF.h"
#include "BPX.h"
#include "BPM.h"
#include "x86.h"
#include "utils.h"
#include "bp_address.h"
#include "one_time_INT3_BP.h"
#include "SEH.h"
#include "fmt_utils.h"

bool detaching=false;

void detach()
{
    L ("Detaching...\n");
    detaching=true;
};

static bool handle_OEP_breakpoint (process *p, thread *t, MemoryCache *mc)
{
    if (cycle_c_debug)
        L ("%s() begin\n", __func__);

    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_ALL;
    BOOL B;
    B=GetThreadContext (t->THDL, &ctx); oassert (B!=FALSE);

    CONTEXT_decrement_PC(&ctx);
    address PC=CONTEXT_get_PC(&ctx);

    B=SetThreadContext (t->THDL, &ctx); oassert (B!=FALSE);

    bool b=MC_WriteByte (mc, PC, p->executable_module->saved_OEP_byte);
    oassert (b && "cannot restore original byte at OEP");

    if (load_filename)
        p->we_are_loading_and_OEP_was_executed=true;

    set_or_update_all_DRx_breakpoints(p); // for all threads! only DRx breakpoints set/updated!

    if (cycle_c_debug)
        L ("%s() end\n", __func__);

    return true;
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

    MemoryCache *mc=MC_MemoryCache_ctor (p->PHDL, true);

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
        b=GetThreadContext (t->THDL, &ctx); oassert (b!=FALSE);

        if (check_for_onetime_INT3_BP(p, t, adr, mc, tmp.buf, &ctx))
            rt=DBG_CONTINUE; // handled

        b=SetThreadContext (t->THDL, &ctx); oassert (b!=FALSE);
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
                BOOL B=GetThreadContext (t->THDL, &ctx); oassert (B!=FALSE);           
                MemoryCache *mc=MC_MemoryCache_ctor (p->PHDL, true);

                if (cycle_c_debug)
                {
                    L ("EXCEPTION_SINGLE_STEP %s (0x" PRI_ADR_HEX ") DR6=", tmp.buf, adr); 
                    dump_DR6 (&cur_fds, ctx.Dr6); L(" (0x%x)\n", ctx.Dr6);
                    //L ("DR7="); dump_DR7 (&cur_fds, ctx.Dr7); L("\n");
                    //L ("DR0=0x" PRI_REG_HEX "\n", ctx.Dr0);
                    L ("ctx before handle_Bx:\n");
                    dump_CONTEXT (&cur_fds, &ctx, dump_fpu, false /*DRx?*/, dump_xmm);
                };

                handle_Bx (p, t, &ctx, mc);

                MC_Flush (mc);
                MC_MemoryCache_dtor (mc, false);
                if (cycle_c_debug)
                {
                    L ("ctx before writing:\n");
                    dump_CONTEXT (&cur_fds, &ctx, dump_fpu, false /*DRx?*/, dump_xmm);
                };
                B=SetThreadContext (t->THDL, &ctx); oassert (B!=FALSE);
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
                tmpd=GetThreadContext (t->THDL, &ctx); oassert (tmpd!=FALSE);           

                dump_CONTEXT (&cur_fds, &ctx, dump_fpu, false /* dump_DRx */, dump_xmm);
                MemoryCache *mc=MC_MemoryCache_ctor (p->PHDL, true);
    
                if (dump_seh)
                    dump_SEH_chain (&cur_fds, p, t, &ctx, mc);

                MC_MemoryCache_dtor (mc, false);
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
    oassert(b && "can't read byte at breakpoint start");
    b=MC_WriteByte (mc, m->OEP, 0xCC);
    oassert(b && "can't write 0xCC byte at breakpoint start");
};

void handle_CREATE_PROCESS_DEBUG_EVENT(DEBUG_EVENT *de)
{
    CREATE_PROCESS_DEBUG_INFO *i=&de->u.CreateProcessInfo;
    DWORD PID=de->dwProcessId, TID=de->dwThreadId;

    if (cycle_c_debug)
        L ("%s() begin\n", __func__);

    process* p=process_init (PID, i->hProcess, i->hFile, i->lpBaseOfImage);
    
    add_thread (p, TID, i->hThread, (address)i->lpStartAddress, (address)i->lpThreadLocalBase);
    rbtree_insert(processes, (void*)PID, p);
    
    MemoryCache *mc=MC_MemoryCache_ctor (p->PHDL, true);

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

    add_thread (p, de->dwThreadId, i->hThread, (address)i->lpStartAddress, (address)i->lpThreadLocalBase);
    set_or_update_all_DRx_breakpoints(p); // overkill...
};

void handle_LOAD_DLL_DEBUG_EVENT (DEBUG_EVENT *de)
{
    LOAD_DLL_DEBUG_INFO *i=&de->u.LoadDll;
    DWORD PID=de->dwProcessId;
    process *p=find_process(PID);
    strbuf sb=STRBUF_INIT;

    bool b=GetFileNameFromHandle(de->u.LoadDll.hFile, &sb);
    oassert (b);

    if (cycle_c_debug)
        L ("%s() LOAD_DLL_DEBUG_EVENT: %s 0x%x\n", __func__, sb.buf, i->lpBaseOfDll);
    strbuf_deinit (&sb);
    
    MemoryCache *mc=MC_MemoryCache_ctor (p->PHDL, true);

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
    oassert (de->u.DebugString.fUnicode==0); // TODO
    MemoryCache* mc=MC_MemoryCache_ctor(p->PHDL, true);
    bool b=MC_ReadBuffer(mc, (address)de->u.DebugString.lpDebugStringData, de->u.DebugString.nDebugStringLength, (BYTE*)buf);
    oassert (b);

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
            oassert(!"unknown dwDebugEventCode");
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
