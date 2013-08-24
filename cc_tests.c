#include "oassert.h"
#include "dmalloc.h"
#include "cc.h"
#include "process.h"
#include "thread.h"
#include "module.h"
#include "memutils.h"

void cc_tests()
{
	CONTEXT ctx;
	thread *t=DCALLOC(thread, 1, "thread");
	module *m=DCALLOC(module, 1, "module");
	bool b;

	bzero (&ctx, sizeof(CONTEXT));
	BYTE* memory_test=DCALLOC(BYTE, PAGE_SIZE, "BYTE");

	MemoryCache *mc=MC_MemoryCache_ctor_testing (memory_test, PAGE_SIZE);

	process *p=process_init (0, 0, 0, 0);
	
	m->size=PAGE_SIZE;
	rbtree_insert (p->modules, NULL, (void*)m);

    	rbtree_insert (p->threads, (void*)0, (void*)t);

	// addr 0
#define X86_INC_EAX "\x40"
#define X86_INC_EAX_LEN 1

	tetrabyte EAX_array_at_0[]={0xa, 0xb, 0xc, 0xd, 0x10, 0x12, 0x15, 0x20, 0x29a};
	ctx.Eip=0;
	for (tetrabyte i=0; i<sizeof(EAX_array_at_0)/sizeof(tetrabyte); i++)
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
	CONTEXT_set_reg_STx (&ctx, R_ST0, 1234.1);
	FPU_set_tag (&ctx, 0); // ST0 is present
	cur_fds.fd1=stdout;
	//dump_CONTEXT (&cur_fds, &ctx, true, false, false);
	ctx.Esi=0x30;
	Da* d=Da_Da(Fuzzy_False, (BYTE*)X86_FSTP_ESI, ctx.Eip);
	handle_cc(d, p, t, &ctx, mc, false, false);
	Da_free (d);
	ctx.Eip+=X86_FSTP_ESI_LEN;

	//printf ("m->PC_infos=0x%p\n", m->PC_infos);
	//printf ("cnt=%d\n", rbtree_count(m->PC_infos));
	for (rbtree_node *i=rbtree_minimum(m->PC_infos); i; i=rbtree_succ(i))
	{
		strbuf sb=STRBUF_INIT;
		address a=(address)i->key;
		PC_info *info=i->value;

		construct_common_string(&sb, a, info);
		printf ("a=0x" PRI_ADR_HEX " [%s]\n", a, sb.buf);
		strbuf_deinit (&sb);
	};


	m->PC_infos=NULL;

	MC_MemoryCache_dtor (mc, false);
	DFREE (memory_test);
	process_free (p);
};
