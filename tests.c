#include "cc_tests.h"
#include "X86_emu_tests.h"

void opts_test(); // opts_test.c

void tests()
{
	opts_test();
	cc_tests();
	Da_emulate_tests();
};
