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

struct BP* breakpoints[4]={ NULL, NULL, NULL, NULL }; // 0..3 - DR0-3
dlist* addresses_to_be_resolved=NULL;
char* load_filename=NULL;
char* attach_filename=NULL;
char *load_command_line=NULL;
int attach_PID=-1;
bool debug_children=false;
bool dash_s=false, quiet=false;
bool dump_fpu=false, dump_xmm=false, dump_seh=false;
struct BPX_option *current_BPX_option=NULL; // temporary, while parsing...
struct BPF* current_BPF=NULL; // filled while parsing
struct bp_address* current_BPF_address; // filled while parsing
bool run_thread_b=true;
bool dump_all_symbols=false;
regex_t *dump_all_symbols_re=NULL;
int verbose=0;
bool emulator_testing=false, opt_loading=false, create_new_console=true;
int limit_trace_nestedness=1; // default value

// from opts.l:
void flex_set_str(char *s);
void flex_cleanup();
void flex_restart();

void add_new_BP (struct BP* bp)
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

void add_new_address_to_be_resolved (struct bp_address *a)
{
    if (addresses_to_be_resolved==NULL)
        addresses_to_be_resolved=dlist_init();
    
    dlist_insert_at_begin (&addresses_to_be_resolved, a);
};


#line 155 "opts_parse.tab.c" /* yacc.c:339  */

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
#line 91 "opts_parse.y" /* yacc.c:355  */

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

#line 281 "opts_parse.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_OPTS_PARSE_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 298 "opts_parse.tab.c" /* yacc.c:358  */

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
#define YYFINAL  38
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   143

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  88
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  20
/* YYNRULES -- Number of rules.  */
#define YYNRULES  85
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  156

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   327

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      73,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    78,     2,     2,    85,     2,     2,
      79,    77,    80,    81,    74,     2,    86,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    84,     2,
       2,    82,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    87,     2,    83,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    76,     2,     2,     2,     2,    75,
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
      65,    66,    67,    68,    69,    70,    71,    72
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   133,   133,   134,   138,   139,   140,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   179,   188,
     190,   195,   197,   205,   206,   210,   217,   222,   223,   227,
     229,   231,   233,   235,   237,   239,   247,   258,   259,   260,
     261,   262,   263,   264,   265,   266,   268,   274,   275,   276,
     277,   278,   286,   296,   297,   298,   299,   303,   307,   308,
     312,   313,   317,   321,   330,   337,   343,   352,   353,   357,
     361,   362,   366,   367,   368,   372
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
  "NO_NEW_CONSOLE", "VERBOSE1", "VERBOSE2", "VERBOSE3", "EMULATOR_TESTING",
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
     325,   326,   327,    10,    44,   119,   114,    41,    34,    40,
      42,    43,    61,    95,    58,    37,    46,    93
};
# endif

