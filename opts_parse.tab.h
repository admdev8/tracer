/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_OPTS_PARSE_TAB_H_INCLUDED
# define YY_YY_OPTS_PARSE_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    SKIP = 258,
    COLON = 259,
    EOL = 260,
    BYTEMASK = 261,
    BYTEMASK_END = 262,
    BPX_EQ = 263,
    BPF_EQ = 264,
    _EOF = 265,
    DUMP_OP = 266,
    SET = 267,
    SET_OP = 268,
    COPY_OP = 269,
    BPF_CC = 270,
    BPF_PAUSE = 271,
    BPF_RT_PROBABILITY = 272,
    CHILD = 273,
    BPF_TRACE = 274,
    BPF_TRACE_COLON = 275,
    DASH_S = 276,
    DASH_Q = 277,
    DASH_T = 278,
    DONT_RUN_THREAD_B = 279,
    DUMP_FPU = 280,
    DUMP_XMM = 281,
    DUMP_SEH = 282,
    BPF_ARGS = 283,
    BPF_DUMP_ARGS = 284,
    BPF_RT = 285,
    BPF_SKIP = 286,
    BPF_SKIP_STDCALL = 287,
    BPF_UNICODE = 288,
    BPF_MICROSOFT_FASTCALL = 289,
    BPF_BORLAND_FASTCALL = 290,
    WHEN_CALLED_FROM_ADDRESS = 291,
    WHEN_CALLED_FROM_FUNC = 292,
    ARG_ = 293,
    LOADING = 294,
    NO_NEW_CONSOLE = 295,
    VERBOSE1 = 296,
    VERBOSE2 = 297,
    VERBOSE3 = 298,
    EMULATOR_TESTING = 299,
    TRACING_DEBUG = 300,
    NEWLINE = 301,
    ARG = 302,
    TYPE = 303,
    TYPE_INT = 304,
    TYPE_PTR_TO_DOUBLE = 305,
    TYPE_QSTRING = 306,
    TYPE_PTR_TO_QSTRING = 307,
    DEC_NUMBER = 308,
    HEX_NUMBER = 309,
    HEX_BYTE = 310,
    BPM_width = 311,
    CSTRING_BYTE = 312,
    ATTACH_PID = 313,
    DMALLOC_BREAK_ON = 314,
    LIMIT_TRACE_NESTEDNESS = 315,
    BYTE_WORD_DWORD_DWORD64 = 316,
    REGISTER = 317,
    FPU_REGISTER = 318,
    FLOAT_NUMBER = 319,
    FILENAME_EXCLAMATION = 320,
    SYMBOL_NAME_RE = 321,
    SYMBOL_NAME_RE_PLUS = 322,
    LOAD_FILENAME = 323,
    ATTACH_FILENAME = 324,
    CMDLINE = 325,
    ALL_SYMBOLS = 326,
    ONE_TIME_INT3_BP = 327
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 91 "opts_parse.y" /* yacc.c:1909  */

    char * str;
    REG num;
    double dbl;
    struct obj * o;
    struct bp_address *a;
    struct BPM *bpm;
    struct BP *bp;
    struct BPX_option *bpx_option;
    enum X86_register x86reg;
    enum function_type func_type;

#line 140 "opts_parse.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_OPTS_PARSE_TAB_H_INCLUDED  */
