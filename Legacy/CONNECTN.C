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

#define ColAdd 3
// secondary color = primary color + ColAdd

// Rodney Rule
// Ronda Random
// Peter Prune
// Nathan Neuro
// Minny Maxx

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
int win_occured;
WIN *win;
int grid_cols;
int grid_rows;
GRID *playgrid;
GRID *entrygrid;
PLAYER **players; // array of player *
PLAYER *player_one;
PLAYER *player_two;
PLAYER *player_current;
TXTBOX *msgbox;
VIEWSYS *vs;

void NextPlayer()
{
    if (player_current==player_one)
        player_current=player_two;
    else
        player_current=player_one;
}

void Exit_Failure(char *message)
{
    ShutDown();
    printf("\n%s\n",message);
}

void DrawHuman(int primary,int secondary)
// draws a human in the current viewport
{
    _setcolor(primary);
    _ellipse_w(_GFILLINTERIOR,0.2,0.1,0.8,0.25);  //head
    _moveto_w(0.5,0.25); _lineto_w(0.5,0.7);     // body
    _setcolor(secondary);
    _moveto_w(0.5,0.7); _lineto_w(0.8,0.9);     // right leg
    _moveto_w(0.5,0.7); _lineto_w(0.2,0.9);     // left leg
    _moveto_w(0.1,0.4); _lineto_w(0.9,0.4);     // arms
}

void DrawComputer(int primary,int secondary)
// draws a computer in the current viewport
{
    _setcolor(primary);
    _rectangle_w(_GBORDER,0.1,0.1,0.9,0.65);
    _rectangle_w(_GBORDER,0.45,0.65,0.55,0.75);
    _rectangle_w(_GBORDER,0.1,0.75,0.9,0.9);
    _setcolor(secondary);
    _rectangle_w(_GFILLINTERIOR,0.2,0.2,0.8,0.55);
}

int Continuance(GRID *grid,int size)
// removes bottom layers of grid such that size-1 high minimum remains
// ( size-1 because all tokens below this are irrelevant )
// and slides the tokens down to the bottom
// returns TRUE if a continuance was made else FALSE if not.
{
    int x;
    int minimum=grid->numincol[0];
    int shift;
    for (x=1;x<grid->width;x++)
    {
        if (grid->numincol[x]<minimum) minimum=grid->numincol[x];
    }
    if (minimum<=size-1) return FALSE;
    shift=minimum-size+1;
    MoveAll(grid,0,shift,0);
    return TRUE;
}

int InsertTokenTarget(GRID *grid,int col)
// returns which row the token will fall to.
{
    return (grid->height)-(grid->numincol[col])-1;
}

int InsertToken(GRID *grid,int col,PLAYER *player)
// returns if token was successfully inserted
{
    return InsertTokenAtCR(grid,col,InsertTokenTarget(grid,col),player);
}

void DisplayPlayerColor(PLAYER *player)
{
    int pc=PlayerGetColor(player);
    _setcolor(pc+ColAdd);
    _rectangle_w(_GFILLINTERIOR,0.0,0.0,1.0,1.0);
    _setcolor(pc);
    _rectangle_w(_GFILLINTERIOR,0.15,0.15,0.85,0.85);
}

void DisplayPlayerType(PLAYER *player)
{
    int pc=PlayerGetColor(player);
    if (PlayerIsHuman(player)) DrawHuman(pc,pc+ColAdd);
    else                       DrawComputer(pc,pc+ColAdd);
}

void DisplayPlayerName(PLAYER *player)
{
    _setcolor(GREEN);
    _settextalign(_LEFT,_BOTTOM);
    _setcharspacing_w(-0.01);
    _setcharsize_w(0.04,0.03);
    _grtext_w(0.0,1.0,PlayerGetName(player));
}

void DisplayPlayerScore(PLAYER *player)
{
    char buffer[10];
    _setcolor(GREEN);
    sprintf(buffer,"%d",PlayerGetScore(player));
    _grtext_w(0.0,1.0,buffer);
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
    left=NumInDirection(grid,col,row,col_dir,row_dir,grid->grid[col][row]);
    if (left>=size) left=size-1;    // ensures token is a part of the win
    right=NumInDirection(grid,col,row,-col_dir,-row_dir,grid->grid[col][row]);
    if (right>=size) right=size-1;  // ensures token is a part of the win

    if ((left+right+1)>=size)                   // A win has been found
    {
        start_col=col+(col_dir*left);
        start_row=row+(row_dir*left);

        for (x=0;x<size;x++)
        {
            win->win[x].col=start_col+(x*-col_dir);
            win->win[x].row=start_row+(x*-row_dir);
        }
        return TRUE;
    }
    else return FALSE;
}

