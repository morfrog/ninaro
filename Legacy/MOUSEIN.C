#include <stdlib.h>
#include <stdio.h>
#include <graph.h>
#include "mouse.h"
#include "screen.h"
#include "defines.h"
#include "mousein.h"

int xadjust,yadjust;

int MouseX(int x)
{
//    int ret;
//    if ((x<0) || (x>640)) {ret=(x)%640; if (ret<0) ret+=640; return ret;}
//    else {ret=x%640; if (ret<0) ret+=640; return ret;}
    return x;
}

int MouseY(int y)
{
//    int ret;
//    if ((y<0) || (y>350)) {ret=(y-60)%350; if (ret<0) ret+=350; return ret;}
//    else {ret=y%350; if (ret<0) ret+=350; return ret;}
    return y;
}


int GetMouseInput(int *x,int *y)
// purpose : to get mouse input, *button==LMB or RMB
// returns : RMB if right button input found, LMB if left, NMB if none
{
    static int lmbpress=0;
    static int rmbpress=0;
    static int lastx,lasty;
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

/*
int GetMouseInput(int *x,int *y)
// purpose : to get mouse input, *button==LMB or RMB
// returns : RMB if right button input found, LMB if left, NMB if none
{
    static int lmbpress=0;
    static int lmbavailable=0;
    static int rmbpress=0;
    static int rmbavailable=0;
    MouseStatus gmims={0};
    
    MouseStat(&gmims);
    gmims.left=1;
    PressInfo(&gmims);
    if (gmims.ButtonCount) ++lmbpress;
    gmims.left=1;
    ReleaseInfo(&gmims);
    if (gmims.ButtonCount) {--lmbpress; ++lmbavailable;}
    gmims.left=0;
    gmims.right=1;
    PressInfo(&gmims);
    if (gmims.ButtonCount) ++rmbpress;
    gmims.left=0;
    gmims.right=1;
    ReleaseInfo(&gmims);
    if (gmims.ButtonCount) {--rmbpress; ++rmbavailable;}

    if (lmbavailable)
    {   
        MouseStat(&gmims);
        *x=gmims.xpos-37683200;
        *y=gmims.ypos-8847360;
        --lmbavailable;
        return LMB;
    }
    else if (rmbavailable)
    {
        MouseStat(&gmims);
        *x=gmims.xpos-37683200;
        *y=gmims.ypos-8847360;
        --rmbavailable;
        return RMB;
    }
    else
    {
        MouseStat(&gmims);
        *x=gmims.xpos-37683200;
        *y=gmims.ypos-8847360;
        return NMB;
    }
}
*/

void InitMouse(void)
{
    int x,y,thisx,thisy;
    char bfg[80];
    xadjust=0;
    yadjust=0;
    ResetMouse();
    ShowMouse();
    while (LMB!=GetMouseInput(&x,&y))
    {
        _clearscreen(_GCLEARSCREEN);
        _grtext(30,200,"Move the mouse to the top left");
        _grtext(30,240,"corner of the screen and press");
        _grtext(30,280,"the left mouse button.");
        sprintf(bfg,"%10d,%10d",x,y);
        _grtext(30,100,bfg);
        SwapPage();
    }
    xadjust=x;
    yadjust=y;
    _clearscreen(_GCLEARSCREEN);
    SwapPage();
    HideMouse();
}
