/****************************************************************
Copyright 1998 the author
Module Name    : IMAGE
Module Purpose : to explore imaging features of C
Creation Date  : 
Updated        : 
****************************************************************/
#include <stdlib.h>
#include <graph.h>
#include <conio.h>
#include "screen.h"

void main()
{
    char *image;
    ScreenInit();
    image=malloc(_imagesize(0,0,639,349));
    _setactivepage(0);
    _ellipse(_GFILLINTERIOR,0,0,100,100);
    _setactivepage(1);
    _ellipse(_GFILLINTERIOR,400,200,500,300);
    _setvisualpage(0);
    // vis is tl, act is br
    _getimage(0,0,639,349,image); // save the image - from active page
    _setcolor(2);
    _setactivepage(1);_setvisualpage(0);
    _rectangle(_GFILLINTERIOR,0,0,639,349); // clear both
    _setactivepage(0);_setvisualpage(1);
    _rectangle(_GFILLINTERIOR,0,0,639,349);
    _putimage(0,0,image,_GPSET); // on to active page
    _setvisualpage(1);
    getch();
    _setvisualpage(0);
    getch();
    ShutDown();
}