int CheckForWin(GRID *grid,int size,int col,int row,WIN *win)
// checks for win which includes the token at col,row
// if there isn't one 0 is returned else
// 1 is returned and the tokens which are the win are in the WIN *
{
    // check vertical    0  -1
    if (CheckAngle(grid,size,col,row,0,-1,win)) return TRUE;
    // check horizontal -1   0
    else if (CheckAngle(grid,size,col,row,-1,0,win)) return TRUE;
    // check /          +1  -1
    else if (CheckAngle(grid,size,col,row,1,-1,win)) return TRUE;
    // check \          -1  -1
    else if (CheckAngle(grid,size,col,row,1,1,win)) return TRUE;
    else return FALSE;
}

void DisplayEntryGrid(void) {_setcolor(YELLOW);DisplayGrid(entrygrid);}

void DisplayPlayGrid(void)
{
    int x;
    int col,row;
    _setcolor(GREEN);
    DisplayGrid(playgrid);

    for (x=0;x<win->tokens;x++)
    {
        col=win->win[x].col;
        row=win->win[x].row;
        DrawDisc(col,playgrid->width,row,playgrid->height,PlayerGetColor(playgrid->grid[col][row]),0.9);
    }
}

void DisplayGameName(void)
{
    _setcharspacing(-0.3);
    _settextalign(_LEFT,_BASE);
    _setcolor(LTBLUE);
    _setcharsize_w(0.1,0.04);
    _grtext_w(0.0,0.7,"Ninaro");
    _grtext_w(0.01,0.7,"Ninaro");
    _setcharsize_w(0.03,0.013);
    _grtext_w(0.65,0.25,"By the author");
    _grtext_w(0.65,0.55,"");
    _grtext_w(0.65,0.85,"   1998  ");
}

void DisplayMsgBox(void) {TextBoxShow(msgbox);}

void DisplayP1Color(void) {DisplayPlayerColor(player_one);}
void DisplayP1Type(void) {DisplayPlayerType(player_one);}
void DisplayP1Name(void) {DisplayPlayerName(player_one);}
void DisplayP1Score(void) {DisplayPlayerScore(player_one);}
void DisplayP2Color(void) {DisplayPlayerColor(player_two);}
void DisplayP2Type(void) {DisplayPlayerType(player_two);}
void DisplayP2Name(void) {DisplayPlayerName(player_two);}
void DisplayP2Score(void) {DisplayPlayerScore(player_two);}

VIEWSYS *ViewSysInit(int maxwindows,int maxkeys,int x1,int y1,int x2,int y2)
{
    VIEWSYS *vs=ViewSysNew(x1,y1,x2,y2,maxwindows,maxkeys);
    ViewSysNewWindow(vs,0.0,0.0,0.6,0.09,ACTIVE,
        DisplayEntryGrid,NULL,NULL,"Enter Your Token Here");
    ViewSysNewWindow(vs,0.0,0.1,0.6,1.0,ACTIVE,
        DisplayPlayGrid,NULL,NULL,"Enter Your Token Here");
    ViewSysNewWindow(vs,0.61,0.0,1.0,0.1,ACTIVE,
        DisplayGameName,NULL,NULL,"Ninaro - By the author 1998");
    ViewSysNewWindow(vs,0.61,0.1,1.0,0.4,ACTIVE,
        DisplayMsgBox,NULL,NULL,"Messages appear here");
    ViewSysNewWindow(vs,0.61,0.4,0.65,0.448,ACTIVE,
        DisplayP1Color,NULL,NULL,"Player One's Color");
    ViewSysNewWindow(vs,0.65,0.4,0.70,0.448,ACTIVE,
        DisplayP1Type,NULL,NULL,"Player One's Type");
    ViewSysNewWindow(vs,0.7,0.4,0.96,0.448,ACTIVE,
        DisplayP1Name,NULL,NULL,"Player One's Name");
    ViewSysNewWindow(vs,0.96,0.4,1.0,0.448,ACTIVE,
        DisplayP1Score,NULL,NULL,"Player One's Score");
    ViewSysNewWindow(vs,0.61,0.452,0.65,0.5,ACTIVE,
        DisplayP2Color,NULL,NULL,"Player Two's Color");
    ViewSysNewWindow(vs,0.65,0.452,0.7,0.5,ACTIVE,
        DisplayP2Type,NULL,NULL,"Player Two's Type");
    ViewSysNewWindow(vs,0.7,0.452,0.96,0.5,ACTIVE,
        DisplayP2Name,NULL,NULL,"Player Two's Name");
    ViewSysNewWindow(vs,0.96,0.452,1.0,0.5,ACTIVE,
        DisplayP2Score,NULL,NULL,"Player Two's Score");
    return vs;
}

