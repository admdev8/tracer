#include <assert.h>

#include "process.h"
#include "tracer.h"
#include "module.h"
#include "thread.h"
#include "logging.h"
#include "porg_utils.h"

bool process_c_debug=false;

void process_free (process *p)
{
    if (process_c_debug)
    {
        L ("%s() begin\n", __func__);
        L ("count of p->threads: %d\n", rbtree_count(p->threads));
        L ("count of p->modules: %d\n", rbtree_count(p->modules));
    };

    rbtree_foreach(p->threads, NULL, NULL, (void (*)(void*))thread_free);
    rbtree_foreach(p->modules, NULL, NULL, (void (*)(void*))unload_module_and_free);
    rbtree_deinit(p->threads);
    rbtree_deinit(p->modules);
    DFREE (p);
};

process *find_process(DWORD PID)
{
    process *p=(process*)rbtree_lookup(processes, (void*)PID);
    assert (p!=NULL && "PID not found in processes table");
    return p;
};

module *find_module_for_address (process *p, address a)
{
    module *prev_v, *m=rbtree_lookup2 (p->modules, (void*)a, NULL, (void**)&prev_v, NULL, NULL);

    if (m) // a==base address of some module
        return m;
    else
    {
        //assert (prev_v && "address $a$ is not in limits of any registered module");
        if (prev_v==NULL)
            return NULL;

        if (address_in_module(prev_v, a)) 
            return prev_v;
        return NULL;
    };
};

// may return NULL
symbol *process_sym_exist_at (process *p, address a)
{
    module *m=find_module_for_address (p, a);

    return module_sym_exist_at (m, a);
};

void process_get_sym (process *p, address a, bool add_module_name, bool add_offset, strbuf *out)
{
    if (0 && process_c_debug)
        L ("%s() a=0x" PRI_ADR_HEX "\n", __func__, a);
    
    module *m=find_module_for_address (p, a);
    
    if (m)
        module_get_sym (m, a, add_module_name, add_offset, out);
    else
    {
        if (add_module_name)
            strbuf_addstr (out, "<unknown module>!<unknown symbol>");
        else
            strbuf_addstr (out, "<unknown symbol>");
    };
};

address process_get_next_sym_address_after (process *p, address a)
{
    module *m=find_module_for_address (p, a);
    
    if (m==NULL)
        return 0;
    
    return module_get_next_sym_address_after (m, a);
};

bool adr_in_executable_section(process *p, address a)
{
    module *m=find_module_for_address (p, a);
    if(m==NULL)
        return false;
    return module_adr_in_executable_section (m, a);
};

