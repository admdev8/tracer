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
#include "ostrings.h"
#include "BPF.h"

#include "stuff.h"
#include "dmalloc.h"
#include "X86_register.h"

#include "opts_parse.h"
#include "opts_aux.h"

// BCC mangled names may be started with @

%}

%option stack
%option noyywrap

DEC            (0|[1-9][[:digit:]]*)
HEX            0[Xx][[:xdigit:]]+
FILENAME       [[:alnum:]_\+]+\.[[:alnum:]_]+
FILEPATH_FILENAME ([[:alnum:]\\:]+)?[[:alnum:]_\+]+\.[[:alnum:]_]+
SYMBOL_NAME    @?[[:alpha:]][[:alnum:]_@$]*
REGEXP         @?[[:alnum:]_@$.|*^$?\\]*
REGEXP_EXCL    @?[[:alnum:]_@$.|*^$?\\!]*
SYMBOL_NAME_RE @?[[:alpha:]\\?_$]{REGEXP}

%x STATE_BPM STATE_BPX STATE_BPF
%x STATE_ADDRESS STATE_ADDRESS_OR_REG STATE_BYTEMASK STATE_BYTEMASK_SKIP_OPT STATE_READ_ADR_OFS
%x STATE_FLOAT_NUMBER

%%

<INITIAL>bpx=   { BEGIN(STATE_BPX); yy_push_state(STATE_ADDRESS); return BPX_EQ; }
<INITIAL>bpf=   { 
    oassert (current_BPF==NULL); 
    current_BPF=DCALLOC (BPF, 1, "BPF"); 
    BEGIN(STATE_BPF); 
    yy_push_state(STATE_ADDRESS); 
    return BPF_EQ; 
    }
<INITIAL>bpmb=  { yylval.num=1; BEGIN(STATE_BPM); yy_push_state(STATE_ADDRESS); return BPM_width; }
<INITIAL>bpmw=  { yylval.num=2; BEGIN(STATE_BPM); yy_push_state(STATE_ADDRESS); return BPM_width; }
<INITIAL>bpmd=  { yylval.num=4; BEGIN(STATE_BPM); yy_push_state(STATE_ADDRESS); return BPM_width; }
<INITIAL>bpmq=  { yylval.num=8; BEGIN(STATE_BPM); yy_push_state(STATE_ADDRESS); return BPM_width; }
<INITIAL>-l:{FILEPATH_FILENAME} { yylval.str=DSTRDUP(yytext+3, "yytext"); return LOAD_FILENAME; };
<INITIAL>-a:{DEC}      { yylval.num=strtol(yytext+3, NULL, 10); return ATTACH_PID; };
<INITIAL>-a:{HEX}      { yylval.num=strtol(yytext+3, NULL, 16); return ATTACH_PID; };
<INITIAL>-a:{FILEPATH_FILENAME} { yylval.str=DSTRDUP(yytext+3, "yytext"); return ATTACH_FILENAME; };
<INITIAL>-c:.*         { 
    yylval.str=DSTRDUP(yytext+3, "yytext"); 
    //str_trim_all_lf_cr_right(yylval.str); 
    remove_char_begin_end_if_present(yylval.str, '"');
    return CMDLINE; 
    };
<INITIAL>--child       { return CHILD; }
<INITIAL>--dump-fpu    { return DUMP_FPU; }
<INITIAL>-q            { return DASH_Q; }
<INITIAL>-s            { return DASH_S; }
<INITIAL>-t            { return DASH_T; }
<INITIAL>--dump-xmm    { return DUMP_XMM; }
<INITIAL>--dump-seh    { return DUMP_SEH; }
<INITIAL>-dont-run-thread-b { return DONT_RUN_THREAD_B; }
<INITIAL>--one-time-INT3-bp:{REGEXP_EXCL}  {
        yylval.str=DSTRDUP(yytext+strlen("--one-time-INT3-bp:"), "yytext");
        return ONE_TIME_INT3_BP;
    }
