/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "opts_parse.y" /* yacc.c:339  */


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
#include "opts_aux.h"
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


#line 154 "opts_parse.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "opts_parse.tab.h".  */
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
    MODULE_DEBUG = 296,
    SYMBOL_DEBUG = 297,
    CYCLE_DEBUG = 298,
    BPX_DEBUG = 299,
    UTILS_DEBUG = 300,
    CC_DEBUG = 301,
    BPF_DEBUG = 302,
    EMULATOR_TESTING = 303,
    TRACING_DEBUG = 304,
    NEWLINE = 305,
    ARG = 306,
    TYPE = 307,
    TYPE_INT = 308,
    TYPE_PTR_TO_DOUBLE = 309,
    TYPE_QSTRING = 310,
    TYPE_PTR_TO_QSTRING = 311,
    DEC_NUMBER = 312,
    HEX_NUMBER = 313,
    HEX_BYTE = 314,
    BPM_width = 315,
    CSTRING_BYTE = 316,
    ATTACH_PID = 317,
    DMALLOC_BREAK_ON = 318,
    LIMIT_TRACE_NESTEDNESS = 319,
    BYTE_WORD_DWORD_DWORD64 = 320,
    REGISTER = 321,
    FPU_REGISTER = 322,
    FLOAT_NUMBER = 323,
    FILENAME_EXCLAMATION = 324,
    SYMBOL_NAME_RE = 325,
    SYMBOL_NAME_RE_PLUS = 326,
    LOAD_FILENAME = 327,
    ATTACH_FILENAME = 328,
    CMDLINE = 329,
    ALL_SYMBOLS = 330,
    ONE_TIME_INT3_BP = 331
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 90 "opts_parse.y" /* yacc.c:355  */

    char * str;
    REG num;
    double dbl;
    struct _obj * o;
    struct _bp_address *a;
    struct _BPM *bpm;
    struct _BP *bp;
    struct _BPX_option *bpx_option;
    enum X86_register x86reg;
    function_type func_type;

