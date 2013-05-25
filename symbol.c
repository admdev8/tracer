#include "symbol.h"
#include "dmalloc.h"
#include "tracer.h"

static symbol *create_symbol (symbol_type t, char *n)
{
    symbol *rt=DCALLOC (symbol, 1, "symbol");
    rt->t=t;
    rt->name=DSTRDUP(n, "name");
    return rt;
};

void add_symbol (process *p, module *m, address a, char *name, symbol_type t)
{
    rbtree *symtbl=m->symbols;
    if ((dump_all_symbols_re && (regexec (dump_all_symbols_re, name, 0, NULL, 0)==0)) ||
        (dump_all_symbols_re==NULL && dump_all_symbols))
    {
        dump_PID_if_need(p);
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
    symbol *new_sym=create_symbol(t, name);
    new_sym->next=l->s;
    l->s=new_sym;
};

void PE_add_symbols(process *p, module *m, const char * filename, address base, PE_info *info)
{
    // priority matters

    // add OEP
    add_symbol(p, m, info->OEP, "OEP", SYM_TYPE_OEP);
    // add BASE
    add_symbol(p, m, base, "BASE", SYM_TYPE_BASE);
    
    // add image exports
    for (obj* i=info->exports_ordinals; i; i=cdr(i))
    {
        char tmp[20];
        snprintf(tmp, sizeof(tmp), "ordinal_%d", obj_get_as_tetrabyte(cdr(car(i))));
        add_symbol (p, m, obj_get_as_REG(car(car(i))), tmp, SYM_TYPE_PE_EXPORT_ORDINAL);
    };

    for (obj* i=info->exports; i; i=cdr(i))
        add_symbol (p, m, obj_get_as_REG(car(car(i))), obj_get_as_cstring(cdr(car(i))), SYM_TYPE_PE_EXPORT);
    // add from .MAP file
    // add from .PDB file
    // add from Oracle SYM file
};

