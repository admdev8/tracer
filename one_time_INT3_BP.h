#pragma once

#include <windows.h>

typedef struct _process process;
typedef struct _module module;
typedef struct _MemoryCache MemoryCache;
typedef struct _thread thread;

void set_onetime_INT3_BP(address a, process *p, module *m, char *name, MemoryCache *mc);
bool check_for_onetime_INT3_BP(process *p, thread *t, address a, MemoryCache *mc, char *resolved_name, CONTEXT *ctx);


