CC=gcc
OUTDIR=$(MSYSTEM)_debug
OCTOTHORPE=../octothorpe
OCTOTHORPE_LIBRARY=$(OCTOTHORPE)/$(MSYSTEM)_debug/octothorped.a
X86_DISASM=../x86_disasm
X86_DISASM_LIBRARY=$(X86_DISASM)/$(MSYSTEM)_debug/x86_disasmd.a
PORG=../porg
PORG_LIBRARY=$(PORG)/$(MSYSTEM)_debug/porgd.a
BOLT=../bolt
BOLT_LIBRARY=$(BOLT)/$(MSYSTEM)_debug/boltd.a
CPPFLAGS=-I$(OCTOTHORPE) -I$(X86_DISASM) -I$(PORG) -I$(BOLT) -D_DEBUG -DYYDEBUG=1
#GPROF_FLAG=-pg
GPROF_FLAG=
CFLAGS=-Wall -g $(GPROF_FLAG) -std=gnu99
FLEX=flex
FLEX_PATH=$(HOME)/flex-2.5.37
BISON=bison
SOURCES=opts.tab.c opts.lex.c tracer.c cycle.c module.c process.c symbol.c thread.c BP.c \
	BPF.c BPX.c BPM.c bp_address.c utils.c cc.c opts_test.c
DEP_FILES=$(addprefix $(OUTDIR)/,$(SOURCES:.c=.d))
OBJECTS=$(addprefix $(OUTDIR)/,$(SOURCES:.c=.o))
LIBS=$(OCTOTHORPE_LIBRARY) $(X86_DISASM_LIBRARY) $(PORG_LIBRARY) $(BOLT_LIBRARY)

ifeq ($(MSYSTEM),MINGW64)
    TRACER_EXE_NAME = tracer64.exe
else
    TRACER_EXE_NAME = tracer.exe
endif

#all:    $(OUTDIR) $(OUTDIR)/tracer.exe $(DEP_FILES) $(OUTDIR)/opts_test.exe
all:    $(OUTDIR) $(OUTDIR)/$(TRACER_EXE_NAME) $(DEP_FILES)

$(OUTDIR):
	mkdir $(OUTDIR)

$(OUTDIR)/%.d: %.c
	$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< -c > $@

$(OUTDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(OUTDIR)/$(TRACER_EXE_NAME): $(OBJECTS) $(LIBS)
	$(CC) $(GPROF_FLAG) $^ $(LIBS) -o $@ -L$(FLEX_PATH) -lfl -lpsapi -ldbghelp -limagehlp

$(OUTDIR)/opts_test.exe: $(OUTDIR)/opts_test.o $(OUTDIR)/opts.tab.o $(OUTDIR)/opts.lex.o \
	$(OUTDIR)/BP.o $(OUTDIR)/BPF.o $(OUTDIR)/BPX.o $(OUTDIR)/bp_address.o $(OUTDIR)/process.o \
	$(OUTDIR)/thread.o $(OUTDIR)/module.o $(OUTDIR)/symbol.o $(OUTDIR)/utils.o \
	$(OUTDIR)/cc.o $(OUTDIR)/cycle.o $(LIBS)
	$(CC) $(GPROF_FLAG) $^ $(LIBS) -o $@ -L$(FLEX_PATH) -lfl -lpsapi -limagehlp
	
clean:
	$(RM) opts.tab.h opts.tab.c opts.lex.c $(OUTDIR)/$(TRACER_EXE_NAME) $(OUTDIR)/opts_test.exe
	$(RM) $(DEP_FILES)
	$(RM) $(OBJECTS)

opts.tab.h opts.tab.c: opts.y
	$(BISON) -d opts.y -t
#$(BISON) -d opts.y

opts.lex.c: opts.l opts.tab.h opts.h
	$(FLEX) -oopts.lex.c opts.l
#$(FLEX) -d -oopts.lex.c opts.l
