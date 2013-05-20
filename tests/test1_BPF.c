#include <stdio.h>
#include <windows.h>

int main()
{
    printf ("hello: %d\n", 123);
    printf ("0x%p\n", GetProcAddress(GetModuleHandle("msvcrt.dll"), "printf"));
};