#define YYPACT_NINF -112

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-112)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      71,    -3,    -3,  -112,  -112,  -112,  -112,  -112,  -112,  -112,
    -112,  -112,  -112,  -112,  -112,  -112,  -112,    -3,  -112,  -112,
    -112,  -112,  -112,  -112,  -112,  -112,    10,   -59,  -112,  -112,
    -112,    -2,  -112,   -45,   -48,  -112,   -32,   -30,  -112,  -112,
      -5,  -112,   -34,    51,    -2,  -112,  -112,  -112,   -29,    36,
      -1,    -6,   -27,  -112,  -112,  -112,  -112,  -112,  -112,     1,
       9,     2,  -112,     7,    -4,   -29,   -41,  -112,    58,   -29,
     -29,   -29,  -112,  -112,  -112,  -112,  -112,    -3,    -3,    21,
    -112,     8,  -112,    11,  -112,   -20,    -9,    13,    16,    17,
      25,    36,    15,  -112,    18,  -112,  -112,  -112,  -112,  -112,
    -112,  -112,  -112,    19,    -1,  -112,   -29,  -112,   -29,  -112,
     -29,    37,     5,    22,  -112,    30,  -112,    57,  -112,    29,
      31,    32,    39,    50,    50,    38,    33,  -112,  -112,  -112,
    -112,    50,    41,    42,    43,   -12,  -112,    44,    46,    86,
    -112,  -112,  -112,  -112,  -112,  -112,  -112,    72,    45,  -112,
     -29,    55,    52,   -29,    56,  -112
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,    10,    11,    12,    13,    17,    14,    15,
      16,    19,    24,    20,    21,    22,    23,     0,     9,    25,
      26,     7,     8,    18,    27,    28,     0,     2,     4,     5,
       6,     0,    79,     0,    31,    72,    33,     0,     1,     3,
       0,    82,     0,     0,    81,    84,    75,    74,     0,     0,
       0,     0,     0,    83,    76,    80,    77,    78,    73,     0,
       0,     0,    32,    36,     0,     0,     0,    50,     0,     0,
       0,     0,    52,    53,    47,    48,    49,     0,     0,     0,
      34,    38,    29,     0,    85,     0,     0,     0,     0,     0,
       0,     0,     0,    54,     0,    68,    56,    51,    57,    58,
      55,    59,    60,     0,     0,    30,     0,    42,     0,    40,
       0,     0,     0,     0,    35,     0,    69,     0,    37,     0,
       0,     0,     0,     0,     0,     0,     0,    41,    39,    43,
      44,    71,     0,     0,     0,     0,    70,     0,     0,     0,
      63,    64,    65,    66,    62,    46,    45,     0,     0,    67,
       0,     0,     0,     0,     0,    61
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
    -112,  -112,  -112,  -112,  -112,  -112,    47,    24,  -112,  -112,
    -112,  -112,  -112,  -111,     0,   -65,  -112,    91,  -112,  -112
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    26,    27,    28,    29,    30,    62,    80,    63,    81,
     144,   148,    96,   132,    34,    58,    35,    43,    44,    45
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      93,    40,    36,    31,    98,    99,   100,    31,    31,    46,
      38,    64,    94,   133,    39,    65,    66,    37,    67,    68,
     136,    47,    48,    95,    56,    57,    49,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,   140,   141,   142,
     143,   119,    50,   120,    51,   121,    79,    59,    52,    60,
      61,    32,    53,    41,   106,    32,    32,   107,    54,    86,
      84,    90,    33,    85,    89,   108,    33,    33,   109,    82,
      83,    87,    88,    97,   103,    92,   115,   101,   102,     1,
       2,    91,   104,   123,    42,   151,   105,   110,   154,     3,
     111,   112,     4,     5,     6,     7,     8,     9,    10,   113,
     124,   122,   117,   116,   125,   126,   127,   131,   128,   129,
      11,    12,    13,    14,    15,    16,   130,   135,   134,   137,
     138,   145,   139,   146,   147,   149,   150,    17,   118,    18,
      19,    20,   152,   155,   153,    55,     0,     0,   114,    21,
      22,    23,    24,    25
};

