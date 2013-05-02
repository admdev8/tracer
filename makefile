OCTOTHORPE=../octothorpe/
OCTOTHORPE_LIBRARY=$(OCTOTHORPE)octothorped.a
X86_DISASM=../x86_disasm/
X86_DISASM_LIBRARY=$(X86_DISASM)x86_disasmd.a

all:    opts.l opts.y opts.tab.c opts.lex.c
	$(CC) -g opts.tab.c opts.lex.c opts_func.c -I$(OCTOTHORPE) -I$(X86_DISASM) -D_DEBUG -o opts.exe $(OCTOTHORPE_LIBRARY) $(X86_DISASM_LIBRARY) -L/lib -lfl
#	-DYYDEBUG=1

clean:
	rm opts.tab.h
	rm opts.tab.c
	rm opts.lex.c
	rm opts.exe

opts.tab.h opts.tab.c: opts.y
	bison -d opts.y -t

opts.lex.c: opts.l opts.tab.h opts.h
	flex -oopts.lex.c opts.l
	#flex -d -oopts.lex.c opts.l

