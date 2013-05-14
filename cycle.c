#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

#include "dmalloc.h"
#include "logging.h"
#include "porg_utils.h"
#include "opts.h"
#include "thread.h"
#include "module.h"
#include "rbtree.h"
#include "process.h"
#include "tracer.h"

bool cycle_c_debug=true;

DWORD handle_EXCEPTION_DEBUG_INFO(DEBUG_EVENT *de)
{
    EXCEPTION_DEBUG_INFO *e=&de->u.Exception;
    switch (e->ExceptionRecord.ExceptionCode)
    {
        case EXCEPTION_SINGLE_STEP:
            L ("EXCEPTION_SINGLE_STEP\n");
            break;
        case EXCEPTION_BREAKPOINT:
            L ("EXCEPTION_BREAKPOINT\n");
            break;
        case EXCEPTION_ACCESS_VIOLATION:
            L ("EXCEPTION_ACCESS_VIOLATION\n");
            break;
        default:
            L ("unknown ExceptionCode: %ld\n", e->ExceptionRecord.ExceptionCode);
            break;
    };
    return DBG_EXCEPTION_NOT_HANDLED; // default
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
    process_resolve_path_and_filename_from_hdl(i->hFile, p);
    L ("(name=%s)\n", p->filename);
    
    p->modules=rbtree_create(true, "modules", compare_size_t); // compare_REGs?
    p->threads=rbtree_create(true, "threads", compare_tetrabytes);
    add_thread (p, TID, i->hThread, (address)i->lpStartAddress);
    rbtree_insert(processes, (void*)PID, p);

    add_module(p, (address)i->lpBaseOfImage, p->file_handle);

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

    L ("EXIT_PROCESS_DEBUG_EVENT. ExitCode=%d (0x%x)\n", i->dwExitCode);
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

    while (WaitForDebugEvent (&de, 1000)) // 1000ms. INFINITE shouldn't be here!
    {
        ContinueStatus=handle_debug_event(&de);
        if (rbtree_empty(processes))
        {
            if (cycle_c_debug)
                L ("%s() no more processes to receive events from\n", __func__);
            break; 
        };
        ContinueDebugEvent (de.dwProcessId, de.dwThreadId, ContinueStatus);
    };
    
    if (cycle_c_debug)
        L ("%s() end\n", __func__);
};

