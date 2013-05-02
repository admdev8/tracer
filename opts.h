#pragma once

#include <stdbool.h>
#include <datatypes.h>
#include "X86_register.h"
#include "lisp.h"

#define BYTEMASK_WILDCARD_BYTE 257

typedef tetrabyte address;
//typedef octabyte address;

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
    address abs_address;

    char *filename; // for OPTS_ADR_TYPE_FILENAME_SYMBOL and OPTS_ADR_TYPE_FILENAME_ADR

    // OPTS_ADR_TYPE_FILENAME_SYMBOL case
    char *symbol;
    unsigned ofs;

    // OPTS_ADR_TYPE_FILENAME_ADR case
    address adr;

    // OPTS_ADR_TYPE_BYTEMASK case
    obj* bytemask;

} bp_address;

bp_address *create_address_filename_symbol(const char *filename, const char *symbol, unsigned ofs);
bp_address *create_address_filename_address(const char *filename, address adr);
bp_address *create_address_abs(unsigned adr);
bp_address *create_address_bytemask(obj *bytemask);
void bp_address_free(bp_address *a);

enum BPM_type
{
    BPM_type_W,
    BPM_type_RW
};

typedef struct _BPM
{
    bp_address *a;
    unsigned width;
    enum BPM_type t;
} BPM;

BPM *create_BPM(bp_address *a, unsigned width, enum BPM_type t);
void dump_BPM(BPM *);
void BPM_free(BPM *);

enum BPX_option_type
{
    BPX_option_DUMP,
    BPX_option_SET,
    BPX_option_COPY
};

#define BPX_DUMP_DEFAULT 0x10

typedef struct _BPX_option
{
    enum BPX_option_type t;
    bp_address *a; // if NULL, see reg
    X86_register reg;
    octabyte size_or_value;
    // in case of COPY
    obj *copy_string;
} BPX_option;

void BPX_option_free(BPX_option *);

void dump_BPX_option(BPX_option *b);

typedef struct _BPX
{
    bp_address *a;
    obj* options; // list of opaque objects. each object - ptr to one BPX_option.
                  // may be NULL if options absent
} BPX;

void BPX_free(BPX *);
BPX* create_BPX(bp_address *a, obj *options);
void dump_BPX(BPX *);

typedef struct _BPF
{
    bp_address *a;
    bool unicode, skip, skip_stdcall, trace, trace_cc;
    // these params may be NULL
    obj* rt;
    double rt_probability;
    unsigned args, dump_args, pause;
    bp_address *when_called_from_address, *when_called_from_func;
} BPF;

void BPF_free(BPF*);

enum BP_type
{
    BP_type_BPM,
    BP_type_BPX,
    BP_type_BPF
};

typedef struct _BP
{
    enum BP_type t;
    union
    {
        BPM* bpm;
        BPX* bpx;
        BPF* bpf;
        void *p;
    } u;
} BP;

BP* create_BP (enum BP_type t, void* p);
void dump_BP (BP* b);
void BP_free(BP*);

extern BPF* current_BPF; // filled while parsing
