#pragma once

#include <stdbool.h>
#include "CONTEXT_utils.h"
#include "memorycache.h"

typedef struct _BP BP;
typedef struct _process process;
typedef struct _thread thread;

void handle_BPX(BP *bp, process *p, thread *t, int DRx_no /* -1 for OEP */, CONTEXT *ctx, MemoryCache *mc);
