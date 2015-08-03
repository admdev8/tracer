/*
 *  _______                      
 * |__   __|                     
 *    | |_ __ __ _  ___ ___ _ __ 
 *    | | '__/ _` |/ __/ _ \ '__|
 *    | | | | (_| | (_|  __/ |   
 *    |_|_|  \__,_|\___\___|_|   
 *
 * Written by Dennis Yurichev <dennis(a)yurichev.com>, 2013
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/3.0/.
 *
 */

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

void clean_all_DRx();

/* vim: set expandtab ts=4 sw=4 : */
