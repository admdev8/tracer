%{

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

#include "oassert.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "datatypes.h"
#include "stuff.h"
#include "opts.h"
#include "dmalloc.h"
#include "lisp.h"
#include "dlist.h"
#include "X86_register.h"
#include "stuff.h"
#include "BP.h"
#include "BPF.h"
#include "BPX.h"
#include "BPM.h"
#include "bp_address.h"
#include "logging.h"

// ,set(byte,*arg_0=0x123)
// ,set(byte,*(arg_0+0x150)=0x123)

// globals to be set here:

BP* breakpoints[4]={ NULL, NULL, NULL, NULL }; // 0..3 - DR0-3
dlist* addresses_to_be_resolved=NULL;
char* load_filename=NULL;
char* attach_filename=NULL;
char *load_command_line=NULL;
int attach_PID=-1;
bool debug_children=false;
bool dash_s=false, quiet=false;
bool dump_fpu=false, dump_xmm=false, dump_seh=false;
BPX_option *current_BPX_option=NULL; // temporary, while parsing...
BPF* current_BPF=NULL; // filled while parsing
bp_address* current_BPF_address; // filled while parsing
bool run_thread_b=true;
bool dump_all_symbols=false;
regex_t *dump_all_symbols_re=NULL;
bool module_c_debug=false, symbol_c_debug=false, cycle_c_debug=false, bpx_c_debug=false, utils_c_debug=false, cc_c_debug=false, BPF_c_debug=false, tracing_debug=false, emulator_testing=false, opt_loading=false, create_new_console=true;
int limit_trace_nestedness=1; // default value

// from opts.l:
void flex_set_str(char *s);
void flex_cleanup();
void flex_restart();

void add_new_BP (BP* bp)
{
    for (int i=0; i<4; i++)
        if (breakpoints[i]==NULL)
        {
            //printf ("%s() setting breakpoints[%d]...\n", __func__, i);
            breakpoints[i]=bp;
            return;
        };
    die ("No more free DRx slots. Only 4 breakpoints allowed!\n");
};

void add_new_address_to_be_resolved (bp_address *a)
{
    if (addresses_to_be_resolved==NULL)
        addresses_to_be_resolved=dlist_init();
    
    dlist_insert_at_begin (&addresses_to_be_resolved, a);
};

%}

%union 
{
    char * str;
    REG num;
    double dbl;
    struct _obj * o;
    struct _bp_address *a;
    struct _BPM *bpm;
    struct _BP *bp;
    struct _BPX_option *bpx_option;
    X86_register x86reg;
}

%token SKIP COLON EOL BYTEMASK BYTEMASK_END BPX_EQ BPF_EQ
%token _EOF DUMP_OP SET SET_OP COPY_OP BPF_CC BPF_PAUSE BPF_RT_PROBABILITY CHILD
%token BPF_TRACE BPF_TRACE_COLON DASH_S DASH_Q DASH_T DONT_RUN_THREAD_B DUMP_FPU DUMP_XMM DUMP_SEH
%token BPF_ARGS BPF_DUMP_ARGS BPF_RT BPF_SKIP BPF_SKIP_STDCALL BPF_UNICODE 
%token WHEN_CALLED_FROM_ADDRESS WHEN_CALLED_FROM_FUNC ARG_ LOADING NO_NEW_CONSOLE
%token MODULE_DEBUG SYMBOL_DEBUG CYCLE_DEBUG BPX_DEBUG UTILS_DEBUG CC_DEBUG BPF_DEBUG EMULATOR_TESTING TRACING_DEBUG NEWLINE
%token <num> DEC_NUMBER HEX_NUMBER HEX_BYTE
%token <num> BPM_width CSTRING_BYTE ATTACH_PID DMALLOC_BREAK_ON LIMIT_TRACE_NESTEDNESS
%token <num> BYTE_WORD_DWORD_DWORD64
%token <x86reg> REGISTER FPU_REGISTER
%token <dbl> FLOAT_NUMBER
%token <str> FILENAME_EXCLAMATION SYMBOL_NAME_RE SYMBOL_NAME_RE_PLUS LOAD_FILENAME ATTACH_FILENAME CMDLINE
%token <str> ALL_SYMBOLS ONE_TIME_INT3_BP

