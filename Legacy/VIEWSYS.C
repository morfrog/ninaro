// made for the one top-size window, can't change it, new windows oords
// are calculated at creation time only
// window->active : only active windows receive display signals
// TODO : might want a view system to include keys from another view system,
// maybe have a link rather than a copy, or maybe copy better
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <graph.h>
#include "sys.h"
#include "sound.h"
#include "mempage.h"
#include "viewsys.h"

VIEWSYS *ViewSysNew(int x1,int y1,int x2,int y2,int maxwindows,int maxkeys,
        void (* background)(VIEWPORT *))
// creates a new view system and returns it,
// if getbkgrnd==TRUE the current screen is assumed to be the background
// else the background is not set. It can be set using ViewSysSetBackground
// which takes a MEMPAGE as a parameter which is the background.
// this enables multiple view systems to have the same background thus saving
// memory.
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
    viewsys->current=MemPageNew(x1,y1,x2,y2);
    viewsys->background=background;
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

void ViewportFromViewport(VIEWPORT *new,VIEWWIN *vw,VIEWPORT *vp)
{
    int width=(vp->x2)-(vp->x1), height=(vp->y2)-(vp->y1);
    new->x1=vp->x1+(int)(vw->x1*(double)width);
    new->y1=vp->y1+(int)(vw->y1*(double)height);
    new->x2=vp->x1+(int)(vw->x2*(double)width);
    new->y2=vp->y1+(int)(vw->y2*(double)height);
}

void ViewSysWindowNew(VIEWSYS *vs,double x1,double y1,double x2,double y2,
                int active,int (* display)(void),char *help)
// adds a window to the view system
{
    VIEWWIN vw;
    if (vs->windows>=vs->maxwindows-1) {printf("Exceeded Max Windows!!"); exit(0);}

    vs->window[vs->windows].active=active;
    vw.x1=x1; vw.y1=y1; vw.x2=x2; vw.y2=y2;
    ViewportFromViewport(&(vs->window[vs->windows].vp),&vw,&(vs->main));
    vs->window[vs->windows].display=display;
    vs->window[vs->windows].changed=NOCHANGE;
    vs->window[vs->windows].help=malloc((strlen(help)+1)*sizeof(char));
    strcpy(vs->window[vs->windows].help,help);
    ++(vs->windows);
}

void ViewSysKeyAdd(VIEWSYS *vs,int key,int (* func)(void),int enable)
// adds a key to the view system, a key can have multiple functions run from
// it, they are run in the order in which they are added to the system
{
    if (vs->keys==vs->maxkeys-1) {printf("Exceeded Max Keys!!"); exit(0);}
    else
    {
        vs->key[vs->keys].key=key;
        vs->key[vs->keys].keyfunc=func;
        vs->key[vs->keys].enable=enable;
        ++vs->keys;
    }
}

void SetViewport(VIEWPORT *vp)
{
    _setviewport(vp->x1,vp->y1,vp->x2,vp->y2);
}

void ViewSysClear(VIEWPORT *vp)
// clears the vp viewport
{
    SetViewport(vp);
    _clearscreen(_GVIEWPORT);
}

int ViewSysDisplay(VIEWSYS *vs,int mode)
// mode==DisplayAll displays all active windows
// mode==DisplayChanged displays only windows which have changed
// mode==NoDisplay makes function have no effect
// returns TRUE if display updated else FALSE
// TODO - make more efficient, possibly by splitting DisplayAll & DisplayChanged
{
    WINDOW *wp;
    int n;
    switch (mode)
    {
        case DisplayAll:
            vs->background(&(vs->main));
            for (n=0;n<vs->windows;n++)
            {
                wp=&(vs->window[n]);
                if ((wp->active) && (wp->display!=NULL))
                {
                    SetViewport(&(wp->vp));
                    wp->display();
                    wp->changed=NOCHANGE;
                }
            }
            break;
        case DisplayChanged :
            MemPagePut(vs->current);
            for (n=0;n<vs->windows;n++)
            {
                wp=&(vs->window[n]);
                if ((wp->changed) && (wp->active) && (wp->display!=NULL))
                {
                    vs->background(&(wp->vp));
                    SetViewport(&(wp->vp));
                    wp->display();
                    wp->changed=NOCHANGE;
                }
            }
            break;
        default : return FALSE; break; // NoDisplay
    }
    MemPageGet(vs->current); // update current to reflect changes
    return TRUE;
}

int ViewSysKey(VIEWSYS *vs,int key,int *used)
// *used=TRUE if the key was used else FALSE
// if the key was used, it will request display using NoDisplay,DisplayAll
// or DisplayChanged, which is returned, the maximum amount which can be
// displayed will be, so if one function requests NoDisplay and another
// requests DisplayAll, DisplayAll will be returned 
{
    int n,ret=NoDisplay;
    *used=FALSE;
    for (n=0;n<vs->keys;n++)
        if ((key==vs->key[n].key) // key matches
        &&  (vs->key[n].enable)  // key enabled  
        &&  (vs->key[n].keyfunc!=NULL)) // key has a function
        {
            *used=TRUE;
            switch(vs->key[n].keyfunc())
            {
                case DisplayAll :
                    ret = DisplayAll; break;
                case DisplayChanged :
                    if (ret==NoDisplay) ret=DisplayChanged; break;
            //  case NoDisplay is already set or over-ridden
            }
        }
        // else try next key
    return ret;
}

void ViewSysKeyEnable(VIEWSYS *vs,int enable,int n_key)
// sets the n_key'th key created to enable state
{
    vs->key[n_key].enable=enable;
}

void ViewSysKeyEnableAll(VIEWSYS *vs,int enable,int key)
// sets all keys which match key to enabled state
{
    int n;
    for (n=0;n<vs->keys;n++)
        if (vs->key[n].key==key) vs->key[n].enable=enable;
}

void ViewSysKeyEnableGroup(VIEWSYS *vs,int enable,int num,...)
// sets the nth keys to enable, e.g. ViewSysEnableGroup(vs,ENABLE,3,0,1,2);
{
    int n,n_key;
    va_list ap;

    va_start(ap,num);
    for (n=0;n<num;n++)
    {
        n_key=va_arg(ap,int);
        vs->key[n_key].enable=enable;
    }
    va_end(ap);
}

void ViewSysWindowChanged(VIEWSYS *vs,int window,int changed)
{
    vs->window[window].changed=changed;
}

void ViewSysWindowActivate(VIEWSYS *vs,int window,int activity)
{
    vs->window[window].active=activity;
}

char *ViewSysWindowHelp(VIEWSYS *vs,int window)
{
    return vs->window[window].help;
}


void ViewSysFree(VIEWSYS *vs)
{
    int n;
    free(vs->key);
    for (n=0;n<vs->windows;n++)
        free(vs->window[n].help);
    free(vs->window);
    free(vs);
}

