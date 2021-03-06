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

#include <stdbool.h>

#include "datatypes.h"
#include "x86_disas.h"
#include "X86_register.h"
#include "lisp.h"
#include "dlist.h"
#include "regex.h"
#include "BP.h"

extern struct BPF* current_BPF; // filled while parsing
extern struct bp_address* current_BPF_address; // filled while parsing

struct BP* parse_option(char *s);
bool is_there_OEP_breakpoint_for_fname(char *fname);

// from opts.y
extern struct BP* breakpoints[4];
extern dlist *addresses_to_be_resolved;
extern char *load_filename, *attach_filename, *load_command_line;
extern int attach_PID;
extern bool debug_children, dash_s, quiet, dump_fpu, dump_xmm, dump_seh, dump_all_symbols;
regex_t *dump_all_symbols_re, *one_time_int3_bp_re;
extern int verbose;
extern bool opt_loading, create_new_console;
extern bool emulator_testing;
extern int limit_trace_nestedness;

// you may want to disable thread B while debugging in gdb, because thread B's input
// interfering with gdb
extern bool run_thread_b; 

void add_new_BP (struct BP* bp);
void add_new_address_to_be_resolved (struct bp_address *a);

void yyerror(char *s);

/* vim: set expandtab ts=4 sw=4 : */