static const yytype_int16 yycheck[] =
{
      65,     3,     2,     6,    69,    70,    71,     6,     6,    54,
       0,    12,    53,   124,    73,    16,    17,    17,    19,    20,
     131,    66,    67,    64,    53,    54,    74,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    49,    50,    51,
      52,   106,    74,   108,    74,   110,    47,    11,    53,    13,
      14,    54,    86,    55,    74,    54,    54,    77,     7,    59,
      87,    61,    65,    62,    62,    74,    65,    65,    77,    75,
      76,    62,    63,    15,    53,    79,    61,    77,    78,     8,
       9,    74,    74,    78,    86,   150,    75,    74,   153,    18,
      74,    74,    21,    22,    23,    24,    25,    26,    27,    74,
      78,    64,    83,    85,    74,    48,    77,    57,    77,    77,
      39,    40,    41,    42,    43,    44,    77,    84,    80,    78,
      78,    77,    79,    77,    38,    53,    81,    56,   104,    58,
      59,    60,    77,    77,    82,    44,    -1,    -1,    91,    68,
      69,    70,    71,    72
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     8,     9,    18,    21,    22,    23,    24,    25,    26,
      27,    39,    40,    41,    42,    43,    44,    56,    58,    59,
      60,    68,    69,    70,    71,    72,    89,    90,    91,    92,
      93,     6,    54,    65,   102,   104,   102,   102,     0,    73,
       3,    55,    86,   105,   106,   107,    54,    66,    67,    74,
      74,    74,    53,    86,     7,   105,    53,    54,   103,    11,
      13,    14,    94,    96,    12,    16,    17,    19,    20,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    47,
      95,    97,    75,    76,    87,    62,   102,    62,    63,    62,
     102,    74,    79,   103,    53,    64,   100,    15,   103,   103,
     103,   102,   102,    53,    74,    75,    74,    77,    74,    77,
      74,    74,    74,    74,    94,    61,    85,    83,    95,   103,
     103,   103,    64,    78,    78,    74,    48,    77,    77,    77,
      77,    57,   101,   101,    80,    84,   101,    78,    78,    79,
      49,    50,    51,    52,    98,    77,    77,    38,    99,    53,
      81,   103,    77,    82,   103,    77
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    88,    89,    89,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    90,    90,    90,    90,    90,    90,    91,
      91,    92,    92,    93,    93,    94,    94,    95,    95,    96,
      96,    96,    96,    96,    96,    96,    96,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    98,    98,    98,    98,    99,   100,   100,
     101,   101,   102,   102,   102,   102,   102,   103,   103,   104,
     105,   105,   106,   106,   106,   107
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     4,
       5,     2,     4,     2,     4,     3,     1,     3,     1,     5,
       3,     5,     3,     5,     5,     7,     7,     1,     1,     1,
       1,     2,     1,     1,     2,     2,     2,     2,     2,     2,
       2,    13,     6,     1,     1,     1,     1,     2,     1,     2,
       2,     1,     1,     3,     2,     2,     3,     1,     1,     1,
       2,     1,     1,     2,     1,     3
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
#line 138 "opts_parse.y" /* yacc.c:1646  */
    { add_new_BP ((yyvsp[0].bp)); }
#line 1497 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 139 "opts_parse.y" /* yacc.c:1646  */
    { add_new_BP ((yyvsp[0].bp)); }
#line 1503 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 140 "opts_parse.y" /* yacc.c:1646  */
    { 
   if (current_BPF->rt_present==false && current_BPF->rt_probability_present==true)
       die ("rt_probability option without rt option is useless. exiting.\n");
   struct BP *bp=create_BP(BP_type_BPF, current_BPF_address, current_BPF);
   add_new_BP (bp); 
   current_BPF=NULL;
   current_BPF_address=NULL;
 }
#line 1516 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 148 "opts_parse.y" /* yacc.c:1646  */
    { load_filename=(yyvsp[0].str); }
#line 1522 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 149 "opts_parse.y" /* yacc.c:1646  */
    { attach_filename=(yyvsp[0].str); }
#line 1528 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 150 "opts_parse.y" /* yacc.c:1646  */
    { attach_PID=(yyvsp[0].num); }
#line 1534 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 151 "opts_parse.y" /* yacc.c:1646  */
    { debug_children=true; }
#line 1540 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 152 "opts_parse.y" /* yacc.c:1646  */
    { dash_s=true; }
#line 1546 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 153 "opts_parse.y" /* yacc.c:1646  */
    { quiet=true; }
#line 1552 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 154 "opts_parse.y" /* yacc.c:1646  */
    { L_timestamp=true; }
#line 1558 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 155 "opts_parse.y" /* yacc.c:1646  */
    { dump_fpu=true; }
#line 1564 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 156 "opts_parse.y" /* yacc.c:1646  */
    { dump_xmm=true; }
#line 1570 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 157 "opts_parse.y" /* yacc.c:1646  */
    { dump_seh=true; }
#line 1576 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 158 "opts_parse.y" /* yacc.c:1646  */
    { run_thread_b=false; }
#line 1582 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 159 "opts_parse.y" /* yacc.c:1646  */
    { load_command_line=(yyvsp[0].str); }
#line 1588 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 160 "opts_parse.y" /* yacc.c:1646  */
    { opt_loading=true; }
#line 1594 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 161 "opts_parse.y" /* yacc.c:1646  */
    { verbose=1; }
#line 1600 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 162 "opts_parse.y" /* yacc.c:1646  */
    { verbose=2; }
#line 1606 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 163 "opts_parse.y" /* yacc.c:1646  */
    { verbose=3; }
#line 1612 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 164 "opts_parse.y" /* yacc.c:1646  */
    { emulator_testing=true; }
#line 1618 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 165 "opts_parse.y" /* yacc.c:1646  */
    { create_new_console=false; }
#line 1624 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 166 "opts_parse.y" /* yacc.c:1646  */
    { dmalloc_break_at_seq_n ((yyvsp[0].num)); }
#line 1630 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 167 "opts_parse.y" /* yacc.c:1646  */
    { limit_trace_nestedness=(yyvsp[0].num); }
#line 1636 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 168 "opts_parse.y" /* yacc.c:1646  */
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
#line 1652 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 179 "opts_parse.y" /* yacc.c:1646  */
    { 
    oassert(one_time_int3_bp_re==NULL);
    one_time_int3_bp_re=DCALLOC(regex_t, 1, "regex_t");
    regcomp_or_die(one_time_int3_bp_re, (yyvsp[0].str), REG_EXTENDED | REG_ICASE | REG_NEWLINE);
    DFREE((yyvsp[0].str));
    }
#line 1663 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 189 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bp)=create_BP(BP_type_BPM, (yyvsp[-2].a), create_BPM ((yyvsp[-3].num), BPM_type_W)); }
#line 1669 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 191 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bp)=create_BP(BP_type_BPM, (yyvsp[-3].a), create_BPM ((yyvsp[-4].num), BPM_type_RW)); }
#line 1675 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 196 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bp)=create_BP(BP_type_BPX, (yyvsp[0].a), create_BPX (NULL)); }
#line 1681 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 198 "opts_parse.y" /* yacc.c:1646  */
    { 
       (yyval.bp)=create_BP(BP_type_BPX, (yyvsp[-2].a), create_BPX (current_BPX_option)); 
       current_BPX_option=NULL;
   }
#line 1690 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 205 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF_address=(yyvsp[0].a); }
#line 1696 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 206 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF_address=(yyvsp[-2].a); }
#line 1702 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 211 "opts_parse.y" /* yacc.c:1646  */
    { 
     struct BPX_option *o;
     oassert(current_BPX_option);
     for (o=current_BPX_option; o->next; o=o->next);
     o->next=(yyvsp[-2].bpx_option);
 }
#line 1713 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 218 "opts_parse.y" /* yacc.c:1646  */
    { current_BPX_option=(yyvsp[0].bpx_option); }
#line 1719 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 228 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bpx_option)=DCALLOC(struct BPX_option, 1, "BPX_option"); (yyval.bpx_option)->t=BPX_option_DUMP; (yyval.bpx_option)->a=(yyvsp[-3].a); (yyval.bpx_option)->size_or_value=(yyvsp[-1].num); }
#line 1725 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 230 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bpx_option)=DCALLOC(struct BPX_option, 1, "BPX_option"); (yyval.bpx_option)->t=BPX_option_DUMP; (yyval.bpx_option)->a=(yyvsp[-1].a); (yyval.bpx_option)->size_or_value=BPX_DUMP_DEFAULT; }
#line 1731 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 232 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bpx_option)=DCALLOC(struct BPX_option, 1, "BPX_option"); (yyval.bpx_option)->t=BPX_option_DUMP; (yyval.bpx_option)->reg=(yyvsp[-3].x86reg); (yyval.bpx_option)->size_or_value=(yyvsp[-1].num); }
#line 1737 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 234 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bpx_option)=DCALLOC(struct BPX_option, 1, "BPX_option"); (yyval.bpx_option)->t=BPX_option_DUMP; (yyval.bpx_option)->reg=(yyvsp[-1].x86reg); (yyval.bpx_option)->size_or_value=BPX_DUMP_DEFAULT; }
#line 1743 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 236 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bpx_option)=DCALLOC(struct BPX_option, 1, "BPX_option"); (yyval.bpx_option)->t=BPX_option_SET; (yyval.bpx_option)->reg=(yyvsp[-3].x86reg); (yyval.bpx_option)->size_or_value=(yyvsp[-1].num); }
#line 1749 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 238 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.bpx_option)=DCALLOC(struct BPX_option, 1, "BPX_option"); (yyval.bpx_option)->t=BPX_option_SET; (yyval.bpx_option)->reg=(yyvsp[-3].x86reg); (yyval.bpx_option)->float_value=(yyvsp[-1].dbl); }
#line 1755 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 240 "opts_parse.y" /* yacc.c:1646  */
    {
    (yyval.bpx_option)=DCALLOC(struct BPX_option, 1, "BPX_option"); 
    (yyval.bpx_option)->t=BPX_option_COPY; 
    (yyval.bpx_option)->a=(yyvsp[-5].a); 
    list_of_bytes_to_array (&((yyval.bpx_option)->copy_string), &((yyval.bpx_option)->copy_string_len), (yyvsp[-2].o)); 
    obj_free((yyvsp[-2].o));
 }
#line 1767 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 248 "opts_parse.y" /* yacc.c:1646  */
    {
    (yyval.bpx_option)=DCALLOC(struct BPX_option, 1, "BPX_option"); 
    (yyval.bpx_option)->t=BPX_option_COPY; 
    (yyval.bpx_option)->reg=(yyvsp[-5].x86reg); 
    list_of_bytes_to_array (&((yyval.bpx_option)->copy_string), &((yyval.bpx_option)->copy_string_len), (yyvsp[-2].o)); 
    obj_free((yyvsp[-2].o));
 }
#line 1779 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 258 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->unicode=true; }
#line 1785 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 259 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->microsoft_fastcall=true; }
#line 1791 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 260 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->borland_fastcall=true; }
#line 1797 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 261 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->trace=true; }
#line 1803 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 262 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->trace=true; current_BPF->cc=true; }
#line 1809 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 263 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->skip=true; }
#line 1815 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 264 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->skip_stdcall=true; }
#line 1821 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 265 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->pause=(yyvsp[0].num); }
#line 1827 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 266 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->rt=(yyvsp[0].num); 
    current_BPF->rt_present=true; }