#line 284 "opts_parse.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_OPTS_PARSE_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 301 "opts_parse.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  43
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   151

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  92
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  20
/* YYNRULES -- Number of rules.  */
#define YYNRULES  90
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  161

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   331

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      77,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    82,     2,     2,    89,     2,     2,
      83,    81,    84,    85,    78,     2,    90,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    88,     2,
       2,    86,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    91,     2,    87,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    80,     2,     2,     2,     2,    79,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   132,   132,   133,   137,   138,   139,   147,   148,   149,
     150,   151,   152,   153,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
     170,   171,   172,   183,   192,   194,   199,   201,   209,   210,
     214,   221,   226,   227,   231,   233,   235,   237,   239,   241,
     243,   251,   262,   263,   264,   265,   266,   267,   268,   269,
     270,   272,   278,   279,   280,   281,   282,   290,   300,   301,
     302,   303,   307,   311,   312,   316,   317,   321,   325,   334,
     341,   347,   356,   357,   361,   365,   366,   370,   371,   372,
     376
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SKIP", "COLON", "EOL", "BYTEMASK",
  "BYTEMASK_END", "BPX_EQ", "BPF_EQ", "_EOF", "DUMP_OP", "SET", "SET_OP",
  "COPY_OP", "BPF_CC", "BPF_PAUSE", "BPF_RT_PROBABILITY", "CHILD",
  "BPF_TRACE", "BPF_TRACE_COLON", "DASH_S", "DASH_Q", "DASH_T",
  "DONT_RUN_THREAD_B", "DUMP_FPU", "DUMP_XMM", "DUMP_SEH", "BPF_ARGS",
  "BPF_DUMP_ARGS", "BPF_RT", "BPF_SKIP", "BPF_SKIP_STDCALL", "BPF_UNICODE",
  "BPF_MICROSOFT_FASTCALL", "BPF_BORLAND_FASTCALL",
  "WHEN_CALLED_FROM_ADDRESS", "WHEN_CALLED_FROM_FUNC", "ARG_", "LOADING",
  "NO_NEW_CONSOLE", "MODULE_DEBUG", "SYMBOL_DEBUG", "CYCLE_DEBUG",
  "BPX_DEBUG", "UTILS_DEBUG", "CC_DEBUG", "BPF_DEBUG", "EMULATOR_TESTING",
  "TRACING_DEBUG", "NEWLINE", "ARG", "TYPE", "TYPE_INT",
  "TYPE_PTR_TO_DOUBLE", "TYPE_QSTRING", "TYPE_PTR_TO_QSTRING",
  "DEC_NUMBER", "HEX_NUMBER", "HEX_BYTE", "BPM_width", "CSTRING_BYTE",
  "ATTACH_PID", "DMALLOC_BREAK_ON", "LIMIT_TRACE_NESTEDNESS",
  "BYTE_WORD_DWORD_DWORD64", "REGISTER", "FPU_REGISTER", "FLOAT_NUMBER",
  "FILENAME_EXCLAMATION", "SYMBOL_NAME_RE", "SYMBOL_NAME_RE_PLUS",
  "LOAD_FILENAME", "ATTACH_FILENAME", "CMDLINE", "ALL_SYMBOLS",
  "ONE_TIME_INT3_BP", "'\\n'", "','", "'w'", "'r'", "')'", "'\"'", "'('",
  "'*'", "'+'", "'='", "'_'", "':'", "'%'", "'.'", "']'", "$accept",
  "tracer_option", "tracer_option_without_newline", "bpm", "bpx", "bpf",
  "BPX_options", "BPF_options", "BPX_option", "BPF_option", "FN_TYPE",
  "ARGUMENT_N", "float_or_perc", "cstring", "address", "DEC_OR_HEX",
  "abs_address", "bytemask", "bytemask_element", "skip_n", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,    10,    44,   119,
     114,    41,    34,    40,    42,    43,    61,    95,    58,    37,
      46,    93
};
# endif

