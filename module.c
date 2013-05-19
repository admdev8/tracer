#include <assert.h>

#include "module.h"
#include "process.h"
#include "strbuf.h"
#include "dmalloc.h"
#include "porg_utils.h"
#include "PE.h"
#include "symbol.h"
#include "logging.h"
#include "stuff.h"
#include "dmalloc.h"
#include "tracer.h"

bool module_c_debug=true;

static address module_translate_adr_to_abs_address(module *m, address original_adr)
{
    return m->base + (original_adr - m->original_base);
};

static bool search_for_symbol_re_in_module(module *m, regex_t *symbol_re, address *out)
{
    // enumerate all symbols in module in order for searching

    for (struct rbtree_node_t* i=rbtree_minimum(m->symbols); i; i=rbtree_succ(i))
        for (symbol* s=((symbols_list*)i->value)->s; s; s=s->next)
            if (regexec(symbol_re, s->name, 0, NULL, 0)==0)
                return *out=(address)i->key, true;

    return false;
};

static bool try_to_resolve_bytemask(bp_address *a)
{
    assert(!"not implemented");
};

static bool try_to_resolve_bp_address_if_need(module *module_just_loaded, bp_address *a)
{
    printf ("%s() a=", __func__);
    dump_address (a);
    printf ("\n");
    assert (a->resolved==false);

    if (a->t==OPTS_ADR_TYPE_BYTEMASK)
        return try_to_resolve_bytemask(a);

    if (a->t==OPTS_ADR_TYPE_FILENAME_SYMBOL || a->t==OPTS_ADR_TYPE_FILENAME_ADR)
    {
        // is $a$ related to module_just_loaded?
        if (stricmp(module_just_loaded->internal_name, a->filename)!=0 &&
                stricmp (module_just_loaded->filename, a->filename)!=0)
            return false;

        if (a->t==OPTS_ADR_TYPE_FILENAME_ADR)
        {
            a->abs_address=module_translate_adr_to_abs_address(module_just_loaded, a->adr);
            L ("Symbol %s resolved to 0x" PRI_ADR_HEX "\n", a->symbol, a->abs_address);
            return a->resolved=true;
        };

        if (a->t==OPTS_ADR_TYPE_FILENAME_SYMBOL)
        {
            address found;
            if (search_for_symbol_re_in_module(module_just_loaded, &a->symbol_re, &found))
            {
                a->abs_address=found + a->ofs;
                L ("Symbol %s resolved to 0x" PRI_ADR_HEX "\n", a->symbol, a->abs_address);
                return a->resolved=true;
            }
            else
                die ("Error: Module %s was just loaded, but symbol %s was not found in it!\n", 
                        module_just_loaded->internal_name, a->symbol);
        };
    };

    assert(!"unknown bp_address type");
    return false; // TMCH
};

static bool try_to_resolve_bp_addresses_if_need (module *module_just_loaded)
{
    bool rt=false;

    if (module_c_debug)
        printf ("%s() scan addresses_to_be_resolved...\n", __func__);

    for (dlist *i=addresses_to_be_resolved; i;) // breakpoints is a list
    {
        if (try_to_resolve_bp_address_if_need(module_just_loaded, (bp_address*)i->data))
        {
            dlist *tmp=i->next;
            dlist_unlink(&addresses_to_be_resolved, i);
            i=tmp;
            rt=true;
        }
        else
            i=i->next;
    };
    return rt;
};