#line 1834 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 268 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->rt_probability=(yyvsp[0].dbl); 
    current_BPF->rt_probability_present=true;
#ifdef _DEBUG 
    //fprintf (stderr, "rt_probability=%f", $2); 
#endif
    }
#line 1845 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 274 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->args=(yyvsp[0].num); }
#line 1851 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 275 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->dump_args=(yyvsp[0].num); }
#line 1857 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 276 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->when_called_from_address=(yyvsp[0].a); }
#line 1863 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 277 "opts_parse.y" /* yacc.c:1646  */
    { current_BPF->when_called_from_func=(yyvsp[0].a); }
#line 1869 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 278 "opts_parse.y" /* yacc.c:1646  */
    {
    // FIXME: there should be support of multiple SET options!
    current_BPF->set_present=true;
    current_BPF->set_width=(yyvsp[-10].num);
    current_BPF->set_arg_n=(yyvsp[-6].num);
    current_BPF->set_ofs=(yyvsp[-4].num);
    current_BPF->set_val=(yyvsp[-1].num);
 }
#line 1882 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 286 "opts_parse.y" /* yacc.c:1646  */
    { 
   oassert ((yyvsp[-4].num) >= 1);
   oassert ((yyvsp[-4].num) <= current_BPF->args);
   if (current_BPF->arg_types==NULL)
        current_BPF->arg_types=DCALLOC (enum function_type, current_BPF->args, "function_type");
   current_BPF->arg_types[((yyvsp[-4].num))-1]=(yyvsp[0].func_type);
 }
