#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <graph.h>
#include <i86.h>
#include "mouse.h"
#include "sound.h"
#include "txtbox.h"
#include "viewsys.h"
#include "sys.h"
#include "screen.h"
#include "grid.h"
#include "player.h"

// Rodney Rule      
// Ronda Random
// Peter Prune
// Nathan Neuro

struct TokenTag
{
    int row;
    int col;
};

typedef struct TokenTag TOKEN;

struct winTag
{
    int tokens;
    TOKEN *win; // array of tokens which make up the win
};

typedef struct winTag WIN;

int numplayers;
GRID *playgrid;
GRID *entrygrid;
PLAYER *players; // array of player
TXTBOX *msgbox;
VIEWSYSTEM *vs;

void DrawHuman()
// draws a human in the current viewport
{
    _ellipse_w(_GFILLINTERIOR,0.2,0.0,0.8,0.2);  //head
    _moveto_w(0.5,0.2); _lineto_w(0.5,0.8);     // body
    _moveto_w(0.5,0.8); _lineto_w(0.8,1.0);     // right leg
    _moveto_w(0.5,0.8); _lineto_w(0.2,1.0);     // left leg
    _moveto_w(0.0,0.4); _lineto_w(1.0,0.4);     // arms
}

void DrawComputer()
// draws a computer in the current viewport
{
    _rectangle_w(_GBORDER,0.0,0.0,1.0,0.65);
    _rectangle_w(_GBORDER,0.45,0.65,0.55,0.7);
    _rectangle_w(_GBORDER,0.0,0.7,1.0,1.0);
}

int Continuance(GRID *grid,int size)
// removes bottom layers of grid such that size-1 high minimum remains
// ( size-1 because all tokens below this are irrelevant )
// and slides the tokens down to the bottom
// returns 1 if a continuance was made else 0 if not.
{
    int x;
    int minimum=grid->numincol[0];
    int shift;
    for (x=1;x<grid->width;x++)
    {
        if (grid->numincol[x]<minimum) minimum=grid->numincol[x];
    }
    if (minimum<=size-1) return 0;
    shift=minimum-size+1;
    MoveAll(grid,0,shift,0);
    return 1;
}

int InsertTokenTarget(GRID *grid,int col)
// returns which row the token will fall to.
{
    return (grid->height)-(grid->numincol[col])-1;
}

int InsertToken(GRID *grid,int col,int player)
// returns if token was successfully inserted
{
    return InsertTokenAtCR(grid,col,InsertTokenTarget(grid,col),player);
}

void ShowPlayer(PLAYER *player)
// shows the player in the current viewport
{
    char buffer[20];
    double tmp;
    //                             start // player level
    ViewSysSubLevel(vs,0.0,0.0,0.1,1.0); // color section
    _setcolor(GetPlayerColor(player));
    _rectangle_w(_GFILLINTERIOR,0.0,0.0,1.0,1.0);
    _setcolor(GREEN);
    _rectangle_w(_GBORDER,0.0,0.0,1.0,1.0);
    ViewSysUpLevel(vs);                 // player level

    tmp=ViewSysPixelsW(vs,2);
    ViewSysSubLevel(vs,0.1+tmp,0.2,0.2-tmp,0.8); // icon section
    _setcolor(GetPlayerColor(player));
    if (IsHuman(player))
        DrawHuman();
    else
        DrawComputer();
    ViewSysUpLevel(vs);                         // player level

    ViewSysSubLevel(vs,0.2,0.0,0.85,1.0);        // player name level
    _setcolor(GREEN);
    _settextalign(_LEFT,_BOTTOM);
    _setcharspacing_w(-0.01);
    _setcharsize_w(0.04,0.03);
    _grtext_w(0.0,1.0,GetPlayerName(player));

    ViewSysUpLevel(vs);                         // player level
    ViewSysSubLevel(vs,0.85+ViewSysPixelsW(vs,2),0.0,1.0,1.0);        // score level
    sprintf(buffer,"%d",GetPlayerScore(player));
    _grtext_w(0.0,1.0,buffer);

    ViewSysUpLevel(vs);                         // player level
 
}

