/****************************************************************
Copyright 1998 the author
Module Name    : COLOR
Module Purpose : to test colors
Creation Date  : 7 April 1998
Updated        : None
****************************************************************/
#include <graph.h>
#include "sys.h"
#include "screen.h"

void main()
{

int x;
long colors[ 16 ] ={_BLACK, _RED, _BROWN, _GREEN,
                    _GRAY, _LIGHTRED, _LIGHTGREEN, _YELLOW,
                    _BLUE, _MAGENTA, _CYAN, _WHITE,
                    _LIGHTBLUE, _LIGHTMAGENTA, _LIGHTCYAN, _BRIGHTWHITE};
    ScreenInit();
//    _remapallpalette(colors);
    for (x=0;x<16;x++)
    {
        _setcolor(x);
        _rectangle(_GFILLINTERIOR,0,x*5,100,(x+1)*5);
    }
    SwapPage();
    GetKey();
    ShutDown();
}
