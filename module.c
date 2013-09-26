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

#include "oassert.h"
#include <windows.h>
#include <dbghelp.h>

#include "oracle_sym.h"
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
#include "files.h"
#include "tracer.h"
#include "bp_address.h"
#include "utils.h"
#include "cc.h"
#include "bitfields.h"
#include "cycle.h"

static bool is_it_code_section (IMAGE_SECTION_HEADER *s)
{
    return IS_SET (s->Characteristics, IMAGE_SCN_CNT_CODE) ||
        IS_SET (s->Characteristics, IMAGE_SCN_MEM_EXECUTE);
};

static address module_translate_adr_to_abs_address(module *m, address original_adr)
{
    return m->base + (original_adr - m->original_base);
};

static bool search_for_symbol_re_in_module(module *m, regex_t *symbol_re, address *out)
{
    // enumerate all symbols in module in order for searching

    for (struct rbtree_node_t* i=rbtree_minimum(m->symbols); i; i=rbtree_succ(i))
        for (symbol* s=(symbol*)i->value; s; s=s->next)
            if (regexec(symbol_re, s->name, 0, NULL, 0)==0)
                return *out=(address)i->key, true;

    return false;
};
               
#ifdef THIS_CODE_IS_NOT_WORKING
static bool search_bytemask_in_PE_section(LOADED_IMAGE *im, module *m, IMAGE_SECTION_HEADER *sect, wyde bytemask, 
        unsigned bytemask_len, address *out)
{
        address begin=sect->VirtualAddress;
        oassert(sect->Misc.VirtualSize);
        SIZE_T=sect->Misc.VirtualSize;
};

static bool try_to_resolve_bytemask(module *m, bp_address *a)
{
    LOADED_IMAGE im;
    bool rt=false;

    MapAndLoad_or_die (m->filename, m->path, &im, true, true);
    for (unsigned i=0; i<m->sections_total; i++)
    {
        IMAGE_SECTION_HEADER *sect=m->sections[i];

        if (is_it_code_section(sect) && 
                search_bytemask_in_PE_section(&im, m, sect, a->bytemask, a->bytemask_len, &a->abs_address))
        {
            rt=a->resolved=true;
            goto exit;
        };
    };
exit:
    UnMapAndLoad_or_die(&im);
    return rt;
};
#endif

