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

#include "stuff.h"
#include "bp_address.h"
#include "fmt_utils.h"
#include "dmalloc.h"

bool is_address_OEP(bp_address *a)
{
    return a->t==OPTS_ADR_TYPE_FILENAME_SYMBOL &&
        stricmp(a->symbol, "OEP")==0 &&
        a->ofs==0;
};

bool is_address_fname_OEP(bp_address* a, char *fname)
{
    return a->t==OPTS_ADR_TYPE_FILENAME_SYMBOL &&
        stricmp(a->symbol, "OEP")==0 &&
        stricmp(a->filename, fname)==0 &&
        a->ofs==0;
};

void address_to_string (bp_address *a, strbuf *out)
{
    oassert(a);

    switch (a->t)
    {
        case OPTS_ADR_TYPE_ABS:
            strbuf_addf (out, "0x" PRI_ADR_HEX, a->abs_address);
            break;
        case OPTS_ADR_TYPE_FILENAME_SYMBOL:
            strbuf_addf (out, "%s!%s", a->filename, a->symbol);
            if (a->ofs)
                strbuf_addf (out, "+0x%x", a->ofs);
            break;
        case OPTS_ADR_TYPE_FILENAME_ADR:
            strbuf_addf (out, "%s!0x" PRI_ADR_HEX, a->filename, a->adr);
            break;
        case OPTS_ADR_TYPE_BYTEMASK:
            strbuf_addstr (out, "bytemask:\"");
            for (int i=0; i<a->bytemask_len; i++)
            {
                wyde bm=a->bytemask[i];
                if (bm==BYTEMASK_WILDCARD_BYTE)
                    strbuf_addstr (out, "..");
                else
                    strbuf_addf (out, "%02X", bm);
            };
            strbuf_addstr (out, "\"");
            break;
        default:
            oassert(0);
            fatal_error();
            break;
    };
};

void dump_address (bp_address *a)
{
    oassert(a);
    strbuf sb=STRBUF_INIT;

    address_to_string (a, &sb);
    printf ("%s", sb.buf);
    strbuf_deinit (&sb);
};

bp_address *create_address_filename_symbol_re(const char *filename, const char *symbol_re, unsigned ofs)
{
    bp_address *rt=DCALLOC (bp_address, 1, "bp_address");

    rt->t=OPTS_ADR_TYPE_FILENAME_SYMBOL;
    rt->filename=DSTRDUP (filename, "");
    rt->symbol=DSTRDUP (symbol_re, "");
    strbuf sb=STRBUF_INIT;
    strbuf_addc(&sb, '^');
    strbuf_addstr(&sb, symbol_re);
    strbuf_addc(&sb, '$');

    regcomp_or_die(&rt->symbol_re, sb.buf, REG_EXTENDED | REG_ICASE | REG_NEWLINE);

    strbuf_deinit(&sb);

    rt->ofs=ofs;

    return rt;
};
bp_address *create_address_filename_address(const char *filename, address adr)
{
    bp_address *rt=DCALLOC (bp_address, 1, "bp_address");

    rt->t=OPTS_ADR_TYPE_FILENAME_ADR;
    rt->filename=DSTRDUP (filename, "");
    rt->adr=adr;

    return rt;
};

bp_address *create_address_abs(unsigned adr)
{
    bp_address *rt=DCALLOC (bp_address, 1, "bp_address");

    rt->t=OPTS_ADR_TYPE_ABS;
    rt->resolved=true;
    rt->abs_address=adr;

    return rt;
};
bp_address *create_address_bytemask(obj *bytemask)
{
    bp_address *rt=DCALLOC (bp_address, 1, "bp_address");

    rt->t=OPTS_ADR_TYPE_BYTEMASK;
    list_of_wydes_to_array(&rt->bytemask, &rt->bytemask_len, bytemask);

    return rt;
};

void bp_address_free(bp_address *a)
{
    if (a==NULL)
        return;
    if (a->t==OPTS_ADR_TYPE_FILENAME_SYMBOL || a->t==OPTS_ADR_TYPE_FILENAME_ADR)
        DFREE(a->filename);
    if (a->t==OPTS_ADR_TYPE_FILENAME_SYMBOL)
    {
        DFREE(a->symbol);
        regfree(&a->symbol_re);
    };
    if (a->t==OPTS_ADR_TYPE_BYTEMASK)
        DFREE(a->bytemask);
    DFREE(a);
};

/* vim: set expandtab ts=4 sw=4 : */
