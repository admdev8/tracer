OCTOTHORPE=../octothorpe/
OCTOTHORPE_LIBRARY=$(OCTOTHORPE)octothorped.a
X86_DISASM=../x86_disasm/
X86_DISASM_LIBRARY=$(X86_DISASM)x86_disasmd.a
PORG=../porg/
PORG_LIBRARY=$(PORG)porgd.a
BOLT=../bolt/
BOLT_LIBRARY=$(BOLT)boltd.a
CPPFLAGS=-I$(OCTOTHORPE) -I$(X86_DISASM) -I$(PORG) -I$(BOLT) -D_DEBUG -DYYDEBUG=1
#CFLAGS=-Wall -g -std=c99
CFLAGS=-Wall -g -std=gnu99
FLEX=flex
BISON=bison
SOURCES=opts.tab.c opts.lex.c opts_func.c tracer.c cycle.c module.c process.c symbol.c thread.c BP.c BPF.c BPX.c
OBJECTS=$(SOURCES:.c=.o)
DEP_FILES=$(SOURCES:.c=.d)
LIBS=$(OCTOTHORPE_LIBRARY) $(X86_DISASM_LIBRARY) $(PORG_LIBRARY) $(BOLT_LIBRARY)

all:    tracer.exe $(DEP_FILES) opts_test.exe

%.d: %.c
	$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< -c > $@

tracer.exe: $(OBJECTS) $(LIBS)
	$(CC) $^ $(LIBS) -o $@ -L/lib -lfl -lpsapi -limagehlp

opts_test.exe: opts_test.o opts_func.o opts.tab.o opts.lex.o BP.o $(LIBS)
	$(CC) $^ $(LIBS) -o $@ -L/lib -lfl 
	
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
