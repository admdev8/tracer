#pragma once

#include "rbtree.h"
#include "opts.h"

struct _process;
typedef struct _process process;

typedef struct _module
{
    char *filename;
    char *path;
    char *internal_name;
    address base;
    address original_base;
    SIZE_T size;
    rbtree *symbols; // -> address, symbols_list
    // is_all_symbols_in_module_skipped ? 
} module;

void add_module (process *p, address img_base, HANDLE file_hdl);
void module_free(module *m);
void remove_module (process *p, address img_base);
bool address_in_module (module *m, address a);
void module_get_sym (module *m, address a, strbuf *out);
char *get_module_name (module *m);

