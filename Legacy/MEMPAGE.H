/****************************************************************
Copyright 1998 the author
Module Name    : MEMPAGE
Module Purpose : to provide a memory page mechanism, so that screens can
                 be saved and loaded.
Creation Date  : 17 March 1998
Updated        : None
****************************************************************/
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
