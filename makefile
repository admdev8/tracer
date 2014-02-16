CC=gcc

DEBUG_FLAGS=-D_DEBUG -DYYDEBUG=1
GCOV_FLAGS=-fprofile-arcs -ftest-coverage
ifeq ($(BUILD),debug)
bsuffix=debug
CPPFLAGS_ADD=$(DEBUG_FLAGS)
else 
	ifeq ($(BUILD),gcov)
	bsuffix=debug
	CPPFLAGS_ADD=$(DEBUG_FLAGS) $(GCOV_FLAGS)
	else
	bsuffix=release
	CPPFLAGS_ADD=-O3
	#CPPFLAGS_ADD=
	endif
endif

OUTDIR=$(MSYSTEM)_$(bsuffix)

OCTOTHORPE=../octothorpe
OCTOTHORPE_LIBRARY=$(OCTOTHORPE)/$(MSYSTEM)_$(bsuffix)/octothorpe.a
X86_DISASM=../x86_disasm
X86_DISASM_LIBRARY=$(X86_DISASM)/$(MSYSTEM)_$(bsuffix)/x86_disasm.a
PORG=../porg
PORG_LIBRARY=$(PORG)/$(MSYSTEM)_$(bsuffix)/porg.a
BOLT=../bolt
BOLT_LIBRARY=$(BOLT)/$(MSYSTEM)_$(bsuffix)/bolt.a

CPPFLAGS=-I$(OCTOTHORPE) -I$(X86_DISASM) -I$(PORG) -I$(BOLT) $(CPPFLAGS_ADD)
#GPROF_FLAG=-pg
GPROF_FLAG=
#CFLAGS=-Wall -g $(GPROF_FLAG) -std=gnu99
CFLAGS=-Wall -g $(GPROF_FLAG) -std=c11
FLEX=flex
FLEX_PATH=$(HOME)/flex-2.5.37
BISON=bison
SOURCES=opts.tab.c opts.lex.c tracer.c cycle.c module.c process.c symbol.c thread.c BP.c \
	BPF.c BPX.c BPM.c bp_address.c utils.c cc.c opts_test.c one_time_INT3_BP.c tests.c \
	cc_tests.c SEH.c
DEP_FILES=$(addprefix $(OUTDIR)/,$(SOURCES:.c=.d))
OBJECTS=$(addprefix $(OUTDIR)/,$(SOURCES:.c=.o))
LIBS=$(OCTOTHORPE_LIBRARY) $(X86_DISASM_LIBRARY) $(PORG_LIBRARY) $(BOLT_LIBRARY)

ifeq ($(MSYSTEM),MINGW64)
    TRACER_EXE_NAME = tracer64.exe
else
    TRACER_EXE_NAME = tracer.exe
endif

all:    $(OUTDIR) $(OUTDIR)/$(TRACER_EXE_NAME) $(DEP_FILES)
ifeq ($(bsuffix),debug)
	echo running $(TRACER_EXE_NAME) test
	$(OUTDIR)/$(TRACER_EXE_NAME) test
else
	strip $(OUTDIR)/$(TRACER_EXE_NAME)
endif
	tests/tests.sh $(CURDIR)/$(OUTDIR)/$(TRACER_EXE_NAME)

$(OUTDIR):
	mkdir $(OUTDIR)

$(OUTDIR)/%.d: %.c
	$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< -c > $@

$(OUTDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OUTDIR)/$(TRACER_EXE_NAME): $(OBJECTS) $(LIBS)
	$(CC) $(GCOV_FLAGS) $(GPROF_FLAG) $^ $(LIBS) -o $@ -L$(FLEX_PATH) -lfl -lpsapi -ldbghelp -limagehlp -lwinhttp
ifeq ($(BUILD),release)
	strip $(OUTDIR)/$(TRACER_EXE_NAME)
endif

clean:
	$(RM) opts.tab.h opts.tab.c opts.lex.c $(OUTDIR)/$(TRACER_EXE_NAME)
	$(RM) $(DEP_FILES)
	$(RM) $(OBJECTS)
	$(RM) *.gcov *.gcda *.gcno
	$(RM) $(OUTDIR)/*.gcov
	$(RM) $(OUTDIR)/*.gcda
	$(RM) $(OUTDIR)/*.gcno

opts.tab.h opts.tab.c: opts.y
ifeq ($(BUILD),debug)
#	$(BISON) -d opts.y -t
	$(BISON) -d opts.y
else	
	$(BISON) -d opts.y
endif	

opts.lex.c: opts.l opts.tab.h opts.h
ifeq ($(BUILD),debug)
#	$(FLEX) -d -oopts.lex.c opts.l
	$(FLEX) -oopts.lex.c opts.l
else	
	$(FLEX) -oopts.lex.c opts.l
endif
