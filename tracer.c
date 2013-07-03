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
#include <signal.h>

#include "files.h"
#include "fuzzybool.h"
#include "logging.h"
#include "dmalloc.h"
#include "stuff.h"
#include "rbtree.h"
#include "porg_utils.h"
#include "memorycache.h"
#include "opts.h"
#include "cycle.h"
#include "thread.h"
#include "process.h"
#include "CONTEXT_utils.h"
#include "utils.h"
#include "BP.h"
#include "bp_address.h"
#include "tracer.h"
#include "rand.h"

rbtree *processes=NULL; // PID, ptr to process

bool tracer_c_debug=false;

void help_and_exit()
{
    printf ("--dump-fpu-never: never dump FPU registers\n");
    printf ("--dump-xmm:       dump MMX/XMM registers\n");
    printf ("\n");
    printf ("BPF options:\n");
    printf ("\n");
    printf ("pause:<number> - make delay in milliseconds at each breakpoint\n");
    printf ("\n");
    printf ("rt_probability:<number> - rt option will trigger in some probability,\n");
    printf ("defined as [0..1] float number or as [0%..100%] percentage\n");
    printf ("\n");
    exit(0);
};

void load_process()
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    strbuf cmd_line=STRBUF_INIT;
    DWORD flags;

    strbuf_addstr (&cmd_line, load_filename);

    if (load_command_line)
    {
        strbuf_addc (&cmd_line, ' ');
        strbuf_addstr (&cmd_line, load_command_line);
    };

    GetStartupInfo (&si);

    if (debug_children)
        flags=DEBUG_PROCESS | CREATE_NEW_CONSOLE;
    else
        flags=DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE;

    if (CreateProcess (load_filename, (LPSTR)cmd_line.buf, 0, 0, 0, flags, 0, 0, &si, &pi)==FALSE)
    {
        if (GetLastError()==ERROR_ELEVATION_REQUIRED)
            die ("UAC issues: run 'as Administrator'\n");
        die_GetLastError ("CreateProcess failed");
    };
    strbuf_deinit (&cmd_line);
};

void attach_process(obj *PIDs)
{
    for (obj *i=PIDs; i; i=cdr(i))
    {
        if (DebugActiveProcess (obj_get_as_tetrabyte(car(i)))==FALSE)
            die_GetLastError ("DebugActiveProcess() failed\n");
    };
};

void clean_all_DRx()
{
    if (tracer_c_debug)
        L ("%s()\n", __func__);

    for (struct rbtree_node_t *_p=rbtree_minimum(processes); _p; _p=rbtree_succ(_p))
    {
        process *p=(process*)(_p->value);

        for (struct rbtree_node_t *_t=rbtree_minimum(p->threads); _t; _t=rbtree_succ(_t))
        {
            thread *t=(thread*)(_t->value);
            CONTEXT ctx;
            ctx.ContextFlags = CONTEXT_ALL;
            DWORD tmpd;
            tmpd=GetThreadContext (t->THDL, &ctx); assert (tmpd!=FALSE);           

            ctx.Dr0=ctx.Dr1=ctx.Dr2=ctx.Dr3=ctx.Dr7=0;

            tmpd=SetThreadContext (t->THDL, &ctx); assert (tmpd!=FALSE);
        };
    };
};

void debug_or_attach()
{
    obj* attach_PIDs=NULL;

    if (load_filename)
    {
        load_process();
    } else if (attach_filename)
    {
        attach_PIDs=FindProcessByName(attach_filename);
        if (attach_PIDs==NULL)
            die ("Can't find process named %s\n", attach_filename);
        attach_process(attach_PIDs);
        obj_free(attach_PIDs);
    } else if (attach_PID!=-1)
    {
        attach_PIDs=cons(obj_tetrabyte(attach_PID), NULL);
        attach_process(attach_PIDs);
        obj_free(attach_PIDs);
    }
    else
        die ("No program specified for load or attach\n");
};

