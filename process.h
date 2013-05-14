#pragma once

#include <windows.h>

#include "rbtree.h"
#include "dmalloc.h"
#include "address.h"

typedef struct _process
{
    DWORD PID;
    HANDLE PHDL;
    address base_of_image;
    HANDLE file_handle;
    char *path;
    char *filename;

    rbtree *threads; // -> TID, thread
    rbtree *modules; // base_address (in process), module

} process;

void process_free (process *p);
process *find_process(DWORD PID);
void process_resolve_path_and_filename_from_hdl(HANDLE file_hdl, process *p);

