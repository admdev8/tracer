#pragma once

#include "rbtree.h"

rbtree *processes; // PID, process
typedef struct _thread thread;

extern bool detaching;
void clean_all_DRx();

void dump_PID_if_need(process *p);
void dump_TID_if_need(process *p, thread *t);
void set_or_update_all_breakpoints(process *p);

