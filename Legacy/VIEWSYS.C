// made for the one top-size window, can't change it, new windows oords
// are calculated at creation time only
// window->active : only active windows receive mouse,key and display signals

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <graph.h>
#include "sys.h"
#include "mouse.h"
#include "viewsys.h"

VIEWSYS *ViewSysNew(int x1,int y1,int x2,int y2,int maxwindows,int maxkeys)
// creates a new view system and returns it
{
    int x,y,xrange=(x2-x1+1),yrange=(y2-y1+1);
    VIEWSYS *viewsys=malloc(sizeof(VIEWSYS));
    viewsys->main.x1=x1;
    viewsys->main.y1=y1;
    viewsys->main.x2=x2;
    viewsys->main.y2=y2;
    viewsys->maxwindows=maxwindows;
    viewsys->windows=0;
    viewsys->maxkeys=maxkeys;
    viewsys->keys=0;
    viewsys->window=malloc(maxwindows*sizeof(WINDOW));
    viewsys->key=malloc(maxkeys*sizeof(KEYIN));
    viewsys->map=malloc(xrange*sizeof(MAPPING **));
    printf("Creating Map...\n");
    for (x=0;x<xrange;x++)
    {
        viewsys->map[x]=malloc(yrange*sizeof(MAPPING *));
        for (y=0;y<yrange;y++)
        {
            viewsys->map[x][y]=malloc(sizeof(MAPPING));
            viewsys->map[x][y]->window=NULL;
            viewsys->map[x][y]->x=x;
            viewsys->map[x][y]->y=y;
        }
    }
    return viewsys;
}

int PointInsideViewport(int x,int y,VIEWPORT *vp)
{
    if (x>vp->x2) return FALSE;
    if (x<vp->x1) return FALSE;
    if (y>vp->y2) return FALSE;
    if (y<vp->y1) return FALSE;
    return TRUE;
}

MAPPING *ViewSysWindow(VIEWSYS *vs,int x,int y)
{
    if (PointInsideViewport(x,y,&(vs->main)))
        return vs->map[x-vs->main.x1][y-vs->main.y1];
    else return NULL;
}

void ViewportFromViewport(VIEWPORT *new,VIEWWIN *vw,VIEWPORT *vp)
{
    int width=(vp->x2)-(vp->x1), height=(vp->y2)-(vp->y1);
    new->x1=vp->x1+(int)(vw->x1*(double)width);
    new->y1=vp->y1+(int)(vw->y1*(double)height);
    new->x2=vp->x1+(int)(vw->x2*(double)width);
    new->y2=vp->y1+(int)(vw->y2*(double)height);
}

void ViewSysMapWindow(VIEWSYS *vs,int window)
// fits the window from the viewsys into the viewsys' map
{
    int x,y,
        fromx=vs->window[window].vp.x1-vs->main.x1,
        fromy=vs->window[window].vp.y1-vs->main.y1,
        tox=vs->window[window].vp.x2-vs->main.x1,
        toy=vs->window[window].vp.y2-vs->main.y1;
    for (x=fromx;x<tox;x++)
        for (y=fromy;y<toy;y++)
        {
            vs->map[x][y]->window=&(vs->window[window]);
            vs->map[x][y]->x=x-fromx;
            vs->map[x][y]->y=y-fromy;
        }
}

void ViewSysNewWindow(VIEWSYS *vs,double x1,double y1,double x2,double y2,
                int active,int (* display)(void),void (* lmb)(int,int),
                void (* rmb)(int,int),char *help)
// adds a window to the view system
{
    VIEWWIN vw;
    if (vs->windows>=vs->maxwindows-1) {printf("Exceeded Max Windows!!"); exit(0);}

    vs->window[vs->windows].active=active;
    vw.x1=x1; vw.y1=y1; vw.x2=x2; vw.y2=y2;
    ViewportFromViewport(&(vs->window[vs->windows].vp),&vw,&(vs->main));
    vs->window[vs->windows].display=display;
    vs->window[vs->windows].lmb=lmb;
    vs->window[vs->windows].rmb=rmb;
    vs->window[vs->windows].changed=0;
    strcpy(vs->window[vs->windows].help,help);
    printf("Initialising Window %d.\n",vs->windows);
    ViewSysMapWindow(vs,vs->windows);
    ++(vs->windows);
}

void ViewSysAddKey(VIEWSYS *vs,int key,int window,int lmb)
// adds a key to the view system
{
    if (vs->keys==vs->maxkeys-1) {printf("Exceeded Max Keys!!"); exit(0);}
    else
    {
        vs->key[vs->keys].key=key;
        vs->key[vs->keys].window=window;
        vs->key[vs->keys].lmb=lmb;
        ++vs->keys;
    }
}

void SetViewport(VIEWPORT *vp)
{
    _setviewport(vp->x1,vp->y1,vp->x2,vp->y2);
}

void ViewSysDisplay(VIEWSYS *vs,int mode)
// mode==DisplayAll displays all active windows
// mode==DisplayChanged displays only windows which have changed
{
    int n;
    _clearscreen(_GCLEARSCREEN);
    for (n=0;n<vs->windows;n++)
        if ((vs->window[n].active) && (mode==DisplayAll?1:(vs->window[n].changed)))
            if (vs->window[n].display!=NULL)
            {
                SetViewport(&(vs->window[n].vp));
                vs->window[n].display();
            }
            // else nothing to display
        // else inactive/hasn't changed, so don't display
}

int ViewSysKey(VIEWSYS *vs,int key)
// returns TRUE if the key was used else FALSE is returned
{
    int n;
    int used=FALSE;
    int window;
    for (n=0;n<vs->keys;n++)
        if (key==vs->key[n].key) // key matches
        {
            window=(vs->key[n].window);
            if (vs->window[window].active) // active so can receive key
                if (vs->key[n].lmb)
                {
                    if (vs->window[window].lmb!=NULL)
                    {
                        vs->window[window].lmb(Key,Key);
                        used=TRUE;
                    }
                }
                else
                {
                    if (vs->window[window].rmb!=NULL)
                    {
                        vs->window[window].rmb(Key,Key);
                        used=TRUE;
                    }
                }
        }
        // else keys not the same
        return used;
}

void ViewSysActivate(VIEWSYS *vs,int window,int activity)
{
    vs->window[window].active=activity;
}

char *ViewSysHelp(VIEWSYS *vs,int window)
{
    return vs->window[window].help;
}

int ViewSysMouse(VIEWSYS *vs,MouseStatus *ms)
// returns TRUE if a button signal was sent else returns FALSE
{
    static int cleared=TRUE; // cleared = mouse click can be done
    int x,y,n;
    if (!(ms->left || ms->right))
    {
        cleared=TRUE; return FALSE;
    }
    if (ms->left)
    {
        cleared=FALSE;
        for (n=vs->windows-1;n>=0;n--)
            if ((vs->window[n].lmb!=NULL) && (vs->window[n].active))
            {
                // x= y=
                vs->window[n].lmb(x,y);
                return TRUE;
            }
    }

}

void ViewSysFree(VIEWSYS *vs)
{
    int x,y,xrange=(vs->main.x2-vs->main.x1+1),yrange=(vs->main.y2-vs->main.y1+1);    
    printf("Freeing ViewSystem...");
    flushall();
    for (x=0;x<xrange;x++)
    {
        if (x%20==0) putch('.');
        for (y=0;y<yrange;y++)
        {
            free(vs->map[x][y]);
        }
        free(vs->map[x]);
    }
    free(vs->map);
    free(vs->key);
    free(vs->window);
    free(vs);
    printf("ViewSystem Freed\n");
}

