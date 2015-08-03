#include "ostrings.h"
#include "oassert.h"
#include "module.h"
#include "memorycache.h"
#include "process.h"
#include "thread.h"
#include "bolt_stuff.h"
#include "fmt_utils.h"

struct my_EXCEPTION_REGISTRATION
{
	address prev;
	address handler;
};

struct my_VC_EXCEPTION_REGISTRATION_RECORD
{
	address prev;
	address handler;
	address scopetable;
	REG previous_trylevel; // -1 for SEH3, -2 for SEH4
	address EBP;
};

struct my_SCOPETABLE_ENTRY
{
	REG previousTryLevel; // AKA enclosing level
	address filter;   // if NULL here, 3rd pointer is Finally function
	address handler;  // Handler/Finally function
};

struct my_EH4_SCOPETABLE_HEADER
{
	address GSCookieOffset;
	address GSCookieXOROffset;
	address EHCookieOffset;
	address EHCookieXOROffset;
};

void dump_scopetable_entry (MemoryCache *mc, process *p, int i, address a)
{
	bool b;
	struct my_SCOPETABLE_ENTRY e;

	b=MC_ReadBuffer (mc, a, sizeof(struct my_SCOPETABLE_ENTRY), (BYTE*)&e);
	if (b==false)
	{
		L ("%s() cannot read scopetable entry\n", __FUNCTION__);
		return;
	};
	strbuf sb2=STRBUF_INIT;
	process_get_sym (p, e.handler, true /* add_module_name */, true /* add_offset */, &sb2);
	if (e.filter)
	{
		strbuf sb1=STRBUF_INIT;
		process_get_sym (p, e.filter, true /* add_module_name */, true /* add_offset */, &sb1);
		L ("scopetable entry[%d]. previous try level=%d, filter=0x" PRI_ADR_HEX 
				" (%s) handler=0x" PRI_ADR_HEX " (%s)\n", i, e.previousTryLevel, 
				e.filter, sb1.buf,
				e.handler, sb2.buf);
		strbuf_deinit(&sb1);
	}
	else
		L ("scopetable entry[%d]. previous try level=%d, finally=0x" PRI_ADR_HEX " (%s)\n", 
				i, e.previousTryLevel, 
				e.handler, sb2.buf);
	strbuf_deinit(&sb2);
};

void dump_scopetable(MemoryCache *mc, process *p, address a, size_t total)
{
	for (size_t i=0; i<total; i++)
	{
		dump_scopetable_entry (mc, p, i, a);
		a+=sizeof(struct my_SCOPETABLE_ENTRY);
	};
};

void check_SEH4_cookie(MemoryCache *mc, address adr_of_EBP, 
		address CookieOffset, address CookieXOROffset, REG security_cookie, const char *name)
{
	REG value_in_stack=0;
	// CookieOffset is a distance between stack frame begin and ebp^security_cookie
	address adr_in_stack=adr_of_EBP + CookieOffset;
	if (MC_ReadREG(mc, adr_in_stack, &value_in_stack)==false)
	{
		L ("%s() cannot read at %sCookieOffset+EBP (0x" PRI_ADR_HEX ")\n", __FUNCTION__, name, adr_in_stack);
		return;
	};
	// CookieXOROffset is additional difference between ebp^security_cookie value and what is
	// stored in the stack
	REG shifted_EBP=CookieXOROffset + adr_of_EBP;

	if ((security_cookie^shifted_EBP) != value_in_stack)
	{
		L ("%s security cookie is not correct\n", name);
		L ("%s() adr_in_stack=0x" PRI_ADR_HEX " value_in_stack=0x" PRI_REG_HEX " adr_of_EBP=0x" PRI_REG_HEX " shifted_EBP=0x" PRI_REG_HEX "\n",
				__FUNCTION__, adr_in_stack, value_in_stack, adr_of_EBP, shifted_EBP);
		L ("%s() security_cookie=0x" PRI_REG_HEX "\n", __FUNCTION__, security_cookie);
	};
};

