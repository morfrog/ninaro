#include <stdlib.h>
#include <stdio.h>
#include <graph.h>
#include "screen.h"
#include "mempage.h"
 
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
/*
    printf("%d %d\n",(one->image[0] & (mask << 4)),(one->image[0] & mask));
    printf("%d %d\n",(one->image[1] & (mask << 4)),(one->image[1] & mask));
    printf("%d %d\n",(one->image[2] & (mask << 4)),(one->image[2] & mask));
    printf("%d %d\n",(one->image[3] & (mask << 4)),(one->image[3] & mask));
    printf("%d %d\n",(one->image[4] & (mask << 4)),(one->image[4] & mask));
    printf("%d %d\n",(one->image[5] & (mask << 4)),(one->image[5] & mask));
    printf("%d %d\n",(one->image[6] & (mask << 4)),(one->image[6] & mask));
*/
