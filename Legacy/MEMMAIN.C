/****************************************************************
Copyright 1998 the author
Module Name    : MEMMAIN
Module Purpose : to test the module MEMPAGE
Creation Date  : 17 March 1998
Updated        : None
****************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <graph.h>
#include "screen.h"
#include "mempage.h"
 

void main()
{
    MEMPAGE *mp;
    VIEWPORT vp={0,0,639,349};
    ScreenInit();
    mp=MemPageNew(&vp);
    _setactivepage(0);
    _setvisualpage(1);
    _ellipse(_GFILLINTERIOR,0,0,100,100);
    getch();
    MemPageGet(mp);
    _clearscreen(_GCLEARSCREEN);
    MemPagePut(mp);
    _setvisualpage(0);
    getch();
    ShutDown();
    
}
/*
void main()
{
    MEMPAGE *one;
    int x,y,z;
    char mask=15; // 00001111
    
    ScreenInit();
    SwapPage();
    one=MemPageNew(1,1,5,2);
    x=(int)_imagesize(0,0,639,349);
    y=(int)_imagesize(0,0,639,475);
    z=(int)_imagesize(0,0,799,599);
    _setcolor(1);
    _rectangle(_GFILLINTERIOR,0,0,639,349);
    MemPageGet(one);
    ShutDown();
    printf("%d=%d,%d=%d,%d=%d\n",640*350/2,x,640*480/2,y,800*600/2,z);
}
*/
/*
    printf("%d %d\n",(one->image[0] & (mask << 4)),(one->image[0] & mask));
    printf("%d %d\n",(one->image[1] & (mask << 4)),(one->image[1] & mask));
    printf("%d %d\n",(one->image[2] & (mask << 4)),(one->image[2] & mask));
    printf("%d %d\n",(one->image[3] & (mask << 4)),(one->image[3] & mask));
    printf("%d %d\n",(one->image[4] & (mask << 4)),(one->image[4] & mask));
    printf("%d %d\n",(one->image[5] & (mask << 4)),(one->image[5] & mask));
    printf("%d %d\n",(one->image[6] & (mask << 4)),(one->image[6] & mask));
*/