#line 1894 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 296 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.func_type)=TY_INT; }
#line 1900 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 297 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.func_type)=TY_PTR_TO_DOUBLE;  /* temporary, as I hope! */ }
#line 1906 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 298 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.func_type)=TY_QSTRING; }
#line 1912 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 299 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.func_type)=TY_PTR_TO_QSTRING; /* temporary, as I hope! */ }
#line 1918 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 303 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.num)=(yyvsp[0].num); }
#line 1924 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 308 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.dbl)=(double)(yyvsp[-1].num)/(double)100; }
#line 1930 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 312 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.o)=NCONC (cons(obj_byte((yyvsp[-1].num)), NULL), (yyvsp[0].o)); }
#line 1936 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 313 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.o)=cons (obj_byte((yyvsp[0].num)), NULL); }
#line 1942 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 318 "opts_parse.y" /* yacc.c:1646  */
    { 
        (yyval.a)=create_address_abs ((yyvsp[0].num)); 
     }
#line 1950 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 322 "opts_parse.y" /* yacc.c:1646  */
    { 
        (yyval.a)=create_address_filename_symbol_re ((yyvsp[-2].str), (yyvsp[-1].str), (yyvsp[0].num)); 
         DFREE ((yyvsp[-2].str)); 
         DFREE ((yyvsp[-1].str)); 
         // every new address, except of abs-address (which is already resolved)
         // is added to addresses resolving queue
         add_new_address_to_be_resolved ((yyval.a)); 
     }