void check_option_constraints()
{
    if (load_filename && (attach_filename || attach_PID!=-1))
        die ("Load process and attach process options shouldn't be mixed\n");
    if (load_filename==NULL)
    {
        if (load_command_line)
            die ("-c options useless without -l option\n");
        if (debug_children)
            die ("--child option useless without -l option\n");
    };
};

static void print_stack_for_all_processes_and_threads()
{
    for (struct rbtree_node_t *_p=rbtree_minimum(processes); _p; _p=rbtree_succ(_p))
    {
        process *p=(process*)(_p->value);

        for (struct rbtree_node_t *_t=rbtree_minimum(p->threads); _t; _t=rbtree_succ(_t))
        {
            thread *t=(thread*)(_t->value);
            CONTEXT ctx;
            ctx.ContextFlags = CONTEXT_ALL;
            DWORD tmpd;
            tmpd=GetThreadContext (t->THDL, &ctx); 
            if (tmpd==FALSE)
                die_GetLastError ("GetThreadContext() failed\n");

            MemoryCache *mc=MC_MemoryCache_ctor (p->PHDL, false);

            if (rbtree_count(processes)>1)
                L ("PID=%d\n", p->PID);
            if (rbtree_count(p->threads)>1)
                L ("TID=%d\n", t->TID);
            dump_stack (p, t, &ctx, mc);

            MC_MemoryCache_dtor(mc, true);
        };
    };
};

static void print_DRx_values_for_all_processes_and_threads()
{
    for (struct rbtree_node_t *_p=rbtree_minimum(processes); _p; _p=rbtree_succ(_p))
    {
        process *p=(process*)(_p->value);

        for (struct rbtree_node_t *_t=rbtree_minimum(p->threads); _t; _t=rbtree_succ(_t))
        {
            thread *t=(thread*)(_t->value);
            CONTEXT ctx;
            ctx.ContextFlags = CONTEXT_ALL;
            DWORD tmpd;
            tmpd=GetThreadContext (t->THDL, &ctx); 
            if (tmpd==FALSE)
                die_GetLastError ("GetThreadContext() failed\n");

            if (rbtree_count(processes)>1)
                L ("PID=%d\n", p->PID);
            if (rbtree_count(p->threads)>1)
                L ("TID=%d\n", t->TID);

            dump_DRx (&cur_fds, &ctx);
        };
    };
};

static void WINAPI thread_B (DWORD param) 
{
    HANDLE hConsoleInput=GetStdHandle (STD_INPUT_HANDLE);

    if (hConsoleInput!=INVALID_HANDLE_VALUE)
    {
        while (TRUE)
        {
            DWORD inp_read;
            INPUT_RECORD inp_record;
            if (ReadConsoleInput (hConsoleInput, &inp_record, 1, &inp_read)!=FALSE)
            {
                if (inp_record.EventType==KEY_EVENT)
                    if (inp_record.Event.KeyEvent.bKeyDown==FALSE)
                    {
                        switch (inp_record.Event.KeyEvent.wVirtualKeyCode)
                        {
                            case VK_ESCAPE:
                                L ("ESC pressed...\n");
                                detach();
                                break;

                            case VK_SPACE:
                                print_stack_for_all_processes_and_threads();
                                break;

                            case VK_F1:
                                print_DRx_values_for_all_processes_and_threads();
                                break;
                        };
                    };
            }
            else
                return;
        };
    }
    else
        return;
};

static void __cdecl signal_handler (int signo)
{
    if (signo==SIGINT)
    {
        L ("Ctrl-C or Ctrl-Break pressed\n");
        detach();
    };
};

static DWORD thread_B_id;
static HANDLE thread_B_handle;
BOOL (WINAPI * DebugActiveProcessStop_ptr)(DWORD pid)=NULL;