void main()
{
    int ever=1,keypos=0,keyrow;
    
    win=malloc(sizeof(WIN));
    win->tokens=4;
    win->win=malloc(win->tokens*sizeof(TOKEN));
    numplayers=7;
    win_occured=FALSE;  
    grid_rows=8;
    grid_cols=7;


    vs=ViewSysInit(20,20,0,0,639,349);

    msgbox=TextBoxNew(5,RED,WHITE,40);

    players=malloc(numplayers*sizeof(PLAYER *));
    players[0]=PlayerNew("Player1",YELLOW,Human,Low);
    players[1]=PlayerNew("Player2",RED,Human,Low);
    players[2]=PlayerNew("Ronda Random",LTBLUE,Computer,Low);
    players[3]=PlayerNew("Alfa B Prune",GREEN,Computer,Medium);
    players[4]=PlayerNew("Nathan Neuro",BLUE,Computer,High);
    players[5]=PlayerNew("Rodney Rule",ORANGE,Computer,Medium);
    players[6]=PlayerNew("Minny Maxx",PURPLE,Computer,Medium);
    player_one=players[0];
    player_two=players[2];
    player_current=player_one;

    TextBoxAddLine(msgbox,"Player1 Arrives",ATEND);
    TextBoxAddLine(msgbox,"Ronda Random Arrives",ATEND);
    TextBoxAddLine(msgbox,"Let The Game Begin",ATEND);
    TextBoxAddLine(msgbox,"This is the first line",ATSTART);
    TextBoxScrollEnd(msgbox);
                
    playgrid=NewGrid(grid_cols,grid_rows);
    entrygrid=NewGrid(grid_cols,1);

    InsertTokenAtCR(entrygrid,keypos,0,player_current);

    ScreenInit();
    _setcolor(RED);
    _setcharsize_w(0.02,0.02);
    _setcharspacing_w(-(0.01));
    _registerfonts( "*.fon" );
    _setfont("t'Modern'");
    _settextalign(_CENTER,_HALF);
    
    ViewSysDisplay(vs,DisplayAll);
    SwapPage();
    ViewSysDisplay(vs,DisplayAll);
    SwapPage();

    for (;ever;)
    {
        if (kbhit())
        {
            switch (GetKey())
            {
                case ESC   : ever=0; break;
                case SPACE :
                case ENTER :
                case KP2 :
                    keyrow=InsertTokenTarget(playgrid,keypos);
                    if (InsertToken(playgrid,keypos,player_current))
                    {
                        RemoveTokenAtCR(entrygrid,keypos,0);
                        NextPlayer();
                        InsertTokenAtCR(entrygrid,keypos,0,player_current);
                        ViewSysDisplay(vs,DisplayAll);
                        SwapPage();

                        if (CheckForWin(playgrid,win->tokens,keypos,keyrow,win))
                        {
                            PlayTune(4,100,50,300,50,500,100,400,5);
                            win_occured=TRUE;
                            ViewSysDisplay(vs,DisplayAll);
                            SwapPage();
                        }
                    }
                    else // already full so entry not possible
                    {
                        Buzz(200,400,20,100);
                    }
                    break;
                case KP4 :
                    if (MoveTokenAtCR(entrygrid,keypos,0,T_LEFT)) --keypos;
                    ViewSysDisplay(vs,DisplayAll);
                    SwapPage();
                    break;
                case KP6 :
                    if (MoveTokenAtCR(entrygrid,keypos,0,T_RIGHT)) ++keypos;
                    ViewSysDisplay(vs,DisplayAll);
                    SwapPage();
                    break;
                case 'c' : Continuance(playgrid,win->tokens);
                    ViewSysDisplay(vs,DisplayAll);
                    SwapPage();
                    break;
            } // switch (GetKey)
        } // if kbhit
    } // for (;ever;)
    ShutDown();
}
