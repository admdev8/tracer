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

#include "symbol.h"
#include "dmalloc.h"
#include "tracer.h"
#include "utils.h"
#include "one_time_INT3_BP.h"
#include "oassert.h"
#include "ostrings.h"
#include "opts_aux.h"
#include "memorycache.h"
#include "fmt_utils.h"

static symbol *create_symbol (symbol_type t, char *n)
{
    symbol *rt=DCALLOC (symbol, 1, "symbol");
    rt->t=t;
    rt->skip_on_tracing=Fuzzy_Undefined;
    rt->name=DSTRDUP(n, "name");
    return rt;
};

void add_symbol (address a, char *name, add_symbol_params *params)
{
    module *m=params->m;
    rbtree *symtbl=m->symbols;
    oassert(symtbl && "symbols=NULL in module");
    MemoryCache *mc=params->mc;

    if (one_time_int3_bp_re && params->t==SYM_TYPE_PE_EXPORT && module_adr_in_executable_section (m, a))
    {
        strbuf sb=STRBUF_INIT;
        strbuf_addstr (&sb, get_module_name(m));
        strbuf_addc (&sb, '!');
        strbuf_addstr (&sb, name);

        if (regexec (one_time_int3_bp_re, sb.buf, 0, NULL, 0)==0)
            set_onetime_INT3_BP(a, params->p, m, name, mc);

        strbuf_deinit (&sb);
    };

    if (dump_seh && string_is_ends_with (name, "security_cookie"))
    {
        m->security_cookie_adr=a;
        m->security_cookie_adr_known=true;
        if (symbol_c_debug)
            L ("%s() got address of security_cookie (0x" PRI_REG_HEX ") for %s!%s\n", __FUNCTION__, a, get_module_name(m), name);
    };

    bool dump_symbol=false;
    if (dump_all_symbols_re)
    {
        strbuf sb=STRBUF_INIT;
        strbuf_addstr (&sb, get_module_name(m));
        strbuf_addc (&sb, '!');
        strbuf_addstr (&sb, name);

        if (regexec (dump_all_symbols_re, sb.buf, 0, NULL, 0)==0)
            dump_symbol=true;

        strbuf_deinit (&sb);
    };

    if (dump_symbol || (dump_all_symbols_re==NULL && dump_all_symbols))
    {
        dump_PID_if_need(params->p);
        L("New symbol. Module=[%s], address=[0x" PRI_ADR_HEX "], name=[%s]\n", get_module_name(m), a, name);
    };

    symbol *new_sym=create_symbol(params->t, name);
    symbol *first_sym=(symbol*)rbtree_lookup(symtbl, (void*)a);

    if (first_sym)
        new_sym->next=first_sym; // insert at beginning of list

    rbtree_insert(symtbl, (void*)a, (void*)new_sym);
};

bool symbol_skip_on_tracing(module *m, symbol *s)
{
    if (m->skip_all_symbols_in_module_on_trace)
        return true;

    if (s->skip_on_tracing==Fuzzy_False)
        return false;
    if (s->skip_on_tracing==Fuzzy_True)
        return true;

    // run regexp, etc
    int j;
    trace_skip_element * i;
    for (i=trace_skip_options, j=0; i; i=i->next, j++)
    {
        if (regexec (&i->re_path, m->path, 0, NULL, 0)==0)
            if (regexec (&i->re_module, get_module_name(m), 0, NULL, 0)==0)
                if (regexec (&i->re_function, s->name, 0, NULL, 0)==0)
                {
                    L ("Symbol %s!%s!%s will be skipped during tracing\n", m->path, get_module_name(m), s->name);
                    s->skip_on_tracing=Fuzzy_True;
                    return true;
                };
    };

    s->skip_on_tracing=Fuzzy_False;
    return false;
};

/* vim: set expandtab ts=4 sw=4 : */
