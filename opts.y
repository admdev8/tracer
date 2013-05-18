%{
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "opts.h"
#include "dmalloc.h"
#include "lisp.h"
#include "X86_register.h"

BP* breakpoints=NULL; // list of opaque objects-pointers to BP structures
obj* addresses_to_be_resolved=NULL; // list of opaque objects-pointers to bp_address structures. don't free them.
char* load_filename=NULL;
char* attach_filename=NULL;
char *load_command_line=NULL;
int attach_PID=-1;
bool debug_children=false;
BPX_option *current_BPX_option=NULL; 

// from opts.l:

void flex_set_str(char *s);
void flex_cleanup();
void flex_restart();

void add_new_BP (BP* bp)
{
    if (breakpoints==NULL)
        breakpoints=bp;
    else
    {
        BP *t;
        for (t=breakpoints; t->next; t=t->next);
        t->next=bp;
    };
};

void add_new_address_to_be_resolved (bp_address *a)
{
    addresses_to_be_resolved=NCONC(addresses_to_be_resolved, 
        cons (create_obj_opaque(a, (void(*)(void*))dump_address, NULL), NULL));
};

%}

%union 
{
    char * str;
    int num;
    double dbl;
    struct _obj * o;
    struct _bp_address *a;
    struct _BPM *bpm;
    struct _BP *bp;
    struct _BPX_option *bpx_option;
    X86_register x86reg;
}

%token COMMA PLUS TWO_POINTS R_SQUARE_BRACKET SKIP COLON EOL BYTEMASK BYTEMASK_END BPX_EQ BPF_EQ
%token W RW _EOF DUMP_OP SET_OP COPY_OP CP QUOTE PERCENT BPF_CC BPF_PAUSE BPF_RT_PROBABILITY CHILD
%token BPF_TRACE BPF_TRACE_COLON
%token BPF_ARGS BPF_DUMP_ARGS BPF_RT BPF_SKIP BPF_SKIP_STDCALL BPF_UNICODE 
%token WHEN_CALLED_FROM_ADDRESS WHEN_CALLED_FROM_FUNC
%token <num> DEC_NUMBER HEX_NUMBER HEX_BYTE
%token <num> BPM_width CSTRING_BYTE ATTACH_PID
%token <x86reg> REGISTER
%token <dbl> FLOAT_NUMBER
%token <str> FILENAME_EXCLAMATION SYMBOL_NAME SYMBOL_NAME_PLUS LOAD_FILENAME ATTACH_FILENAME CMDLINE

%type <a> address
%type <o> bytemask bytemask_element cstring
%type <num> skip_n DEC_OR_HEX abs_address
%type <bp> bpm bpx
%type <bpx_option> BPX_option
%type <dbl> float_or_perc

%error-verbose

%%

tracer_option
 : bpm             { add_new_BP ($1); }
 | bpx             { add_new_BP ($1); }
 | bpf             { 
   if (is_address_OEP(current_BPF_address)) 
       current_BPF->INT3_style=true; 
   add_new_BP (create_BP(BP_type_BPF, current_BPF_address, current_BPF)); 
   current_BPF=NULL;
   current_BPF_address=NULL;
 }
 | LOAD_FILENAME   { load_filename=$1; }
 | ATTACH_FILENAME { attach_filename=$1; }
 | ATTACH_PID      { attach_PID=$1; }
 | CHILD           { debug_children=true; }
 | CMDLINE         { load_command_line=$1; }
 ;

bpm
 : BPM_width address COMMA W
   { $$=create_BP(BP_type_BPM, $2, create_BPM ($1, BPM_type_W)); }
 | BPM_width address COMMA RW
   { $$=create_BP(BP_type_BPM, $2, create_BPM ($1, BPM_type_RW)); }
 ;

bpx
 : BPX_EQ address
   { $$=create_BP(BP_type_BPX, $2, create_BPX (NULL)); }
 | BPX_EQ address COMMA BPX_options
   { 
       $$=create_BP(BP_type_BPX, $2, create_BPX (current_BPX_option)); 
       current_BPX_option=NULL;
   }
 ;

bpf
 : BPF_EQ address                   { current_BPF_address=$2; } 
 | BPF_EQ address COMMA BPF_options { current_BPF_address=$2; }
 ;

BPX_options
 : BPX_option COMMA BPX_options
 { 
     BPX_option *o;
     assert(current_BPX_option);
     for (o=current_BPX_option; o->next; o=o->next);
     o->next=$1;
 }
 | BPX_option
 { current_BPX_option=$1; }
 ;

