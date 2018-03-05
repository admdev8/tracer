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

#include <stdbool.h>

#include "strbuf.h"
#include "lisp.h"
#include "regex.h"
#include "datatypes.h"

#define BYTEMASK_WILDCARD_BYTE 0x100

enum adr_type
{
    OPTS_ADR_TYPE_ABS, // always resolved
    OPTS_ADR_TYPE_FILENAME_SYMBOL,
    OPTS_ADR_TYPE_FILENAME_ADR,
    OPTS_ADR_TYPE_BYTEMASK
};

struct bp_address
{
    enum adr_type t;
    bool resolved;

    // OPTS_ADR_TYPE_ABS case
    address abs_address; // always set if resolved

    char *filename; // for OPTS_ADR_TYPE_FILENAME_SYMBOL and OPTS_ADR_TYPE_FILENAME_ADR

    // OPTS_ADR_TYPE_FILENAME_SYMBOL case
    char *symbol;
    regex_t symbol_re; // all symbols are in regex form (?) so here it is always present (in compiled form)
    unsigned ofs;

    // OPTS_ADR_TYPE_FILENAME_ADR case
    address adr;

    // OPTS_ADR_TYPE_BYTEMASK case
    wyde* bytemask;
    unsigned bytemask_len;
};

struct bp_address *create_address_filename_symbol_re(const char *filename, const char *symbol_re, unsigned ofs);
struct bp_address *create_address_filename_address(const char *filename, address adr);
struct bp_address *create_address_abs(address adr);
struct bp_address *create_address_bytemask(obj *bytemask);
void bp_address_free(struct bp_address *a);
bool is_address_OEP(struct bp_address *a);
bool is_address_fname_OEP(struct bp_address* a, char *fname);
void address_to_string (struct bp_address *a, strbuf *out);
void dump_address (struct bp_address *a);

/* vim: set expandtab ts=4 sw=4 : */
