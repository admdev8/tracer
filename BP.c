#include <stdbool.h>
#include <assert.h>

#include "dmalloc.h"
#include "BP.h"
#include "strbuf.h"
#include "stuff.h"
#include "opts.h"
#include "bp_address.h"
#include "BPF.h"

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
        if (breakpoints[OEP_BP_NO])
            die ("Only one breakpoint at OEP can be present\n");
        breakpoints[OEP_BP_NO]=rt; 
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
