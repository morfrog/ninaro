// made for the one top-size window, can't change it, new windows oords
// are calculated at creation time only
// window->active : only active windows receive mouse,key and display signals

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <graph.h>
#include "sys.h"
#include "viewsys.h"

VIEWSYS *ViewSysNew(int x1,int y1,int x2,int y2,int maxwindows,int maxkeys)
// creates a new view system and returns it
{
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
    return viewsys;
}

void ViewportFromViewport(VIEWPORT *new,VIEWWIN *vw,VIEWPORT *vp)
{
    int width=(vp->x2)-(vp->x1), height=(vp->y2)-(vp->y1);
    new->x1=vp->x1+(int)(vw->x1*(double)width);
    new->y1=vp->y1+(int)(vw->y1*(double)height);
    new->x2=vp->x1+(int)(vw->x2*(double)width);
    new->y2=vp->y1+(int)(vw->y2*(double)height);
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
    ++(vs->windows);
}

void ViewSysAddKey(VIEWSYS *vs,char key,char extend,int window,int lmb)
// adds a key to the view system
{
    if (vs->keys==vs->maxkeys-1) {printf("Exceeded Max Keys!!"); exit(0);}
    else
    {
        vs->key[vs->keys].key=key;
        vs->key[vs->keys].extend=extend;
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

int MatchKeys(char key,char extend,char key2,char extend2)
{
    if (key!=key2) return FALSE;
    if (extend!=extend2) return FALSE;
    return TRUE;
}

int ViewSysKey(VIEWSYS *vs,char key,char extend)
// returns TRUE if the key was used else FALSE is returned
{
    int n;
    int used=FALSE;
    int window;
    for (n=0;n<vs->keys;n++)
        if (MatchKeys(key,extend,vs->key[n].key,vs->key[n].extend)) // key matches
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
