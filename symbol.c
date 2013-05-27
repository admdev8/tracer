#include "symbol.h"
#include "dmalloc.h"
#include "tracer.h"
#include "utils.h"

static symbol *create_symbol (symbol_type t, char *n)
{
    symbol *rt=DCALLOC (symbol, 1, "symbol");
    rt->t=t;
    rt->name=DSTRDUP(n, "name");
    return rt;
};

void add_symbol (address a, char *name, add_symbol_params *params)
{
    module *m=params->m;
    rbtree *symtbl=m->symbols;

    if ((dump_all_symbols_re && (regexec (dump_all_symbols_re, name, 0, NULL, 0)==0)) ||
        (dump_all_symbols_re==NULL && dump_all_symbols))
    {
        dump_PID_if_need(params->p);
        L("New symbol. Module=[%s], address=[0x" PRI_ADR_HEX "], name=[%s]\n", get_module_name(m), a, name);
    };

    symbols_list *l=(symbols_list*)rbtree_lookup(symtbl, (void*)a);

    if (l==NULL)
    {
        // create new symbols_list
        l=DCALLOC(symbols_list, 1, "symbols_list");
        rbtree_insert(symtbl, (void*)a, (void*)l);
    };

    // insert at beginning of list
    symbol *new_sym=create_symbol(params->t, name);
    new_sym->next=l->s;
    l->s=new_sym;
};

