#pragma once

#include "x86_disas.h"
#include "X86_register.h"
#include "datatypes.h"
#include "address.h" // from bolt
#include "regex.h"
#include "lisp.h"
#include "BPF.h"
#include "bp_address.h"

enum BPM_type
{
    BPM_type_W,
    BPM_type_RW
};

typedef struct _BPM
{
    unsigned width;
    enum BPM_type t;
} BPM;

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
    REG size_or_value;
    // in case of COPY
    byte *copy_string; // may be NULL if absent
    unsigned copy_string_len;
    struct _BPX_option *next;
} BPX_option;

typedef struct _BPX
{
    BPX_option* opts; // may be NULL if options absent
} BPX;

enum BP_type
{
    BP_type_BPM,
    BP_type_BPX,
    BP_type_BPF
};

typedef struct _BP
{
    enum BP_type t;
    Da* ins; // disassembled instruction at the place of INT3

    bp_address *a;
    union
    {
        BPM* bpm;
        BPX* bpx;
        BPF* bpf;
        void *p;
    } u;
    struct _BP *next;
} BP;

bp_address *create_address_filename_symbol_re(const char *filename, const char *symbol_re, unsigned ofs);
bp_address *create_address_filename_address(const char *filename, address adr);
bp_address *create_address_abs(unsigned adr);
bp_address *create_address_bytemask(obj *bytemask);
void bp_address_free(bp_address *a);
BPM *create_BPM(unsigned width, enum BPM_type t);
void dump_BPM(BPM *);
void BPM_free(BPM *);
void BPX_option_free(BPX_option *);
void dump_BPX_option(BPX_option *b);
void BPX_free(BPX *);
BPX* create_BPX(BPX_option *opts);
void dump_BPX(BPX *);
void BPF_free(BPF*);
BP* create_BP (enum BP_type t, bp_address* a, void* p);
void dump_BP (BP* b);
void BP_free(BP*);
bool is_address_OEP(bp_address *a);
bool is_address_fname_OEP(bp_address* a, char *fname);
void address_to_string (bp_address *a, strbuf *out);
void dump_address (bp_address *a);
