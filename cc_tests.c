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
	CONTEXT ctx;
	thread *t=DCALLOC(thread, 1, "thread");
	module *m=DCALLOC(module, 1, "module");
    	m->symbols=rbtree_create(true, "symbols", compare_size_t);
	m->size=0x10000;
	m->internal_name=DSTRDUP ("dummy_module", "char*");
	bool b;

	bzero (&ctx, sizeof(CONTEXT));
	BYTE* memory_test=DCALLOC(BYTE, PAGE_SIZE, "BYTE");

	MemoryCache *mc=MC_MemoryCache_ctor_testing (memory_test, PAGE_SIZE);

	process *p=process_init (0, 0, 0, 0);
	
	rbtree_insert (p->modules, NULL, (void*)m);

    	rbtree_insert (p->threads, (void*)0, (void*)t);

	add_symbol_params sym_params={ p, m, SYM_TYPE_MAP, mc };
	add_symbol (0x1000, "dummy_symbol", &sym_params);

	// addr 0
#define X86_INC_EAX "\x40"
#define X86_INC_EAX_LEN 1

	tetrabyte EAX_array_at_0[]={0xa, 0xb, 0xc, 0xd, 0x10, 0x12, 0x15, 0x20, 0x29a};
	ctx.Eip=0;
	for (unsigned i=0; i<sizeof(EAX_array_at_0)/sizeof(tetrabyte); i++)
	{
		ctx.Eax=EAX_array_at_0[i];
		Da* d=Da_Da(Fuzzy_False, (BYTE*)X86_INC_EAX, ctx.Eip);
		handle_cc(d, p, t, &ctx, mc, false, false);
		Da_free (d);
	};
	ctx.Eip+=X86_INC_EAX_LEN;
	
	// addr 1
	for (tetrabyte EAX=0xa; EAX<0x70; EAX+=4)
	{
		ctx.Eax=EAX;
		Da* d=Da_Da(Fuzzy_False, (BYTE*)X86_INC_EAX, ctx.Eip);
		handle_cc(d, p, t, &ctx, mc, false, false);
		Da_free (d);
	};
	ctx.Eip+=X86_INC_EAX_LEN;
	
	// addr 2
	for (tetrabyte EAX=0xa; EAX<0x70; EAX+=8)
	{
		ctx.Eax=EAX;
		Da* d=Da_Da(Fuzzy_False, (BYTE*)X86_INC_EAX, ctx.Eip);
		handle_cc(d, p, t, &ctx, mc, false, false);
		Da_free (d);
	};
	ctx.Eip+=X86_INC_EAX_LEN;

	// addr 3
#define X86_MOV_EAX_ESI "\x89\xF0"
#define X86_MOV_EAX_ESI_LEN 2

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
		Da* d=Da_Da(Fuzzy_False, (BYTE*)X86_MOV_EAX_ESI, ctx.Eip);
		handle_cc(d, p, t, &ctx, mc, false, false);
		Da_free (d);
	};
	ctx.Eip+=X86_MOV_EAX_ESI_LEN;
	
#define X86_FSTP_ESI "\xD9\x1E"
#define X86_FSTP_ESI_LEN 2

	// addr 5
	double ST0_array_at_0[]={ 1234.1, 666.7, 0.1, 1, 100, 100 };
	for (unsigned i=0; i<sizeof(ST0_array_at_0)/sizeof(double); i++)
	{
		CONTEXT_set_reg_STx (&ctx, R_ST0, ST0_array_at_0[i]);
		FPU_set_tag (&ctx, 0); // ST0 is present
		cur_fds.fd1=stdout;
		//dump_CONTEXT (&cur_fds, &ctx, true, false, false);
		ctx.Esi=0x30;
		Da* d=Da_Da(Fuzzy_False, (BYTE*)X86_FSTP_ESI, ctx.Eip);
		handle_cc(d, p, t, &ctx, mc, false, false);
		Da_free (d);
	};
	ctx.Eip+=X86_FSTP_ESI_LEN;

	// addr 7
	for (double i=0; i<1000; i+=0.1)
	{
		CONTEXT_set_reg_STx (&ctx, R_ST0, i);
		FPU_set_tag (&ctx, 0); // ST0 is present
		//cur_fds.fd1=stdout;
		//dump_CONTEXT (&cur_fds, &ctx, true, false, false);
		ctx.Esi=0x30;
		Da* d=Da_Da(Fuzzy_False, (BYTE*)X86_FSTP_ESI, ctx.Eip);
		handle_cc(d, p, t, &ctx, mc, false, false);
		Da_free (d);
	};
	ctx.Eip+=X86_FSTP_ESI_LEN;

	// addr 9
#define X86_JA_NEXT "\x77\x00"
#define X86_JA_NEXT_LEN 2
	{
		Da* d=Da_Da(Fuzzy_False, (BYTE*)X86_JA_NEXT, ctx.Eip);
		handle_cc(d, p, t, &ctx, mc, false, false);
		Da_free (d);
	};
	ctx.Eip+=X86_JA_NEXT_LEN;

	// addr 11 (0xb)
	SET_BIT(ctx.EFlags, FLAG_ZF);
	SET_BIT(ctx.EFlags, FLAG_CF);
	{
		Da* d=Da_Da(Fuzzy_False, (BYTE*)X86_JA_NEXT, ctx.Eip);
		handle_cc(d, p, t, &ctx, mc, false, false);
		Da_free (d);
	};
	ctx.Eip+=X86_JA_NEXT_LEN;

	// addr 13 (0xd)
	{
		Da* d=Da_Da(Fuzzy_False, (BYTE*)X86_JA_NEXT, ctx.Eip);
		handle_cc(d, p, t, &ctx, mc, false, false);
		Da_free (d);
	};
	REMOVE_BIT(ctx.EFlags, FLAG_ZF);
	REMOVE_BIT(ctx.EFlags, FLAG_CF);
	// one more time
	{
		Da* d=Da_Da(Fuzzy_False, (BYTE*)X86_JA_NEXT, ctx.Eip);
		handle_cc(d, p, t, &ctx, mc, false, false);
		Da_free (d);
	};
	ctx.Eip+=X86_JA_NEXT_LEN;

	// addr 15 (0xf)
#define X86_CALL_EAX "\xFF\xD0"
#define X86_CALL_EAX_LEN 2
	ctx.Eax=0x1006;
	{
		Da* d=Da_Da(Fuzzy_False, (BYTE*)X86_CALL_EAX, ctx.Eip);
		handle_cc(d, p, t, &ctx, mc, false, false);
		Da_free (d);
	};
	ctx.Eip+=X86_CALL_EAX_LEN;

	// addr 17 (0x11)
#define X86_CMP_EAX_EBX "\x39\xD8"
#define X86_CMP_EAX_EBX_LEN 2
	for (tetrabyte EAX=0x100; EAX<0x200; EAX+=4)
		for (tetrabyte EBX=0x200; EBX<0x220; EBX++)
		{
			ctx.Eax=EAX;
			ctx.Ebx=EBX;
			Da* d=Da_Da(Fuzzy_False, (BYTE*)X86_CMP_EAX_EBX, ctx.Eip);
			handle_cc(d, p, t, &ctx, mc, false, false);
			Da_free (d);
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
				   break;
		};
		oassert(strcmp(should_be, sb.buf)==0);
		strbuf_deinit (&sb);
	};

	m->PC_infos=NULL;

	MC_MemoryCache_dtor (mc, false);
	DFREE (memory_test);
	process_free (p);
};
