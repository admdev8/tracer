#include <stdio.h>

int global_var_1;
int global_var_2;
int global_var_3;

void f(void** a)
{
	a[3]=&global_var_3;
};

int main()
{
	void* a[5];
	int i;

	a[0]=a[1]=a[2]=a[3]=a[4]=NULL;
	a[1]=&global_var_1;
	a[3]=&global_var_2;
	f(a);
	for (i=0; i<5; i++)
		printf ("0x%p\n", a[i]);
};
