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

void add_module (process *p, address img_base, HANDLE file_hdl)
{
    module *m=DMALLOC(module, 1, "module");
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
    rbtree_foreach(m->symbols, NULL, NULL, (void(*)(void*))obj_free);
    rbtree_deinit (m->symbols);
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

