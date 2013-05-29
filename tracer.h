#pragma once

#include "rbtree.h"

rbtree *processes; // PID, process
typedef struct _thread thread;
strbuf ORACLE_HOME;
int oracle_version; // -1 mean 'unknown'
trace_skip_element *trace_skip_options;

extern bool detaching;
void clean_all_DRx();
