#pragma once

#include <windows.h>

#include "address.h"
#include "module.h"
#include "lisp.h"
#include "PE.h"
#include "rbtree.h"

typedef struct _module module;

typedef enum _symbol_type
{
    SYM_TYPE_OEP,
    SYM_TYPE_BASE,
    SYM_TYPE_PE_EXPORT,
    SYM_TYPE_MAP,
    SYM_TYPE_ORACLE_SYM,
    SYM_TYPE_PDB
    // to be added here: SYM_TYPE_INLINE_FUNCTION - to be added while scanning for patterns
} symbol_type;

typedef struct _symbol
{
    symbol_type t;
    char *name;
    struct _symbol *next;
} symbol;

typedef struct _symbols_list
{
    symbol *s;
    // will it be skipped during tracing?
} symbols_list;

typedef struct _add_symbol_params
{
    process* p;
    module* m;
    symbol_type t;
} add_symbol_params;

void add_symbol (address a, char *name, add_symbol_params *params);

