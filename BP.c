#include <stdbool.h>
#include <assert.h>
#include "dmalloc.h"
#include "BP.h"
#include "strbuf.h"
#include "stuff.h"
#include "opts.h"

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
    assert(a);

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
            assert(0);
            break;
    };
};

void dump_address (bp_address *a)
{
    assert(a);
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
    int rc;
    strbuf sb=STRBUF_INIT;
    strbuf_addc(&sb, '^');
    strbuf_addstr(&sb, symbol_re);
    strbuf_addc(&sb, '$');

    if ((rc=regcomp(&rt->symbol_re, sb.buf, REG_EXTENDED | REG_ICASE | REG_NEWLINE))!=0)
    {
        char buffer[100];
        regerror(rc, &rt->symbol_re, buffer, 100);
        die("failed regcomp() for pattern '%s' (%s)", symbol_re, buffer);
    };

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

void BPX_option_free(BPX_option *o)
{
    bp_address_free (o->a);
    DFREE (o->copy_string);
    DFREE (o);
};

void BPX_free(BPX *o)
{
    if (o->opts)
    {
        BPX_option *t=o->opts, *t_next=o->opts;
        for (;t_next;t=t_next)
        {
           t_next=t->next;
           BPX_option_free(t);
        };
    };
    DFREE (o);
};

BPM *create_BPM(unsigned width, enum BPM_type t)
{
    BPM *rt=DCALLOC (BPM, 1, "BPM");

    rt->width=width;
    rt->t=t;

    return rt;
};

void dump_BPM(BPM *bpm)
{
    printf ("BPM. width=%d, ", bpm->width);
    if (bpm->t==BPM_type_RW)
        printf ("type=RW");
    else
        printf ("type=W");
    printf ("\n");
};

void BPM_free(BPM *o)
{
    DFREE (o);
};

void BPF_free(BPF* o)
{
    if (o->rt)
        obj_free (o->rt);
    if (o->when_called_from_address)
        bp_address_free(o->when_called_from_address);
    if (o->when_called_from_func)
        bp_address_free(o->when_called_from_func);
    DFREE (o);
};

void BP_free(BP* b)
{
    if (b==NULL)
        return; // free(NULL) behaviour
    bp_address_free(b->a);
    switch (b->t)
    {
        case BP_type_BPM:
            BPM_free(b->u.bpm);
            break;
        case BP_type_BPX:
            BPX_free(b->u.bpx);
            break;
        case BP_type_BPF:
            BPF_free(b->u.bpf);
            break;
        default:
            assert(0);
    };
    Da_free(b->ins);
    DFREE(b);
};

BPX* create_BPX(BPX_option *opts)
{
    BPX* rt=DCALLOC (BPX, 1, "BPX");
    rt->opts=opts;

    return rt;
};

BP* create_BP (enum BP_type t, bp_address* a, void* p)
{
    BP* rt=DCALLOC(BP, 1, "BP");
    rt->t=t;
    rt->a=a;
    rt->u.p=p;
    
    if (is_address_OEP(a))
    {
        if (OEP_breakpoint)
            die ("Only one breakpoint at OEP can be present\n");
        OEP_breakpoint=rt; 
    };

    return rt;
};

void dump_BPX(BPX *b)
{
    printf ("BPX.");
    if (b->opts)
    {
        printf (" options: ");
        for (BPX_option *o=b->opts; o; o=o->next)
            dump_BPX_option(o);
    };
    printf ("\n");
};

void dump_BPF(BPF *b)
{
    printf ("BPF. options: ");
    if (b->unicode)
        printf ("unicode ");
    if (b->skip)
        printf ("skip ");
    if (b->skip_stdcall)
        printf ("skip_stdcall ");
    if (b->trace)
        printf ("trace ");
    if (b->trace_cc)
        printf ("trace_cc ");
    if (b->rt)
    {
        printf ("rt: ");
        obj_dump (b->rt);
        printf (" ");
    };
    
    if (b->rt_probability!=1)
        printf ("rt_probability: %f ", b->rt_probability);
    
    if (b->args)
        printf ("args: %d ", b->args);
    
    if (b->dump_args)
        printf ("dump_args: %d ", b->dump_args);
    
    if (b->pause)
        printf ("pause: %d ", b->pause);

    printf ("\n");
    if (b->when_called_from_address)
    {
        printf ("when_called_from_address: ");
        dump_address (b->when_called_from_address);
        printf ("\n");
    };
    if (b->when_called_from_func)
    {
        printf ("when_called_from_func: ");
        dump_address (b->when_called_from_func);
        printf ("\n");
    };
};

void dump_BP (BP* b)
{
    printf ("bp_address=");
    dump_address (b->a);
    printf (". ");

    switch (b->t)
    {
        case BP_type_BPM:
            dump_BPM(b->u.bpm);
            break;
        case BP_type_BPX:
            dump_BPX(b->u.bpx);
            break;
        case BP_type_BPF:
            dump_BPF(b->u.bpf);
            break;
        default:
            assert(0);
    };
    //printf ("next=0x%p\n", b->next);
};

void dump_BPX_option(BPX_option *b)
{
    switch (b->t)
    {
        case BPX_option_DUMP:
            printf ("[DUMP ");
            if (b->a)
                dump_address(b->a);
            else
                printf ("reg:%s", X86_register_ToString(b->reg));
            printf (" size: %d]", b->size_or_value);
            break;

        case BPX_option_SET:
            assert (b->a==NULL); // must be always register
            printf("[SET reg:%s value:%d]", X86_register_ToString(b->reg), b->size_or_value);
            break;

        case BPX_option_COPY:
            printf ("[COPY ");
            if (b->a)
                dump_address(b->a);
            else
                printf ("reg:%s", X86_register_ToString(b->reg));
            assert(b->copy_string);
            printf ("[");
            for (int i=0; i<b->copy_string_len; i++)
                printf ("0x%02X ", b->copy_string[i]);
            printf ("]");
            break;

        default:
            assert(0);
    };
};
