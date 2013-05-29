#pragma once

#include "rbtree.h"
#include "regex.h"

rbtree *processes; // PID, process
typedef struct _thread thread;
strbuf ORACLE_HOME;
int oracle_version; // -1 mean 'unknown'

typedef struct _trace_skip_element
{
   regex_t re_path;
   regex_t re_module;
   regex_t re_function;
   bool is_function_wildcard; // is re_function = '.*'?
   struct _trace_skip_element *next;
} trace_skip_element;

trace_skip_element *trace_skip_options;

extern bool detaching;
void clean_all_DRx();
