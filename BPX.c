#include <assert.h>

#include "bp_address.h"
#include "dmalloc.h"
#include "BPX.h"

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

BPX* create_BPX(BPX_option *opts)
{
    BPX* rt=DCALLOC (BPX, 1, "BPX");
    rt->opts=opts;

    return rt;
};

void handle_BPX(process *p, thread *t, int DRx_no /* -1 for OEP */, CONTEXT *ctx, MemoryCache *mc)
{
    assert (!"not implemented");
};
