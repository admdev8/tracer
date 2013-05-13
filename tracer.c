#include <assert.h>

#include "logging.h"
#include "dmalloc.h"
#include "stuff.h"
#include "rbtree.h"
#include "porg_utils.h"

#include "opts.h"
#include "cycle.h"
#include "thread.h"
#include "process.h"

rbtree *processes=NULL;

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

int main(int argc, char *argv[])
{
    int i;

    //dmalloc_break_at_seq_n (620);
    //
    if (argc==1)
        help_and_exit();

    for (i=1; i<argc; i++)
        parse_option(argv[i]);

    L_init ("tracer.log");
    debug_or_attach();
    processes=rbtree_create(true, "processes", compare_tetrabytes);
    cycle();

    rbtree_deinit(processes);

    if (breakpoints)
        obj_free(breakpoints);
    DFREE(load_filename);
    DFREE(attach_filename);
    DFREE(load_command_line);

    dump_unfreed_blocks();
    return 0;
};
