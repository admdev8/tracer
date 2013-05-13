#include <assert.h>

#include "process.h"
#include "tracer.h"
#include "module.h"
#include "thread.h"
#include "logging.h"

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
    DFREE (p);
};

process *find_process(DWORD PID)
{
    process *p=(process*)rbtree_lookup(processes, (void*)PID);
    assert (p!=NULL && "PID not found in processes table");
    return p;
};


