/****************************************************************
Copyright 1998 the author
Module Name    : VIEWSYS
Module Purpose : to provide a fully scalable view system, made of windows
                 which have VIEWPORT and display function
Creation Date  : 27 February 1998
Updated        : 2 May 1998         ADDED ViewSysMouse
****************************************************************/
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
#include "defines.h"
#include "screen.h"
#include "mempage.h"
#include "viewsys.h"

VIEWSYS *ViewSysNew(int x1,int y1,int x2,int y2,int maxwindows,
        void (* background)(VIEWPORT *))
// purpose : creates a new view system, which can contain up to maxwindows
//           which uses background function for each windows background
// returns : the viewsystem
{
    VIEWSYS *viewsys=malloc(sizeof(VIEWSYS));
    viewsys->main.x1=x1;
    viewsys->main.y1=y1;
    viewsys->main.x2=x2;
    viewsys->main.y2=y2;
    viewsys->maxwindows=maxwindows;
    viewsys->windows=0;
    viewsys->window=malloc(maxwindows*sizeof(WINDOW));
    viewsys->current=MemPageNew(&(viewsys->main));
    viewsys->background=background;
    viewsys->changed=0;
    return viewsys;
}

MEMPAGE *ViewSysCurrent(VIEWSYS *vs)
// purpose : to get the viewsys current view
// returns : the current view of the viewsystem, a MEMPAGE
{
    return vs->current;
}

void ViewSysWindowNew(VIEWSYS *vs,double x1,double y1,double x2,double y2,
                int active,int (* display)(void),
                int (* mouse)(int,int,int,int),char *help)
// purpose : to add a window to the view system
// returns : void
{
    VIEWWIN vw;
    if (vs->windows>=vs->maxwindows-1) {printf("Exceeded Max Windows!!"); exit(0);}

    vs->window[vs->windows].active=active;
    vw.x1=x1; vw.y1=y1; vw.x2=x2; vw.y2=y2;
    ViewportFromViewport(&(vs->window[vs->windows].vp),&vw,&(vs->main));
    vs->window[vs->windows].display=display;
    vs->window[vs->windows].mouse=mouse;
    vs->window[vs->windows].changed=FALSE;
    vs->window[vs->windows].help=malloc((strlen(help)+1)*sizeof(char));
    strcpy(vs->window[vs->windows].help,help);
    ++(vs->windows);
}

int ViewSysDisplay(VIEWSYS *vs,int mode)
// purpose : to display the viewsys, using mode
//           mode==DisplayAll displays all active windows
//           mode==DisplayChanged displays only windows which have changed
//           mode==NoDisplay makes function have no effect
// returns : TRUE if display updated else FALSE
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
                    wp->changed=FALSE;
                }
            }
            MemPageGet(vs->current); // update current to reflect changes
            break;
        case DisplayChanged :
            MemPagePut(vs->current);
            if (vs->changed>0) // else just display current
            {
                for (n=0;n<vs->windows;n++)
                {
                    wp=&(vs->window[n]);
                    if ((wp->changed) && (wp->active) && (wp->display!=NULL))
                    {
                        vs->background(&(wp->vp));
                        SetViewport(&(wp->vp));
                        wp->display();
                        wp->changed=FALSE;
                    }
                }
                MemPageGet(vs->current); // update current to reflect changes
                vs->changed=0;
            }
            break;
        case DisplayCurrent :
            MemPagePut(vs->current);
            break;
        default : return FALSE; break; // NoDisplay
    }
    return TRUE;
}

void ViewSysWindowChanged(VIEWSYS *vs,int window,int changed)
// purpose : to register window from viewsys as changed
// returns : void
{
    if ((vs->window[window].changed==TRUE) && (changed==FALSE))
        vs->changed-=1;
    else if ((vs->window[window].changed==FALSE) && (changed==TRUE))
        vs->changed+=1;
    vs->window[window].changed=changed;
}       

void ViewSysWindowActivate(VIEWSYS *vs,int window,int active)
// purpose : to register window from viewsys as active
// returns : void
// NOTE    : only active windows can be displayed or recieve input
{
    vs->window[window].active=active;
}

char *ViewSysWindowHelp(VIEWSYS *vs,int window)
// purpose : to get the help string associated with window in viewsys
// returns : the string
{
    return vs->window[window].help;
}

int ViewSysMouse(VIEWSYS *vs,int x,int y)
// purpose : to send a mouse signal to the first relevant window in the viewsys
// returns : FALSE if no window found at x,y
//           TRUE if a window received the signal and ran its mouse func
//           UNKNOWN if a window received the signal but had no mouse func
{
    WINDOW *wp;
    VIEWPORT *vp;
    int ax,ay,n;
    for (n=0;n<vs->windows;n++)
    {
        wp=&(vs->window[n]);
        vp=&(wp->vp);
        if ((wp->active) && (PointInsideViewport(x,y,vp)))
        {
            if (wp->mouse==NULL) return UNKNOWN;
            // else mouse has func
            ax=x-(wp->vp.x1);   // viewport mouse funcs dont care exactly
            ay=y-(wp->vp.y1);   // where viewport is, only how big it is
            wp->mouse(ax,ay,vp->x2-vp->x1+1,vp->y2-vp->y1+1);
            return TRUE; 
        }
    }
    return FALSE;   // no window found at x,y
}

void ViewSysFree(VIEWSYS *vs)
// purpose : to free memory allocated to the viewsys
// returns : void
{
    int n;
    for (n=0;n<vs->windows;n++)
        free(vs->window[n].help);
    free(vs->window);
    free(vs);
}