<INITIAL>--allsymbols:{REGEXP_EXCL}  {
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
<INITIAL>--limit-trace-nestedness:{DEC} {
        yylval.num=strtol(yytext+25, NULL, 10); 
        return LIMIT_TRACE_NESTEDNESS;
    }
<INITIAL>--loading          { return LOADING; }
<INITIAL>--module-debug     { return MODULE_DEBUG; }
<INITIAL>--symbol-debug     { return SYMBOL_DEBUG; }
<INITIAL>--cycle-debug      { return CYCLE_DEBUG; }
<INITIAL>--BPX-debug        { return BPX_DEBUG; }
<INITIAL>--utils-debug      { return UTILS_DEBUG; }
<INITIAL>--cc-debug         { return CC_DEBUG; }
<INITIAL>--BPF-debug        { return BPF_DEBUG; }
<INITIAL>--tracing-debug    { return TRACING_DEBUG; }
<INITIAL>--emulator-testing { return EMULATOR_TESTING; }
<INITIAL>--no-new-console   { return NO_NEW_CONSOLE; }

<STATE_ADDRESS_OR_REG>(e|r)(ax|bx|cx|dx|si|di|bp|sp|ip)   { yylval.x86reg=X86_register_from_string(yytext); yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>(p|s|a|z|o|c|d)f    { yylval.x86reg=X86_register_from_string(yytext); yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r(8|9)d?l?w?    { yylval.x86reg=X86_register_from_string(yytext); yy_pop_state(); return REGISTER; }
<STATE_ADDRESS_OR_REG>r1[0-5]d?l?w?   { yylval.x86reg=X86_register_from_string(yytext); return REGISTER; }
<STATE_ADDRESS_OR_REG>st[0-7]   { yylval.x86reg=X86_register_from_string(yytext); BEGIN(STATE_FLOAT_NUMBER); return FPU_REGISTER; }

<STATE_ADDRESS,STATE_ADDRESS_OR_REG>"bytemask:\""             { yy_push_state(STATE_BYTEMASK); return BYTEMASK; }
<STATE_BYTEMASK>[[:xdigit:]][[:xdigit:]] { yylval.num = strtol(yytext, NULL, 16); return HEX_BYTE; }
<STATE_BYTEMASK>"[skip:"                  { yy_push_state(STATE_BYTEMASK_SKIP_OPT); return SKIP; }
<STATE_BYTEMASK>"\""                     { 
    if (YYSTATE==STATE_BYTEMASK)
    {
        yy_pop_state(); /* switch back to STATE_ADDRESS */
        yy_pop_state(); /* switch back to what was before */
    };
    return BYTEMASK_END; }

<STATE_READ_ADR_OFS,STATE_ADDRESS,STATE_ADDRESS_OR_REG,STATE_BPM,INITIAL,STATE_BPX,STATE_BPF>{HEX} {
    if ((YYSTATE==STATE_ADDRESS) || (YYSTATE==STATE_READ_ADR_OFS))
        yy_pop_state(); /* switch back to what was before STATE_ADDRESS */
    yylval.num=strtol_or_strtoll(yytext, NULL, 16); 
    return HEX_NUMBER; 
   }
<STATE_READ_ADR_OFS,INITIAL,STATE_ADDRESS,STATE_ADDRESS_OR_REG,STATE_BYTEMASK_SKIP_OPT,STATE_BPX,STATE_BPF>{DEC}   { 
    if (YYSTATE==STATE_READ_ADR_OFS || YYSTATE==STATE_BYTEMASK_SKIP_OPT)
        yy_pop_state(); /* switch back to what was before STATE_ADDRESS */
    yylval.num = strtol_or_strtoll(yytext, NULL, 10); 
    return DEC_NUMBER; 
   }
<STATE_BPF,STATE_FLOAT_NUMBER>-?[0-9]+(\.[0-9]+)?   { 
    if (YYSTATE==STATE_FLOAT_NUMBER)
        yy_pop_state(); /* switch back to what was before */
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

<STATE_BPX>"dump("  { yy_push_state(STATE_ADDRESS_OR_REG); return DUMP_OP; }
<STATE_BPX>"set("   { yy_push_state(STATE_ADDRESS_OR_REG); return SET_OP; }
<STATE_BPX>"copy("  { yy_push_state(STATE_ADDRESS_OR_REG); return COPY_OP; }
<STATE_BPX>"\\x"[[:xdigit:]][[:xdigit:]]   { yylval.num = strtol(yytext+2, NULL, 16); return CSTRING_BYTE ; }
<STATE_BPX>[^\\\",)\n]                         { yylval.num = yytext[0]; return CSTRING_BYTE ; }

<STATE_BPF>unicode                   { return BPF_UNICODE; }
<STATE_BPF>microsoft_fastcall        { return BPF_MICROSOFT_FASTCALL; }
<STATE_BPF>borland_fastcall          { return BPF_BORLAND_FASTCALL; }
<STATE_BPF>trace:                    { return BPF_TRACE_COLON; }
<STATE_BPF>trace                     { return BPF_TRACE; }
<STATE_BPF>skip                      { return BPF_SKIP; }
<STATE_BPF>skip_stdcall              { return BPF_SKIP_STDCALL; }
<STATE_BPF>pause:                    { return BPF_PAUSE; }
<STATE_BPF>rt:                       { return BPF_RT; }
<STATE_BPF>rt_probability:           { return BPF_RT_PROBABILITY; }
<STATE_BPF>args:                     { return BPF_ARGS; }
<STATE_BPF>cc                        { return BPF_CC; }
<STATE_BPF>dump_args:                { return BPF_DUMP_ARGS; }
<STATE_BPF>when_called_from_address: { yy_push_state(STATE_ADDRESS); return WHEN_CALLED_FROM_ADDRESS; }
<STATE_BPF>when_called_from_func:    { yy_push_state(STATE_ADDRESS); return WHEN_CALLED_FROM_FUNC; }
<STATE_BPF>set                       { return SET; }
<STATE_BPF>(byte|word|dword|dword64) { 
    if (stricmp(yytext, "byte")==0) { yylval.num=1; return BYTE_WORD_DWORD_DWORD64; };
    if (stricmp(yytext, "word")==0) { yylval.num=2; return BYTE_WORD_DWORD_DWORD64; };
    if (stricmp(yytext, "dword")==0) { yylval.num=4; return BYTE_WORD_DWORD_DWORD64; };
    if (stricmp(yytext, "dword64")==0) { yylval.num=8; return BYTE_WORD_DWORD_DWORD64; };
    }
<STATE_BPF>arg_                     { return ARG_; }
<STATE_BPF>arg                      { return ARG; }
<STATE_BPF>type                     { return TYPE; }
<STATE_BPF>int                      { return TYPE_INT; }
<STATE_BPF>\*double                 { return TYPE_PTR_TO_DOUBLE;  /* FIXME: do something with asterisk! */ }
<STATE_BPF>\*QString                { return TYPE_PTR_TO_QSTRING; /* FIXME: do something with asterisk! */ }
<STATE_BPF>QString                  { return TYPE_QSTRING; }

<*>. { 
    //fprintf (stderr, "Catch-all symbol: '%c'\n", yytext[0]); 
    return yytext[0]; 
}
%%

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

/* vim: set expandtab ts=4 sw=4 : */
