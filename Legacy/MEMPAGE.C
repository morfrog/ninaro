/****************************************************************
Copyright 1998 the author
Module Name    : MEMPAGE
Module Purpose : to provide a memory page mechanism, so that screens can
                 be saved and loaded.
Creation Date  : 17 March 1998
Updated        : None
****************************************************************/
#include <stdlib.h>
#include <graph.h>
#include "screen.h"
#include "mempage.h"

MEMPAGE *MemPageNew(VIEWPORT *vp)
// creates a new memory page in the viewport (x1,y1)-(x2,y2)
{
    MEMPAGE *mp=malloc(sizeof(MEMPAGE));
    mp->vp=vp;
    mp->image=malloc(_imagesize(vp->x1,vp->y1,vp->x2,vp->y2));
    return mp;
}

void MemPageGet(MEMPAGE *mp)
// gets the image of the mempages viewport and saves it
{
    SetViewport(mp->vp);
    _getimage_w(0.0,0.0,1.0,1.0,mp->image);
}

void MemPagePut(MEMPAGE *mp)
// puts the image of the mempages viewport on the screen
{
    SetViewport(mp->vp);
    _putimage_w(0.0,0.0,mp->image,_GPSET);
}

void MemPageFree(MEMPAGE *mp)
{
    free(mp->image);
    free(mp);
}
