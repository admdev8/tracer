#pragma once

#include <stdbool.h>
#include "CONTEXT_utils.h"
#include "memorycache.h"

typedef struct _BP BP;
typedef struct _process process;
typedef struct _thread thread;

typedef enum _BPF_state
{
    BPF_state_default=0,
    BPF_state_at_return
} BPF_state;

void handle_BPF(process *p, thread *t, int DRx_no /* -1 for OEP */, CONTEXT *ctx, MemoryCache *mc);
