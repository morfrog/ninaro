/****************************************************************
Copyright 1998 the author
Module Name    : SCREEN
Module Purpose : to implement screen functions
Creation Date  : 29 February 1998
Updated        : 30 March 1998          ADDED BorderedBox
****************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <graph.h>
#include "defines.h"
#include "mouse.h"
#include "screen.h"

VIEWPORT *currentvp; // for external use until viewsys properly reworked
                   // to allow display functions to have a VIEWPORT param

void SetViewport(VIEWPORT *vp)
{
    _setviewport(vp->x1,vp->y1,vp->x2,vp->y2);
    currentvp=vp;
}

void ClearViewport(VIEWPORT *vp)
{
    SetViewport(vp);
    _setvieworg(0,0);
    _clearscreen(_GVIEWPORT);
}

VIEWPORT *CurrentViewport()
{
    return currentvp;
}

void ResetViewport(VIEWPORT *vp,int x1,int y1,int x2,int y2)
{
    vp->x1=x1; vp->y1=y1; vp->x2=x2; vp->y2=y2;
}

void MoveViewport(VIEWPORT *vp,int x1,int y1,int x2,int y2)
{
    vp->x1+=x1; vp->y1+=y1; vp->x2+=x2; vp->y2+=y2;
}

int PointInsideViewport(int x,int y,VIEWPORT *vp)
// purpose : is point x,y inside viewport
// returns : TRUE if is, else FALSE
{
    if (x>vp->x2) return FALSE;
    if (x<vp->x1) return FALSE;
    if (y>vp->y2) return FALSE;
    if (y<vp->y1) return FALSE;
    return TRUE;
}

void ViewportFromViewport(VIEWPORT *new,VIEWWIN *vw,VIEWPORT *vp)
// purpose : to create a new viewport as the viewwin section of vp
// returns : void
{
    int width=(vp->x2)-(vp->x1), height=(vp->y2)-(vp->y1);
    new->x1=vp->x1+(int)(vw->x1*(double)width);
    new->y1=vp->y1+(int)(vw->y1*(double)height);
    new->x2=vp->x1+(int)(vw->x2*(double)width);
    new->y2=vp->y1+(int)(vw->y2*(double)height);
}

void BorderedBox(int x1,int y1,int x2,int y2,int p,int s)
{
    _setcolor(p);
    _rectangle(_GFILLINTERIOR,x1,y1,x2,y2);
    _setcolor(s);
    _rectangle(_GBORDER,x1,y1,x2,y2);
}

void ResetPalette(void)
{
    long colors[ 16 ] =
        { _BLACK, _RED,_YELLOW-0x010101,_BLUE,_GREEN,_MAGENTA,_BROWN,_WHITE,
        _CYAN,_LIGHTRED,_LIGHTYELLOW,_LIGHTBLUE,_LIGHTGREEN,_LIGHTMAGENTA,
        _GRAY,_GRAY+0x080808};
    _remapallpalette(colors);
}

void SwapPage()
{
    static int viewpage=0;
    SetMousePage(viewpage);
    viewpage=_setvisualpage(viewpage);
    _setactivepage(viewpage);
}

void ScreenInit()
{
    _setvideomode(16); // 640x350 2 pages 16 colors
    _settextwindow(0,0,24,79);
    _setwindow(0,0.0,0.0,1.0,1.0);
    _registerfonts( "*.fon" );
    _setfont("t'Modern'");
    _setactivepage(1);
    ResetPalette();
    ResetMouse();
    _setviewport(0,0,639,349);
    SwapPage();
}

void DrawGrid(int x,int y)
{
    double at=0.0;
    double height=1.0/y;
    double width=1.0/x;
    int row,col;

    for (row=0;row<=y;row++)
    {
        _moveto_w(0.0,height*row);
        _lineto_w(1.0,height*row);
    }
    for (col=0;col<=x;col++)
    {
        _moveto_w(width*col,0.0);
        _lineto_w(width*col,1.0);
    }
}

void DrawDisc(int col,int cols,int row,int rows,int color,double discsize)
// Draws a disc in row,col, rows and cols being 0.., perfect = 0 is ellipse, else circle
{
    double colwidth=1.0/cols;
    double rowheight=1.0/rows;
    double side=discsize/2;      
    
    double wherex=(col+0.5)*colwidth; // center of grid square
    double wherey=(row+0.5)*rowheight; // center of grid square
  
    _setcolor(color);
    _ellipse_w(_GFILLINTERIOR,wherex-(side*colwidth),wherey-(side*rowheight),
                              wherex+(side*colwidth),wherey+(side*rowheight));
}

void Exit_Failure(char *message)
{
    ShutDown();
    printf("\n%s\n",message);
}

int ShutDown( void )
{
    _unregisterfonts();
    _setvideomode( _DEFAULTMODE );      /* reset the screen */
    return 1;
}

/*
void main()
{
    ScreenInit();
    DrawGrid(7,8);
    DrawDisc(3,7,3,8,15,0.75);
    SwapPage();
    getch();
    ShutDown();
}
*/

