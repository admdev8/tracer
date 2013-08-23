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

	bzero (&ctx, sizeof(CONTEXT));
	BYTE* memory_test=DCALLOC(BYTE, PAGE_SIZE, "BYTE");

	MemoryCache *mc=MC_MemoryCache_ctor_testing (memory_test, PAGE_SIZE);

	process *p=process_init (0, 0, 0, 0);
	
	m->size=PAGE_SIZE;
	rbtree_insert (p->modules, NULL, (void*)m);

    	rbtree_insert (p->threads, (void*)0, (void*)t);

	//Da* d=Da_Da(false, (BYTE*)"\x90", 0); // NOP
	Da* d=Da_Da(Fuzzy_False, (BYTE*)"\x40", 0); // INC EAX
	handle_cc(d, p, t, &ctx, mc, false, false);

	//printf ("m->PC_infos=0x%p\n", m->PC_infos);
	//printf ("cnt=%d\n", rbtree_count(m->PC_infos));
	for (rbtree_node *i=rbtree_minimum(m->PC_infos); i; i=rbtree_succ(i))
	{
		strbuf sb=STRBUF_INIT;
		address a=(address)i->key;
		PC_info *info=i->value;

		construct_common_string(&sb, a, info);
		printf ("[%s]\n", sb.buf);
		strbuf_deinit (&sb);
	};

	m->PC_infos=NULL;

	MC_MemoryCache_dtor (mc, false);
	DFREE (memory_test);
	Da_free (d);
	process_free (p);
};