BPF_options
 : BPF_option COMMA BPF_options
 | BPF_option
 ;

BPX_option
 : DUMP_OP address COMMA DEC_OR_HEX CP
 { $$=DCALLOC(BPX_option, 1, "BPX_option"); $$->t=BPX_option_DUMP; $$->a=$2; $$->size_or_value=$4; }
 | DUMP_OP address CP
 { $$=DCALLOC(BPX_option, 1, "BPX_option"); $$->t=BPX_option_DUMP; $$->a=$2; $$->size_or_value=BPX_DUMP_DEFAULT; }
 | DUMP_OP REGISTER COMMA DEC_OR_HEX CP
 { $$=DCALLOC(BPX_option, 1, "BPX_option"); $$->t=BPX_option_DUMP; $$->reg=$2; $$->size_or_value=$4; }
 | DUMP_OP REGISTER CP
 { $$=DCALLOC(BPX_option, 1, "BPX_option"); $$->t=BPX_option_DUMP; $$->reg=$2; $$->size_or_value=BPX_DUMP_DEFAULT; }
 | SET_OP REGISTER COMMA DEC_OR_HEX CP
 { $$=DCALLOC(BPX_option, 1, "BPX_option"); $$->t=BPX_option_SET; $$->reg=$2; $$->size_or_value=$4; }
 | COPY_OP address COMMA QUOTE cstring QUOTE CP
 { 
    $$=DCALLOC(BPX_option, 1, "BPX_option"); 
    $$->t=BPX_option_COPY; 
    $$->a=$2; 
    list_of_bytes_to_array (&($$->copy_string), &($$->copy_string_len), $5); 
    obj_free($5);
 }
 | COPY_OP REGISTER COMMA QUOTE cstring QUOTE CP
 { 
    $$=DCALLOC(BPX_option, 1, "BPX_option"); 
    $$->t=BPX_option_COPY; 
    $$->reg=$2; 
    list_of_bytes_to_array (&($$->copy_string), &($$->copy_string_len), $5); 
    obj_free($5);
 }
 ;

BPF_option
 : BPF_UNICODE                      { current_BPF->unicode=1; }
 | BPF_TRACE                        { current_BPF->trace=1; }
 | BPF_TRACE_COLON BPF_CC           { current_BPF->trace=1; current_BPF->trace_cc=1; }
 | BPF_SKIP                         { current_BPF->skip=1; } 
 | BPF_SKIP_STDCALL                 { current_BPF->skip_stdcall=1; }
 | BPF_PAUSE DEC_OR_HEX             { current_BPF->pause=$2; }
 | BPF_RT DEC_OR_HEX                { current_BPF->rt=obj_REG($2); }
 | BPF_RT_PROBABILITY float_or_perc { current_BPF->rt_probability=$2; }
 | BPF_ARGS DEC_OR_HEX              { current_BPF->args=$2; }
 | BPF_DUMP_ARGS DEC_OR_HEX         { current_BPF->dump_args=$2; }
 | WHEN_CALLED_FROM_ADDRESS address { current_BPF->when_called_from_address=$2; }
 | WHEN_CALLED_FROM_FUNC address    { current_BPF->when_called_from_func=$2; }
 ;

float_or_perc
 : FLOAT_NUMBER
 | DEC_NUMBER PERCENT { $$=(double)$1/(double)100; }
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
 | FILENAME_EXCLAMATION SYMBOL_NAME_PLUS DEC_OR_HEX
     { 
        $$=create_address_filename_symbol ($1, $2, $3); 
         DFREE ($1); 
         DFREE ($2); 
         // every new address, except of abs-address (which is already resolved)
         // is added to addresses resolving queue
         add_new_address_to_be_resolved ($$); 
     }
 | FILENAME_EXCLAMATION SYMBOL_NAME
     { 
        $$=create_address_filename_symbol ($1, $2, 0); 
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
 | TWO_POINTS   { $$=cons (obj_wyde(BYTEMASK_WILDCARD_BYTE), NULL); }
 | skip_n       { $$=obj_wyde_n_times (BYTEMASK_WILDCARD_BYTE, $1); }
 ;

skip_n
 : SKIP DEC_NUMBER R_SQUARE_BRACKET { $$=$2; }
 ;

%%

BP* parse_option(char *s)
{
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

    if (r==0 && breakpoints)
        return breakpoints;
    else
        return NULL;
};

void yyerror(char *s)
{
    fprintf(stderr, "bison error: %s\n", s);
}
