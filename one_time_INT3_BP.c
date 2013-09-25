#include "oassert.h"

#include "module.h"
#include "process.h"
#include "thread.h"
#include "address.h"
#include "memorycache.h"
#include "CONTEXT_utils.h"
#include "strbuf.h"
#include "utils.h"

void set_onetime_INT3_BP(address a, process *p, module *m, char *name, MemoryCache *mc)
{
	bool b;
	byte byte_to_save;

	//L ("%s(a=0x" PRI_ADR_HEX " m->filename=%s name=%s)\n", __func__, a, m->filename, name);
	b=MC_ReadByte (mc, a, &byte_to_save);
	oassert(b && "can't read byte at breakpoint start");
	if (byte_to_save==0xCC)
	{
		//L ("%s() bp is probably already set\n", __func__);
	}
	else
	{
		L ("Setting one-time INT3 breakpoint on %s!%s (0x" PRI_ADR_HEX ")\n", m->filename, name, a);
		b=MC_WriteByte (mc, a, 0xCC);
		oassert(b && "can't write 0xCC byte at breakpoint start");
		rbtree_insert(m->INT3_BP_bytes, (void*)a, (void*)byte_to_save);
	};
};

bool check_for_onetime_INT3_BP(process *p, thread *t, address a, MemoryCache *mc, char *resolved_name, CONTEXT *ctx)
{
	//L ("%s(a=0x" PRI_ADR_HEX ", resolved_name=%s)\n", __func__, a, resolved_name);

	module *m=find_module_for_address (p, a);
	oassert (m);

	if (rbtree_is_key_present(m->INT3_BP_bytes, (void*)a)==false)
		return false;

	CONTEXT_decrement_PC(ctx);

	byte byte_to_restore=(byte)rbtree_lookup (m->INT3_BP_bytes, (void*)a);
	rbtree_delete (m->INT3_BP_bytes, (void*)a);

	bool b=MC_WriteByte (mc, a, byte_to_restore);
	oassert(b && "can't restore byte at INT3 breakpoint");

	// print info about it

	address SP=CONTEXT_get_SP(ctx);
	strbuf sb=STRBUF_INIT;
	REG ret_adr;
	bool got_ret_adr=MC_ReadREG(mc, SP, &ret_adr);
	if (got_ret_adr)
		process_get_sym (p, ret_adr, true, true, &sb);

	dump_PID_if_need(p); dump_TID_if_need(p, t);
	L ("One-time INT3 breakpoint: %s (0x" PRI_ADR_HEX ")", resolved_name, a);
	if (got_ret_adr)
		L (" (called from %s (0x" PRI_ADR_HEX "))", sb.buf, ret_adr);
	L ("\n");
	strbuf_deinit(&sb);

	if (dash_s)
		dump_stack (p, t, ctx, mc);

	return true;
};

