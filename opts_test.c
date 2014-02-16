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

#include "oassert.h"
#include "ostrings.h"
#include "opts.h"
#include "dmalloc.h"

#ifdef _DEBUG

// opts.tab.c
int yydebug;

static void do_test(char *s, char *should_be)
{
    BP *b;
    strbuf sb=STRBUF_INIT;
    //printf ("do_test(%s)\n", s);

    b=parse_option(s);
    oassert(b && "parse_option() failed\n");
    BP_ToString(b, &sb);

    if (strcmp (sb.buf, should_be)!=0)
    {
        printf ("test failed. we got=[%s]\n", sb.buf);
        printf ("but should be=      [%s]\n", should_be);
        exit(0);
    };

    strbuf_deinit (&sb);
    //BP_free(b);
    BP_free (breakpoints[0]);
    breakpoints[0]=NULL;
};

void opts_test()
{
#ifdef O_BITS64
    oassert(strtol_or_strtoll("0xAB12345678", NULL, 16)==0xAB12345678);
#elif defined O_BITS32
    oassert(strtol_or_strtoll("0x12345678", NULL, 16)==0x12345678);
#else
#error "O_BITS64 or O_BITS32 should be defined"
#endif

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
    do_test("bpmq=file.dll!symbol,rw\0", 
            "bp_address=file.dll!symbol. BPM. width=8, type=RW\n");
    do_test("bpmq=file.dll!symbol+0x123,w\0", 
            "bp_address=file.dll!symbol+0x123. BPM. width=8, type=W\n");
    do_test("bpmb=0x123123,w\0", 
            "bp_address=0x123123. BPM. width=1, type=W\n");
    do_test("bpmq=file.dll!symbol+123,rw\0", 
            "bp_address=file.dll!symbol+0x7b. BPM. width=8, type=RW\n");
    do_test("bpmd=0x12345678,w\0", 
            "bp_address=0x12345678. BPM. width=4, type=W\n");
    do_test("bpmw=bytemask:\"001122\",w\0", 
            "bp_address=bytemask:\"001122\". BPM. width=2, type=W\n");
    do_test("bpmd=bytemask:\"0011..22\",rw\0", 
            "bp_address=bytemask:\"0011..22\". BPM. width=4, type=RW\n");
    do_test("bpmq=bytemask:\"0011[skip:2]22\",w\0", 
            "bp_address=bytemask:\"0011....22\". BPM. width=8, type=W\n");
    do_test("bpmb=bytemask:\"001122..3355[skip:2]1166..77\",rw\0", 
            "bp_address=bytemask:\"001122..3355....1166..77\". BPM. width=1, type=RW\n");
    do_test("bpx=0x123123\0", 
            "bp_address=0x123123. BPX.\n");
    do_test("bpx=0x123123,dump(eax,1234)\0", 
            "bp_address=0x123123. BPX. options: [DUMP EAX size: 1234]\n");
    do_test("bpx=0x123123,dump(filename.dll!symbol,0x29a)\0", 
            "bp_address=0x123123. BPX. options: [DUMP filename.dll!symbol size: 666]\n");
    do_test("bpx=filename.dll!symbol1,dump(filename.dll!symbol2,0x29a)\0", 
            "bp_address=filename.dll!symbol1. BPX. options: [DUMP filename.dll!symbol2 size: 666]\n");
    do_test("bpx=filename.dll!symbol1,dump(filename.dll!symbol3),dump(eax,123)\0", 
            "bp_address=filename.dll!symbol1. BPX. options: [DUMP EAX size: 123][DUMP filename.dll!symbol3 size: 16]\n");
    do_test("bpx=filename.dll!symbol1,copy(eax,\"hahaha\\x00\\x11hoho\")\0", 
            "bp_address=filename.dll!symbol1. BPX. options: [COPY reg:EAX[0x68 0x61 0x68 0x61 0x68 0x61 0x00 0x11 0x68 0x6F 0x68 0x6F ]\n");
    do_test("bpx=bytemask:\"001122..3355[skip:2]1166..77\",copy(eax,\"hahaha\\x00\\x11hoho\")\0", 
            "bp_address=bytemask:\"001122..3355....1166..77\". BPX. options: [COPY reg:EAX[0x68 0x61 0x68 0x61 0x68 0x61 0x00 0x11 0x68 0x6F 0x68 0x6F ]\n");
    do_test("bpx=filename.dll!symbol1,copy(filename.dll!symbol2,\"hahaha\\x00\\x11hoho\")\0", 
            "bp_address=filename.dll!symbol1. BPX. options: [COPY filename.dll!symbol2[0x68 0x61 0x68 0x61 0x68 0x61 0x00 0x11 0x68 0x6F 0x68 0x6F ]\n");
    do_test("bpx=filename.dll!symbol1,set(eax,111),set(ebx,222),dump(filename.dll!symbol3,333)\0", 
            "bp_address=filename.dll!symbol1. BPX. options: [DUMP filename.dll!symbol3 size: 333][SET reg:EBX value:222][SET reg:EAX value:111]\n");
    do_test("bpf=filename.dll!symbol1,args:6,skip,unicode,when_called_from_func:filename.dll!func,rt:123\0", 
            "bp_address=filename.dll!symbol1. BPF. options: unicode skip rt: 7b args: 6 \nwhen_called_from_func: filename.dll!func\n");
    do_test("bpf=filename.dll!symbol1,dump_args:6,skip_stdcall,when_called_from_address:filename.dll!func+0x1234,rt:123,rt_probability:0.17,pause:700\0", 
            "bp_address=filename.dll!symbol1. BPF. options: skip_stdcall rt: 7b rt_probability: 0.170000 dump_args: 6 pause: 700 \nwhen_called_from_address: filename.dll!func+0x1234\n");
    do_test("bpf=filename.dll!symbol1,rt_probability:50%,rt:123\0", 
            "bp_address=filename.dll!symbol1. BPF. options: rt: 7b rt_probability: 0.500000 \n");
    do_test("bpf=filename.dll!symbol1,rt_probability:0%,rt:123\0", 
            "bp_address=filename.dll!symbol1. BPF. options: rt: 7b \n");
    do_test("bpf=filename.dll!symbol1,rt_probability:1%,rt:123\0", 
            "bp_address=filename.dll!symbol1. BPF. options: rt: 7b rt_probability: 0.010000 \n");
    do_test("bpf=filename.dll!symbol1,rt_probability:100%,rt:123\0", 
            "bp_address=filename.dll!symbol1. BPF. options: rt: 7b \n");
    do_test("bpx=filename.dll!0x12345678,dump(eax,123)\0", 
            "bp_address=filename.dll!0x12345678. BPX. options: [DUMP EAX size: 123]\n");
    do_test("bpf=filename.dll!0x14036FE50,rt:0\n\0", 
#ifdef _WIN64
            "bp_address=filename.dll!0x14036fe50. BPF. options: rt: 0 \n");
#else
            "bp_address=filename.dll!0x7fffffff. BPF. options: rt: 0 \n");
