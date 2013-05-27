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

typedef struct _bp_address
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

} bp_address;

bp_address *create_address_filename_symbol_re(const char *filename, const char *symbol_re, unsigned ofs);
bp_address *create_address_filename_address(const char *filename, address adr);
bp_address *create_address_abs(unsigned adr);
bp_address *create_address_bytemask(obj *bytemask);
void bp_address_free(bp_address *a);
bool is_address_OEP(bp_address *a);
bool is_address_fname_OEP(bp_address* a, char *fname);
void address_to_string (bp_address *a, strbuf *out);
void dump_address (bp_address *a);
