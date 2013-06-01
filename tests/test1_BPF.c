#include <stdio.h>
#include <windows.h>

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
};