%type <a> address
%type <o> bytemask bytemask_element cstring
%type <num> skip_n DEC_OR_HEX abs_address ARGUMENT_N
%type <bp> bpm bpx
%type <bpx_option> BPX_option
%type <dbl> float_or_perc

%error-verbose

%%

tracer_option
 : tracer_option_without_newline
 | tracer_option_without_newline '\n'
 ;

tracer_option_without_newline
 : bpm             { add_new_BP ($1); }
 | bpx             { add_new_BP ($1); }
 | bpf             { 
   if (current_BPF->rt_present==false && current_BPF->rt_probability_present==true)
       die ("rt_probability option without rt option is useless. exiting.\n");
   BP *bp=create_BP(BP_type_BPF, current_BPF_address, current_BPF);
   add_new_BP (bp); 
   current_BPF=NULL;
   current_BPF_address=NULL;
 }
 | LOAD_FILENAME           { load_filename=$1; }
 | ATTACH_FILENAME         { attach_filename=$1; }
 | ATTACH_PID              { attach_PID=$1; }
 | CHILD                   { debug_children=true; }
 | DASH_S                  { dash_s=true; }
 | DASH_Q                  { quiet=true; }
 | DASH_T                  { L_timestamp=true; }
 | DUMP_FPU                { dump_fpu=true; }
 | DUMP_XMM                { dump_xmm=true; }
 | DUMP_SEH                { dump_seh=true; }
 | DONT_RUN_THREAD_B       { run_thread_b=false; }
 | CMDLINE                 { load_command_line=$1; }
 | LOADING                 { opt_loading=true; }
 | MODULE_DEBUG            { module_c_debug=true; }
 | SYMBOL_DEBUG            { symbol_c_debug=true; }
 | CYCLE_DEBUG             { cycle_c_debug=true; }
 | BPX_DEBUG               { bpx_c_debug=true; }
 | UTILS_DEBUG             { utils_c_debug=true; }
 | CC_DEBUG                { cc_c_debug=true; }
 | BPF_DEBUG               { BPF_c_debug=true; }
 | TRACING_DEBUG           { tracing_debug=true; }
 | EMULATOR_TESTING        { emulator_testing=true; }
 | NO_NEW_CONSOLE          { create_new_console=false; }
 | DMALLOC_BREAK_ON        { dmalloc_break_at_seq_n ($1); }
 | LIMIT_TRACE_NESTEDNESS  { limit_trace_nestedness=$1; }
 | ALL_SYMBOLS     { 
    dump_all_symbols=true;
    if ($1)
    {
        oassert(dump_all_symbols_re==NULL);
        dump_all_symbols_re=DCALLOC(regex_t, 1, "regex_t");
        regcomp_or_die(dump_all_symbols_re, $1, REG_EXTENDED | REG_ICASE | REG_NEWLINE);
        DFREE($1);
        //printf ("dump_all_symbols_re is set\n");
    }
    }
 | ONE_TIME_INT3_BP     { 
    oassert(one_time_int3_bp_re==NULL);
    one_time_int3_bp_re=DCALLOC(regex_t, 1, "regex_t");
    regcomp_or_die(one_time_int3_bp_re, $1, REG_EXTENDED | REG_ICASE | REG_NEWLINE);
    DFREE($1);
    }
 ;

bpm
 : BPM_width address ',' 'w'
   { $$=create_BP(BP_type_BPM, $2, create_BPM ($1, BPM_type_W)); }
 | BPM_width address ',' 'r' 'w'
   { $$=create_BP(BP_type_BPM, $2, create_BPM ($1, BPM_type_RW)); }
 ;

