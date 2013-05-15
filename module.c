#include "module.h"
#include "process.h"
#include "strbuf.h"
#include "dmalloc.h"
#include "porg_utils.h"
#include "PE.h"
#include "symbol.h"
#include "logging.h"
#include "dmalloc.h"

bool module_c_debug=true;

bool try_to_resolve_bp_address_if_need(module *module_just_loaded, bp_address *a)
{
    // scan symbols in loaded module_just_loaded
    return false;
};

void try_to_resolve_bp_addresses_if_need (module *module_just_loaded)
{
    if (addresses_to_be_resolved==NULL)
        return;

    obj* new_addresses_to_be_resolved=NULL;

    printf ("%s() scan addresses_to_be_resolved...\n", __func__);
    for (obj *i=addresses_to_be_resolved; i; i=cdr(i)) // breakpoints is a list
    {
        bp_address *bp_a=(bp_address*)obj_unpack_opaque(car(i));
        if (try_to_resolve_bp_address_if_need(module_just_loaded, bp_a)==false)
            new_addresses_to_be_resolved=NCONC(new_addresses_to_be_resolved, cons(car(i), NULL));
    };
    obj_free_conses_of_list(addresses_to_be_resolved);

    addresses_to_be_resolved=new_addresses_to_be_resolved;
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

    try_to_resolve_bp_addresses_if_need(m);

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

    for (struct rbtree_node_t *i=rbtree_minimum(m->symbols); i!=NULL; i=rbtree_succ(i))
    {
        //L ("(to be freed) address: 0x%x, ", (address)i->key);
        symbols_list *l=(symbols_list*)i->value;
        //obj_dump(l->symbols);
        obj_free(l->symbols);
        //L ("\n");
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

