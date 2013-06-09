#pragma once

#include <stdbool.h>
#include "CONTEXT_utils.h"
#include "memorycache.h"

typedef struct _BP BP;
typedef struct _process process;
typedef struct _thread thread;

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

void BPX_option_free(BPX_option *);
void dump_BPX_option(BPX_option *b);
void BPX_free(BPX *);
BPX* create_BPX(BPX_option *opts);
void dump_BPX(BPX *);

void handle_BPX(process *p, thread *t, int DRx_no, CONTEXT *ctx, MemoryCache *mc);
