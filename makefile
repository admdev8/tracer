OCTOTHORPE=../octothorpe/
OCTOTHORPE_LIBRARY=$(OCTOTHORPE)octothorped.a
X86_DISASM=../x86_disasm/
X86_DISASM_LIBRARY=$(X86_DISASM)x86_disasmd.a
PORG=../porg/
PORG_LIBRARY=$(PORG)porgd.a
BOLT=../bolt/
BOLT_LIBRARY=$(BOLT)boltd.a
CPPFLAGS=-I$(OCTOTHORPE) -I$(X86_DISASM) -I$(PORG) -I$(BOLT) -D_DEBUG -DYYDEBUG=1
#GPROF_FLAG=-pg
GPROF_FLAG=
CFLAGS=-Wall -g $(GPROF_FLAG) -std=gnu99
FLEX=flex
BISON=bison
SOURCES=opts.tab.c opts.lex.c tracer.c cycle.c module.c process.c symbol.c thread.c BP.c \
	BPF.c BPX.c bp_address.c utils.c cc.c
OBJECTS=$(SOURCES:.c=.o)
DEP_FILES=$(SOURCES:.c=.d)
LIBS=$(OCTOTHORPE_LIBRARY) $(X86_DISASM_LIBRARY) $(PORG_LIBRARY) $(BOLT_LIBRARY)

all:    tracer.exe $(DEP_FILES) opts_test.exe

%.d: %.c
	$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< -c > $@

tracer.exe: $(OBJECTS) $(LIBS)
	$(CC) $(GPROF_FLAG) $^ $(LIBS) -o $@ -L/lib -lfl -lpsapi -limagehlp

opts_test.exe: opts_test.o opts.tab.o opts.lex.o BP.o BPF.o bp_address.o process.o thread.o module.o symbol.o utils.o cc.o $(LIBS)
	$(CC) $(GPROF_FLAG) $^ $(LIBS) -o $@ -L/lib -lfl -lpsapi -limagehlp
	
clean:
	$(RM) opts.tab.h opts.tab.c opts.lex.c tracer.exe opts_test.exe
	$(RM) $(DEP_FILES)
	$(RM) $(OBJECTS)

opts.tab.h opts.tab.c: opts.y
	$(BISON) -d opts.y -t
#$(BISON) -d opts.y

opts.lex.c: opts.l opts.tab.h opts.h
	$(FLEX) -oopts.lex.c opts.l
#$(FLEX) -d -oopts.lex.c opts.l
