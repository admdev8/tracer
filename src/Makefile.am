tracerdir = $(exec_prefix)
BUILT_SOURCES = opts_parse.h
AM_YFLAGS = -d

tracer_PROGRAMS = tracer

tracer_SOURCES = opts_scan.l opts_parse.y opts_test.c BP.c BPF.c BPM.c BPX.c SEH.c bp_address.c \
	cc.c cc_tests.c cycle.c module.c one_time_INT3_BP.c process.c symbol.c thread.c tracer.c \
	utils.c

tracer_LDADD = @BOLT_LIBS@ @PORG_LIBS@ @X86_DISASM_LIBS@ @OCTOTHORPE_LIBS@ -ldbghelp -limagehlp -lwinhttp
