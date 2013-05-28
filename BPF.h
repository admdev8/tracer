#pragma once

#include <stdbool.h>
#include "CONTEXT_utils.h"
#include "memorycache.h"
//#include "bp_address.h"
#include "lisp.h"

typedef struct _bp_address bp_address;
typedef struct _BP BP;
typedef struct _process process;
typedef struct _thread thread;

typedef enum _BPF_state
{
    BPF_state_default=0,
    BPF_state_at_return,
    BPF_state_tracing_inside_function,
    BPF_state_tracing_skipping
} BPF_state;

typedef struct _BPF
{
    bool unicode, skip, skip_stdcall, trace, cc;
    // these params may be NULL
    obj* rt;
    double rt_probability;
    unsigned args, dump_args, pause;
    bp_address *when_called_from_address, *when_called_from_func;
} BPF;

void BPF_free(BPF*);
void dump_BPF(BPF *b);
void handle_BPF(process *p, thread *t, int DRx_no /* -1 for OEP */, CONTEXT *ctx, MemoryCache *mc);