#define YYPACT_NINF -122

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-122)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      30,    22,    22,  -122,  -122,  -122,  -122,  -122,  -122,  -122,
    -122,  -122,  -122,  -122,  -122,  -122,  -122,  -122,  -122,  -122,
    -122,  -122,    22,  -122,  -122,  -122,  -122,  -122,  -122,  -122,
    -122,    16,   -51,  -122,  -122,  -122,    -1,  -122,   -57,   -42,
    -122,   -41,   -38,  -122,  -122,   -36,  -122,   -46,    35,    -1,
    -122,  -122,  -122,   -28,     6,    96,   -48,   -45,  -122,  -122,
    -122,  -122,  -122,  -122,    29,   -33,    41,  -122,   -29,   -24,
     -28,   -50,  -122,    45,   -28,   -28,   -28,  -122,  -122,  -122,
    -122,  -122,    22,    22,    -7,  -122,   -17,  -122,   -16,  -122,
     -56,   -54,   -14,   -12,   -10,     3,     6,    -3,  -122,     7,
    -122,  -122,  -122,  -122,  -122,  -122,  -122,  -122,    -5,    96,
    -122,   -28,  -122,   -28,  -122,   -28,    18,    15,    19,  -122,
      31,  -122,    48,  -122,    33,    36,    37,    38,    50,    50,
      39,    32,  -122,  -122,  -122,  -122,    50,    40,    52,    53,
     -44,  -122,    54,    56,    83,  -122,  -122,  -122,  -122,  -122,
    -122,  -122,    81,    55,  -122,   -28,    58,    57,   -28,    60,
    -122
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,    10,    11,    12,    13,    17,    14,    15,
      16,    19,    29,    20,    21,    22,    23,    24,    25,    26,
      28,    27,     0,     9,    30,    31,     7,     8,    18,    32,
      33,     0,     2,     4,     5,     6,     0,    84,     0,    36,
      77,    38,     0,     1,     3,     0,    87,     0,     0,    86,
      89,    80,    79,     0,     0,     0,     0,     0,    88,    81,
      85,    82,    83,    78,     0,     0,     0,    37,    41,     0,
       0,     0,    55,     0,     0,     0,     0,    57,    58,    52,
      53,    54,     0,     0,     0,    39,    43,    34,     0,    90,
       0,     0,     0,     0,     0,     0,     0,     0,    59,     0,
      73,    61,    56,    62,    63,    60,    64,    65,     0,     0,
      35,     0,    47,     0,    45,     0,     0,     0,     0,    40,
       0,    74,     0,    42,     0,     0,     0,     0,     0,     0,
       0,     0,    46,    44,    48,    49,    76,     0,     0,     0,
       0,    75,     0,     0,     0,    68,    69,    70,    71,    67,
      51,    50,     0,     0,    72,     0,     0,     0,     0,     0,
      66
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
    -122,  -122,  -122,  -122,  -122,  -122,    46,    42,  -122,  -122,
    -122,  -122,  -122,  -121,     1,   -70,  -122,    95,  -122,  -122
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    31,    32,    33,    34,    35,    67,    85,    68,    86,
     149,   153,   101,   137,    39,    63,    40,    48,    49,    50
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      98,    51,    45,    41,   103,   104,   105,    99,   138,   145,
     146,   147,   148,    52,    53,   141,    43,    64,   100,    65,
      66,    57,   111,    42,   113,   112,    44,   114,    36,    61,
      62,    87,    88,    92,    93,    36,    54,    55,     1,     2,
      56,   124,    59,   125,    58,   126,    89,    36,     3,    96,
     108,     4,     5,     6,     7,     8,     9,    10,    46,    97,
     102,   109,   120,   110,   115,    91,   116,    95,   117,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      37,   118,   122,   106,   107,   156,   127,    37,   159,    47,
      22,    38,    23,    24,    25,    90,   121,   128,    38,    37,
     131,   129,    26,    27,    28,    29,    30,    94,    69,   130,
      38,   136,    70,    71,   132,    72,    73,   133,   134,   135,
     140,   152,   142,   139,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,   143,   150,   144,   151,   154,   157,
     155,   160,   119,   158,    60,     0,     0,    84,     0,     0,
       0,   123
};

