#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "dmalloc.h"
#include "X86_register.h"
#include "opts.h"

BPF* current_BPF=NULL; // filled while parsing

void dump_address (bp_address *a)
{
    assert(a);

    switch (a->t)
    {
        case OPTS_ADR_TYPE_ABS:
            printf ("0x%x", a->abs_address);
            break;
        case OPTS_ADR_TYPE_FILENAME_SYMBOL:
            printf ("(symbol) %s!%s", a->filename, a->symbol);
            if (a->ofs)
                printf ("+0x%x", a->ofs);
            break;
        case OPTS_ADR_TYPE_FILENAME_ADR:
            printf ("(adr in PE module) %s!0x%x", a->filename, a->adr);
            break;
        case OPTS_ADR_TYPE_BYTEMASK:
            printf ("bytemask:\"");
            obj_dump(a->bytemask);
            printf ("\"");
            break;
        default:
            assert(0);
            break;
    };
};

bp_address *create_address_filename_symbol(const char *filename, const char *symbol, unsigned ofs)
{
    bp_address *rt=DCALLOC (bp_address, 1, "bp_address");

    rt->t=OPTS_ADR_TYPE_FILENAME_SYMBOL;
    rt->filename=strdup (filename);
    rt->symbol=strdup (symbol);
    rt->ofs=ofs;

    return rt;
};

bp_address *create_address_filename_address(const char *filename, address adr)
{
    bp_address *rt=DCALLOC (bp_address, 1, "bp_address");

    rt->t=OPTS_ADR_TYPE_FILENAME_ADR;
    rt->filename=strdup (filename);
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
    rt->bytemask=bytemask;

    return rt;
};

void bp_address_free(bp_address *a)
{
    if (a->t==OPTS_ADR_TYPE_FILENAME_SYMBOL || a->t==OPTS_ADR_TYPE_FILENAME_ADR)
        free(a->filename);
    if (a->t==OPTS_ADR_TYPE_FILENAME_SYMBOL)
        free (a->symbol);
    if (a->t==OPTS_ADR_TYPE_BYTEMASK)
        obj_free(a->bytemask);
    DFREE(a);
};

void BPX_option_free(BPX_option *o)
{
    if (o->a)
        bp_address_free (o->a);
    if (o->copy_string)
        obj_free (o->copy_string);
    DFREE (o);
};

void BPX_free(BPX *o)
{
    if (o->a)
        bp_address_free(o->a);
    if (o->options)
        obj_free(o->options);
    DFREE (o);
};

BPM *create_BPM(bp_address *a, unsigned width, enum BPM_type t)
{
    BPM *rt=DCALLOC (BPM, 1, "bp_address");

    rt->a=a;
    rt->width=width;
    rt->t=t;

    return rt;
};

void dump_BPM(BPM *bpm)
{
    printf ("BPM. width=%d, ", bpm->width);
    if (bpm->t==BPM_type_RW)
        printf ("type=RW, ");
    else
        printf ("type=W, ");
    printf ("bp_address=");
    dump_address (bpm->a);
    printf ("\n");
};

void BPM_free(BPM *o)
{
    if (o->a)
        bp_address_free(o->a);
    DFREE (o);
};

void BPF_free(BPF* o)
{
    if (o->a)
        bp_address_free(o->a);
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
    DFREE(b);
};

BPX* create_BPX(bp_address *a, obj *options)
{
    BPX* rt=DCALLOC (BPX, 1, "bp_address");
    rt->a=a;
    rt->options=options;
};

BP* create_BP (enum BP_type t, void* p)
{
    BP* rt=DCALLOC(BP, 1, "bp_address");
    rt->t=t;
    rt->u.p=p;
};

void dump_BPX(BPX *b)
{
    assert (b->a);
    printf ("BPX. bp_address=");
    dump_address (b->a);
    printf ("\n");
    printf ("options: ");
    obj_dump(b->options);
    printf ("\n");
};

void dump_BPF(BPF *b)
{
    assert (b->a);
    printf ("BPF. bp_address=");
    dump_address (b->a);
    printf ("\n");
    printf ("options: ");
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
            obj_dump(b->copy_string);
            printf ("]");
            break;

        default:
            assert(0);
    };
};
