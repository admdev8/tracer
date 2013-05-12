/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2006, 2009-2010 Free Software
   Foundation, Inc.
   
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


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     COMMA = 258,
     PLUS = 259,
     TWO_POINTS = 260,
     R_SQUARE_BRACKET = 261,
     SKIP = 262,
     COLON = 263,
     EOL = 264,
     BYTEMASK = 265,
     BYTEMASK_END = 266,
     BPX_EQ = 267,
     BPF_EQ = 268,
     W = 269,
     RW = 270,
     _EOF = 271,
     DUMP_OP = 272,
     SET_OP = 273,
     COPY_OP = 274,
     CP = 275,
     QUOTE = 276,
     PERCENT = 277,
     BPF_CC = 278,
     BPF_PAUSE = 279,
     BPF_RT_PROBABILITY = 280,
     CHILD = 281,
     BPF_TRACE = 282,
     BPF_TRACE_COLON = 283,
     BPF_ARGS = 284,
     BPF_DUMP_ARGS = 285,
     BPF_RT = 286,
     BPF_SKIP = 287,
     BPF_SKIP_STDCALL = 288,
     BPF_UNICODE = 289,
     WHEN_CALLED_FROM_ADDRESS = 290,
     WHEN_CALLED_FROM_FUNC = 291,
     DEC_NUMBER = 292,
     HEX_NUMBER = 293,
     HEX_BYTE = 294,
     BPM_width = 295,
     CSTRING_BYTE = 296,
     ATTACH_PID = 297,
     REGISTER = 298,
     FLOAT_NUMBER = 299,
     FILENAME_EXCLAMATION = 300,
     SYMBOL_NAME = 301,
     SYMBOL_NAME_PLUS = 302,
     LOAD_FILENAME = 303,
     ATTACH_FILENAME = 304,
     CMDLINE = 305
   };
#endif
/* Tokens.  */
#define COMMA 258
#define PLUS 259
#define TWO_POINTS 260
#define R_SQUARE_BRACKET 261
#define SKIP 262
#define COLON 263
#define EOL 264
#define BYTEMASK 265
#define BYTEMASK_END 266
#define BPX_EQ 267
#define BPF_EQ 268
#define W 269
#define RW 270
#define _EOF 271
#define DUMP_OP 272
#define SET_OP 273
#define COPY_OP 274
#define CP 275
#define QUOTE 276
#define PERCENT 277
#define BPF_CC 278
#define BPF_PAUSE 279
#define BPF_RT_PROBABILITY 280
#define CHILD 281
#define BPF_TRACE 282
#define BPF_TRACE_COLON 283
#define BPF_ARGS 284
#define BPF_DUMP_ARGS 285
#define BPF_RT 286
#define BPF_SKIP 287
#define BPF_SKIP_STDCALL 288
#define BPF_UNICODE 289
#define WHEN_CALLED_FROM_ADDRESS 290
#define WHEN_CALLED_FROM_FUNC 291
#define DEC_NUMBER 292
#define HEX_NUMBER 293
#define HEX_BYTE 294
#define BPM_width 295
#define CSTRING_BYTE 296
#define ATTACH_PID 297
#define REGISTER 298
#define FLOAT_NUMBER 299
#define FILENAME_EXCLAMATION 300
#define SYMBOL_NAME 301
#define SYMBOL_NAME_PLUS 302
#define LOAD_FILENAME 303
#define ATTACH_FILENAME 304
#define CMDLINE 305




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1685 of yacc.c  */
#line 30 "opts.y"

    char * str;
    int num;
    double dbl;
    struct _obj * o;
    struct _bp_address *a;
    struct _BPM *bpm;
    struct _BP *bp;
    struct _BPX_option *bpx_option;
    X86_register x86reg;



/* Line 1685 of yacc.c  */
#line 165 "y.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


