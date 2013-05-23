#include <assert.h>

#include "process.h"
#include "tracer.h"
#include "module.h"
#include "thread.h"
#include "logging.h"
#include "porg_utils.h"

bool process_c_debug=true;

void process_free (process *p)
{
    if (process_c_debug)
    {
        L ("%s() begin\n", __func__);
        L ("count of p->threads: %d\n", rbtree_count(p->threads));
        L ("count of p->modules: %d\n", rbtree_count(p->modules));
    };

    rbtree_foreach(p->threads, NULL, NULL, (void (*)(void*))thread_free);
    rbtree_foreach(p->modules, NULL, NULL, (void (*)(void*))module_free);
    rbtree_deinit(p->threads);
    rbtree_deinit(p->modules);
    DFREE(p->filename);
    DFREE(p->path);
    DFREE (p);
};

process *find_process(DWORD PID)
{
    process *p=(process*)rbtree_lookup(processes, (void*)PID);
    assert (p!=NULL && "PID not found in processes table");
    return p;
};

void process_resolve_path_and_filename_from_hdl(HANDLE file_hdl, process *p)
{
    strbuf fullpath_filename=STRBUF_INIT;

    if (GetFileNameFromHandle(file_hdl, &fullpath_filename))
    {
        strbuf sb_filename=STRBUF_INIT, sb_path=STRBUF_INIT;

        full_path_and_filename_to_path_only (&sb_path, fullpath_filename.buf);
        full_path_and_filename_to_filename_only (&sb_filename, fullpath_filename.buf);
        
        p->filename=strbuf_detach(&sb_filename, NULL);
        p->path=strbuf_detach(&sb_path, NULL);
        strbuf_deinit (&sb_filename);
        strbuf_deinit (&sb_path);
    }
    else
    {
        p->filename=DSTRDUP("?", "");
        p->path=DSTRDUP("?", "");
    };

    strbuf_deinit(&fullpath_filename);
};

module *find_module_for_address (process *p, address a)
{
    module *prev_v, *m=rbtree_lookup2 (p->modules, (void*)a, NULL, (void**)&prev_v, NULL, NULL);
    
    if (m) // a==base address of some module
        return m;
    else
    {
        assert (prev_v && "address $a$ is not in limits of any registered module");
        return prev_v;
    };
};

void process_get_sym (process *p, address a, strbuf *out)
{
    module *m=find_module_for_address (p, a);
    
    if (0 && process_c_debug)
        L ("%s() a=0x" PRI_ADR_HEX " m->base=0x " PRI_ADR_HEX " m->size=0x%x\n", __func__, a, m->base, m->size);
    
    module_get_sym (m, a, out);
};
