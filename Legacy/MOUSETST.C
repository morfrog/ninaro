#include <stdlib.h>
#include <stdio.h>
#include <graph.h>
#include "screen.h"
#include "mouse.h"
#include "mousein.h"

void main()
{
    char bfg[50];
    int x,y,btn;
    ScreenInit();
    ShowMouse();
    _setcharsize(20,20);
    _settextalign(_LEFT,_TOP);
    _setcolor(GREEN);
    
    SetMousePos(400,200);
    for (;;)
    {
        btn=GetMouseInput(&x,&y);
        if (btn==RMB) exit(0);
        else // if (btn==LMB)
        {
            _clearscreen(_GCLEARSCREEN);
            sprintf(bfg,"%d,%d",x,y);
            _grtext(30,100,bfg);
            SwapPage();
        }
    }
}
