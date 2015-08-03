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
	dump_fpu=true;

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

	add_symbol (0x1000, "dummy_symbol", &(add_symbol_params){ p, m, SYM_TYPE_MAP, mc });

	// addr 0

	REG AX_array_at_0[]={0xa, 0xb, 0xc, 0xd, 0x10, 0x12, 0x15, 0x20, 0x29a};
	CONTEXT_set_PC (&ctx, 0);
	for (unsigned i=0; i<sizeof(AX_array_at_0)/sizeof(REG); i++)
	{
		CONTEXT_set_Accum(&ctx, AX_array_at_0[i]);
		b=Da_Da(Fuzzy_Undefined, (BYTE*)X86_OR_X64_INC_AX, CONTEXT_get_PC (&ctx), &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	CONTEXT_add_to_PC(&ctx, X86_OR_X64_INC_AX_LEN);
	
	// addr 1
	CONTEXT_set_PC (&ctx, 1);
	for (REG AX=0xa; AX<0x70; AX+=4)
	{
		CONTEXT_set_Accum (&ctx, AX);
		b=Da_Da(Fuzzy_Undefined, (BYTE*)X86_OR_X64_INC_AX, CONTEXT_get_PC (&ctx), &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	CONTEXT_add_to_PC(&ctx, X86_OR_X64_INC_AX_LEN);
	
	// addr 2
	CONTEXT_set_PC (&ctx, 2);
	for (REG AX=0xa; AX<0x70; AX+=8)
	{
		CONTEXT_set_Accum (&ctx, AX);
		b=Da_Da(Fuzzy_Undefined, (BYTE*)X86_OR_X64_INC_AX, CONTEXT_get_PC (&ctx), &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	CONTEXT_add_to_PC(&ctx, X86_OR_X64_INC_AX_LEN);

	// addr 3

	CONTEXT_set_PC (&ctx, 3);
#define TEST_STRING_1 "test_string_1"
#define TEST_STRING_2 "test_string_2"
#define TEST_STRING_3 "test_string_3"

	b=MC_WriteBuffer (mc, 0, strlen(TEST_STRING_1)+1, (BYTE*)TEST_STRING_1);
	oassert(b);
	b=MC_WriteBuffer (mc, 0x10, strlen(TEST_STRING_2)+1, (BYTE*)TEST_STRING_2);
	oassert(b);
	b=MC_WriteBuffer (mc, 0x20, strlen(TEST_STRING_3)+1, (BYTE*)TEST_STRING_3);
	oassert(b);

	for (address i=0; i<0x30; i+=0x10)
	{
		CONTEXT_set_xSI(&ctx, i);
		b=Da_Da(Fuzzy_Undefined, (BYTE*)X86_OR_X64_MOV_AX_SI, CONTEXT_get_PC(&ctx), &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	CONTEXT_add_to_PC(&ctx, X86_OR_X64_MOV_AX_SI_LEN);
	
	// addr 5
	CONTEXT_set_PC (&ctx, 5);
	double ST0_array_at_0[]={ 1234.1, 666.7, 0.1, 1, 100, 100 };
	for (unsigned i=0; i<sizeof(ST0_array_at_0)/sizeof(double); i++)
	{
		CONTEXT_set_reg_STx (&ctx, 0, ST0_array_at_0[i]);
#ifdef THIS_CODE_IS_NOT_WORKING		
		_FPU_set_tag (&ctx, 0, FPU_TAG_NON_ZERO); // ST0 is present
#endif		
		//dump_CONTEXT (&cur_fds, &ctx, true, false, false);
		CONTEXT_set_xSI(&ctx, 0x30);
		b=Da_Da(Fuzzy_Undefined, (BYTE*)X86_OR_X64_FSTP_SI, CONTEXT_get_PC(&ctx), &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	CONTEXT_add_to_PC(&ctx, X86_OR_X64_FSTP_SI_LEN);

	// addr 7
	CONTEXT_set_PC (&ctx, 7);
	for (double i=0; i<1000; i+=0.1)
	{
		CONTEXT_set_reg_STx (&ctx, 0, i);
#ifdef THIS_CODE_IS_NOT_WORKING		
		_FPU_set_tag (&ctx, 0, FPU_TAG_NON_ZERO); // ST0 is present
#endif		
		//cur_fds.fd1=stdout;
		//dump_CONTEXT (&cur_fds, &ctx, true, false, false);
		CONTEXT_set_xSI(&ctx, 0x30);
		b=Da_Da(Fuzzy_Undefined, (BYTE*)X86_OR_X64_FSTP_SI, CONTEXT_get_PC(&ctx), &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	CONTEXT_add_to_PC(&ctx, X86_OR_X64_FSTP_SI_LEN);

	// addr 9
	CONTEXT_set_PC (&ctx, 9);
	{
		b=Da_Da(Fuzzy_Undefined, (BYTE*)X86_OR_X64_JA_NEXT, CONTEXT_get_PC(&ctx), &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	CONTEXT_add_to_PC(&ctx, X86_OR_X64_JA_NEXT_LEN);

	// addr 11 (0xb)
	CONTEXT_set_PC (&ctx, 0xb);
	SET_BIT(ctx.EFlags, FLAG_ZF);
	SET_BIT(ctx.EFlags, FLAG_CF);
	{
		b=Da_Da(Fuzzy_Undefined, (BYTE*)X86_OR_X64_JA_NEXT, CONTEXT_get_PC(&ctx), &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	CONTEXT_add_to_PC(&ctx, X86_OR_X64_JA_NEXT_LEN);

	// addr 13 (0xd)
	CONTEXT_set_PC (&ctx, 0xd);
	{
		b=Da_Da(Fuzzy_Undefined, (BYTE*)X86_OR_X64_JA_NEXT, CONTEXT_get_PC(&ctx), &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	REMOVE_BIT(ctx.EFlags, FLAG_ZF);
	REMOVE_BIT(ctx.EFlags, FLAG_CF);
	// one more time
	{
		b=Da_Da(Fuzzy_Undefined, (BYTE*)X86_OR_X64_JA_NEXT, CONTEXT_get_PC(&ctx), &da);
		oassert(b);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	CONTEXT_add_to_PC(&ctx, X86_OR_X64_JA_NEXT_LEN);

	// addr 15 (0xf)
	CONTEXT_set_PC (&ctx, 0xf);
	CONTEXT_set_Accum(&ctx, 0x1006);
	{
		b=Da_Da(Fuzzy_Undefined, (BYTE*)X86_OR_X64_CALL_xAX, CONTEXT_get_PC(&ctx), &da);
		oassert(b);
		//cur_fds.fd1=stdout;
		//Da_DumpString(&cur_fds, &da);
		handle_cc(&da, p, t, &ctx, mc, false, false);
	};
	CONTEXT_add_to_PC(&ctx, X86_OR_X64_CALL_xAX_LEN);

	// addr 17 (0x11)
	CONTEXT_set_PC (&ctx, 0x11);
	for (REG AX=0x100; AX<0x200; AX+=4)
		for (REG BX=0x200; BX<0x220; BX++)
		{
			CONTEXT_set_Accum(&ctx, AX);
			CONTEXT_set_xBX(&ctx, BX);
			b=Da_Da(Fuzzy_Undefined, (BYTE*)X86_OR_X64_CMP_xAX_xBX, CONTEXT_get_PC(&ctx), &da);
			handle_cc(&da, p, t, &ctx, mc, false, false);
		};
	CONTEXT_add_to_PC(&ctx, X86_OR_X64_CMP_xAX_xBX_LEN);

	// addr 32 (0x20)
	CONTEXT_set_PC (&ctx, 0x20);
	CONTEXT_set_xCX(&ctx, 0x11223344);
	b=Da_Da(Fuzzy_Undefined, (BYTE*)X86_OR_X64_CMP_xCX_1, CONTEXT_get_PC(&ctx), &da);
	handle_cc(&da, p, t, &ctx, mc, false, false);
	CONTEXT_add_to_PC(&ctx, X86_OR_X64_CMP_xCX_1_LEN);

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
			case 0: should_be=AX_REGISTER_NAME "=0xa..0xd, 0x10, 0x12, 0x15, 0x20, 0x29a";
				break;
			case 1: should_be=AX_REGISTER_NAME "=0xa..0x1e(step=4) (16 items skipped) 0x5e..0x6e(step=4)";
				break;
			case 2: should_be=AX_REGISTER_NAME "=0xa..0x32(step=8) (3 items skipped) 0x4a..0x6a(step=8)";
				break;
			case 3: should_be=SI_REGISTER_NAME "=0, 0x10, 0x20, \"test_string_1\", \"test_string_2\", \"test_string_3\"";
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
			case 0xf: should_be="op1=dummy_module!dummy_symbol+0x6 (0x2001001)";
				  break;
			case 0x11: should_be=AX_REGISTER_NAME "=0x100..0x114(step=4) (54 items skipped) 0x1ec..0x1fc(step=4) " BX_REGISTER_NAME "=0x200..0x205 (22 items skipped) 0x21b..0x21f";
				   break;
			case 0x20: should_be=CX_REGISTER_NAME "=0x11223344";
				  break;
			default:
				   fatal_error();
				   break;
		};
		if(strcmp(should_be, sb.buf)!=0)
		{
			printf ("strcmp (%s, %s) failed\n", should_be, sb.buf);
			exit(1);
		};
		strbuf_deinit (&sb);
		free_PC_info (info);
	};

    	rbtree_deinit(m->PC_infos);
	m->PC_infos=NULL;

	MC_MemoryCache_dtor (mc, false);
	DFREE (memory_test);
	process_free (p);
	dump_unfreed_blocks();
};
