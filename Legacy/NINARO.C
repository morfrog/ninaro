// will have to use DisplayAll until a mechanism is worked out that will
// enable the display of changed windows to appear properly without destroying
// the background - in hopes that one day the background will be a picture,
// not just an inconsequential black emptiness
// could store the background in a page I create, then switch that relevant
// section into the changed window and restore the changed window on top...
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <graph.h>
#include <i86.h>
#include "sys.h"
#include "sound.h"
#include "txtbox.h"
#include "viewsys.h"
#include "menu.h"
#include "screen.h"
#include "grid.h"
#include "player.h"
#include "connectn.h"

#define ColAdd 3
// secondary color = primary color + ColAdd

// Rodney Rule
// Ronda Random
// Peter Prune
// Nathan Neuro
// Minny Maxx

int numplayers;
int win_occured;
WIN *win;
int grid_cols;
int grid_rows;
int keypos;     // the column at which the keys <- & -> place the token
GRID *playgrid;
GRID *entrygrid;
PLAYER **players; // array of player *
PLAYER *player_one; 
PLAYER *player_two; 
PLAYER *player_current;
TXTBOX *msgbox;
VIEWSYS *vs;
MENU *mainmenu;

void SetP1Current()
{
    int type=PlayerGetType(player_one);
    player_current=player_one;

    // disable irrelevant keys
    ViewSysKeyEnableGroup(vs,DISABLE,4,0,1,2,3);
    if (type==Human) ViewSysKeyEnableGroup(vs,ENABLE,4,4,5,6,7);
    else             ViewSysKeyEnableGroup(vs,DISABLE,4,4,5,6,7);
                    // is Computer so disable all keys
}

void SetP2Current()
{
    int type=PlayerGetType(player_two);
    player_current=player_two;

    // disable irrelevant keys
    ViewSysKeyEnableGroup(vs,DISABLE,4,4,5,6,7);
    if (type==Human) ViewSysKeyEnableGroup(vs,ENABLE,4,0,1,2,3);
    else             ViewSysKeyEnableGroup(vs,DISABLE,4,0,1,2,3);
                    // is Computer so disable all keys
}

void NextPlayer()
{
    if (player_current==player_one) SetP2Current();
    else                            SetP1Current();
}

void DisplayHuman(int primary,int secondary)
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

