#pragma once

#include "rbtree.h"
#include "opts.h"

typedef struct _process process;
typedef struct _symbol symbol;

typedef struct _module
{
    process *parent_process;
    char *filename;
    char *filename_without_ext;
    char *path;
    address base;
    byte saved_OEP_byte; // in case of 'main' executable
    rbtree *symbols; // -> address, symbol
    bool skip_all_symbols_in_module_on_trace;
    
    // cc
    rbtree *PC_infos; // address, PC_info
    
    // PE info
    address OEP;
    SIZE_T size;
    DWORD PE_timestamp;
    address original_base;
    char *internal_name; // may be NULL
    IMAGE_SECTION_HEADER *sections; // ptr to array of sections. allocated via DMALLOC. deep copy here.
    unsigned sections_total;
} module;

module* add_module (process *p, address img_base, HANDLE file_hdl);
void unload_module_and_free(module *m);
void remove_module (process *p, address img_base);
bool address_in_module (module *m, address a);
void module_get_sym (module *m, address a, bool add_module_name, bool add_offset, strbuf *out);
symbol* module_sym_exist_at (module *m, address a);
char *get_module_name (module *m);
bool module_adr_in_executable_section (module *m, address a);
address get_module_end(module *m);
address module_get_next_sym_address_after (module *m, address a);

