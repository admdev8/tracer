#pragma once

#include <windows.h>

#include "address.h"
#include "module.h"
#include "lisp.h"
#include "PE.h"
#include "rbtree.h"

struct _module;
typedef struct _module module;

typedef enum _symbol_type
{
    SYM_TYPE_OEP,
    SYM_TYPE_BASE,
    SYM_TYPE_PE_EXPORT_ORDINAL,
    SYM_TYPE_PE_EXPORT,
    SYM_TYPE_MAP,
    SYM_TYPE_PDB
} symbol_type;

typedef struct _symbols_list
{
    obj *symbols; // list of conses, each cons: type and name
    // will it be skipped during tracing?
} symbols_list;

void add_symbols(rbtree *symtbl, const char * filename, address base, PE_info *info);