#endif
    do_test("bpf=filename.dll!0x14036FE50,rt:0,set(byte,*(arg_5+0x123)=0x567)\0",
#ifdef _WIN64
            "bp_address=filename.dll!0x14036fe50. BPF. options: rt: 0 \nset_width=1 set_arg_n=5 set_ofs=0x123 set_val=0x567\n");
#else
            "bp_address=filename.dll!0x7fffffff. BPF. options: rt: 0 \nset_width=1 set_arg_n=5 set_ofs=0x123 set_val=0x567\n");
#endif
    do_test("bpx=filename.dll!0x12345678,set(st0,123.456)\0", 
            "bp_address=filename.dll!0x12345678. BPX. options: [SET reg:ST0 float_value:123.456000]\n");
    do_test("bpx=filename.dll!0x12345678,set(st0,-123.456)\0", 
            "bp_address=filename.dll!0x12345678. BPX. options: [SET reg:ST0 float_value:-123.456000]\n");
    do_test("bpx=filename.dll!0x12345678,set(st0,-123.456)\n\0",
            "bp_address=filename.dll!0x12345678. BPX. options: [SET reg:ST0 float_value:-123.456000]\n");
    do_test("bpx=bytemask:\"DC45E883EC08DD1C2483EC08DD45E8DD1C24\",set(st0,2.34567890)\0", 
            "bp_address=bytemask:\"DC45E883EC08DD1C2483EC08DD45E8DD1C24\". BPX. options: [SET reg:ST0 float_value:2.345679]\n");
    do_test("bpx=bytemask:\"DC45E883EC08DD1C2483EC08DD45E8DD1C24\",set(st0,2.34567890)\n\0",
            "bp_address=bytemask:\"DC45E883EC08DD1C2483EC08DD45E8DD1C24\". BPX. options: [SET reg:ST0 float_value:2.345679]\n");
    dlist_free(addresses_to_be_resolved, NULL);
    DFREE(load_filename);
    DFREE(attach_filename);
    DFREE(load_command_line);
    
    dump_unfreed_blocks();
}
#endif

/* vim: set expandtab ts=4 sw=4 : */
