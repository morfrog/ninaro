// made for the one top-size window, can't change it, new windows oords
// are calculated at creation time only
// window->active : only active windows receive display signals

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <graph.h>
#include "sys.h"
#include "sound.h"
#include "viewsys.h"

VIEWSYS *ViewSysNew(int x1,int y1,int x2,int y2,int maxwindows,int maxkeys,
                    void (* background)(void))
// creates a new view system and returns it
{
    VIEWSYS *viewsys=malloc(sizeof(VIEWSYS));
    viewsys->main.x1=x1;
    viewsys->main.y1=y1;
    viewsys->main.x2=x2;
    viewsys->main.y2=y2;
    viewsys->background=background;
    viewsys->maxwindows=maxwindows;
    viewsys->windows=0;
    viewsys->maxkeys=maxkeys;
    viewsys->keys=0;
    viewsys->window=malloc(maxwindows*sizeof(WINDOW));
    viewsys->key=malloc(maxkeys*sizeof(KEYIN));
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

void ViewSysNewWindow(VIEWSYS *vs,double x1,double y1,double x2,double y2,
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
    strcpy(vs->window[vs->windows].help,help);
    ++(vs->windows);
}

void ViewSysAddKey(VIEWSYS *vs,int key,int (* func)(void),int enable)
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

int ViewSysDisplay(VIEWSYS *vs,int mode)
// mode==DisplayAll displays all active windows
// mode==DisplayChanged displays only windows which have changed
// mode==NoDisplay makes function have no effect
// returns TRUE if display updated else FALSE
// TODO - make more efficient, possibly by splitting DisplayAll & DisplayChanged
{
    int n,changed;
    if (mode==NoDisplay) return FALSE;
    if (mode==DisplayAll) {SetViewport(&(vs->main)); vs->background();}
    for (n=0;n<vs->windows;n++)
    {
        changed=(vs->window[n].changed==CHANGED);
        if ((vs->window[n].active) && (mode==DisplayAll?1:changed))
            if (vs->window[n].display!=NULL)
            {
                SetViewport(&(vs->window[n].vp));
                vs->window[n].display();
                if (changed) vs->window[n].changed=NOCHANGE;
            }
            // else nothing to display
        // else inactive/hasn't changed, so don't display
    }
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

void ViewSysEnable(VIEWSYS *vs,int enable,int n_key)
// sets the n_key'th key created to enable state
{
    vs->key[n_key].enable=enable;
}

void ViewSysEnableAll(VIEWSYS *vs,int enable,int key)
// sets all keys which match key to enabled state
{
    int n;
    for (n=0;n<vs->keys;n++)
        if (vs->key[n].key==key) vs->key[n].enable=enable;
}

void ViewSysEnableGroup(VIEWSYS *vs,int enable,int num,...)
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

void ViewSysChanged(VIEWSYS *vs,int window,int changed)
{
    vs->window[window].changed=changed;
}

void ViewSysActivate(VIEWSYS *vs,int window,int activity)
{
    vs->window[window].active=activity;
}

char *ViewSysHelp(VIEWSYS *vs,int window)
{
    return vs->window[window].help;
}


void ViewSysFree(VIEWSYS *vs)
{
    free(vs->key);
    free(vs->window);
    free(vs);
}

