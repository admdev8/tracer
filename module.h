#pragma once

#include "rbtree.h"
#include "opts.h"

typedef struct _process process;
typedef struct _symbol symbol;

typedef struct _module
{
    char *filename;
    char *filename_without_ext;
    char *path;
    char *internal_name; // may be NULL
    address base;
    address original_base;
    address OEP;
    DWORD PE_timestamp;
    byte saved_OEP_byte; // in case of 'main' executable
    SIZE_T size;
    rbtree *symbols; // -> address, symbol
    bool skip_all_symbols_in_module_on_trace;
} module;

module* add_module (process *p, address img_base, HANDLE file_hdl);
void module_free(module *m);
void remove_module (process *p, address img_base);
bool address_in_module (module *m, address a);
void module_get_sym (module *m, address a, strbuf *out);
symbol* module_sym_exist_at (module *m, address a);
char *get_module_name (module *m);

