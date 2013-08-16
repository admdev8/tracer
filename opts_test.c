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

#include "opts.h"
#include "dmalloc.h"

#ifdef _DEBUG

// opts.tab.c
int yydebug;

static void do_test(char *s)
{
    BP *b;
    printf ("do_test(%s)\n", s);
    
    b=parse_option(s);
    if (b)
    {
        dump_BP(b);
        printf ("\n");     
        //BP_free(b);
        BP_free (breakpoints[0]);
        breakpoints[0]=NULL;
    }
    else
    {
        exit(0);
    };
};

int opts_test()
{
    //yydebug=1;
    parse_option ("-l:haha.exe");
    if (load_filename)
        printf ("load_filename=%s\n", load_filename);
    parse_option ("-a:haha.exe");
    if (attach_filename)
        printf ("attach_filename=%s\n", attach_filename);
    parse_option ("-a:1234");
    printf ("attach_PID=%d\n", attach_PID);
    parse_option ("-a:0x29a");
    printf ("attach_PID=%d\n", attach_PID);
    do_test("bpmq=file.dll!symbol,rw\0");
    do_test("bpmq=file.dll!symbol+0x123,w\0");
    do_test("bpmb=0x123123,w\0");
    do_test("bpmq=file.dll!symbol+123,rw\0");
    do_test("bpmd=0x12345678,w\0");
    do_test("bpmw=bytemask:\"001122\",w\0");
    do_test("bpmd=bytemask:\"0011..22\",rw\0");
    do_test("bpmq=bytemask:\"0011[skip:2]22\",w\0");
    do_test("bpmb=bytemask:\"001122..3355[skip:2]1166..77\",rw\0");
    do_test("bpx=0x123123\0");
    do_test("bpx=0x123123,dump(eax,1234)\0");
    do_test("bpx=0x123123,dump(filename.dll!symbol,0x29a)\0");
    do_test("bpx=filename.dll!symbol1,dump(filename.dll!symbol2,0x29a)\0");
    do_test("bpx=filename.dll!symbol1,dump(filename.dll!symbol3),dump(eax,123)\0");
    do_test("bpx=filename.dll!symbol1,copy(eax,\"hahaha\\x00\\x11hoho\")\0");
    do_test("bpx=bytemask:\"001122..3355[skip:2]1166..77\",copy(eax,\"hahaha\\x00\\x11hoho\")\0");
    do_test("bpx=filename.dll!symbol1,copy(filename.dll!symbol2,\"hahaha\\x00\\x11hoho\")\0");
    do_test("bpx=filename.dll!symbol1,set(eax,111),set(ebx,222),dump(filename.dll!symbol3,333)\0");
    do_test("bpf=filename.dll!symbol1,args:6,skip,unicode,when_called_from_func:filename.dll!func,rt:123\0");
    do_test("bpf=filename.dll!symbol1,dump_args:6,skip_stdcall,when_called_from_address:filename.dll!func+0x1234,rt_probability:0.17,pause:700\0");
    do_test("bpf=filename.dll!symbol1,rt_probability:50%,rt:123\0");
    do_test("bpf=filename.dll!symbol1,rt_probability:0%,rt:123\0");
    do_test("bpf=filename.dll!symbol1,rt_probability:1%,rt:123\0");
    do_test("bpf=filename.dll!symbol1,rt_probability:100%,rt:123\0");
    do_test("bpx=filename.dll!0x12345678,dump(eax,123)\0");
    do_test("bpf=filename.dll!0x14036FE50,rt:0\n\0");
    do_test("bpf=filename.dll!0x14036FE50,rt:0,set(byte,*(arg_5+0x123)=0x567)\0");
    do_test("bpx=filename.dll!0x12345678,set(st0,123.456)\0");
    do_test("bpx=filename.dll!0x12345678,set(st0,-123.456)\0");
    do_test("bpx=filename.dll!0x12345678,set(st0,-123.456)\n\0");
    do_test("bpx=bytemask:\"DC45E883EC08DD1C2483EC08DD45E8DD1C24\",set(st0,2.34567890)\0");
    do_test("bpx=bytemask:\"DC45E883EC08DD1C2483EC08DD45E8DD1C24\",set(st0,2.34567890)\n\0");
    dlist_free(addresses_to_be_resolved, NULL);
    DFREE(load_filename);
    DFREE(attach_filename);
    DFREE(load_command_line);
    
    dump_unfreed_blocks();

    return 0;
}
#endif

/* vim: set expandtab ts=4 sw=4 : */
