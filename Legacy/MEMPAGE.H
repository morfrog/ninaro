#ifndef MEMPAGE_H
#define MEMPAGE_H
#include "screen.h"

struct mempageTag
{
    VIEWPORT *vp;
    char *image;
};

typedef struct mempageTag MEMPAGE;

MEMPAGE *MemPageNew(VIEWPORT *vp);
void MemPageGet(MEMPAGE *mp);
void MemPagePut(MEMPAGE *mp);
void MemPageFree(MEMPAGE *mp);

#endif