static const yytype_int16 yycheck[] =
{
      70,    58,     3,     2,    74,    75,    76,    57,   129,    53,
      54,    55,    56,    70,    71,   136,     0,    11,    68,    13,
      14,    57,    78,    22,    78,    81,    77,    81,     6,    57,
      58,    79,    80,    66,    67,     6,    78,    78,     8,     9,
      78,   111,     7,   113,    90,   115,    91,     6,    18,    78,
      57,    21,    22,    23,    24,    25,    26,    27,    59,    83,
      15,    78,    65,    79,    78,    64,    78,    66,    78,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      58,    78,    87,    82,    83,   155,    68,    58,   158,    90,
      60,    69,    62,    63,    64,    66,    89,    82,    69,    58,
      52,    82,    72,    73,    74,    75,    76,    66,    12,    78,
      69,    61,    16,    17,    81,    19,    20,    81,    81,    81,
      88,    38,    82,    84,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    82,    81,    83,    81,    57,    81,
      85,    81,    96,    86,    49,    -1,    -1,    51,    -1,    -1,
      -1,   109
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     8,     9,    18,    21,    22,    23,    24,    25,    26,
      27,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    60,    62,    63,    64,    72,    73,    74,    75,
      76,    93,    94,    95,    96,    97,     6,    58,    69,   106,
     108,   106,   106,     0,    77,     3,    59,    90,   109,   110,
     111,    58,    70,    71,    78,    78,    78,    57,    90,     7,
     109,    57,    58,   107,    11,    13,    14,    98,   100,    12,
      16,    17,    19,    20,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    51,    99,   101,    79,    80,    91,
      66,   106,    66,    67,    66,   106,    78,    83,   107,    57,
      68,   104,    15,   107,   107,   107,   106,   106,    57,    78,
      79,    78,    81,    78,    81,    78,    78,    78,    78,    98,
      65,    89,    87,    99,   107,   107,   107,    68,    82,    82,
      78,    52,    81,    81,    81,    81,    61,   105,   105,    84,
      88,   105,    82,    82,    83,    53,    54,    55,    56,   102,
      81,    81,    38,   103,    57,    85,   107,    81,    86,   107,
      81
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    92,    93,    93,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    94,    94,    94,
      94,    94,    94,    94,    95,    95,    96,    96,    97,    97,
      98,    98,    99,    99,   100,   100,   100,   100,   100,   100,
     100,   100,   101,   101,   101,   101,   101,   101,   101,   101,
     101,   101,   101,   101,   101,   101,   101,   101,   102,   102,
     102,   102,   103,   104,   104,   105,   105,   106,   106,   106,
     106,   106,   107,   107,   108,   109,   109,   110,   110,   110,
     111
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     4,     5,     2,     4,     2,     4,
       3,     1,     3,     1,     5,     3,     5,     3,     5,     5,
       7,     7,     1,     1,     1,     1,     2,     1,     1,     2,
       2,     2,     2,     2,     2,     2,    13,     6,     1,     1,
       1,     1,     2,     1,     2,     2,     1,     1,     3,     2,
       2,     3,     1,     1,     1,     2,     1,     1,     2,     1,
       3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:
#line 137 "opts_parse.y" /* yacc.c:1646  */
    { add_new_BP ((yyvsp[0].bp)); }
#line 1511 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 138 "opts_parse.y" /* yacc.c:1646  */
    { add_new_BP ((yyvsp[0].bp)); }
#line 1517 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 139 "opts_parse.y" /* yacc.c:1646  */
    { 
   if (current_BPF->rt_present==false && current_BPF->rt_probability_present==true)
       die ("rt_probability option without rt option is useless. exiting.\n");
   BP *bp=create_BP(BP_type_BPF, current_BPF_address, current_BPF);
   add_new_BP (bp); 
   current_BPF=NULL;
   current_BPF_address=NULL;
 }
#line 1530 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 147 "opts_parse.y" /* yacc.c:1646  */
    { load_filename=(yyvsp[0].str); }
#line 1536 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 148 "opts_parse.y" /* yacc.c:1646  */
    { attach_filename=(yyvsp[0].str); }
#line 1542 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 149 "opts_parse.y" /* yacc.c:1646  */
    { attach_PID=(yyvsp[0].num); }
#line 1548 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 150 "opts_parse.y" /* yacc.c:1646  */
    { debug_children=true; }
#line 1554 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 151 "opts_parse.y" /* yacc.c:1646  */
    { dash_s=true; }
#line 1560 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 152 "opts_parse.y" /* yacc.c:1646  */
    { quiet=true; }
#line 1566 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 153 "opts_parse.y" /* yacc.c:1646  */
    { L_timestamp=true; }
#line 1572 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 154 "opts_parse.y" /* yacc.c:1646  */
    { dump_fpu=true; }
#line 1578 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 155 "opts_parse.y" /* yacc.c:1646  */
    { dump_xmm=true; }
#line 1584 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 156 "opts_parse.y" /* yacc.c:1646  */
    { dump_seh=true; }
#line 1590 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 157 "opts_parse.y" /* yacc.c:1646  */
    { run_thread_b=false; }
#line 1596 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 158 "opts_parse.y" /* yacc.c:1646  */
    { load_command_line=(yyvsp[0].str); }
#line 1602 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 159 "opts_parse.y" /* yacc.c:1646  */
    { opt_loading=true; }
#line 1608 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 160 "opts_parse.y" /* yacc.c:1646  */
    { module_c_debug=true; }
#line 1614 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 161 "opts_parse.y" /* yacc.c:1646  */
    { symbol_c_debug=true; }
#line 1620 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 162 "opts_parse.y" /* yacc.c:1646  */
    { cycle_c_debug=true; }
#line 1626 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 163 "opts_parse.y" /* yacc.c:1646  */
    { bpx_c_debug=true; }
#line 1632 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 164 "opts_parse.y" /* yacc.c:1646  */
    { utils_c_debug=true; }
#line 1638 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 165 "opts_parse.y" /* yacc.c:1646  */
    { cc_c_debug=true; }
#line 1644 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 166 "opts_parse.y" /* yacc.c:1646  */
    { BPF_c_debug=true; }
#line 1650 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 167 "opts_parse.y" /* yacc.c:1646  */
    { tracing_debug=true; }
#line 1656 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 168 "opts_parse.y" /* yacc.c:1646  */
    { emulator_testing=true; }
#line 1662 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 169 "opts_parse.y" /* yacc.c:1646  */
    { create_new_console=false; }
#line 1668 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 170 "opts_parse.y" /* yacc.c:1646  */
    { dmalloc_break_at_seq_n ((yyvsp[0].num)); }
#line 1674 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 171 "opts_parse.y" /* yacc.c:1646  */
    { limit_trace_nestedness=(yyvsp[0].num); }
#line 1680 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 172 "opts_parse.y" /* yacc.c:1646  */
    { 
    dump_all_symbols=true;
    if ((yyvsp[0].str))
    {
        oassert(dump_all_symbols_re==NULL);
        dump_all_symbols_re=DCALLOC(regex_t, 1, "regex_t");
        regcomp_or_die(dump_all_symbols_re, (yyvsp[0].str), REG_EXTENDED | REG_ICASE | REG_NEWLINE);
        DFREE((yyvsp[0].str));
        //printf ("dump_all_symbols_re is set\n");
    }
    }
#line 1696 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 183 "opts_parse.y" /* yacc.c:1646  */
    { 
    oassert(one_time_int3_bp_re==NULL);
    one_time_int3_bp_re=DCALLOC(regex_t, 1, "regex_t");
    regcomp_or_die(one_time_int3_bp_re, (yyvsp[0].str), REG_EXTENDED | REG_ICASE | REG_NEWLINE);
    DFREE((yyvsp[0].str));
    }
#line 1707 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 193 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bp)=create_BP(BP_type_BPM, (yyvsp[-2].a), create_BPM ((yyvsp[-3].num), BPM_type_W)); }
#line 1713 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 195 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bp)=create_BP(BP_type_BPM, (yyvsp[-3].a), create_BPM ((yyvsp[-4].num), BPM_type_RW)); }
#line 1719 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 200 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bp)=create_BP(BP_type_BPX, (yyvsp[0].a), create_BPX (NULL)); }
#line 1725 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 202 "opts_parse.y" /* yacc.c:1646  */
    { 
       (yyval.bp)=create_BP(BP_type_BPX, (yyvsp[-2].a), create_BPX (current_BPX_option)); 
       current_BPX_option=NULL;
   }
#line 1734 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 209 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF_address=(yyvsp[0].a); }
#line 1740 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 210 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF_address=(yyvsp[-2].a); }
#line 1746 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 215 "opts_parse.y" /* yacc.c:1646  */
    { 
     BPX_option *o;
     oassert(current_BPX_option);
     for (o=current_BPX_option; o->next; o=o->next);
     o->next=(yyvsp[-2].bpx_option);
 }
#line 1757 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 222 "opts_parse.y" /* yacc.c:1646  */
    { current_BPX_option=(yyvsp[0].bpx_option); }
#line 1763 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 232 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bpx_option)=DCALLOC(BPX_option, 1, "BPX_option"); (yyval.bpx_option)->t=BPX_option_DUMP; (yyval.bpx_option)->a=(yyvsp[-3].a); (yyval.bpx_option)->size_or_value=(yyvsp[-1].num); }
#line 1769 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 234 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bpx_option)=DCALLOC(BPX_option, 1, "BPX_option"); (yyval.bpx_option)->t=BPX_option_DUMP; (yyval.bpx_option)->a=(yyvsp[-1].a); (yyval.bpx_option)->size_or_value=BPX_DUMP_DEFAULT; }
#line 1775 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 236 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bpx_option)=DCALLOC(BPX_option, 1, "BPX_option"); (yyval.bpx_option)->t=BPX_option_DUMP; (yyval.bpx_option)->reg=(yyvsp[-3].x86reg); (yyval.bpx_option)->size_or_value=(yyvsp[-1].num); }
#line 1781 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 238 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bpx_option)=DCALLOC(BPX_option, 1, "BPX_option"); (yyval.bpx_option)->t=BPX_option_DUMP; (yyval.bpx_option)->reg=(yyvsp[-1].x86reg); (yyval.bpx_option)->size_or_value=BPX_DUMP_DEFAULT; }
#line 1787 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 240 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bpx_option)=DCALLOC(BPX_option, 1, "BPX_option"); (yyval.bpx_option)->t=BPX_option_SET; (yyval.bpx_option)->reg=(yyvsp[-3].x86reg); (yyval.bpx_option)->size_or_value=(yyvsp[-1].num); }
#line 1793 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 242 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bpx_option)=DCALLOC(BPX_option, 1, "BPX_option"); (yyval.bpx_option)->t=BPX_option_SET; (yyval.bpx_option)->reg=(yyvsp[-3].x86reg); (yyval.bpx_option)->float_value=(yyvsp[-1].dbl); }
#line 1799 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 244 "opts_parse.y" /* yacc.c:1646  */
    {
    (yyval.bpx_option)=DCALLOC(BPX_option, 1, "BPX_option"); 
    (yyval.bpx_option)->t=BPX_option_COPY; 
    (yyval.bpx_option)->a=(yyvsp[-5].a); 
    list_of_bytes_to_array (&((yyval.bpx_option)->copy_string), &((yyval.bpx_option)->copy_string_len), (yyvsp[-2].o)); 
    obj_free((yyvsp[-2].o));
 }
#line 1811 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 252 "opts_parse.y" /* yacc.c:1646  */
    {
    (yyval.bpx_option)=DCALLOC(BPX_option, 1, "BPX_option"); 
    (yyval.bpx_option)->t=BPX_option_COPY; 
    (yyval.bpx_option)->reg=(yyvsp[-5].x86reg); 
    list_of_bytes_to_array (&((yyval.bpx_option)->copy_string), &((yyval.bpx_option)->copy_string_len), (yyvsp[-2].o)); 
    obj_free((yyvsp[-2].o));
 }
#line 1823 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 262 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->unicode=true; }
#line 1829 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 263 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->microsoft_fastcall=true; }
#line 1835 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 264 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->borland_fastcall=true; }
#line 1841 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 265 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->trace=true; }
#line 1847 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 266 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->trace=true; current_BPF->cc=true; }
#line 1853 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 267 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->skip=true; }
#line 1859 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 268 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->skip_stdcall=true; }
#line 1865 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 269 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->pause=(yyvsp[0].num); }
#line 1871 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 270 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->rt=(yyvsp[0].num); 
    current_BPF->rt_present=true; }