void detach_from_all_processes()
{
    if (tracer_c_debug)
        L ("%s() begin\n", __func__);

    for (struct rbtree_node_t *_p=rbtree_minimum(processes); _p; _p=rbtree_succ(_p))
    {
        process *p=(process*)(_p->value);

        if (DebugActiveProcessStop_ptr!=NULL)
        {
            if ((*DebugActiveProcessStop_ptr) (p->PID)==FALSE)
                die_GetLastError ("DebugActiveProcessStop() failed");
        }
        else
        {
            L ("kernel32.dll!DebugActiveProcessStop() was not found, we have to kill process (PID=%d)\n", p->PID);
            BOOL b=TerminateProcess (p->PHDL, 0);
            assert (b);
        };
    };
};

strbuf ORACLE_HOME;
int oracle_version=-1; // -1 mean 'unknown'

void set_ORACLE_HOME()
{
    strbuf_init (&ORACLE_HOME, 0);
    char *tmp=getenv("ORACLE_HOME");
    if (tmp==NULL)
        return;
    L ("ORACLE_HOME is set to [%s]\n", tmp);

    strbuf_addstr(&ORACLE_HOME, tmp);
    if (strbuf_last_char(&ORACLE_HOME)!='\\')
        strbuf_addc(&ORACLE_HOME, '\\');

    strbuf tmp2;

    strbuf_init(&tmp2, 0);
    strbuf_addf (&tmp2, "%sBIN\\oravsn11.dll", ORACLE_HOME.buf);
    if (file_exist(tmp2.buf))
    {
        L ("Oracle RDBMS version 11.x\n");
        oracle_version=11;
    };
    strbuf_deinit(&tmp2);

    strbuf_init(&tmp2, 0);
    strbuf_addf (&tmp2, "%sBIN\\oravsn10.dll", ORACLE_HOME.buf);
    if (file_exist(tmp2.buf))
    {
        L ("Oracle RDBMS version 10.x\n");
        oracle_version=10;
    };
    strbuf_deinit(&tmp2);

    strbuf_init(&tmp2, 0);
    strbuf_addf (&tmp2, "%sBIN\\oravsn9.dll", ORACLE_HOME.buf);
    if (file_exist(tmp2.buf))
    {
        L ("Oracle RDBMS version 9.x\n");
        oracle_version=9;
    };
    strbuf_deinit(&tmp2);

    if (oracle_version==-1)
        L ("Warning: Oracle RDBMS version wasn't determined\n");
};

trace_skip_element *trace_skip_options=NULL;

bool load_cfg(const char *fname)
{
    FILE *f;
    const char* CFG_PAT="^trace_skip=([^!; ]*)!([^!; ]*)!([^!; ]*)([[:space:]])?(;.*)?$";
    regex_t trace_skip_pat;
    bool rt;

    regcomp_or_die(&trace_skip_pat, CFG_PAT, REG_EXTENDED | REG_ICASE | REG_NEWLINE);

    f=fopen (fname, "rt");
    if (f==NULL) // file absent
    {
        rt=false;
        goto exit;
    };

    char buf[1024];
    while(fgets(buf, 1024, f)!=NULL)
    {
        regmatch_t matches[4];

        //printf ("buf=[%s]\n", buf);
        if (regexec (&trace_skip_pat, buf, 4, matches, 0)==0)
        {
            strbuf opt1=STRBUF_INIT;
            strbuf opt2=STRBUF_INIT;
            strbuf opt3=STRBUF_INIT;
            trace_skip_element *tmp;

            strbuf_addstr_range_be(&opt1, buf, matches[1].rm_so, matches[1].rm_eo);
            strbuf_addstr_range_be(&opt2, buf, matches[2].rm_so, matches[2].rm_eo);
            strbuf_addstr_range_be(&opt3, buf, matches[3].rm_so, matches[3].rm_eo);

            env_vars_expansion(&opt1);

            tmp=DCALLOC(trace_skip_element, 1, "trace_skip_element");

            regcomp_or_die(&tmp->re_path, opt1.buf, REG_EXTENDED | REG_ICASE | REG_NEWLINE);
            regcomp_or_die(&tmp->re_module, opt2.buf, REG_EXTENDED | REG_ICASE | REG_NEWLINE);
            regcomp_or_die(&tmp->re_function, opt3.buf, REG_EXTENDED | REG_ICASE | REG_NEWLINE);

            if (strcmp (opt3.buf, ".*")==0)
                tmp->is_function_wildcard=true;

            if (trace_skip_options==NULL)
                trace_skip_options=tmp;
            else
            {
                trace_skip_element *i;
                for (i=trace_skip_options; i->next!=NULL; i=i->next); // find last element
                i->next=tmp; // add new element
            };
            strbuf_deinit(&opt1);
            strbuf_deinit(&opt2);
            strbuf_deinit(&opt3);
        };
    };
    int t_i=fclose (f);
    assert (t_i==0);
    rt=true;

exit:
    regfree(&trace_skip_pat);
    return rt;
};