// returns address of next SEH frame
address dump_SEH_frame (fds* s, process* p, thread* t, MemoryCache *mc, address a)
{
	struct my_EXCEPTION_REGISTRATION current_SEH_frame;
	bool b;

	b=MC_ReadBuffer (mc, a, 
			sizeof(struct my_EXCEPTION_REGISTRATION), (BYTE*)&current_SEH_frame);
	if (b==false)
	{
		L ("%s() cannot read current SEH frame\n", __FUNCTION__);
		return REG_MAX;
	};

	strbuf sb=STRBUF_INIT;
	process_get_sym (p, current_SEH_frame.handler, true /* add_module_name */, true /* add_offset */, &sb);

	L ("* SEH frame at 0x" PRI_ADR_HEX " prev=0x" PRI_ADR_HEX " handler=0x" PRI_ADR_HEX " (%s)\n", 
			a, current_SEH_frame.prev, current_SEH_frame.handler, sb.buf);

	bool SEH3=false, SEH4=false;

	if (string_is_ends_with (sb.buf, "except_handler3"))
		SEH3=true;

	REG security_cookie;
	bool security_cookie_known=false;
	if (string_is_ends_with (sb.buf, "except_handler4"))
	{
		SEH4=true;
		module *m=find_module_for_address(p, current_SEH_frame.handler);
		if (m->security_cookie_adr_known)
		{
			b=MC_ReadREG (mc, m->security_cookie_adr, &security_cookie);
			if (b==false)
				L ("%s() can't read security_cookie at 0x" PRI_ADR_HEX " for %s\n", 
						__FUNCTION__, m->security_cookie_adr, get_module_name (m));
			else
				security_cookie_known=true;
		}
		else
		{
			L ("SEH4 frame is here, but address of security_cookie is not known\n");
			L ("Try to place .PDB or .MAP file here with this symbol in it\n");
		};
	};

	if (SEH3==false && SEH4==false)
		goto exit;

	struct my_VC_EXCEPTION_REGISTRATION_RECORD current_SEH3_frame;
	b=MC_ReadBuffer (mc, a,
			sizeof(struct my_VC_EXCEPTION_REGISTRATION_RECORD), (BYTE*)&current_SEH3_frame);
	if (b==false)
	{
		L ("%s() cannot read current SEH3/4 frame\n", __FUNCTION__);
		return REG_MAX;
	};

	int previous_trylevel=current_SEH3_frame.previous_trylevel;
	L ("SEH%d frame. previous trylevel=%d\n", SEH3 ? 3 : 4, previous_trylevel);
	address scopetable_address=current_SEH3_frame.scopetable;
	if (SEH4 && security_cookie_known)
	{
		scopetable_address^=security_cookie;
		struct my_EH4_SCOPETABLE_HEADER EH4_header;
		b=MC_ReadBuffer (mc, scopetable_address,
				sizeof(struct my_EH4_SCOPETABLE_HEADER), (BYTE*)&EH4_header);
		if (b==false)
		{
			L ("%s() cannot read current SEH4 frame header. scopetable_address=0x" PRI_ADR_HEX "\n", 
					__FUNCTION__, scopetable_address);
			return REG_MAX;
		};
		L ("SEH4 header:\tGSCookieOffset=0x%x GSCookieXOROffset=0x%x\n", EH4_header.GSCookieOffset, EH4_header.GSCookieXOROffset);
		L ("\t\tEHCookieOffset=0x%x EHCookieXOROffset=0x%x\n", EH4_header.EHCookieOffset, EH4_header.EHCookieXOROffset);

		unsigned adr_of_EBP=a + ((byte*)&current_SEH3_frame.EBP - (byte*)&current_SEH3_frame.prev);

		if (EH4_header.EHCookieOffset!=-2)
			check_SEH4_cookie (mc, adr_of_EBP, EH4_header.EHCookieOffset, EH4_header.EHCookieXOROffset, security_cookie, "EH");
		if (EH4_header.GSCookieOffset!=-2)
			check_SEH4_cookie (mc, adr_of_EBP, EH4_header.GSCookieOffset, EH4_header.GSCookieXOROffset, security_cookie, "GS");

		scopetable_address+=sizeof(struct my_EH4_SCOPETABLE_HEADER);
	};
	if (previous_trylevel>=0 && (SEH3 || (SEH4 && security_cookie_known)))
		dump_scopetable(mc, p, scopetable_address, previous_trylevel+1);

exit:
	strbuf_deinit (&sb);
	return current_SEH_frame.prev;
};

void dump_SEH_chain (fds* s, process *p, thread *t, CONTEXT *ctx, MemoryCache *mc)
{
	address current_SEH_frame_address=TIB_get_current_SEH_frame(mc, t->THDL);
	
	do
		current_SEH_frame_address=dump_SEH_frame (s, p, t, mc, current_SEH_frame_address);
	while (current_SEH_frame_address!=REG_MAX);
};
