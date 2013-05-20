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

rbtree *processes=NULL; // PID, ptr to process

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
    };
};

void add_OEP_bp_if_we_loading ()
{
    if (load_filename==NULL)
        return;

    // are there OEP option? enum all breakpoints, search for BPF/BPX-type with filename=ours and address=OEP
    if (is_there_OEP_breakpoint_for_fname(load_filename)) // FIXME: cut path if needed
    {
        L ("is_there_OEP_breakpoint_for_fname(%s) returned true\n", load_filename);
        return;
    };

    L ("adding (hidden) OEP breakpoint\n");
    // if not, add one (hidden)
    BPF *OEP_bpf=DCALLOC (BPF, 1, "OEP_BPF");
    bp_address *OEP_a=create_address_filename_symbol_re(load_filename, "OEP", 0);
    OEP_bpf->hidden=true;
    BP *new_bp=create_BP(BP_type_BPF, OEP_a, OEP_bpf);
    new_bp->INT3_style=true;
    add_new_BP (new_bp);
    add_new_address_to_be_resolved(OEP_a);
};

void set_breakpoint(process *p, BP *bp)
{
    bool b;

    assert (bp->a->resolved);

    printf ("setting INT3-breakpoint for ");
    dump_address(bp->a);
    printf ("\n");

    if (bp->INT3_style) // always set these breakpoints
    {
        assert (bp->t != BP_type_BPM);

        if (bp->ins==NULL) // not yet set
        {
            MemoryCache *mc=MC_MemoryCache_ctor (p->PHDL, false);

            bp->ins=Da_Da_callbacks (Fuzzy_Undefined, bp->a->abs_address, 
                     (bool (*)(void*, disas_address, uint8_t*))MC_ReadByte, 
                     (bool (*)(void*, disas_address, uint16_t*))MC_ReadWyde, 
                     (bool (*)(void*, disas_address, uint32_t*))MC_ReadTetrabyte, 
                     (bool (*)(void*, disas_address, uint64_t*))MC_ReadOctabyte, 
                     (void*)mc);

            assert (bp->ins && "can't disassemble instruction at breakpoint start");

            b=MC_ReadByte (mc, bp->a->abs_address, &bp->saved_byte);
            assert(b && "can't read byte at breakpoint start");
            b=MC_WriteByte (mc, bp->a->abs_address, 0xCC);
            assert(b && "can't write 0xCC byte at breakpoint start");
            MC_Flush (mc);
            MC_MemoryCache_dtor (mc, false);
        };
    }
    else if (p->we_are_loading_and_OEP_was_executed)
    {
        // set DRx breakpoints
        assert (!"not implemented");
    };
};

void set_all_breakpoints(process *p)
{
    // enum all breakpoints, pick out a->resolved ones
    for (BP* b=breakpoints; b; b=b->next)
    {
        dump_BP (b);
        if (b->a->resolved)
            set_breakpoint(p, b);
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

    add_OEP_bp_if_we_loading();

    debug_or_attach();
    processes=rbtree_create(true, "processes", compare_tetrabytes);
    cycle();

    // any left processes?
    rbtree_foreach(processes, NULL, NULL, (void(*)(void*))process_free);

    rbtree_deinit(processes);

    dlist_free(addresses_to_be_resolved, NULL);
    free_all_BPs(breakpoints);
    DFREE(load_filename);
    DFREE(attach_filename);
    DFREE(load_command_line);

    dump_unfreed_blocks();
    return 0;
};