#line 1878 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 272 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->rt_probability=(yyvsp[0].dbl); 
    current_BPF->rt_probability_present=true;
#ifdef _DEBUG 
    //fprintf (stderr, "rt_probability=%f", $2); 
#endif
    }
#line 1889 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 278 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->args=(yyvsp[0].num); }
#line 1895 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 279 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->dump_args=(yyvsp[0].num); }
#line 1901 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 280 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->when_called_from_address=(yyvsp[0].a); }
#line 1907 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 281 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->when_called_from_func=(yyvsp[0].a); }
#line 1913 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 282 "opts_parse.y" /* yacc.c:1646  */
    {
    // FIXME: there should be support of multiple SET options!
    current_BPF->set_present=true;
    current_BPF->set_width=(yyvsp[-10].num);
    current_BPF->set_arg_n=(yyvsp[-6].num);
    current_BPF->set_ofs=(yyvsp[-4].num);
    current_BPF->set_val=(yyvsp[-1].num);
 }
#line 1926 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 290 "opts_parse.y" /* yacc.c:1646  */
    { 
   oassert ((yyvsp[-4].num) >= 1);
   oassert ((yyvsp[-4].num) <= current_BPF->args);
   if (current_BPF->arg_types==NULL)
        current_BPF->arg_types=DCALLOC (function_type, current_BPF->args, "function_type");
   current_BPF->arg_types[((yyvsp[-4].num))-1]=(yyvsp[0].func_type);
 }
