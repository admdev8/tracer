#include <stdio.h>
#include <stdint.h>
#include "../../../octothorpe/fsave.h"

// asm_part.asm
void f (struct FSAVE_info *f, double *d);

void f1(FILE *out)
{
	struct FSAVE_info t;
	double val;

	f(&t, &val);

	fprintf (out, "val=%f\n", val);
	//dump_FSAVE_info (&t);
};

void f2(uint8_t *in)
{
	in[2]=0x22;
	in[4]=0x44;
	in[7]=0x77;
};

void f3(FILE *f, char* in)
{
	fprintf (f, "%s(): %s\n", __FUNCTION__, in);
};

uint8_t to_be_dumped[16]="\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";

void main()
{
	int d=222;

	FILE* f=fopen ("output", "w");
	fprintf (f, "d=%d\n", d);
	f1(f);
	f2(to_be_dumped);
	f3(f, "hello, world");
	fclose (f);
};

