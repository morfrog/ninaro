#include <stdlib.h>
#include <stdio.h>
#include <graph.h>

void ScreenInit()
{
    _setvideomode(16); // 640x350 2 pages 16 colors
    _settextwindow(0,0,24,79);
    _setwindow(0,0.0,0.0,1.0,1.0);
    _registerfonts( "*.fon" );
    _setfont("t'Modern'");
    _setactivepage(0);
    _setvisualpage(1);
    _setviewport(0,0,639,349);
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

void SwapPage()
{
    static int viewpage;
    viewpage=_setvisualpage(viewpage);
    _setactivepage(viewpage);
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

