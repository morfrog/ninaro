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
#include "controls.h"
#include "connectn.h"

#define ColAdd 3
// secondary color = primary color + ColAdd

// Rodney Rule
// Ronda Random
// Peter Prune
// Nathan Neuro
// Minny Maxx

char buffer[40];    //  general use buffer for output
int win_occured;
WIN *win;
int grid_cols;
int grid_rows;
int keypos;     // the column at which the keys <- & -> place the token
GRID *playgrid;
GRID *entrygrid;
PLAYERGROUP *players;
TXTBOX *msgbox;
VIEWSYS *vs;
MENU *mainmenu;
DIALOG *playerdialog;

void SetP1Current()
{
    int type;
    PlayerGroupCurrentSet(players,0);
    type=PlayerGetType(PlayerGroupCurrentGet(players));
    // disable irrelevant keys
    ViewSysKeyEnableGroup(vs,DISABLE,4,0,1,2,3);
    if (type==Human) ViewSysKeyEnableGroup(vs,ENABLE,4,4,5,6,7);
    else             ViewSysKeyEnableGroup(vs,DISABLE,4,4,5,6,7);
                    // is Computer so disable all keys
}

void SetP2Current()
{
    int type;
    PlayerGroupCurrentSet(players,1);
    type=PlayerGetType(PlayerGroupCurrentGet(players));

    // disable irrelevant keys
    ViewSysKeyEnableGroup(vs,DISABLE,4,4,5,6,7);
    if (type==Human) ViewSysKeyEnableGroup(vs,ENABLE,4,0,1,2,3);
    else             ViewSysKeyEnableGroup(vs,DISABLE,4,0,1,2,3);
                    // is Computer so disable all keys
}

void NextPlayer()
{
    if ((PlayerGroupUseGet(players,0))==(PlayerGroupCurrentGet(players)))
        SetP2Current();
    else
        SetP1Current();
}

void ResetGameSize(int cols,int rows,int inarow)
{
    GridFree(playgrid);
    GridFree(entrygrid);
    grid_cols=cols;
    grid_rows=rows;
    keypos=(cols/2)+(cols%2);
    win_occured=FALSE;
    SetWinSize(win,inarow);
    playgrid=NewGrid(grid_cols,grid_rows);
    entrygrid=NewGrid(grid_cols,1);
    InsertTokenAtCR(entrygrid,keypos,0,PlayerGroupCurrentGet(players));
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
    _settextalign(_LEFT,_BASE);
    _setcolor(LTBLUE);
    _setcharspacing_w(-0.01);
    _setcharsize_w(0.1,0.04);
    _grtext_w(0.0,0.7,"Ninaro");
    _grtext_w(0.01,0.7,"Ninaro");
    _setcharspacing_w(-0.001);
    _setcharsize_w(0.03,0.013);
    _grtext_w(0.65,0.25,"By the author");
    _grtext_w(0.65,0.55,"");
    _grtext_w(0.65,0.85,"   1998  ");
}

void DisplayMsgBox(void) {TextBoxShow(msgbox);}

void DisplayP1Color(void) {DisplayPlayerColor(PlayerGroupUseGet(players,0));}
void DisplayP1Type(void) {DisplayPlayerType(PlayerGroupUseGet(players,0));}
void DisplayP1Name(void) {DisplayPlayerName(PlayerGroupUseGet(players,0));}
void DisplayP1Score(void) {DisplayPlayerScore(PlayerGroupUseGet(players,0));}
void DisplayP2Color(void) {DisplayPlayerColor(PlayerGroupUseGet(players,1));}
void DisplayP2Type(void) {DisplayPlayerType(PlayerGroupUseGet(players,1));}
void DisplayP2Name(void) {DisplayPlayerName(PlayerGroupUseGet(players,1));}
void DisplayP2Score(void) {DisplayPlayerScore(PlayerGroupUseGet(players,1));}

void ChangePlayerType(PLAYER *player)
{
    if (PlayerGetType(player)==Human) PlayerSetType(player,Computer);
    else                              PlayerSetType(player,Human);
}

void IncPlayerColor(PLAYER *player) {PlayerChangeColor(player,1);}
void DecPlayerColor(PLAYER *player) {PlayerChangeColor(player,-1);}

void ChangeP1Type(void) {ChangePlayerType(PlayerGroupUseGet(players,0));}
void ChangeP2Type(void) {ChangePlayerType(PlayerGroupUseGet(players,1));}
void IncP1Color(void) {IncPlayerColor(PlayerGroupUseGet(players,0));}
void IncP2Color(void) {IncPlayerColor(PlayerGroupUseGet(players,1));}
void DecP1Color(void) {DecPlayerColor(PlayerGroupUseGet(players,0));}
void DecP2Color(void) {DecPlayerColor(PlayerGroupUseGet(players,1));}

