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
#include "dmalloc.h"
#include "cc.h"
#include "process.h"
#include "thread.h"
#include "module.h"
#include "memutils.h"
#include "memorycache.h"
#include "x86.h"
#include "bitfields.h"
#include "symbol.h"

void cc_tests()
{
#ifndef _WIN64	
	CONTEXT ctx;
	thread *t=DCALLOC(thread, 1, "thread");
	module *m=DCALLOC(module, 1, "module");
    	m->symbols=rbtree_create(true, "symbols", compare_size_t);
	m->size=0x10000;
	m->internal_name=DSTRDUP ("dummy_module", "char*");
	bool b;
	Da da;

	bzero (&ctx, sizeof(CONTEXT));
	BYTE* memory_test=DCALLOC(BYTE, PAGE_SIZE, "BYTE");

	MemoryCache *mc=MC_MemoryCache_ctor_testing (memory_test, PAGE_SIZE);

	process *p=process_init (0, 0, 0, 0);
	
	rbtree_insert (p->modules, NULL, (void*)m);

    	rbtree_insert (p->threads, (void*)0, (void*)t);

	add_symbol_params sym_params={ p, m, SYM_TYPE_MAP, mc };
	add_symbol (0x1000, "dummy_symbol", &sym_params);

	// addr 0

	tetrabyte EAX_array_at_0[]={0xa, 0xb, 0xc, 0xd, 0x10, 0x12, 0x15, 0x20, 0x29a};
	ctx.Eip=0;
	for (unsigned i=0; i<sizeof(EAX_array_at_0)/sizeof(tetrabyte); i++)
	{
		ctx.Eax=EAX_array_at_0[i];
		b=Da_Da(Fuzzy_False, (BYTE*)X86_INC_EAX, ctx.Eip, &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	ctx.Eip+=X86_INC_EAX_LEN;
	
	// addr 1
	for (tetrabyte EAX=0xa; EAX<0x70; EAX+=4)
	{
		ctx.Eax=EAX;
		b=Da_Da(Fuzzy_False, (BYTE*)X86_INC_EAX, ctx.Eip, &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	ctx.Eip+=X86_INC_EAX_LEN;
	
	// addr 2
	for (tetrabyte EAX=0xa; EAX<0x70; EAX+=8)
	{
		ctx.Eax=EAX;
		b=Da_Da(Fuzzy_False, (BYTE*)X86_INC_EAX, ctx.Eip, &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	ctx.Eip+=X86_INC_EAX_LEN;

	// addr 3

#define TEST_STRING_1 "test_string_1"
#define TEST_STRING_2 "test_string_2"
#define TEST_STRING_3 "test_string_3"

	b=MC_WriteBuffer (mc, 0, strlen(TEST_STRING_1)+1, (BYTE*)TEST_STRING_1);
	oassert(b);
	b=MC_WriteBuffer (mc, 0x10, strlen(TEST_STRING_2)+1, (BYTE*)TEST_STRING_2);
	oassert(b);
	b=MC_WriteBuffer (mc, 0x20, strlen(TEST_STRING_3)+1, (BYTE*)TEST_STRING_3);
	oassert(b);

	for (tetrabyte i=0; i<0x30; i+=0x10)
	{
		ctx.Esi=i;
		b=Da_Da(Fuzzy_False, (BYTE*)X86_MOV_EAX_ESI, ctx.Eip, &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	ctx.Eip+=X86_MOV_EAX_ESI_LEN;
	
	// addr 5
	double ST0_array_at_0[]={ 1234.1, 666.7, 0.1, 1, 100, 100 };
	for (unsigned i=0; i<sizeof(ST0_array_at_0)/sizeof(double); i++)
	{
		CONTEXT_set_reg_STx (&ctx, 0, ST0_array_at_0[i]);
#ifdef THIS_CODE_IS_NOT_WORKING		
		_FPU_set_tag (&ctx, 0, FPU_TAG_NON_ZERO); // ST0 is present
#endif		
		//cur_fds.fd1=stdout;
		//dump_CONTEXT (&cur_fds, &ctx, true, false, false);
		ctx.Esi=0x30;
		b=Da_Da(Fuzzy_False, (BYTE*)X86_FSTP_ESI, ctx.Eip, &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	ctx.Eip+=X86_FSTP_ESI_LEN;

	// addr 7
	for (double i=0; i<1000; i+=0.1)
	{
		CONTEXT_set_reg_STx (&ctx, 0, i);
#ifdef THIS_CODE_IS_NOT_WORKING		
		_FPU_set_tag (&ctx, 0, FPU_TAG_NON_ZERO); // ST0 is present
#endif		
		//cur_fds.fd1=stdout;
		//dump_CONTEXT (&cur_fds, &ctx, true, false, false);
		ctx.Esi=0x30;
		b=Da_Da(Fuzzy_False, (BYTE*)X86_FSTP_ESI, ctx.Eip, &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	ctx.Eip+=X86_FSTP_ESI_LEN;

	// addr 9
	{
		b=Da_Da(Fuzzy_False, (BYTE*)X86_JA_NEXT, ctx.Eip, &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	ctx.Eip+=X86_JA_NEXT_LEN;

	// addr 11 (0xb)
	SET_BIT(ctx.EFlags, FLAG_ZF);
	SET_BIT(ctx.EFlags, FLAG_CF);
	{
		b=Da_Da(Fuzzy_False, (BYTE*)X86_JA_NEXT, ctx.Eip, &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	ctx.Eip+=X86_JA_NEXT_LEN;

	// addr 13 (0xd)
	{
		b=Da_Da(Fuzzy_False, (BYTE*)X86_JA_NEXT, ctx.Eip, &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	REMOVE_BIT(ctx.EFlags, FLAG_ZF);
	REMOVE_BIT(ctx.EFlags, FLAG_CF);
	// one more time
	{
		b=Da_Da(Fuzzy_False, (BYTE*)X86_JA_NEXT, ctx.Eip, &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	ctx.Eip+=X86_JA_NEXT_LEN;

	// addr 15 (0xf)
	ctx.Eax=0x1006;
	{
		b=Da_Da(Fuzzy_False, (BYTE*)X86_CALL_EAX, ctx.Eip, &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	ctx.Eip+=X86_CALL_EAX_LEN;

	// addr 17 (0x11)
	for (tetrabyte EAX=0x100; EAX<0x200; EAX+=4)
		for (tetrabyte EBX=0x200; EBX<0x220; EBX++)
		{
			ctx.Eax=EAX;
			ctx.Ebx=EBX;
			b=Da_Da(Fuzzy_False, (BYTE*)X86_CMP_EAX_EBX, ctx.Eip, &da);
			handle_cc(&da, p, t, &ctx, mc, false, false);
		};
	ctx.Eip+=X86_CMP_EAX_EBX_LEN;

	for (rbtree_node *i=rbtree_minimum(m->PC_infos); i; i=rbtree_succ(i))
	{
		strbuf sb=STRBUF_INIT;
		address a=(address)i->key;
		PC_info *info=i->value;

		construct_common_string(&sb, a, info);
		//printf ("a=0x" PRI_ADR_HEX " [%s]\n", a, sb.buf);
		const char *should_be;
		switch (a)
		{
			case 0: should_be="EAX=0xa..0xd, 0x10, 0x12, 0x15, 0x20, 0x29a";
				break;
			case 1: should_be="EAX=0xa..0x1e(step=4) (16 items skipped) 0x5e..0x6e(step=4)";
				break;
			case 2: should_be="EAX=0xa..0x32(step=8) (3 items skipped) 0x4a..0x6a(step=8)";
				break;
			case 3: should_be="ESI=0, 0x10, 0x20, \"test_string_1\", \"test_string_2\", \"test_string_3\"";
				break;
			case 5: should_be="ST0=0.1, 1.0, 100.0, 666.7, 1234.1";
				break;
			case 7: should_be="ST0=0.0, 0.1, 0.2, 0.3, 0.4 (9990 doubles skipped) 999.5, 999.6, 999.7, 999.8, 999.9";
				break;
			case 9: should_be="ZF=false CF=false";
				break;
			case 0xb: should_be="ZF=true CF=true";
				  break;
			case 0xd: should_be="ZF=false,true CF=false,true";
				  break;
			case 0xf: should_be="op1=dummy_module!dummy_symbol+0x6";
				  break;
			case 0x11: should_be="EAX=0x100..0x114(step=4) (54 items skipped) 0x1ec..0x1fc(step=4) EBX=0x200..0x205 (22 items skipped) 0x21b..0x21f";
				   break;
			default:
				   oassert(0);
				   fatal_error();
				   break;
		};
		oassert(strcmp(should_be, sb.buf)==0);
		strbuf_deinit (&sb);
		free_PC_info (info);
	};

    	rbtree_deinit(m->PC_infos);
	m->PC_infos=NULL;

	MC_MemoryCache_dtor (mc, false);
	DFREE (memory_test);
	process_free (p);
	dump_unfreed_blocks();
#endif	
};
