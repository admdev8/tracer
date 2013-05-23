#pragma once

#include <stdbool.h>

#include "datatypes.h"
#include "x86_disas.h"
#include "X86_register.h"
#include "lisp.h"
#include "dlist.h"
#include "regex.h"
#include "BP.h"
#include "address.h" // from bolt

#ifdef _WIN64
typedef octabyte REG;
#else
typedef tetrabyte REG;
#endif

extern BPF* current_BPF; // filled while parsing
extern bp_address* current_BPF_address; // filled while parsing

BP* parse_option(char *s);
bool is_there_OEP_breakpoint_for_fname(char *fname);

// from opts.y
extern BP* OEP_breakpoint;
extern BP* DRx_breakpoints[4];
extern dlist *addresses_to_be_resolved;
extern char* load_filename;
extern char* attach_filename;
extern char *load_command_line;
extern int attach_PID;
extern bool debug_children;
extern bool dash_s;

void add_new_BP (BP* bp);
void add_new_address_to_be_resolved (bp_address *a);

void yyerror(char *s);
