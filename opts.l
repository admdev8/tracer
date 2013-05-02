%{
#include <assert.h>

#include "dmalloc.h"
#include "X86_register.h"

#include "opts.tab.h"
#include "opts.h"

void print_current_state();

%}

%option stack

DEC         (0|[1-9][[:digit:]]*)
HEX         0[Xx][[:xdigit:]]+
FILENAME    [[:alnum:]]+\.[[:alpha:]]+
SYMBOL_NAME [[:alpha:]][[:alnum:]_@]*

%x STATE_BPM STATE_BPX STATE_BPF
%x STATE_ADDRESS STATE_ADDRESS_OR_REG STATE_BYTEMASK STATE_BYTEMASK_SKIP_OPT STATE_READ_ADR_OFS

%%

<INITIAL>bpx=   { BEGIN(STATE_BPX); yy_push_state(STATE_ADDRESS); return BPX_EQ; }
<INITIAL>bpf=   { assert (current_BPF==NULL); current_BPF=DCALLOC (BPF, 1, "BPF"); current_BPF->rt_probability=1; BEGIN(STATE_BPF); yy_push_state(STATE_ADDRESS); return BPF_EQ; }
<INITIAL>bpmb=  { yylval.num=1; BEGIN(STATE_BPM); yy_push_state(STATE_ADDRESS); return BPM_width; }
<INITIAL>bpmw=  { yylval.num=2; BEGIN(STATE_BPM); yy_push_state(STATE_ADDRESS); return BPM_width; }
<INITIAL>bpmd=  { yylval.num=4; BEGIN(STATE_BPM); yy_push_state(STATE_ADDRESS); return BPM_width; }
<INITIAL>bpmq=  { yylval.num=8; BEGIN(STATE_BPM); yy_push_state(STATE_ADDRESS); return BPM_width; }

