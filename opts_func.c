#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "dmalloc.h"
#include "stuff.h"
#include "X86_register.h"
#include "opts.h"

BPF* current_BPF=NULL; // filled while parsing
bp_address* current_BPF_address; // filled while parsing

bool is_there_OEP_breakpoint_for_fname(char *fname)
{
    if (breakpoints==NULL)
        return false;
    for (BP *i=breakpoints; i->next; i=i->next)
    {
        if (is_address_fname_OEP(i->a, fname))
            return true;
    };
    return false;
};

