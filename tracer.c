#include <assert.h>

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

rbtree *processes=NULL; // PID, ptr to process

bool tracer_c_debug=true;

void dump_PID_if_need(process *p)
{
   if (rbtree_count(processes)>1)
       L ("PID=%d|", p->PID);
};

void dump_TID_if_need(process *p, thread *t)
{
   if (rbtree_count(p->threads)>1)
       L ("TID=%d|", t->TID);
};

void help_and_exit()
{
    printf ("help...\n");
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

void debug_or_attach()
{
    if (load_filename)
    {
        load_process();
    } else if (attach_filename)
    {
        if (load_command_line)
            printf ("Command line option will be ignored...\n");
        assert(!"not implemented");
    } else if (attach_PID!=-1)
    {
        if (load_command_line)
            printf ("Command line option will be ignored...\n");
        assert(!"not implemented");
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
        if (OEP_breakpoint)
            die ("OEP breakpoint is useless without -l option\n");
    };
};

void set_or_update_DRx_breakpoint(BP *bp, CONTEXT *ctx, unsigned DRx_no)
{
    if (tracer_c_debug)
    {
        strbuf sb=STRBUF_INIT;
        address_to_string(bp->a, &sb);
        L ("%s(): setting DRx-breakpoint %d for %s\n", __func__, DRx_no, sb.buf);
        strbuf_deinit (&sb);
    };

    CONTEXT_setDRx_and_DR7 (ctx, DRx_no, bp->a->abs_address);
};

void set_or_update_all_breakpoints(process *p)
{
    if (tracer_c_debug)
        L ("%s() begin\n", __func__);

    // enum all breakpoints, pick out a->resolved ones
    for (unsigned DRx_no=0; DRx_no<4; DRx_no++)
    {
        BP *bp=DRx_breakpoints[DRx_no];
        if (bp)
            L ("%s() DRx_breakpoints[%d]=0x%p\n", __func__, DRx_no, bp);
        else
            continue;

        //dump_BP (bp);

        if (bp->a->resolved==false)
            continue;

        if (p->we_are_loading_and_OEP_was_executed==false)
            continue;
        for (struct rbtree_node_t *_t=rbtree_minimum(p->threads); _t; _t=rbtree_succ(_t))
        {
            thread *t=(thread*)(_t->value);
            CONTEXT ctx;
            ctx.ContextFlags = CONTEXT_ALL;
            DWORD rt;
            rt=GetThreadContext (t->THDL, &ctx); assert (rt!=FALSE);      

            set_or_update_DRx_breakpoint(bp, &ctx, DRx_no);

            rt=SetThreadContext (t->THDL, &ctx); assert (rt!=FALSE);
        };
    };
};

int main(int argc, char *argv[])
{
    //dmalloc_break_at_seq_n (102312);
    //
    if (argc==1)
        help_and_exit();

    for (int i=1; i<argc; i++)
        parse_option(argv[i]);

    check_option_constraints();

    L_init ("tracer.log");

    for (int i=0; i<4; i++)
        L ("DRx_breakpoints[%d]=0x%p\n", i, DRx_breakpoints[i]);

    debug_or_attach();
    processes=rbtree_create(true, "processes", compare_tetrabytes);
    cycle();

    // any left processes?
    rbtree_foreach(processes, NULL, NULL, (void(*)(void*))process_free);

    rbtree_deinit(processes);

    dlist_free(addresses_to_be_resolved, NULL);
    BP_free(OEP_breakpoint);
    for (unsigned i=0; i<4; i++)
        BP_free(DRx_breakpoints[i]);

    DFREE(load_filename);
    DFREE(attach_filename);
    DFREE(load_command_line);

    dump_unfreed_blocks();
    return 0;
};
