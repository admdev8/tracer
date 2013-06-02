#include <stdio.h>
#include <windows.h>

void change_some_byte (char *buf, int i)
{
    buf[i]='h';
    buf[i+1]='a';
    buf[i+2]='h';
    buf[i+3]='a';
};

int main()
{
    printf ("hello: %d\n", 123);
    printf ("0x%p\n", GetProcAddress(GetModuleHandle("msvcrt.dll"), "printf"));

    char buf[80];

    for (int i=0; i<10; i++)
    {
        sprintf (buf, "i=%d\n", i);
        puts (buf);
    };

    char *buf2=strdup("The quick brown fox jumps over the lazy dog");
//    char *buf2="The quick brown fox jumps over the lazy dog";
    change_some_byte(buf2, 3);
};