#line 1938 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 300 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.func_type)=TY_INT; }
#line 1944 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 301 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.func_type)=TY_PTR_TO_DOUBLE;  /* temporary, as I hope! */ }
#line 1950 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 302 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.func_type)=TY_QSTRING; }
#line 1956 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 303 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.func_type)=TY_PTR_TO_QSTRING; /* temporary, as I hope! */ }
#line 1962 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 307 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.num)=(yyvsp[0].num); }
#line 1968 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 312 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.dbl)=(double)(yyvsp[-1].num)/(double)100; }
#line 1974 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 316 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.o)=NCONC (cons(obj_byte((yyvsp[-1].num)), NULL), (yyvsp[0].o)); }
#line 1980 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 317 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.o)=cons (obj_byte((yyvsp[0].num)), NULL); }
#line 1986 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 322 "opts_parse.y" /* yacc.c:1646  */
    { 
        (yyval.a)=create_address_abs ((yyvsp[0].num)); 
     }
#line 1994 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 326 "opts_parse.y" /* yacc.c:1646  */
    { 
        (yyval.a)=create_address_filename_symbol_re ((yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].num)); 
         DFREE ((yyvsp[-2].str)); 
         DFREE ((yyvsp[-1].str)); 
         // every new address, except of abs-address (which is already resolved)
         // is added to addresses resolving queue
         add_new_address_to_be_resolved ((yyval.a)); 
     }