void Debug(char *string)
{
    _setviewport(500,100,600,200);
    _clearscreen(_GVIEWPORT);
    _grtext_w(0.5,0.5, string);
}

int CheckAngle(GRID *grid,int size,int col,int row,int col_dir,int row_dir,WIN *win)
{
    int left,right,start_col,start_row,x;
    left=NumInDirection(grid,col,row,col_dir,row_dir);
    if (left>=size) left=size-1;    // ensures token is a part of the win
    right=NumInDirection(grid,col,row,-col_dir,-row_dir);
    if (right>=size) right=size-1;  // ensures token is a part of the win

    if ((left+right+1)>=size)                   // A win has been found
    {
        start_col=col+(col_dir*left);
        start_row=row+(row_dir*left);

        win->tokens=size;
        for (x=0;x<size;x++)
        {
            win->win[x].col=start_col+(x*-col_dir);
            win->win[x].row=start_row+(x*-row_dir);
        }
        return 1;
    }
    else return 0;
}

int CheckForWin(GRID *grid,int size,int col,int row,WIN *win)
// checks for win which includes the token at col,row
// if there isn't one 0 is returned else
// 1 is returned and the tokens which are the win are in the WIN *
{
    // check vertical    0  -1
    if (CheckAngle(grid,size,col,row,0,-1,win)) return 1;
    // check horizontal -1   0
    else if (CheckAngle(grid,size,col,row,-1,0,win)) return 1;
    // check /          +1  -1
    else if (CheckAngle(grid,size,col,row,1,-1,win)) return 1;
    // check \          -1  -1
    else if (CheckAngle(grid,size,col,row,1,1,win)) return 1;
    else return 0;
}

void RefreshScreen()
{
    _clearscreen(_GCLEARSCREEN); // start with cleared screen
    ViewSysHome(vs);
    ViewSysSubLevel(vs,0.0,0.0,0.6,1.0);    // left side of screen
    _setcolor(GREEN);
    _rectangle(_GFILLINTERIOR,0.0,0.0,1.0,1.0);

    ViewSysSubLevel(vs,0.0,0.0,1.0,0.1);    // entry grid
    _setcolor(YELLOW);
    DisplayGrid(entrygrid);

    ViewSysUpLevel(vs);                     // left side of screen
    ViewSysSubLevel(vs,0.0,0.1+ViewSysPixelsH(vs,2),1.0,1.0); // play grid
    _setcolor(GREEN);
    DisplayGrid(playgrid);

    ViewSysHome(vs);
    ViewSysSubLevel(vs,0.6+ViewSysPixelsW(vs,2),0.0,1.0,1.0); // right side of screen    
    
    ViewSysSubLevel(vs,0.0,0.0,1.0,0.1);    // ninaro name window
    _setcharspacing(-0.3);
    _settextalign(_LEFT,_BASE);
    _setcolor(BLUE);
    _setcharsize_w(0.1,0.04);
    _grtext_w(0.0,0.7,"Ninaro");
    _grtext_w(0.01,0.7,"Ninaro");
    _setcharsize_w(0.03,0.013);
    _grtext_w(0.65,0.25,"By the author");
    _grtext_w(0.65,0.55,"");
    _grtext_w(0.65,0.85,"   1998  ");


    ViewSysAcrossLevel(vs,0.0,0.1,1.0,0.5);
    TextBoxShow(msgbox);

    ViewSysAcrossLevel(vs,0.0,0.5,1.0,0.55);
    ShowPlayer(&(players[0]));
    ViewSysAcrossLevel(vs,0.0,0.55,1.0,0.6);
    ShowPlayer(&(players[1]));
    ViewSysHome(vs);
}

