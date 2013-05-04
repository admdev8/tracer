OCTOTHORPE=../octothorpe/
OCTOTHORPE_LIBRARY=$(OCTOTHORPE)octothorped.a
X86_DISASM=../x86_disasm/
X86_DISASM_LIBRARY=$(X86_DISASM)x86_disasmd.a
PORG=../porg/
PORG_LIBRARY=$(PORG)porgd.a
# -DYYDEBUG=1
CPPFLAGS=-I$(OCTOTHORPE) -I$(X86_DISASM) -I$(PORG) -D_DEBUG
CFLAGS=-Wall -g
LEX=flex
SOURCES=y.tab.c opts.lex.c opts_func.c tracer.c cycle.c
OBJECTS=$(SOURCES:.c=.o)
DEP_FILES=$(SOURCES:.c=.d)

all:    tracer.exe $(DEP_FILES)

%.d: %.c
	$(CC) -MM $(CFLAGS) $(CPPFLAGS) $*.c -c > $*.d

tracer.exe: $(OBJECTS)
	$(CC) $(OBJECTS) $(OCTOTHORPE_LIBRARY) $(X86_DISASM_LIBRARY) $(PORG_LIBRARY) -o tracer.exe -L/lib -lfl

clean:
	$(RM) opts.tab.h opts.tab.c opts.lex.c tracer.exe
	$(RM) $(DEP_FILES)
	$(RM) $(OBJECTS)

y.tab.h y.tab.c: opts.y
	#$(YACC) -d opts.y -t
	$(YACC) -d opts.y

opts.lex.c: opts.l y.tab.h opts.h
	$(LEX) -oopts.lex.c opts.l
	#$(LEX) -d -oopts.lex.c opts.l

