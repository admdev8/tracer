#include <assert.h>

#include "thread.h"
#include "dmalloc.h"
#include "rbtree.h"
#include "logging.h"

#include "process.h"

bool thread_c_debug=true;

void thread_free (thread *t)
{
    if (thread_c_debug)
        L ("%s() begin\n", __func__);
    DFREE (t);
};

thread *find_thread (DWORD PID, DWORD TID)
{
    process *p=find_process(PID);
    thread *t=(thread*)rbtree_lookup (p->threads, (void*)TID);
    assert (t!=NULL && "TID not found in threads table");
    return t;
};

void add_thread (process *p, DWORD TID, HANDLE THDL, address start)
{
    thread *t=DCALLOC (thread, 1, "thread");

    if (thread_c_debug)
        L ("%s() begin\n", __func__);

    t->TID=TID;
    t->THDL=THDL;
    t->start=start;
    assert (rbtree_lookup(p->threads, (void*)TID)==NULL && "this TID is already in table");
    rbtree_insert (p->threads, (void*)TID, t);
    
    if (thread_c_debug)
        L ("%s() end\n", __func__);
};


