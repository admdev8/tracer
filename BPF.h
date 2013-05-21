#pragma once

#include <stdbool.h>

typedef struct _BP BP;
typedef struct _process process;
typedef struct _thread thread;

void handle_BPF(BP *bp, process *p, thread *t, bool INT3_or_DRx);
