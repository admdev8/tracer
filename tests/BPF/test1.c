#include <stdio.h>
#include <wchar.h>

int f1(char *s1, char *s2)
{
	return strcmp (s1, s2);
};

int f2(wchar_t *s1, wchar_t *s2)
{
	return wcscmp (s1, s2);
};

int global=666;

void to_be_skipped1(int a, int b)
{
	global=a*b;
};

void __stdcall to_be_skipped2(int a, int b)
{
	global=a+b;
};

int main()
{
	FILE *f=fopen ("test1.output", "w+");
	fprintf (f, "%s:%d:%s - %d\n", __FILE__, __LINE__, __FUNCTION__, f1("asd","def"));
	fprintf (f, "%s:%d:%s - %d\n", __FILE__, __LINE__, __FUNCTION__, f2(L"widestring1",L"widestring2"));
	to_be_skipped1(123,456);
	to_be_skipped2(1234,4567);
	fprintf (f, "(at the end) global=%d\n", global);
	fclose(f);
};
