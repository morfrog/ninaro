#include <stdlib.h>
#include <graph.h>
#include "mempage.h"

MEMPAGE *MemPageNew(int x1,int y1,int x2,int y2)
// creates a new memory page in the viewport (x1,y1)-(x2,y2)
{
    MEMPAGE *mp=malloc(sizeof(MEMPAGE));
    mp->vp.x1=x1;
    mp->vp.y1=y1;
    mp->vp.x2=x2;
    mp->vp.y2=y2;
    mp->image=malloc(_imagesize(x1,y1,x2,y2));
    return mp;
}

void MemPageGet(MEMPAGE *mp)
// gets the image of the mempages viewport and saves it
{
    _getimage(mp->vp.x1,mp->vp.y1,mp->vp.x2,mp->vp.y2,mp->image);
}

void MemPagePut(MEMPAGE *mp)
// puts the image of the mempages viewport on the screen
{
    _putimage(mp->vp.x1,mp->vp.x2,mp->image,_GPSET);
}

void MemPageFree(MEMPAGE *mp)
{
    free(mp->image);
    free(mp);
}

// incorrect, as _image uses one char to store two pixels...
void MemPageCopySection(MEMPAGE *target,MEMPAGE *source,VIEWPORT *vp)
// copies the viewport section of source into the same viewport section
// of target, assumes both mempages are of the same size, and that all points
// in vp are a subset of those in source
{
    int x,y;
    int vpwidth=vp->x2-vp->x1+1;
    int vpheight=vp->y2-vp->y1+1;
    int srcwidth=source->vp.x2-source->vp.x1+1;
    int start=1+((vp->y1-source->vp.y1)*(srcwidth))+(vp->x1-source->vp.x1);

    start+=6; // is amount for non-pixels in _image
    for (y=0;y<vpheight;y++)
    {
        for (x=start;x<start+vpwidth;x++)
            target->image[x]=source->image[x];
        start+=srcwidth;
    }
}

