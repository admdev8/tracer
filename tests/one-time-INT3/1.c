#include <stdio.h>

int main()
{
	FILE *f=fopen ("output", "w+");
	fprintf (f, "hello, world\n");
	fprintf (f, "hello, world #2\n"); // shouldn't be triggered
	printf ("hello again\n"); // that shouldn't be triggered
	fclose(f);
};