void add_module (process *p, address img_base, HANDLE file_hdl)
{
    module *m=DCALLOC(module, 1, "module");
    strbuf fullpath_filename=STRBUF_INIT;
    PE_info info;

    if (module_c_debug)
        L ("%s() begin\n", __func__);

    if (GetFileNameFromHandle(file_hdl, &fullpath_filename))
    {
        strbuf sb_filename=STRBUF_INIT, sb_path=STRBUF_INIT;

        if (0 && module_c_debug)
            L ("fullpath_filename=%s\n", fullpath_filename.buf);

        full_path_and_filename_to_path_only (&sb_path, fullpath_filename.buf);
        full_path_and_filename_to_filename_only (&sb_filename, fullpath_filename.buf);
        
        if (0 && module_c_debug)
        {
            L ("sb_path=%s\n", sb_path.buf);
            L ("sb_filename=%s\n", sb_filename.buf);
        };
        
        m->filename=strbuf_detach(&sb_filename, NULL);
        m->path=strbuf_detach(&sb_path, NULL);
        strbuf_deinit (&sb_filename);
        strbuf_deinit (&sb_path);
        if (0 && module_c_debug)
        {
            L ("m->filename=%s\n", m->filename);
            L ("m->path=%s\n", m->path);
        };
    }
    else
    {
        m->filename=DSTRDUP("?", "");
        m->path=DSTRDUP("?", "");
    };
    
    PE_get_info (fullpath_filename.buf, img_base, &info);
    
    m->base=img_base;
    m->original_base=info.original_base;
    m->size=info.size;
    if (info.internal_name)
        m->internal_name=DSTRDUP(info.internal_name, "internal_name");
    m->symbols=rbtree_create(true, "symbols", compare_size_t);
    add_symbols(m->symbols, fullpath_filename.buf, img_base, &info);

    strbuf_deinit(&fullpath_filename);
    
    rbtree_insert (p->modules, (void*)img_base, (void*)m);

    if (try_to_resolve_bp_addresses_if_need(m))
        set_all_breakpoints(p);

    PE_info_free(&info);

    if (module_c_debug)
        L ("%s() end\n", __func__);
};

void module_free(module *m)
{
    if (module_c_debug)
    {
        L ("%s() begin\n", __func__);
        L ("m->filename=%s\n", m->filename);
        L ("m->symbols count=%d\n", rbtree_count (m->symbols));
    };
    DFREE(m->filename);
    DFREE(m->path);
    DFREE(m->internal_name);

    for (struct rbtree_node_t *i=rbtree_minimum(m->symbols); i; i=rbtree_succ(i))
    {
        symbols_list *l=(symbols_list*)i->value;
        for (symbol* s=l->s; s; )
        {
            symbol *tmp=s;
            s=s->next;
            DFREE(tmp->name);
            DFREE(tmp);
        };

        DFREE(l);
    };

    rbtree_deinit(m->symbols);
    DFREE(m);
};

void remove_module (process *p, address img_base)
{
    if (module_c_debug)
        L ("%s() begin\n", __func__);
    module *m=rbtree_lookup(p->modules, (void*)img_base);

    if (m==NULL)
    {
        L ("no module registered with image base 0x%x\n", img_base);
        return;
    };

    module_free(m);
    rbtree_delete(p->modules, (void*)img_base);
};

bool address_in_module (module *m, address a)
{
    return (a >= m->base) && (a < m->base + m->size);
};

#if 0
// may return module.dll!symbol or NULL
char* sym_exist_at (module *m, address a)
{
};
#endif

// may return module.dll!symbol+0x1234
void module_get_sym (module *m, address a, strbuf *out)
{
    assert (address_in_module (m, a));

    address prev_k;
    symbols_list *prev_v;
    
    symbols_list *l=rbtree_lookup2(m->symbols, (void*)a, (void**)&prev_k, (void**)&prev_v, NULL, NULL);
    if (l)
    {
        // take 'top' symbol
        symbol *s=l->s;
        strbuf_addf (out, "%s!%s", get_module_name (m), s->name);
    }
    else
    {
        symbol *s=prev_v->s;
        strbuf_addf (out, "%s!%s+0x%x", get_module_name (m), s->name, a-prev_k);
    };
};

char *get_module_name (module *m)
{
    if (m->internal_name)
        return m->internal_name;
    else
        return m->filename;
};

