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
#include "x86.h"
#include "utils.h"
#include "bp_address.h"

bool cycle_c_debug=false;

void handle_BP(process *p, thread *t, int bp_no, CONTEXT *ctx, MemoryCache *mc)
{
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
        default:
            assert(0);
    };
    L ("%s() end\n", __func__);
};

bool handle_OEP_breakpoint (process *p, thread *t)
{
    if (cycle_c_debug)
        L ("%s() begin\n", __func__);

    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_ALL;
    DWORD rt;
    rt=GetThreadContext (t->THDL, &ctx); assert (rt!=FALSE);
    MemoryCache *mc=MC_MemoryCache_ctor (p->PHDL, false);

    CONTEXT_decrement_PC(&ctx);
    rt=SetThreadContext (t->THDL, &ctx); assert (rt!=FALSE);

    bool b=MC_WriteByte (mc, CONTEXT_get_PC(&ctx), p->executable_module->saved_OEP_byte);
    assert (b && "cannot restore original byte at OEP");

    if (load_filename)
        p->we_are_loading_and_OEP_was_executed=true;

    set_or_update_all_DRx_breakpoints(p); // for all threads! only DRx breakpoints set/updated!

    MC_Flush (mc);
    MC_MemoryCache_dtor (mc, true);

    if (cycle_c_debug)
        L ("%s() end\n", __func__);

    return true;
};

void handle_Bx (process *p, thread *t, CONTEXT *ctx, MemoryCache *mc)
{
    L ("%s() begin\n", __func__);
    if (IS_SET(ctx->Dr6, FLAG_DR6_B0))
    {
        assert (breakpoints[0]);
        handle_BP(p, t, 0, ctx, mc);
    };
    
    if (IS_SET(ctx->Dr6, FLAG_DR6_B1))
    {
        assert (breakpoints[1]);
        handle_BP(p, t, 1, ctx, mc);
    };
    
    if (IS_SET(ctx->Dr6, FLAG_DR6_B2))
    {
        assert (breakpoints[2]);
        handle_BP(p, t, 2, ctx, mc);
    };

    if (IS_SET(ctx->Dr6, FLAG_DR6_B3))
    {
        assert (breakpoints[3]);
        handle_BP(p, t, 3, ctx, mc);
    };

    if (IS_SET(ctx->Dr6, FLAG_DR6_BS))
    {
        assert (t->tracing && "BS flag in DR6, but no breakpoint in tracing mode");
        handle_BP(p, t, t->tracing_bp, ctx, mc);
    };
/*
    if (ctx->Dr6==0) // sometimes observed while tracing
    {
        assert (t->tracing && "DR6 is zero, but no breakpoint in tracing mode");
        handle_BP(p, t, t->tracing_bp, ctx, mc);
    };
*/
    L ("%s() end\n", __func__);
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
                process_get_sym (p, adr, true, &tmp);
                CONTEXT ctx;
                ctx.ContextFlags = CONTEXT_ALL;
                DWORD tmpd;
                tmpd=GetThreadContext (t->THDL, &ctx); assert (tmpd!=FALSE);           
                MemoryCache *mc=MC_MemoryCache_ctor (p->PHDL, false);

                L ("EXCEPTION_SINGLE_STEP %s (0x" PRI_ADR_HEX ") DR6=", tmp.buf, adr); 
                dump_DR6 (&cur_fds, ctx.Dr6); L(" (0x%x)\n", ctx.Dr6);
                L ("DR7="); dump_DR7 (&cur_fds, ctx.Dr7); L("\n");
                L ("DR0=0x" PRI_REG_HEX "\n", ctx.Dr0);
                
                handle_Bx (p, t, &ctx, mc);

                MC_Flush (mc);
                MC_MemoryCache_dtor (mc, false);
                tmpd=SetThreadContext (t->THDL, &ctx); assert (tmpd!=FALSE);
                strbuf_deinit(&tmp);
                rt=DBG_CONTINUE; // handled
            };
            break;
        case EXCEPTION_BREAKPOINT:
            {
                strbuf tmp=STRBUF_INIT;
                process_get_sym (p, adr, true, &tmp);
                L ("EXCEPTION_BREAKPOINT %s (0x" PRI_ADR_HEX ")\n", tmp.buf, adr);

                if (stricmp(tmp.buf, "ntdll.dll!DbgBreakPoint")==0)
                {
                    L ("We handle this\n");
                    rt=DBG_CONTINUE;
                };

                // we are loading and is it OEP?
                if (rt==DBG_EXCEPTION_NOT_HANDLED && load_filename && (adr==p->executable_module->OEP))
                {
                    handle_OEP_breakpoint (p, t);
                    rt=DBG_CONTINUE; // handled
                };

                if (rt==DBG_EXCEPTION_NOT_HANDLED)
                    L ("Warning: unknown (to us) INT3 breakpoint\n");
                strbuf_deinit(&tmp);
            };
            break;
        case EXCEPTION_ACCESS_VIOLATION:
            {
                L ("EXCEPTION_ACCESS_VIOLATION at 0x" PRI_ADR_HEX " ExceptionInformation[0]=%d\n",
                        (address)de->u.Exception.ExceptionRecord.ExceptionAddress,
                        de->u.Exception.ExceptionRecord.ExceptionInformation[0]);
                CONTEXT ctx;
                ctx.ContextFlags = CONTEXT_ALL;
                DWORD tmpd;
                tmpd=GetThreadContext (t->THDL, &ctx); assert (tmpd!=FALSE);           

                dump_CONTEXT (&cur_fds, &ctx, false, false, false);
            };

            break;
        default:
            L ("unknown ExceptionCode: %ld\n", er->ExceptionCode);
            break;
    };
    return rt;
};

