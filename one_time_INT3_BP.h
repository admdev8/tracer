#pragma once

#include <windows.h>

void set_onetime_INT3_BP(address a, struct process *p, struct module *m, char *name, struct MemoryCache *mc);
bool check_for_onetime_INT3_BP(struct process *p, struct thread *t, address a, struct MemoryCache *mc, char *resolved_name, CONTEXT *ctx);
