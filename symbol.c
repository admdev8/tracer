#include "symbol.h"
#include "dmalloc.h"

static symbol *create_symbol (symbol_type t, char *n)
{
    symbol *rt=DCALLOC (symbol, 1, "symbol");
    rt->t=t;
    rt->name=DSTRDUP(n, "name");
    return rt;
};

static void add_symbol (rbtree *symtbl, address a, char *name, symbol_type t)
{
    // TODO: chk --allsymbols, --allsymbols:
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

void add_symbols(rbtree *symtbl, const char * filename, address base, PE_info *info)
{
    obj *i;
 
    // priority matters

    // add OEP
    add_symbol(symtbl, info->OEP, "OEP", SYM_TYPE_OEP);
    // add BASE
    add_symbol(symtbl, base, "BASE", SYM_TYPE_BASE);
    
    // add image exports
    for (i=info->exports_ordinals; i; i=cdr(i))
    {
        char tmp[20];
        snprintf(tmp, sizeof(tmp), "ordinal_%d", obj_get_as_tetrabyte(cdr(car(i))));
        add_symbol (symtbl, obj_get_as_REG(car(car(i))), tmp, SYM_TYPE_PE_EXPORT_ORDINAL);
    };

    for (i=info->exports; i; i=cdr(i))
        add_symbol (symtbl, obj_get_as_REG(car(car(i))), obj_get_as_cstring(cdr(car(i))), SYM_TYPE_PE_EXPORT);
    // add from .MAP file
    // add from .PDB file
    // add from Oracle SYM file
};

