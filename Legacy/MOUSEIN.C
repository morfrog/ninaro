/****************************************************************
Copyright 1998 the author
Module Name    : MOUSEIN
Module Purpose : to provide a level of abstraction above MOUSE
Creation Date  : 14 May 1998
Updated        : 24 May 1998    FIXED InitMouse, GetMouseInput
****************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <graph.h>
#include <i86.h>
#include "mouse.h"
#include "screen.h"
#include "defines.h"
#include "mousein.h"

int xadjust,yadjust;

int MouseX(void) {return xadjust;}
int MouseY(void) {return yadjust;}
void SetMouseX(int x) {xadjust=x;}
void SetMouseY(int y) {yadjust=y;}

int GetMouseInput(int *x,int *y)
// purpose : to get mouse input, *button==LMB or RMB
// returns : RMB if right button input found, LMB if left, NMB if none
{
    static int lmbpress=0;
    static int rmbpress=0;
    MouseStatus gmims={0};

    gmims.left=1;
    ReleaseInfo(&gmims);
    if (gmims.ButtonCount) lmbpress=1;
    gmims.left=0;
    gmims.right=1;
    ReleaseInfo(&gmims);
    if (gmims.ButtonCount) rmbpress=1;

    if (lmbpress)
    {   
        lmbpress=0;
        return LMB;
    }
    else if (rmbpress)
    {
        rmbpress=0;
        return RMB;
    }
    else
    {
        MouseStat(&gmims);
        *x=gmims.xpos-xadjust;
// DANGEROUS        if (*x<0) {xadjusy-=*x; *x
        *y=gmims.ypos-yadjust;
        return NMB;            
    }
}

int MouseCol(int col,int cols)
{
    return (col*(640/cols))+(320/cols);
}

int MouseRow(int row,int rows)
{
    return (row*(350/rows))+(175/rows);
}

int InitMouse(void)
// returns : TRUE if use mouse, else FALSE
{
    int x,y,ret=TRUE;  // mouse successfully initialised is default
    int lastx=0,lasty=0,col=WHITE;
    int pos=0;
    char bfg[80];

    xadjust=0;
    yadjust=0;
    if (!ResetMouse()) return FALSE;
    _settextalign(_LEFT,_TOP);
    _setcharsize(20,20);
    _setcolor(WHITE);
    _moveto(0,0); _lineto(20,0);
    _moveto(0,0); _lineto(0,20);
    _grtext(30,160,"Mouse Configuration Screen");
    _grtext(30,200,"Move the mouse to the very top");
    _grtext(30,240,"left corner of the screen and");
    _grtext(30,280,"press a mouse button.");
    _grtext(30,320,"Or any key to use no mouse.");
    SwapPage();
    _moveto(0,0); _lineto(20,0);
    _moveto(0,0); _lineto(0,20);
    _grtext(30,160,"Mouse Configuration Screen");
    _grtext(30,200,"Move the mouse to the very top");
    _grtext(30,240,"left corner of the screen and");
    _grtext(30,280,"press a mouse button.");
    _grtext(30,320,"Or any key to use no mouse.");
    SwapPage();
    ShowMouse();
    _setviewport(1,1,220,150);
    _setcharsize_w(0.07,0.017);
    while (NMB==GetMouseInput(&x,&y))
    {
        if (kbhit()) {while (kbhit()) {getch();} ret=FALSE; break;}
//        if ((lastx!=x) || (lasty!=y))
//        {
            HideMouse();
            _clearscreen(_GVIEWPORT);
            _moveto(pos,pos); _lineto(70+pos,70+pos);
            _moveto(pos,pos); _lineto(pos+20,pos);
            _moveto(pos,pos); _lineto(pos,pos+20);
            if (pos<1) pos=50; else pos-=1;
//            sprintf(bfg,"%10d,%10d",x,y);
//            _grtext_w(0.0,0.5,bfg);
            ShowMouse();
            SwapPage();
            delay(20);
//            lastx=x; lasty=y;
//        }
    }
    HideMouse();
    if (ret)
    {
        xadjust=x;
        yadjust=y;
    }
    _clearscreen(_GCLEARSCREEN);
    SwapPage();
    return ret;
}