void free_trace_skip_options(trace_skip_element *i)
{
    if (i==NULL)
        return;
    regfree(&i->re_path);
    regfree(&i->re_module);
    regfree(&i->re_function);
    free_trace_skip_options(i->next);
    DFREE(i);
};

#ifdef _DEBUG
int opts_test(); // opts_test.c
#endif

int main(int argc, char *argv[])
{
    printf ("tracer 0.7 %s by Dennis Yurichev\n", 
#ifdef _WIN64
            "WIN64"
#else
            "WIN32"
#endif
            );

    if (argc==1)
        help_and_exit();

#ifdef _DEBUG
    if (argc==2 && stricmp (argv[1], "opts_test")==0)
    {
        opts_test();
        return 0;
    };
#endif

    for (int i=1; i<argc; i++)
        parse_option(argv[i]);

    check_option_constraints();

    if (quiet==false)
        L_init ("tracer.log"); // if not initialized, all messages are suppressed

    if (load_cfg("tracer.cfg")==false)
        L ("Warning: no tracer.cfg file.\n");
    
    sgenrand(GetTickCount()); // for BPF rt_probability option

    set_ORACLE_HOME();

#if 0
    for (int i=0; i<4; i++)
        L ("DRx_breakpoints[%d]=0x%p\n", i, breakpoints[i]);
#endif

    EnableDebugPrivilege (TRUE);

    debug_or_attach();
    processes=rbtree_create(true, "processes", compare_tetrabytes);
   
    if (run_thread_b && IsDebuggerPresent()==FALSE) // do not start thread B if gdb is used...
    {
        thread_B_handle=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_B, (PVOID)0, 0, &thread_B_id);
        if (thread_B_handle==NULL)
            die ("Cannot create thread B");
    };
    signal(SIGINT, &signal_handler);
    
    DebugActiveProcessStop_ptr=(BOOL (WINAPI *)(DWORD))(GetProcAddress (LoadLibrary ("kernel32.dll"), "DebugActiveProcessStop"));
    if (DebugActiveProcessStop_ptr==NULL)
        L ("DebugActiveProcessStop() was not found in your kernel32.dll. Detach wouldn't be possible.\n");

    cycle();

    detach_from_all_processes();

    // any left processes?
    rbtree_foreach(processes, NULL, NULL, (void(*)(void*))process_free);

    rbtree_deinit(processes);

    free_trace_skip_options(trace_skip_options);

    dlist_free(addresses_to_be_resolved, NULL);
    for (unsigned i=0; i<4; i++)
        BP_free(breakpoints[i]);

    DFREE(load_filename);
    DFREE(attach_filename);
    DFREE(load_command_line);
    if (dump_all_symbols_re)
    {
        regfree (dump_all_symbols_re);
        DFREE(dump_all_symbols_re);
    };

    strbuf_deinit(&ORACLE_HOME);

    L_deinit();

    dump_unfreed_blocks();
    return 0;
};