bpx
 : BPX_EQ address
   { $$=create_BP(BP_type_BPX, $2, create_BPX (NULL)); }
 | BPX_EQ address ',' BPX_options
   { 
       $$=create_BP(BP_type_BPX, $2, create_BPX (current_BPX_option)); 
       current_BPX_option=NULL;
   }
 ;

bpf
 : BPF_EQ address                   { current_BPF_address=$2; } 
 | BPF_EQ address ',' BPF_options { current_BPF_address=$2; }
 ;

BPX_options
 : BPX_option ',' BPX_options
 { 
     BPX_option *o;
     oassert(current_BPX_option);
     for (o=current_BPX_option; o->next; o=o->next);
     o->next=$1;
 }
 | BPX_option
 { current_BPX_option=$1; }
 ;

BPF_options
 : BPF_option ',' BPF_options
 | BPF_option
 ;

BPX_option
 : DUMP_OP address ',' DEC_OR_HEX ')'
 { $$=DCALLOC(BPX_option, 1, "BPX_option"); $$->t=BPX_option_DUMP; $$->a=$2; $$->size_or_value=$4; }
 | DUMP_OP address ')'
 { $$=DCALLOC(BPX_option, 1, "BPX_option"); $$->t=BPX_option_DUMP; $$->a=$2; $$->size_or_value=BPX_DUMP_DEFAULT; }
 | DUMP_OP REGISTER ',' DEC_OR_HEX ')'
 { $$=DCALLOC(BPX_option, 1, "BPX_option"); $$->t=BPX_option_DUMP; $$->reg=$2; $$->size_or_value=$4; }
 | DUMP_OP REGISTER ')'
 { $$=DCALLOC(BPX_option, 1, "BPX_option"); $$->t=BPX_option_DUMP; $$->reg=$2; $$->size_or_value=BPX_DUMP_DEFAULT; }
 | SET_OP REGISTER ',' DEC_OR_HEX ')'
 { $$=DCALLOC(BPX_option, 1, "BPX_option"); $$->t=BPX_option_SET; $$->reg=$2; $$->size_or_value=$4; }
 | SET_OP FPU_REGISTER ',' FLOAT_NUMBER ')'
 { $$=DCALLOC(BPX_option, 1, "BPX_option"); $$->t=BPX_option_SET; $$->reg=$2; $$->float_value=$4; }
 | COPY_OP address ',' '"' cstring '"' ')'
 {
    $$=DCALLOC(BPX_option, 1, "BPX_option"); 
    $$->t=BPX_option_COPY; 
    $$->a=$2; 
    list_of_bytes_to_array (&($$->copy_string), &($$->copy_string_len), $5); 
    obj_free($5);
 }
 | COPY_OP REGISTER ',' '"' cstring '"' ')'
 {
    $$=DCALLOC(BPX_option, 1, "BPX_option"); 
    $$->t=BPX_option_COPY; 
    $$->reg=$2; 
    list_of_bytes_to_array (&($$->copy_string), &($$->copy_string_len), $5); 
    obj_free($5);
 }
 ;