static bool try_to_resolve_bp_address_if_need(module *module_just_loaded, bp_address *a)
{
    if (module_c_debug)
    {
        printf ("%s() a=", __func__);
        dump_address (a);
        printf ("\n");
    };

    oassert (a->resolved==false);

    if (a->t==OPTS_ADR_TYPE_BYTEMASK)
    {
        die ("bytemasks are not implemented yet\n");
        //return try_to_resolve_bytemask(module_just_loaded, a);
    };

    if (a->t==OPTS_ADR_TYPE_FILENAME_SYMBOL || a->t==OPTS_ADR_TYPE_FILENAME_ADR)
    {
        // is $a$ related to module_just_loaded?
        if (stricmp(get_module_name(module_just_loaded), a->filename)!=0)
            return false;

        if (a->t==OPTS_ADR_TYPE_FILENAME_ADR)
        {
            a->abs_address=module_translate_adr_to_abs_address(module_just_loaded, a->adr);

            if (module_c_debug)
            {
                strbuf tmp=STRBUF_INIT;
                address_to_string (a, &tmp);
                L ("Symbol %s resolved to 0x" PRI_ADR_HEX "\n", tmp.buf, a->abs_address);
                strbuf_deinit(&tmp);
            };
#if 0
            // useful only for BPF, BPX?
            if (adr_in_executable_section(p, a->abs_address)==false)
                L ("Warning: address 0x " PRI_ADR_HEX " not in executable section.\n", a->abs_address);
#endif
            return a->resolved=true;
        };

        if (a->t==OPTS_ADR_TYPE_FILENAME_SYMBOL)
        {
            address found;
            if (search_for_symbol_re_in_module(module_just_loaded, &a->symbol_re, &found))
            {
                a->abs_address=found + a->ofs;

                if (module_c_debug)
                {
                    strbuf tmp=STRBUF_INIT;
                    address_to_string (a, &tmp);
                    L ("Symbol %s resolved to 0x" PRI_ADR_HEX "\n", tmp.buf, a->abs_address);
                    strbuf_deinit(&tmp);
                };

                return a->resolved=true;
            }
            else
            {
                L ("Error: Module %s was just loaded, but symbol %s was not found in it!\n", 
                        get_module_name(module_just_loaded), a->symbol);
                detach();
                return false; // FIXME: crashing after this
            };
        };
    };

    oassert(!"unknown bp_address type");
    fatal_error();
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

static void set_filename_and_path_for_module (HANDLE file_hdl, module *m, strbuf *fullpath_filename)
{
    if (GetFileNameFromHandle(file_hdl, fullpath_filename))
    {
        strbuf sb_filename=STRBUF_INIT, sb_filename_without_ext=STRBUF_INIT, sb_path=STRBUF_INIT;

        if (0 && module_c_debug)
            L ("fullpath_filename=%s\n", fullpath_filename->buf);

        full_path_and_filename_to_path_only (&sb_path, fullpath_filename->buf);
        full_path_and_filename_to_filename_only (&sb_filename, &sb_filename_without_ext, fullpath_filename->buf);

        if (0 && module_c_debug)
        {
            L ("sb_path=%s\n", sb_path.buf);
            L ("sb_filename=%s\n", sb_filename.buf);
            L ("sb_filename_without_ext=%s\n", sb_filename_without_ext.buf);
        };

        m->filename=strbuf_detach(&sb_filename, NULL);
        m->filename_without_ext=strbuf_detach(&sb_filename_without_ext, NULL);
        m->path=strbuf_detach(&sb_path, NULL);

        strbuf_deinit (&sb_filename);
        strbuf_deinit (&sb_filename_without_ext);
        strbuf_deinit (&sb_path);
        if (0 && module_c_debug)
        {
            L ("m->filename=%s\n", m->filename);
            L ("m->filename_without_ext=%s\n", m->filename_without_ext);
            L ("m->path=%s\n", m->path);
        };
    }
    else
    {
        m->filename=DSTRDUP("?", "");
        m->filename_without_ext=DSTRDUP("?", "");
        m->path=DSTRDUP("?", "");
    };
};

static PE_info* get_all_info_from_PE(process *p, module *m, strbuf *fullpath_filename, address img_base, 
        MemoryCache *mc)
{
    PE_info *info=DCALLOC(PE_info, 1, "PE_info");

    add_symbol_params params={p, m, SYM_TYPE_PE_EXPORT, mc};

    PE_get_sections_info (fullpath_filename->buf, &m->sections, &m->sections_total);
    m->base=img_base;

    // PE_get_info() will call add_symbol(), and the latter may use m->base, m->sections, 
    // m->sections_total, so these variables in $m$ should be already present!
    // that's weird, I know.

    PE_get_info (fullpath_filename->buf, img_base, info, (void (*)(address,  char *, void *))add_symbol, 
            (void*)&params);
    m->original_base=info->original_base;
    m->OEP=info->OEP;
    m->size=info->size;
    m->PE_timestamp=info->timestamp;
    if (info->internal_name)
        m->internal_name=DSTRDUP(info->internal_name, "internal_name");

    //PE_add_symbols(p, m, fullpath_filename->buf, img_base, info);

    // add OEP
    params.t=SYM_TYPE_OEP;
    add_symbol(info->OEP, "OEP", &params);
    // add BASE
    params.t=SYM_TYPE_BASE;
    add_symbol(img_base, "BASE", &params);
    return info;
};

static void add_symbols_from_MAP_if_exist (process *p, module *m, address img_base, PE_info *info, 
        const char* short_PE_name, MemoryCache *mc)
{
#ifdef _WIN64
    const char *MAP_get_all_PAT="^ ([0-9A-F]{8}):([0-9A-F]{16})       (.*)$";
#else
    const char *MAP_get_all_PAT="^ ([0-9A-F]{4}):([0-9A-F]{8})       (.*)$";
#endif // _WIN64
    regex_t PAT_compiled;
    regcomp_or_die(&PAT_compiled, MAP_get_all_PAT, REG_EXTENDED | REG_ICASE | REG_NEWLINE);

    add_symbol_params params={ p, m, SYM_TYPE_MAP, mc };
    
    strbuf sb_mapfilename=STRBUF_INIT;
    strbuf_addf (&sb_mapfilename, "%s.map", m->filename_without_ext);
    if (file_exist(sb_mapfilename.buf)==false)
        goto exit;

    char buf[1024];
    unsigned loaded=0;

    FILE *f=fopen (sb_mapfilename.buf, "rb");
    if (f==NULL)
    {
        L ("Unable to open file [%s]\n", sb_mapfilename.buf);
        goto exit;
    };

    while (fgets(buf, sizeof(buf), f))
    {
        str_trim_all_lf_cr_right(buf);

        DWORD sect=0;
        address addr=0;
        regmatch_t matches[4];

        if (regexec(&PAT_compiled, buf, 4, matches, 0)==0)
        {
            char *v1=strdup_range (buf, matches[1].rm_so, matches[1].rm_eo-matches[1].rm_so);
            char *v2=strdup_range (buf, matches[2].rm_so, matches[2].rm_eo-matches[2].rm_so);
            char *v3=strdup_range (buf, matches[3].rm_so, matches[3].rm_eo-matches[3].rm_so);
            unsigned t_i;


            t_i=sscanf (v1, "%X", &sect); // DWORD
            oassert (t_i==1);
            sect--;
            t_i=sscanf (v2, "%X", &addr); // REG
            oassert (t_i==1);
            
            //L ("v1=[%s] v2=[%s] v3=[%s]\n", v1, v2, v3);

            if (sect < m->sections_total)
            {
                add_symbol (m->sections[sect].VirtualAddress + img_base + addr, v3, &params);
                loaded++;
            }
            else
                L ("%s: Symbol %s skipped because section %d is not present in %s\n", 
                        sb_mapfilename.buf, v3, sect+1, get_module_name(m));

            DFREE (v1); DFREE (v2); DFREE (v3);
        };
    };

    fclose (f);
    
    if (loaded>0)
        L ("%d symbols loaded from %s\n", loaded, sb_mapfilename.buf);

exit:
    regfree (&PAT_compiled);   
    strbuf_deinit(&sb_mapfilename);
};

typedef struct _PDB_load_callback_info
{
    address cur_module_base;
    unsigned MyEnumSymbolsCallback_total;
    add_symbol_params *params;
} PDB_load_callback_info;

static BOOL CALLBACK MyEnumSymbolsCallback(SYMBOL_INFO* pSymInfo, ULONG SymbolSize, PVOID UserContext) 
{
    PDB_load_callback_info* info=(PDB_load_callback_info*)UserContext;
   
    if (pSymInfo)
    {
        address adr=(address)(info->cur_module_base + (pSymInfo->Address - 0x10000000)); // hack
        add_symbol (adr, pSymInfo->Name, info->params);
        info->MyEnumSymbolsCallback_total++;
    };

    return TRUE; // Continue enumeration 
};

static void add_symbols_from_PDB_if_exist (process *p, module *m, address img_base, PE_info *info, 
        const char* short_PE_name, MemoryCache *mc)
{ 
    add_symbol_params params={ p, m, SYM_TYPE_PDB, mc };
    
    strbuf sb_pdbfilename=STRBUF_INIT;
    strbuf_addf (&sb_pdbfilename, "%s.pdb", m->filename_without_ext);
    if (file_exist(sb_pdbfilename.buf)==false)
        goto exit;
    
    BOOL b = SymInitialize (GetCurrentProcess(), NULL, FALSE); 

    if (b == FALSE)
    {
        L ("%s(): SymInitialize() failed!\n", __func__);
        goto exit;
    };

    DWORD64 ModBase=SymLoadModuleEx (GetCurrentProcess(), NULL, sb_pdbfilename.buf, NULL, 0x10000000, 
            get_file_size (sb_pdbfilename.buf), NULL, 0); // hack. hack? what hack?

    if (ModBase==0) 
    {
        DWORD err=GetLastError();

        if (err==ERROR_SUCCESS)
            L ("%s() Error: SymLoadModule64() failed. error code==ERROR_SUCCESS. module is already loaded?\n", __func__);
        else
            L ("%s() Error: SymLoadModule64() failed. Error code: 0x%x \n", __func__, err);
    }
    else
    {
        PDB_load_callback_info info2;
        info2.cur_module_base=img_base;
        info2.MyEnumSymbolsCallback_total=0;
        info2.params=&params;

        b=SymEnumSymbols(GetCurrentProcess(), ModBase, NULL, MyEnumSymbolsCallback, (PVOID)&info2); 
        if (b==FALSE)
            L("%s() Error: SymEnumSymbols() failed. Error code: 0x%x\n", __func__, GetLastError()); 

        b=SymUnloadModule64(GetCurrentProcess(), ModBase); 
        if (b==FALSE)
            L ("%s() Error: SymUnloadModule64() failed. Error code: 0x%x\n", __func__, GetLastError() ); 

        L ("%d symbols loaded from %s\n", info2.MyEnumSymbolsCallback_total, sb_pdbfilename.buf);
    };

    b=SymCleanup(GetCurrentProcess()); 
    if (b==FALSE)
        L ("%s() SymCleanup() failed!\n", __func__);

exit:
    strbuf_deinit(&sb_pdbfilename);
};

static void add_symbols_from_ORACLE_SYM_if_exist (process *p, module *m, address img_base, PE_info *info, 
        const char* short_PE_name, MemoryCache *mc)
{
    strbuf sb=STRBUF_INIT;
    add_symbol_params params={ p, m, SYM_TYPE_ORACLE_SYM, mc };

    strbuf_addf (&sb, "%sRDBMS\\ADMIN\\%s.sym", ORACLE_HOME.buf, m->filename_without_ext);

    if (file_exist(sb.buf)==false)
        goto exit;

    int err=get_symbols_from_ORACLE_SYM (sb.buf, img_base, info->size, info->timestamp, true, 
            (void (*)(address,  char *, void *))add_symbol, (void*)&params, oracle_version);

    if (err==ORACLE_SYM_IMPORTER_ERROR_FILE_OPENING_ERROR)
        die ("Can't open %s\n", sb.buf);
    if (err==ORACLE_SYM_IMPORTER_ERROR_SIGNATURE_MISMATCH)
        L ("%s: signature mismatch\n", sb.buf);
    if (err==ORACLE_SYM_IMPORTER_ERROR_PE_FILE_MISMATCH)
        L ("%s is not related to %s PE file!\n", sb.buf, short_PE_name);

exit:
    strbuf_deinit(&sb);
};

module* add_module (process *p, address img_base, HANDLE file_hdl, MemoryCache *mc)
{
    module *m=DCALLOC(module, 1, "module");
    strbuf fullpath_filename=STRBUF_INIT;

    if (module_c_debug)
        L ("%s() begin\n", __func__);

    m->parent_process=p;
    m->INT3_BP_bytes=rbtree_create(true, "INT3_BP_bytes", compare_size_t);

    m->symbols=rbtree_create(true, "symbols", compare_size_t);

    set_filename_and_path_for_module(file_hdl, m, &fullpath_filename);
        
    if (opt_loading)
        L ("New module: %s%s, base=0x" PRI_ADR_HEX "\n", m->path, m->filename, img_base);
    
    PE_info* info=get_all_info_from_PE (p, m, &fullpath_filename, img_base, mc);
    if (ORACLE_HOME.strlen>0)
        add_symbols_from_ORACLE_SYM_if_exist (p, m, img_base, info, get_module_name(m), mc);

    // will symbols from this module skipping during tracing?
    int j;
    trace_skip_element * i;
    for (i=trace_skip_options, j=0; i; i=i->next,j++)
    {
        if (regexec (&i->re_path, m->path, 0, NULL, 0)==0)
            if (regexec (&i->re_module, get_module_name(m), 0, NULL, 0)==0)
                if (i->is_function_wildcard)
                {
                    L ("All symbols in module %s will be skipped during tracing\n", get_module_name(m));
                    m->skip_all_symbols_in_module_on_trace=true;
                    break;
                };
    };

    // this function uses PE sections info!
    add_symbols_from_MAP_if_exist (p, m, img_base, info, get_module_name(m), mc);
    
    add_symbols_from_PDB_if_exist (p, m, img_base, info, get_module_name(m), mc);

    PE_info_free(info);
    strbuf_deinit(&fullpath_filename);

    rbtree_insert (p->modules, (void*)img_base, (void*)m);

    if (try_to_resolve_bp_addresses_if_need(m))
        set_or_update_all_DRx_breakpoints(p);

    if (module_c_debug)
        L ("%s() end\n", __func__);

    return m;
};

static void module_free (module *m)
{
    DFREE(m->filename);
    DFREE(m->filename_without_ext);
    DFREE(m->path);
    DFREE(m->internal_name);
    DFREE(m->sections);

    if (m->symbols)
        for (struct rbtree_node_t *i=rbtree_minimum(m->symbols); i; i=rbtree_succ(i))
        {
            for (symbol* s=(symbol*)i->value; s; )
            {
                symbol *tmp=s;
                s=s->next;
                DFREE(tmp->name);
                DFREE(tmp);
            };
        };

    rbtree_deinit(m->symbols);
    rbtree_deinit(m->INT3_BP_bytes);
    DFREE(m);
};

void unload_module_and_free(module *m)
{
    if (module_c_debug)
    {
        L ("%s() begin\n", __func__);
        L ("m->filename=%s\n", m->filename);
        L ("m->symbols count=%d\n", rbtree_count (m->symbols));
    };

    // call cc...
    cc_dump_and_free(m); 

    module_free(m);
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

    unload_module_and_free(m);
    rbtree_delete(p->modules, (void*)img_base);
};

bool address_in_module (module *m, address a)
{
    return (a >= m->base) && (a < (m->base + m->size));
};

// may return some symbol or NULL
symbol* module_sym_exist_at (module *m, address a)
{
    oassert (address_in_module (m, a));

    return rbtree_lookup(m->symbols, (void*)a);
};

// may return module.dll!symbol+0x1234
void module_get_sym (module *m, address a, bool add_module_name, bool add_offset, strbuf *out)
{
    oassert (address_in_module (m, a));

    address prev_k;
    symbol *prev_v;

    if (add_module_name)
    {
        strbuf_addstr (out, get_module_name(m));
        strbuf_addc (out, '!');
    };

    symbol *first_sym=rbtree_lookup2(m->symbols, (void*)a, (void**)&prev_k, (void**)&prev_v, NULL, NULL);
    if (first_sym)
    {
        // take 'top' symbol
        strbuf_addstr (out, first_sym->name);
    }
    else
    {
        strbuf_addstr (out, prev_v->name);
        if (add_offset)
            strbuf_addf (out, "+0x%x", a-prev_k);
    };
};

char *get_module_name (module *m)
{
    if (m->internal_name)
        return m->internal_name;
    else
        return m->filename;
};

static IMAGE_SECTION_HEADER* find_section (module *m, address a)
{
    oassert(m->sections && "PE sections info wasn't yet loaded to module structure");
    for (unsigned i=0; i<m->sections_total; i++)
    {
        IMAGE_SECTION_HEADER *s=&m->sections[i];
        if (module_c_debug)
            L ("%s() trying section %s\n", __func__, s->Name);
        address start=s->VirtualAddress + m->base;
        if (a>=start && a<(start + s->Misc.VirtualSize))
            return s;
    };
    return NULL;
};


bool module_adr_in_executable_section (module *m, address a)
{
    if (module_c_debug)
        L ("%s() module=%s, a=0x" PRI_ADR_HEX "\n", __func__, get_module_name(m), a);
    // find specific section
    oassert(m->sections && "PE sections info wasn't yet loaded to module structure");
    IMAGE_SECTION_HEADER* s=find_section(m, a);
    if (s==NULL)
        return false;

    return is_it_code_section (s);
};

address get_module_end(module *m)
{
    return m->base + m->size;
};

address module_get_next_sym_address_after (module *m, address a)
{
    address rt=0;
    void *tmp;
    tmp=rbtree_lookup2(m->symbols, (void*)a, NULL, NULL, (void**)&rt, NULL); 
    oassert (tmp && "module_get_next_sym_address_after(): address should be symbol start!");
    //printf ("%s(a=0x" PRI_ADR_HEX ") rt=0x" PRI_ADR_HEX "\n", __func__, a, rt);
    //printf ("(module %s, tmp=0x%p)\n", get_module_name(m), tmp);
    return rt;
};

/* vim: set expandtab ts=4 sw=4 : */
