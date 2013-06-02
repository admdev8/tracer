#pragma once

#include <windows.h>

#include "rbtree.h"
#include "dmalloc.h"
#include "address.h"
#include "strbuf.h"

typedef struct _module module;
typedef struct _symbol symbol;

typedef struct _process
{
    DWORD PID;
    HANDLE PHDL;
    address base_of_image;
    HANDLE file_handle;

    module *executable_module;

    rbtree *threads; // -> TID, thread
    rbtree *modules; // base_address (in process), module

    bool we_are_loading_and_OEP_was_executed;

} process;

void process_free (process *p);
process *find_process(DWORD PID);
module *find_module_for_address (process *p, address a);
symbol *process_sym_exist_at (process *p, address a);
void process_get_sym (process *p, address a, bool add_module_name, strbuf *out);

