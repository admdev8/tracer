#pragma once

#include <windows.h>
typedef struct _MemoryCache MemoryCache;
typedef struct _process process;
typedef struct _thread thread;

void dump_SEH_chain (fds* s, process *p, thread *t, CONTEXT *ctx, MemoryCache *mc);
