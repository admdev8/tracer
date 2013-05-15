OCTOTHORPE=../octothorpe/
OCTOTHORPE_LIBRARY=$(OCTOTHORPE)octothorped.a
X86_DISASM=../x86_disasm/
X86_DISASM_LIBRARY=$(X86_DISASM)x86_disasmd.a
PORG=../porg/
PORG_LIBRARY=$(PORG)porgd.a
BOLT=../bolt/
BOLT_LIBRARY=$(BOLT)boltd.a
# -DYYDEBUG=1
CPPFLAGS=-I$(OCTOTHORPE) -I$(X86_DISASM) -I$(PORG) -I$(BOLT) -D_DEBUG
#CFLAGS=-Wall -g -std=c99
CFLAGS=-Wall -g -std=gnu99
LEX=flex
SOURCES=y.tab.c opts.lex.c opts_func.c tracer.c cycle.c module.c process.c symbol.c thread.c
OBJECTS=$(SOURCES:.c=.o)
DEP_FILES=$(SOURCES:.c=.d)
LIBS=$(OCTOTHORPE_LIBRARY) $(X86_DISASM_LIBRARY) $(PORG_LIBRARY) $(BOLT_LIBRARY)

all:    tracer.exe $(DEP_FILES) opts_test.exe

%.d: %.c
	$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< -c > $@

tracer.exe: $(OBJECTS) $(LIBS)
	$(CC) $^ $(LIBS) -o $@ -L/lib -lfl -lpsapi -limagehlp

opts_test.exe: opts_test.o opts_func.o y.tab.o opts.lex.o $(LIBS)
	$(CC) $^ $(LIBS) -o $@ -L/lib -lfl 
	
clean:
	$(RM) opts.tab.h opts.tab.c opts.lex.c tracer.exe opts_test.exe
	$(RM) $(DEP_FILES)
	$(RM) $(OBJECTS)

y.tab.h y.tab.c: opts.y
	#$(YACC) -d opts.y -t
	$(YACC) -d opts.y

opts.lex.c: opts.l y.tab.h opts.h
	$(LEX) -oopts.lex.c opts.l
	#$(LEX) -d -oopts.lex.c opts.l