void ClearScreen(void) {_clearscreen(_GCLEARSCREEN);}
void ClearViewport(void) {_clearscreen(_GVIEWPORT);}

int KeyUp(void)
{
    MenuCall(mainmenu);
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
    switch(InsertTokenInCol(playgrid,keypos,PlayerGroupCurrentGet(players),win))
    {
        case Success :
            RemoveTokenAtCR(entrygrid,keypos,0);
            NextPlayer();
            InsertTokenAtCR(entrygrid,keypos,0,PlayerGroupCurrentGet(players));
            return DisplayAll;
            break;
        case Win :
            PlayTune(4,100,50,300,50,500,100,400,5);
            win_occured=TRUE;
            sprintf(buffer,"%s Wins",PlayerGetName(PlayerGroupCurrentGet(players)));
            TextBoxAddLine(msgbox,buffer,ATEND);
            TextBoxScrollEnd(msgbox);
            RemoveTokenAtCR(entrygrid,keypos,0);
            NextPlayer(); // should be removed from proper system, though
                          // could be used for loser goes first
            InsertTokenAtCR(entrygrid,keypos,0,PlayerGroupCurrentGet(players));
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

void MsgGameN(void)
{
    char buffer[40];
    sprintf(buffer,"Playing %d in a row.",win->tokens);
    TextBoxAddLine(msgbox,buffer,ATEND);
    TextBoxScrollEnd(msgbox);
}

void NewGame(void)
{
    GridEmpty(playgrid); GridEmpty(entrygrid);
    InsertTokenAtCR(entrygrid,keypos,0,PlayerGroupCurrentGet(players));
    win_occured=FALSE; MsgGameN();
}
void EndGame(void) {NewGame();}
void ExitGame(void) {ProgFree(); ShutDown(); exit(0);}

void HelpAbout(void)
{
    ViewSysDisplay(vs,DisplayChanged);
    _setviewport(200,100,439,249);
    BorderedBox(0.0,0.0,1.0,1.0,GREY,LTBLUE);
    _setviewport(210,140,429,209);
    BorderedBox(0.0,0.0,1.0,1.0,BLACK,LTBLUE);
    DisplayGameName();
    SwapPage();
    GetKey();
}

void GameStatus(void)
{
    char buffer[50];
    sprintf(buffer,"----------Game Status----------------");
    TextBoxAddLine(msgbox,buffer,ATEND);
    sprintf(buffer,"%d Columns x %d Rows, Connect %d",grid_cols,grid_rows,win->tokens);
    TextBoxAddLine(msgbox,buffer,ATEND);
    sprintf(buffer,"Player One is %s",PlayerGetName(PlayerGroupUseGet(players,0)));
    TextBoxAddLine(msgbox,buffer,ATEND);
    sprintf(buffer,"Player Two is %s",PlayerGetName(PlayerGroupUseGet(players,1)));
    TextBoxAddLine(msgbox,buffer,ATEND);
    TextBoxScrollEnd(msgbox);
}

void GameSize7x8x4(void) {ResetGameSize(7,8,4); GameStatus();}
void GameSize12x13x6(void) {ResetGameSize(12,13,6); GameStatus();}

void PlayerDialog(PLAYER *player)
{
    
}

void P1Dialog(void)
{
    ControlsDialogCall(&(vs->main),playerdialog,0.01);
}

void P2Dialog(void)
{
    ControlsDialogCall(&(vs->main),playerdialog,0.01);
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
    ViewSysWindowNew(vs,0.61,0.1,1.0,0.6,ACTIVE,
        DisplayMsgBox,"Messages appear here");          // msg box is 3
    ViewSysWindowNew(vs,0.61,0.6,0.65,0.648,ACTIVE,
        DisplayP1Color,"Player One's Color");
    ViewSysWindowNew(vs,0.65,0.6,0.70,0.648,ACTIVE,
        DisplayP1Type,"Player One's Type");
    ViewSysWindowNew(vs,0.7,0.6,0.96,0.648,ACTIVE,
        DisplayP1Name,"Player One's Name");
    ViewSysWindowNew(vs,0.96,0.6,1.0,0.648,ACTIVE,
        DisplayP1Score,"Player One's Score");
    ViewSysWindowNew(vs,0.61,0.652,0.65,0.7,ACTIVE,
        DisplayP2Color,"Player Two's Color");
    ViewSysWindowNew(vs,0.65,0.652,0.7,0.7,ACTIVE,
        DisplayP2Type,"Player Two's Type");
    ViewSysWindowNew(vs,0.7,0.652,0.96,0.7,ACTIVE,
        DisplayP2Name,"Player Two's Name");
    ViewSysWindowNew(vs,0.96,0.652,1.0,0.7,ACTIVE,
        DisplayP2Score,"Player Two's Score");
    return vs;
}

void Test(void)
{
    Buzz(800,900,50,400);
}


void ProgInit() // initialises global variables
{
    win=malloc(sizeof(WIN));
    win->tokens=4;
    win->win=malloc(win->tokens*sizeof(TOKEN));
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

    msgbox=TextBoxNew(8,RED,WHITE,40);

    players=PlayerGroupNew(2,7,
        PlayerNew("Player1",YELLOW,Human,Low),
        PlayerNew("Player2",RED,Human,Low),
        PlayerNew("Ronda Random",LTBLUE,Computer,Low),
        PlayerNew("Alfa B Prune",GREEN,Computer,Medium),
        PlayerNew("Nathan Neuro",BLUE,Computer,High),
        PlayerNew("Rodney Rule",ORANGE,Computer,Medium),
        PlayerNew("Minny Maxx",PURPLE,Computer,Medium));
    SetP1Current();

    TextBoxAddLine(msgbox,"Player1 Arrives",ATEND);
    TextBoxAddLine(msgbox,"Ronda Random Arrives",ATEND);
    TextBoxAddLine(msgbox,"This is the first line",ATSTART);
    TextBoxAddLine(msgbox,"Let The Game Begin",ATEND);
    TextBoxScrollEnd(msgbox);
                
    playgrid=NewGrid(grid_cols,grid_rows);
    entrygrid=NewGrid(grid_cols,1);
    InsertTokenAtCR(entrygrid,keypos,0,PlayerGroupCurrentGet(players));

    playerdialog=ControlsDialogNew(0.2,5,
        TRUE,"Type",     2,  10, 0.1,
            ControlsControlListNew(Radio,FALSE,2,Computer,"Computer",Human,"Human"),
        TRUE,"Name",     2,  10, 0.3,
            ControlsControlListNew(Radio,TRUE,5,2,PlayerGetName(PlayerGroupPlayer(players,2)),
                                           3,PlayerGetName(PlayerGroupPlayer(players,3)),
                                           4,PlayerGetName(PlayerGroupPlayer(players,4)),
                                           5,PlayerGetName(PlayerGroupPlayer(players,5)),
                                           6,PlayerGetName(PlayerGroupPlayer(players,6))),
        FALSE,"Name",     2,  10, 0.3,
            ControlsControlListNew(Radio,TRUE,2,0,PlayerGetName(PlayerGroupPlayer(players,0)),
                                                1,PlayerGetName(PlayerGroupPlayer(players,1))),
        TRUE,"Color",    5,  0,  0.2,
            ControlsControlListNew(Color,TRUE,10,TRUE,1),
        TRUE,"Skill",    1,  6,  0.2,
            ControlsControlListNew(Value,TRUE,10,TRUE,1));
    
    
    mainmenu=MenuNew(5,&(vs->main),ViewSysCurrent(vs));
    MenuNewSub(mainmenu,"Game",5,
        "New",      CN, NewGame,    ItemEnabled,
        "End",      CE, EndGame,    ItemDisabled,
        "Status",   CS, GameStatus, ItemEnabled,
        SEPARATOR,
        "Exit",     CX, ExitGame,   ItemEnabled);
    MenuNewSub(mainmenu,"Board",3,
        "7x8x4",    NONE,GameSize7x8x4,      ItemEnabled,
        "12x13x6",  NONE,GameSize12x13x6,      ItemEnabled, 
        "Custom...",NONE,NULL,      ItemEnabled);
    MenuNewSub(mainmenu,"Player",2,
        "One...",   NONE,P1Dialog,      ItemDisabled,
        "Two...",   NONE,P2Dialog,      ItemEnabled);
    MenuNewSub(mainmenu,"Options",2,
        "Player",   NONE,NULL,      ItemEnabled,
        "Turn",     NONE,NULL,      ItemEnabled);
    MenuNewSub(mainmenu,"Help",3,
        "Contents", NONE,NULL,      ItemEnabled,
        SEPARATOR,
        "About",    NONE,HelpAbout, ItemEnabled);                              
    MenuKeysSet(mainmenu,KP4,KP6,KP8,KP2,ENTER,ESC);
}

void ProgFree() // frees global variables
{
    free(win->win);
    free(win);
    MenuFree(mainmenu);
    free(players);
    GridFree(playgrid);
    GridFree(entrygrid);
    ViewSysFree(vs);
    TextBoxFree(msgbox);
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

    ViewSysDisplay(vs,DisplayAll);
    SwapPage();
    
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