BPF_option
 : BPF_UNICODE                      { current_BPF->unicode=true; }
 | BPF_TRACE                        { current_BPF->trace=true; }
 | BPF_TRACE_COLON BPF_CC           { current_BPF->trace=true; current_BPF->cc=true; }
 | BPF_SKIP                         { current_BPF->skip=true; } 
 | BPF_SKIP_STDCALL                 { current_BPF->skip_stdcall=true; }
 | BPF_PAUSE DEC_OR_HEX             { current_BPF->pause=$2; }
 | BPF_RT DEC_OR_HEX                { current_BPF->rt=$2; 
    current_BPF->rt_present=true; }
 | BPF_RT_PROBABILITY float_or_perc { current_BPF->rt_probability=$2; 
    current_BPF->rt_probability_present=true;
#ifdef _DEBUG 
    //fprintf (stderr, "rt_probability=%f", $2); 
#endif
    }
 | BPF_ARGS DEC_OR_HEX              { current_BPF->args=$2; }
 | BPF_DUMP_ARGS DEC_OR_HEX         { current_BPF->dump_args=$2; }
 | WHEN_CALLED_FROM_ADDRESS address { current_BPF->when_called_from_address=$2; }
 | WHEN_CALLED_FROM_FUNC address    { current_BPF->when_called_from_func=$2; }
 | SET '(' BYTE_WORD_DWORD_DWORD64 ',' '*' '(' ARGUMENT_N '+' DEC_OR_HEX ')' '=' DEC_OR_HEX ')' {
    // FIXME: there should be support of multiple SET options!
    current_BPF->set_present=true;
    current_BPF->set_width=$3;
    current_BPF->set_arg_n=$7;
    current_BPF->set_ofs=$9;
    current_BPF->set_val=$12;
 }
 ;

ARGUMENT_N
 : ARG_ DEC_NUMBER { $$=$2; }
 ;

float_or_perc
 : FLOAT_NUMBER
 | DEC_NUMBER '%' { $$=(double)$1/(double)100; }
 ;

cstring
 : CSTRING_BYTE cstring  { $$=NCONC (cons(obj_byte($1), NULL), $2); }
 | CSTRING_BYTE          { $$=cons (obj_byte($1), NULL); }
 ;

address
 : abs_address 
     { 
        $$=create_address_abs ($1); 
     }
 | FILENAME_EXCLAMATION SYMBOL_NAME_RE_PLUS DEC_OR_HEX
     { 
        $$=create_address_filename_symbol_re ($1, $2, $3); 
         DFREE ($1); 
         DFREE ($2); 
         // every new address, except of abs-address (which is already resolved)
         // is added to addresses resolving queue
         add_new_address_to_be_resolved ($$); 
     }
 | FILENAME_EXCLAMATION SYMBOL_NAME_RE
     { 
        $$=create_address_filename_symbol_re ($1, $2, 0); 
        DFREE ($1); 
        DFREE ($2); 
        add_new_address_to_be_resolved ($$); 
     }
 | FILENAME_EXCLAMATION HEX_NUMBER
     { 
        $$=create_address_filename_address ($1, $2); 
        DFREE ($1); 
        add_new_address_to_be_resolved ($$); 
     }
 | BYTEMASK bytemask BYTEMASK_END 
     { 
        $$=create_address_bytemask ($2); 
        obj_free($2);
        add_new_address_to_be_resolved ($$); 
     }
 ;

DEC_OR_HEX
 : DEC_NUMBER
 | HEX_NUMBER
 ;

abs_address
 : HEX_NUMBER
 ;

bytemask
 : bytemask_element bytemask { $$=NCONC ($1, $2); }
 | bytemask_element
 ;

bytemask_element
 : HEX_BYTE     { $$=cons (obj_wyde($1), NULL); }
 | '.' '.'      { $$=cons (obj_wyde(BYTEMASK_WILDCARD_BYTE), NULL); }
 | skip_n       { $$=obj_wyde_n_times (BYTEMASK_WILDCARD_BYTE, $1); }
 ;

skip_n
 : SKIP DEC_NUMBER ']' { $$=$2; }
 ;

%%

BP* parse_option(char *s)
{
    //printf ("%s(%s)\n", __func__, s);
    int r;
    flex_restart();
    flex_set_str(s);
    r=yyparse();
    flex_cleanup();
    if (r)
    {
        printf ("[%s] option wasn't parsed, exiting\n", s);
        exit(0);
    };

    if (r==0 && breakpoints[0])
        return breakpoints[0];
    else
        return NULL;
};

void yyerror(char *s)
{
    fprintf(stderr, "bison error: %s\n", s);
}

/* vim: set expandtab ts=4 sw=4 : */