void save_OEP_byte_and_set_INT3_breakpoint (MemoryCache *mc, module *m)
{
    bool b;
    
    b=MC_ReadByte (mc, m->OEP, &m->saved_OEP_byte);
    assert(b && "can't read byte at breakpoint start");
    b=MC_WriteByte (mc, m->OEP, 0xCC);
    assert(b && "can't write 0xCC byte at breakpoint start");
};

void handle_CREATE_PROCESS_DEBUG_EVENT(DEBUG_EVENT *de)
{
    CREATE_PROCESS_DEBUG_INFO *i=&de->u.CreateProcessInfo;
    DWORD PID=de->dwProcessId, TID=de->dwThreadId;
    process *p=DCALLOC (process, 1, "process");

    if (cycle_c_debug)
        L ("%s() begin\n", __func__);
    p->PID=PID;
    p->PHDL=i->hProcess;
    p->file_handle=i->hFile;
    p->base_of_image=(address)i->lpBaseOfImage;
    
    p->modules=rbtree_create(true, "modules", compare_size_t); // compare_REGs?
    p->threads=rbtree_create(true, "threads", compare_tetrabytes);
    add_thread (p, TID, i->hThread, (address)i->lpStartAddress);
    rbtree_insert(processes, (void*)PID, p);

    p->executable_module=add_module(p, (address)i->lpBaseOfImage, p->file_handle);
    L ("(name=%s)\n", p->executable_module->filename);

    MemoryCache *mc=MC_MemoryCache_ctor (p->PHDL, false);

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

    L ("CREATE_THREAD_DEBUG_EVENT\n");
    
    add_thread (p, de->dwThreadId, i->hThread, (address)i->lpStartAddress);
};

void handle_LOAD_DLL_DEBUG_EVENT (DEBUG_EVENT *de)
{
    LOAD_DLL_DEBUG_INFO *i=&de->u.LoadDll;
    DWORD PID=de->dwProcessId;
    process *p=find_process(PID);
    strbuf sb=STRBUF_INIT;

    bool b=GetFileNameFromHandle(de->u.LoadDll.hFile, &sb);
    assert (b);

    L ("LOAD_DLL_DEBUG_EVENT: %s 0x%x\n", sb.buf, i->lpBaseOfDll);
    strbuf_deinit (&sb);
    
    add_module (p, (address)i->lpBaseOfDll, i->hFile);
};

void handle_UNLOAD_DLL_DEBUG_EVENT (DEBUG_EVENT *de)
{
    UNLOAD_DLL_DEBUG_INFO *i=&de->u.UnloadDll;
    DWORD PID=de->dwProcessId;
    process *p=find_process(PID);
    L ("UNLOAD_DLL_DEBUG_EVENT 0x%x\n", i->lpBaseOfDll);
    remove_module (p, (address)i->lpBaseOfDll);
};

void handle_EXIT_THREAD_DEBUG_EVENT(DEBUG_EVENT *de)
{
    DWORD PID=de->dwProcessId, TID=de->dwThreadId;
    process *p;
    thread *t=find_thread(PID, TID);

    p=find_process(PID);

    L ("EXIT_THREAD_DEBUG_EVENT\n");
    thread_free (t);
    rbtree_delete (p->threads, (void*)TID);
};

void handle_EXIT_PROCESS_DEBUG_EVENT(DEBUG_EVENT *de)
{
    EXIT_PROCESS_DEBUG_INFO *i=&de->u.ExitProcess;
    DWORD PID=de->dwProcessId;
    process *p=find_process(PID);

    L ("EXIT_PROCESS_DEBUG_EVENT. ExitCode=%d (0x%x)\n", i->dwExitCode, i->dwExitCode);
    process_free (p);
    rbtree_delete (processes, (void*)PID);
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
            L ("OUTPUT_DEBUG_STRING_EVENT\n");
            assert (!"not implemented");
            break;

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

