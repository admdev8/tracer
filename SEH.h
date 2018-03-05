#pragma once

#include <windows.h>

void dump_SEH_chain (fds* s, struct process *p, struct thread *t, CONTEXT *ctx, struct MemoryCache *mc);