void ShowWin(GRID *grid,WIN *win)
{
    int x;
    int col,row;
    int active=_getactivepage();
    _setactivepage(_getvisualpage());
    ViewSysHome(vs);
    ViewSysSubLevel(vs,0.0,0.0,0.6,1.0);    // left
    ViewSysSubLevel(vs,0.0,0.1+ViewSysPixelsH(vs,2),1.0,1.0);   // playgrid

    for (x=0;x<win->tokens;x++)
    {
        col=win->win[x].col;
        row=win->win[x].row;
        DrawDisc(col,grid->width,row,grid->height,grid->player[grid->grid[col][row]-1],0.90);
        delay(300);
    }
    _setactivepage(active);
}

void main()
{
    int ever=1,keypos=0,cur_player=1,keyrow;
    int size=4;
    int grid_width=7;
    int grid_height=8;
    
    WIN *win=malloc(sizeof(WIN));
    win->win=malloc(size*sizeof(TOKEN));
    
    numplayers=5;

    vs=ViewSysInit(10,0,0,639,349);

    msgbox=TextBoxNew(6,RED,WHITE,40);

    players=malloc(numplayers*sizeof(PLAYER));

    ResetPlayer(&(players[0]),"Player1",YELLOW,1);
    TextBoxAddLine(msgbox,"Player1 Arrives",ATEND);

    ResetPlayer(&(players[1]),"Ronda Random",RED,0);
    TextBoxAddLine(msgbox,"Ronda Random Arrives",ATEND);
    
    ResetPlayer(&(players[2]),"Pam Prune",GREEN,0);
    ResetPlayer(&(players[2]),"Nathan Neuro",BLUE,0);
    ResetPlayer(&(players[2]),"Rodney Rule",ORANGE,0);
    
    TextBoxAddLine(msgbox,"Let The Game Begin",ATEND);
    TextBoxAddLine(msgbox,"This is the first line",ATSTART);
    TextBoxScrollEnd(msgbox);
                
    playgrid=NewGrid(grid_width,grid_height,numplayers);
    entrygrid=NewGrid(grid_width,1,numplayers);

    SetPlayerColor(playgrid,1,players[0].color);
    SetPlayerColor(playgrid,2,players[1].color);
    SetPlayerColor(entrygrid,1,players[0].color);
    SetPlayerColor(entrygrid,2,players[1].color);

    InsertTokenAtCR(entrygrid,keypos,0,cur_player);

    ScreenInit();
    _setcolor(RED);
    _setcharsize_w(0.02,0.02);
    _setcharspacing_w(-(0.01));
    _registerfonts( "*.fon" );
    _setfont("t'Modern'");
    _settextalign(_CENTER,_HALF);
    
    RefreshScreen();
    SwapPage();    

    for (;ever;)
    {

        if (kbhit())
        {
            switch (getch())
            {
                case 27 : ever=0; break;
                case ' ':
                case 13 :
                           keyrow=InsertTokenTarget(playgrid,keypos);
                           if (InsertToken(playgrid,keypos,cur_player))
                           {
                               if (cur_player==1)
                                cur_player=2;
                               else
                                cur_player=1;
                               RemoveTokenAtCR(entrygrid,keypos,0);
                               InsertTokenAtCR(entrygrid,keypos,0,cur_player);
                               RefreshScreen();
                               SwapPage();

                               if (CheckForWin(playgrid,size,keypos,keyrow,win))
                               {
                                    PlayTune(4,100,50,300,50,500,100,400,5);
                                    ShowWin(playgrid,win);
                               }
                            }
                            else // already full so entry not possible
                            {
                                Buzz(200,400,20,100);
                            }
                           break;
                case 0  : switch(getch())
                            {
                                case 75 : if (MoveTokenAtCR(entrygrid,keypos,0,T_LEFT)) --keypos; break;
                                case 77 : if (MoveTokenAtCR(entrygrid,keypos,0,T_RIGHT)) ++keypos; break;
                            }
                            RefreshScreen();
                            SwapPage();

                            break;
                case 'c' : Continuance(playgrid,size);
                            RefreshScreen();
                            SwapPage();
                            break;
            }
        }
    }
    ShutDown();
}