void DisplayComputer(int primary,int secondary)
// draws a computer in the current viewport
{
    _setcolor(primary);
    _rectangle_w(_GBORDER,0.1,0.1,0.9,0.65);
    _rectangle_w(_GBORDER,0.45,0.65,0.55,0.75);
    _rectangle_w(_GBORDER,0.1,0.75,0.9,0.9);
    _setcolor(secondary);
    _rectangle_w(_GFILLINTERIOR,0.2,0.2,0.8,0.55);
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
    if (PlayerGetType(player)==Human) DisplayHuman(pc,pc+ColAdd);
    else                       DisplayComputer(pc,pc+ColAdd);
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

void DisplayEntryGrid(void) {_setcolor(YELLOW);DisplayGrid(entrygrid);}

void DisplayPlayGrid(void)
{
    int x;
    int col,row;
    _setcolor(GREEN);
    DisplayGrid(playgrid);
    if (win_occured)
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

void ChangePlayerType(PLAYER *player)
{
    if (PlayerGetType(player)==Human) PlayerSetType(player,Computer);
    else                              PlayerSetType(player,Human);
}
void IncPlayerColor(PLAYER *player) {PlayerChangeColor(player,1);}
void DecPlayerColor(PLAYER *player) {PlayerChangeColor(player,-1);}

void ChangeP1Type(void) {ChangePlayerType(player_one);}
void ChangeP2Type(void) {ChangePlayerType(player_two);}
void IncP1Color(void) {IncPlayerColor(player_one);}
void IncP2Color(void) {IncPlayerColor(player_two);}
void DecP1Color(void) {DecPlayerColor(player_one);}
void DecP2Color(void) {DecPlayerColor(player_two);}

void ClearScreen(void) {_clearscreen(_GCLEARSCREEN);}
void ClearViewport(void) {_clearscreen(_GVIEWPORT);}

int KeyUp(void)
{
    MenuCall(mainmenu,vs->current);
    return TRUE;
}

int KeyLeft(void)
{
    if (MoveTokenAtCR(entrygrid,keypos,0,T_LEFT))
        {--keypos; return DisplayAll;}
        // 0 is entry grid
    else
        return NoDisplay;
}

int KeyRight(void)
{
    if (MoveTokenAtCR(entrygrid,keypos,0,T_RIGHT))
        {++keypos; return DisplayAll;}
        // 0 is entry grid
    else
        return NoDisplay;
}

int KeyDown(void)
{
    switch(InsertTokenInCol(playgrid,keypos,player_current,win))
    {
        case Success :
            RemoveTokenAtCR(entrygrid,keypos,0);
            NextPlayer();
            InsertTokenAtCR(entrygrid,keypos,0,player_current);
            return DisplayAll;
            break;
        case Win :
            PlayTune(4,100,50,300,50,500,100,400,5);
            win_occured=TRUE;
            RemoveTokenAtCR(entrygrid,keypos,0);
            NextPlayer(); // should be removed from proper system, though
                          // could be used for loser goes first
            InsertTokenAtCR(entrygrid,keypos,0,player_current);
            return DisplayAll;
            break;
        case Failure :
            Buzz(200,400,20,100);
            return NoDisplay;
            break;
    }
    Buzz(100,500,20,200);
    return NoDisplay; // should never happen
}

VIEWSYS *ViewSysInit(int x1,int y1,int x2,int y2,int maxwindows,int maxkeys)
{
    VIEWSYS *vs=ViewSysNew(x1,y1,x2,y2,maxwindows,maxkeys,ViewSysClear);
    ViewSysWindowNew(vs,0.0,0.0,0.6,0.09,ACTIVE,
        DisplayEntryGrid,"Enter Your Token Here");      // entrygrid is 0
    ViewSysWindowNew(vs,0.0,0.1,0.6,1.0,ACTIVE,
        DisplayPlayGrid,"Enter Your Token Here");       // playgrid is 1
    ViewSysWindowNew(vs,0.61,0.0,1.0,0.1,ACTIVE,
        DisplayGameName,"Ninaro - By the author 1998");
    ViewSysWindowNew(vs,0.61,0.1,1.0,0.4,ACTIVE,
        DisplayMsgBox,"Messages appear here");
    ViewSysWindowNew(vs,0.61,0.4,0.65,0.448,ACTIVE,
        DisplayP1Color,"Player One's Color");
    ViewSysWindowNew(vs,0.65,0.4,0.70,0.448,ACTIVE,
        DisplayP1Type,"Player One's Type");
    ViewSysWindowNew(vs,0.7,0.4,0.96,0.448,ACTIVE,
        DisplayP1Name,"Player One's Name");
    ViewSysWindowNew(vs,0.96,0.4,1.0,0.448,ACTIVE,
        DisplayP1Score,"Player One's Score");
    ViewSysWindowNew(vs,0.61,0.452,0.65,0.5,ACTIVE,
        DisplayP2Color,"Player Two's Color");
    ViewSysWindowNew(vs,0.65,0.452,0.7,0.5,ACTIVE,
        DisplayP2Type,"Player Two's Type");
    ViewSysWindowNew(vs,0.7,0.452,0.96,0.5,ACTIVE,
        DisplayP2Name,"Player Two's Name");
    ViewSysWindowNew(vs,0.96,0.452,1.0,0.5,ACTIVE,
        DisplayP2Score,"Player Two's Score");
    return vs;
}

void Test(void)
{
    Buzz(800,900,50,400);
}

void ExitGame(void) {ShutDown(); ProgFree(); exit(0);}

void ProgInit() // initialises global variables
{
    win=malloc(sizeof(WIN));
    win->tokens=4;
    win->win=malloc(win->tokens*sizeof(TOKEN));
    numplayers=7;
    win_occured=FALSE;  
    grid_rows=8;
    grid_cols=7;

    vs=ViewSysInit(0,0,639,349,20,20);

    ViewSysKeyAdd(vs,KP4,KeyLeft,DISABLE);  // move token left
    ViewSysKeyAdd(vs,KP6,KeyRight,DISABLE); // move token right
    ViewSysKeyAdd(vs,KP8,KeyUp,DISABLE);    // call menu
    ViewSysKeyAdd(vs,KP2,KeyDown,DISABLE);  // drop token
    ViewSysKeyAdd(vs,'a',KeyLeft,DISABLE);  // move token left
    ViewSysKeyAdd(vs,'d',KeyRight,DISABLE); // move token right
    ViewSysKeyAdd(vs,'w',KeyUp,DISABLE);    // call menu
    ViewSysKeyAdd(vs,'s',KeyDown,DISABLE);  // drop token
    ViewSysKeyAdd(vs,ESC,KeyUp,ENABLE);     // call menu

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
    player_two=players[1];
    SetP1Current();

    TextBoxAddLine(msgbox,"Player1 Arrives",ATEND);
    TextBoxAddLine(msgbox,"Ronda Random Arrives",ATEND);
    TextBoxAddLine(msgbox,"This is the first line",ATSTART);
    TextBoxAddLine(msgbox,"Let The Game Begin",ATEND);
    TextBoxScrollEnd(msgbox);
                
    playgrid=NewGrid(grid_cols,grid_rows);
    entrygrid=NewGrid(grid_cols,1);

    mainmenu=MenuNew(4,&(vs->main),ViewSysCurrent(vs));
    MenuNewSub(mainmenu,"Game",2,"New",CN,Test,ItemEnabled,
                             "Exit",CX,ExitGame,ItemEnabled);
    MenuNewSub(mainmenu,"Board",3,"7x8x4",NONE,NULL,ItemEnabled,
                              "12x13x6",NONE,NULL,ItemEnabled, 
                              "Custom...",NONE,NULL,ItemEnabled);
    MenuNewSub(mainmenu,"Player",2,"One...",NONE,NULL,ItemEnabled,
                               "Two...",NONE,NULL,ItemDisabled);
    MenuNewSub(mainmenu,"Help",2,"Contents",NONE,NULL,ItemEnabled,
                             "About",NONE,NULL,ItemEnabled);                              
    MenuKeysSet(mainmenu,KP4,KP6,KP8,KP2,ENTER,ESC);
}

void ProgFree() // frees global variables
{
    int x;
    printf("Freeing Memory...\n");
    free(win->win);
    free(win);
    printf("Freeing ViewSystem...\n");
    ViewSysFree(vs);
    printf("Freeing Players...\n");
    for (x=0;x<numplayers;x++)
        free(players[x]);
    free(players);
    printf("Freeing Textbox...\n");
    TextBoxFree(msgbox);
    printf("Freeing Grids...\n");
    GridFree(playgrid);
    GridFree(entrygrid);
    printf("Freeing Menu...");
    MenuFree(mainmenu);
}

void main()
{
    int ever=1,keyused=FALSE,key;
    keypos=0;

    ScreenInit();
    _setcolor(RED);
    _setcharsize_w(0.02,0.02);
    _setcharspacing_w(-(0.01));
    _registerfonts( "*.fon" );
    _setfont("t'Modern'");
    _settextalign(_CENTER,_HALF);
    ProgInit(); // some parts rely on graphics mode
    InsertTokenAtCR(entrygrid,keypos,0,player_current);

    ViewSysDisplay(vs,DisplayAll);
    SwapPage();
//    ViewSysDisplay(vs,DisplayAll);
//    SwapPage();
    
    for (;ever;)
    {
        if (kbhit())
        {
            key=GetKey();
            switch (key)
            {
                case CF10 : case CQ : case CX : ever=0; break;
                case 'c' :
                    if (Continuance(playgrid,win->tokens))
                    {
                        ViewSysWindowChanged(vs,1,CHANGED);
                        ViewSysDisplay(vs,DisplayChanged);
                        SwapPage();
                    }
                    break;
                default :
                    if (ViewSysDisplay(vs,ViewSysKey(vs,key,&keyused)))
                        SwapPage();
                    break;
            } // switch (GetKey)
        } // if kbhit
    } // for (;ever;)
    ShutDown();
    ProgFree();
}
