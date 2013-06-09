#pragma once

#include "x86_disas.h"
#include "X86_register.h"
#include "datatypes.h"
#include "regex.h"
#include "lisp.h"

typedef struct _bp_address bp_address;
typedef struct _BPF BPF;
typedef struct _BPX BPX;

enum BPM_type
{
    BPM_type_W,
    BPM_type_RW
};

typedef struct _BPM
{
    unsigned width;
    enum BPM_type t;
} BPM;

enum BP_type
{
    BP_type_BPM,
    BP_type_BPX,
    BP_type_BPF
};

typedef struct _BP
{
    enum BP_type t;
    Da* ins; // disassembled instruction at the place of INT3

    bp_address *a;
    union
    {
        BPM* bpm;
        BPX* bpx;
        BPF* bpf;
        void *p;
    } u;
    struct _BP *next;
} BP;

BPM *create_BPM(unsigned width, enum BPM_type t);
void dump_BPM(BPM *);
BP* create_BP (enum BP_type t, bp_address* a, void* p);
void dump_BP (BP* b);
void BP_free(BP*);
void BPM_free(BPM *);
