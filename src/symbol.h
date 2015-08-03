/*
 *  _______                      
 * |__   __|                     
 *    | |_ __ __ _  ___ ___ _ __ 
 *    | | '__/ _` |/ __/ _ \ '__|
 *    | | | | (_| | (_|  __/ |   
 *    |_|_|  \__,_|\___\___|_|   
 *
 * Written by Dennis Yurichev <dennis(a)yurichev.com>, 2013
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/3.0/.
 *
 */

#pragma once

#include <windows.h>

#include "module.h"
#include "lisp.h"
#include "PE.h"
#include "rbtree.h"
#include "fuzzybool.h"

typedef struct _module module;

typedef enum _symbol_type
{
    SYM_TYPE_SPECIAL,
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
    TrueFalseUndefined skip_on_tracing;
    struct _symbol *next;
} symbol;

typedef struct _add_symbol_params
{
    process* p;
    module* m;
    symbol_type t;
    MemoryCache *mc;
} add_symbol_params;

void add_symbol (address a, char *name, add_symbol_params *params);
bool symbol_skip_on_tracing(module *m, symbol *s);

/* vim: set expandtab ts=4 sw=4 : */