<STATE_ADDRESS_OR_REG>eax   { yylval.x86reg=R_EAX; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>ebx   { yylval.x86reg=R_EBX; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>ecx   { yylval.x86reg=R_ECX; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>edx   { yylval.x86reg=R_EDX; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>esi   { yylval.x86reg=R_ESI; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>edi   { yylval.x86reg=R_EDI; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>ebp   { yylval.x86reg=R_EBP; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>esp   { yylval.x86reg=R_ESP; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>eip   { yylval.x86reg=R_EIP; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>pf    { yylval.x86reg=R_PF;  yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>sf    { yylval.x86reg=R_SF;  yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>af    { yylval.x86reg=R_AF;  yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>zf    { yylval.x86reg=R_ZF;  yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>of    { yylval.x86reg=R_OF;  yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>cf    { yylval.x86reg=R_CF;  yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>df    { yylval.x86reg=R_DF;  yy_pop_state(); return REGISTER; }

<STATE_ADDRESS,STATE_ADDRESS_OR_REG>"bytemask:\""             { yy_push_state(STATE_BYTEMASK); return BYTEMASK; }
<STATE_BYTEMASK>[[:xdigit:]][[:xdigit:]] { yylval.num = strtol(yytext, NULL, 16); return HEX_BYTE; }
<STATE_BYTEMASK>"[skip:"                  { yy_push_state(STATE_BYTEMASK_SKIP_OPT); return SKIP; }
<STATE_BYTEMASK>".."                     { return TWO_POINTS; }
<STATE_BYTEMASK>"\""                     { 
    if (YYSTATE==STATE_BYTEMASK)
    {
        yy_pop_state(); /* switch back to STATE_ADDRESS */
        yy_pop_state(); /* switch back to what was before */
    };
    return BYTEMASK_END; }
<STATE_BYTEMASK>"]"                      { return R_SQUARE_BRACKET; }

<STATE_READ_ADR_OFS,STATE_ADDRESS,STATE_ADDRESS_OR_REG,STATE_BPM,INITIAL,STATE_BPX,STATE_BPF>{HEX}                     {
    if ((YYSTATE==STATE_ADDRESS) || (YYSTATE==STATE_READ_ADR_OFS))
        yy_pop_state(); /* switch back to what was before STATE_ADDRESS */
    yylval.num = strtol(yytext, NULL, 16); 
    return HEX_NUMBER; }
<STATE_READ_ADR_OFS,INITIAL,STATE_ADDRESS,STATE_ADDRESS_OR_REG,STATE_BYTEMASK_SKIP_OPT,STATE_BPX,STATE_BPF>{DEC}   { 
    if (YYSTATE==STATE_READ_ADR_OFS || YYSTATE==STATE_BYTEMASK_SKIP_OPT)
        yy_pop_state(); /* switch back to what was before STATE_ADDRESS */
    yylval.num = strtol(yytext, NULL, 10); 
    return DEC_NUMBER; 
}
<STATE_BPF>[0-9]+\.([0-9]+)?   { 
    yylval.dbl = atof(yytext); 
    return FLOAT_NUMBER; 
}
<STATE_ADDRESS,STATE_ADDRESS_OR_REG>{FILENAME}!               { yylval.str = strdup(yytext); str_trim_one_char_right(yylval.str); return FILENAME_EXCLAMATION; }
<STATE_ADDRESS,STATE_ADDRESS_OR_REG>{SYMBOL_NAME}\+           { 
    BEGIN (STATE_READ_ADR_OFS);
    yylval.str = strdup(yytext);
    str_trim_one_char_right(yylval.str);
    return SYMBOL_NAME_PLUS; }
<STATE_ADDRESS,STATE_ADDRESS_OR_REG>{SYMBOL_NAME}           { 
    yy_pop_state(); /* switch back to what was before STATE_ADDRESS */
    yylval.str = strdup(yytext); 
    return SYMBOL_NAME; }

<STATE_BPM,STATE_ADDRESS,STATE_ADDRESS_OR_REG,STATE_BPX,STATE_BPF>","                 { return COMMA; }
<STATE_BPM>w                            { return W; }
<STATE_BPM>rw                            { return RW; }

<STATE_BPX>"dump("  { yy_push_state(STATE_ADDRESS_OR_REG); return DUMP_OP; }
<STATE_BPX>"set("  { yy_push_state(STATE_ADDRESS_OR_REG); return SET_OP; }
<STATE_BPX>"copy("  { yy_push_state(STATE_ADDRESS_OR_REG); return COPY_OP; }
<STATE_BPX>")"     { return CP; }
<STATE_BPX>"\""     { return QUOTE; }
<STATE_BPX>"\\x"[[:xdigit:]][[:xdigit:]]   { yylval.num = strtol(yytext+2, NULL, 16); return CSTRING_BYTE ; }
<STATE_BPX>[^\\\"]                         { yylval.num = yytext[0]; return CSTRING_BYTE ; }
<STATE_BPF>unicode                   { return BPF_UNICODE; }
<STATE_BPF>trace:                    { return BPF_TRACE_COLON; }
<STATE_BPF>trace                     { return BPF_TRACE; }
<STATE_BPF>skip                      { return BPF_SKIP; }
<STATE_BPF>skip_stdcall              { return BPF_SKIP_STDCALL; }
<STATE_BPF>pause:                    { return BPF_PAUSE; }
<STATE_BPF>rt:                       { return BPF_RT; }
<STATE_BPF>rt_probability:           { return BPF_RT_PROBABILITY; }
<STATE_BPF>args:                     { return BPF_ARGS; }
<STATE_BPF>cc                        { return BPF_CC; }
<STATE_BPF>"%"                       { return PERCENT; }
<STATE_BPF>dump_args:                { return BPF_DUMP_ARGS; }
<STATE_BPF>when_called_from_address: { yy_push_state(STATE_ADDRESS); return WHEN_CALLED_FROM_ADDRESS; }
<STATE_BPF>when_called_from_func:    { yy_push_state(STATE_ADDRESS); return WHEN_CALLED_FROM_FUNC; }
<STATE_BPM,STATE_BPX,STATE_BPF,STATE_ADDRESS,STATE_ADDRESS_OR_REG,STATE_BYTEMASK,STATE_BYTEMASK_SKIP_OPT,STATE_READ_ADR_OFS>.                        { fprintf (stderr, "Unrecognized symbol: '%c'\n", yytext[0]); return yytext[0]; }
%%

//<STATE_BPX><<EOF>> { return _EOF; }
// [ \t]                    { /* ignore comment and white space */ }
// ":"                      { return COLON; }
// \n                       { return EOL; }
/* <STATE_ADDRESS>"+"                        { return PLUS; } */
// .                        { fprintf(stderr, "Unparsed character %c\n", *yytext); }
void flex_set_str(char *s)
{
    YY_BUFFER_STATE buf;
    buf=yy_scan_string (s);
    yy_switch_to_buffer (buf);
};

void flex_cleanup()
{
    yy_delete_buffer( YY_CURRENT_BUFFER );
};

void flex_restart()
{
    BEGIN(INITIAL);
};
