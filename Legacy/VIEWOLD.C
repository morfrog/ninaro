#include <stdlib.h>
#include <graph.h>
#include "viewsys.h"
/*
    The view system is one where a new viewport can be set using window
    ordinates of the current viewport. This can be done down to a number of
    levels, with the facility to jump back to the level above at will.
*/

void ViewSysSetViewport(VIEWPORT *vp)
{
    _setviewport(vp->x1,vp->y1,vp->x2,vp->y2);
}

void ViewSysHome(VIEWSYSTEM *vs)
{
    vs->at=0;        
    ViewSysSetViewport(&(vs->views[0]));
}

VIEWSYSTEM *ViewSysInit(int size,int x1,int y1,int x2,int y2)
// Creates a new viewsystem with the top window described by
// x1,y1 - x2,y2, with allowance for size levels only
{
    VIEWSYSTEM *vs=malloc(sizeof(VIEWSYSTEM));
    vs->size=size;
    vs->views=malloc(size*sizeof(VIEWPORT));
    vs->views[0].x1=x1;
    vs->views[0].x2=x2;
    vs->views[0].y1=y1;
    vs->views[0].y2=y2;
    ViewSysHome(vs);
    return vs;
}

void ViewSysFree(VIEWSYSTEM *vs)
{
    free(vs->views);
    free(vs);
}

int ViewSysSubLevel(VIEWSYSTEM *vs,double x1,double y1,double x2,double y2)
// Sets the new window as the window inside the current window described by
// x1,y1 - x2,y2, returns 1 on success else 0 if vs full
{
    int t,l,r,b,width,height;
    if (vs->at==(vs->size)-1) // already full
        return 0;
    else
    {
        t=vs->views[vs->at].y1;
        l=vs->views[vs->at].x1;
        b=vs->views[vs->at].y2;
        r=vs->views[vs->at].x2;
        width=r-l;
        height=b-t;

        (vs->at)++;
        vs->views[vs->at].x1=l+(x1*width);
        vs->views[vs->at].y1=t+(y1*height);
        vs->views[vs->at].x2=l+(x2*width);
        vs->views[vs->at].y2=t+(y2*height);
    }   
    ViewSysSetViewport(&(vs->views[vs->at]));
    return 1;
}

int ViewSysUpLevel(VIEWSYSTEM *vs)
// Sets the new window as the one above the current level
{
    if (vs->at==0) // then no change
        return 0;
    else
        (vs->at)--;
    ViewSysSetViewport(&(vs->views[vs->at]));
    return 1;
}

int ViewSysAcrossLevel(VIEWSYSTEM *vs,double x1,double y1,double x2,double y2)
{
    if (ViewSysUpLevel(vs))
    {
        ViewSysSubLevel(vs,x1,y1,x2,y2);
        return 1;
    }
    else
        return 0;

}

double ViewSysPixelsH(VIEWSYSTEM *vs,int num)
// returns the height of num pixels in the current level
{
    VIEWPORT *vp=&(vs->views[vs->at]);
    int height=vp->y2-vp->y1+1;
    return ((double)num/(double)height);
}

double ViewSysPixelsW(VIEWSYSTEM *vs,int num)
// returns the width of num pixels in the current level
{
    VIEWPORT *vp=&(vs->views[vs->at]);
    int width=vp->x2-vp->x1+1;
    return ((double)num/(double)width);
}

void ViewSysSetCurrent(VIEWSYSTEM *vs)
// sets the current viewport as the current one in vs.
{
    ViewSysSetViewport(&(vs->views[vs->at]));
}
