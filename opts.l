%{
#include <assert.h>

#include "stuff.h"
#include "dmalloc.h"
#include "X86_register.h"

#include "opts.tab.h"
#include "opts.h"
#include "BPF.h"
%}

%option stack

DEC            (0|[1-9][[:digit:]]*)
HEX            0[Xx][[:xdigit:]]+
FILENAME       [[:alnum:]_]+\.[[:alnum:]_]+
SYMBOL_NAME    [[:alpha:]][[:alnum:]_@]*
REGEXP         [[:alnum:]_@.|*^$?\\]*
SYMBOL_NAME_RE [[:alpha:]\\?_]{REGEXP}

%x STATE_BPM STATE_BPX STATE_BPF
%x STATE_ADDRESS STATE_ADDRESS_OR_REG STATE_BYTEMASK STATE_BYTEMASK_SKIP_OPT STATE_READ_ADR_OFS

%%

<INITIAL>bpx=   { BEGIN(STATE_BPX); yy_push_state(STATE_ADDRESS); return BPX_EQ; }
<INITIAL>bpf=   { 
    assert (current_BPF==NULL); 
    current_BPF=DCALLOC (BPF, 1, "BPF"); 
    current_BPF->rt_probability=0; 
    BEGIN(STATE_BPF); 
    yy_push_state(STATE_ADDRESS); 
    return BPF_EQ; 
    }
<INITIAL>bpmb=  { yylval.num=1; BEGIN(STATE_BPM); yy_push_state(STATE_ADDRESS); return BPM_width; }
<INITIAL>bpmw=  { yylval.num=2; BEGIN(STATE_BPM); yy_push_state(STATE_ADDRESS); return BPM_width; }
<INITIAL>bpmd=  { yylval.num=4; BEGIN(STATE_BPM); yy_push_state(STATE_ADDRESS); return BPM_width; }
<INITIAL>bpmq=  { yylval.num=8; BEGIN(STATE_BPM); yy_push_state(STATE_ADDRESS); return BPM_width; }
<INITIAL>-l:{FILENAME} { yylval.str=DSTRDUP(yytext+3, "yytext"); return LOAD_FILENAME; };
<INITIAL>-a:{DEC}      { yylval.num=strtol(yytext+3, NULL, 10); return ATTACH_PID; };
<INITIAL>-a:{HEX}      { yylval.num=strtol(yytext+3, NULL, 16); return ATTACH_PID; };
<INITIAL>-a:{FILENAME} { yylval.str=DSTRDUP(yytext+3, "yytext"); return ATTACH_FILENAME; };
<INITIAL>-c:.*         { yylval.str=DSTRDUP(yytext+3, "yytext"); return CMDLINE; };
<INITIAL>--child       { return CHILD; }
<INITIAL>-s            { return DASH_S; }
<INITIAL>-dont-run-thread-b { return DONT_RUN_THREAD_B; }
<INITIAL>--allsymbols:{REGEXP}  {
        yylval.str=DSTRDUP(yytext+strlen("--allsymbols:"), "yytext"); 
        //printf ("--allsymbols: [%s]\n", yylval.str);
        return ALL_SYMBOLS; 
    }
<INITIAL>--allsymbols           {
        yylval.str=NULL; 
        return ALL_SYMBOLS; 
    }
<INITIAL>--dmalloc-break-on:{DEC} {
        yylval.num=strtol(yytext+19, NULL, 10); 
        return DMALLOC_BREAK_ON;
    }
<INITIAL>--module-c-debug   { return MODULE_C_DEBUG; }
<INITIAL>--cycle-c-debug    { return CYCLE_C_DEBUG; }
<INITIAL>--bpx-c-debug      { return BPX_C_DEBUG; }
<INITIAL>--utils-c-debug    { return UTILS_C_DEBUG; }
<INITIAL>--cc-c-debug       { return CC_C_DEBUG; }

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
<STATE_ADDRESS_OR_REG>rax   { yylval.x86reg=R_RAX; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>rbx   { yylval.x86reg=R_RBX; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>rcx   { yylval.x86reg=R_RCX; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>rdx   { yylval.x86reg=R_RDX; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>rsi   { yylval.x86reg=R_RSI; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>rdi   { yylval.x86reg=R_RDI; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>rsp   { yylval.x86reg=R_RSP; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>rbp   { yylval.x86reg=R_RBP; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>rip   { yylval.x86reg=R_RIP; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r8    { yylval.x86reg=R_R8;  yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r9    { yylval.x86reg=R_R9;  yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r10   { yylval.x86reg=R_R10; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r11   { yylval.x86reg=R_R11; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r12   { yylval.x86reg=R_R12; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r13   { yylval.x86reg=R_R13; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r14   { yylval.x86reg=R_R14; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r15   { yylval.x86reg=R_R15; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r8d   { yylval.x86reg=R_R8D;  yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r9d   { yylval.x86reg=R_R9D;  yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r10d  { yylval.x86reg=R_R10D; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r11d  { yylval.x86reg=R_R11D; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r12d  { yylval.x86reg=R_R12D; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r13d  { yylval.x86reg=R_R13D; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r14d  { yylval.x86reg=R_R14D; yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r15d  { yylval.x86reg=R_R15D; yy_pop_state(); return REGISTER; }

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
#ifdef _WIN64
    yylval.num = strtoll(yytext, NULL, 16); 
#else
    yylval.num = strtol(yytext, NULL, 16); 
#endif
    return HEX_NUMBER; }
<STATE_READ_ADR_OFS,INITIAL,STATE_ADDRESS,STATE_ADDRESS_OR_REG,STATE_BYTEMASK_SKIP_OPT,STATE_BPX,STATE_BPF>{DEC}   { 
    if (YYSTATE==STATE_READ_ADR_OFS || YYSTATE==STATE_BYTEMASK_SKIP_OPT)
        yy_pop_state(); /* switch back to what was before STATE_ADDRESS */
#ifdef _WIN64
    yylval.num = strtoll(yytext, NULL, 10); 
#else
    yylval.num = strtol(yytext, NULL, 10); 
#endif
    return DEC_NUMBER; 
}
<STATE_BPF>[0-9]+\.([0-9]+)?   { 
    yylval.dbl = atof(yytext); 
    return FLOAT_NUMBER; 
}
<STATE_ADDRESS,STATE_ADDRESS_OR_REG>{FILENAME}!               { yylval.str = DSTRDUP(yytext, "yytext"); str_trim_one_char_right(yylval.str); return FILENAME_EXCLAMATION; }
<STATE_ADDRESS,STATE_ADDRESS_OR_REG>{SYMBOL_NAME_RE}\+           { 
    BEGIN (STATE_READ_ADR_OFS);
    yylval.str = DSTRDUP(yytext, "yytext");
    str_trim_one_char_right(yylval.str);
    return SYMBOL_NAME_RE_PLUS; }
<STATE_ADDRESS,STATE_ADDRESS_OR_REG>{SYMBOL_NAME_RE}           { 
    yy_pop_state(); /* switch back to what was before STATE_ADDRESS */
    yylval.str = DSTRDUP(yytext, "yytext"); 
    return SYMBOL_NAME_RE; }

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