#line 1963 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 331 "opts_parse.y" /* yacc.c:1646  */
    { 
        (yyval.a)=create_address_filename_symbol_re ((yyvsp[-1].str), (yyvsp[0].str), 0); 
        DFREE ((yyvsp[-1].str)); 
        DFREE ((yyvsp[0].str)); 
        add_new_address_to_be_resolved ((yyval.a)); 
     }
#line 1974 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 338 "opts_parse.y" /* yacc.c:1646  */
    { 
        (yyval.a)=create_address_filename_address ((yyvsp[-1].str), (yyvsp[0].num)); 
        DFREE ((yyvsp[-1].str)); 
        add_new_address_to_be_resolved ((yyval.a)); 
     }
#line 1984 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 344 "opts_parse.y" /* yacc.c:1646  */
    { 
        (yyval.a)=create_address_bytemask ((yyvsp[-1].o)); 
        obj_free((yyvsp[-1].o));
        add_new_address_to_be_resolved ((yyval.a)); 
     }
#line 1994 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 361 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.o)=NCONC ((yyvsp[-1].o), (yyvsp[0].o)); }
#line 2000 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 366 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.o)=cons (obj_wyde((yyvsp[0].num)), NULL); }
#line 2006 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 367 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.o)=cons (obj_wyde(BYTEMASK_WILDCARD_BYTE), NULL); }
#line 2012 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 368 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.o)=obj_wyde_n_times (BYTEMASK_WILDCARD_BYTE, (yyvsp[0].num)); }
#line 2018 "opts_parse.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 372 "opts_parse.y" /* yacc.c:1646  */
    { (yyval.num)=(yyvsp[-1].num); }
#line 2024 "opts_parse.tab.c" /* yacc.c:1646  */
    break;


#line 2028 "opts_parse.tab.c" /* yacc.c:1646  */
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
#line 375 "opts_parse.y" /* yacc.c:1906  */


struct BP* parse_option(char *s)
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
