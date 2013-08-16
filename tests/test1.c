/*
 *  _______                      
 * |__   __|                     
 *    | |_ __ __ _  ___ ___ _ __ 
 *    | | '__/ _` |/ __/ _ \ '__|
 *    | | | | (_| | (_|  __/ |   
 *    |_|_|  \__,_|\___\___|_|   
 *
 * Written by Dennis Yurichev <dennis(a)yurichev.com>, 2013
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/3.0/.
 *
 */

#include <stdio.h>
#include <windows.h>

void change_some_byte (char *buf, int i)
{
    buf[i]='h';
    buf[i+1]='a';
    buf[i+2]='h';
    buf[i+3]='a';
};

double fac(unsigned a)
{
    if (a==0)
        return 0.0;

    if (a==1)
        return 1.0;
    
    return fac(a-1) * (double)a;
};

void FPU_test()
{
    printf ("fac(5)=%f\n", fac(5));
};

int main()
{
    printf ("hello: %d\n", 123);
    printf ("Address of msvcrt.dll!printf: 0x%p\n", GetProcAddress(GetModuleHandle("msvcrt.dll"), "printf"));

    char buf[80];

    for (int i=0; i<10; i++)
    {
        sprintf (buf, "i=%d", i);
        puts (buf);
    };

    char *buf2=strdup("The quick brown fox jumps over the lazy dog");
//    char *buf2="The quick brown fox jumps over the lazy dog";
    change_some_byte(buf2, 3);

    FPU_test();
};

/* vim: set expandtab ts=4 sw=4 : */
