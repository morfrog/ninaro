#ifndef MEMPAGE_H
#define MEMPAGE_H
#include "screen.h"

struct mempageTag
{
    VIEWPORT vp;
    char *image;
};

typedef struct mempageTag MEMPAGE;

MEMPAGE *MemPageNew(int x1,int y1,int x2,int y2);
void MemPageGet(MEMPAGE *mp);
void MemPagePut(MEMPAGE *mp);
void MemPageCopySection(MEMPAGE *target,MEMPAGE *source,VIEWPORT *vp);
void MemPageFree(MEMPAGE *mp);

#endif