#line 2007 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 335 "opts_parse.y" /* yacc.c:1646  */
    { 
        (yyval.a)=create_address_filename_symbol_re ((yyvsp[-1].str), (yyvsp[0].str), 0); 
        DFREE ((yyvsp[-1].str)); 
        DFREE ((yyvsp[0].str)); 
        add_new_address_to_be_resolved ((yyval.a)); 
     }
#line 2018 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 342 "opts_parse.y" /* yacc.c:1646  */
    { 
        (yyval.a)=create_address_filename_address ((yyvsp[-1].str), (yyvsp[0].num)); 
        DFREE ((yyvsp[-1].str)); 
        add_new_address_to_be_resolved ((yyval.a)); 
     }
#line 2028 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 348 "opts_parse.y" /* yacc.c:1646  */
    { 
        (yyval.a)=create_address_bytemask ((yyvsp[-1].o)); 
        obj_free((yyvsp[-1].o));
        add_new_address_to_be_resolved ((yyval.a)); 
     }
#line 2038 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 365 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.o)=NCONC ((yyvsp[-1].o), (yyvsp[0].o)); }
#line 2044 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 370 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.o)=cons (obj_wyde((yyvsp[0].num)), NULL); }
#line 2050 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 371 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.o)=cons (obj_wyde(BYTEMASK_WILDCARD_BYTE), NULL); }
#line 2056 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 372 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.o)=obj_wyde_n_times (BYTEMASK_WILDCARD_BYTE, (yyvsp[0].num)); }
#line 2062 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 376 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.num)=(yyvsp[-1].num); }
#line 2068 "opts_parse.tab.c" /* yacc.c:1646  */
    break;


#line 2072 "opts_parse.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 379 "opts_parse.y" /* yacc.c:1906  */


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
