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
#include "key.h"
#include "txtbox.h"
#include "viewsys.h"
#include "menu.h"
#include "screen.h"
#include "grid.h"
#include "player.h"
#include "dialog.h"
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
KEYGROUP *pkeys;
TXTBOX *msgbox;
VIEWSYS *vs;
MENU *mainmenu;
DIALOG *playerdialog;
DIALOG *boarddialog;
DIALOG *optionsdialog;
KEYSYS *ks;

void SetP1Current()
{
    int type;
    PlayerGroupCurrentSet(players,0);
    type=PlayerGetType(PlayerGroupCurrentGet(players));
    KeyGroupReset(pkeys,'a','d','w','s','e','q');
    MenuItemEnable(mainmenu,2,0,TRUE);
    MenuItemEnable(mainmenu,2,1,FALSE);
    MenuSubCurrentSet(mainmenu,2,0);
}

void SetP2Current()
{
    int type;
    PlayerGroupCurrentSet(players,1);
    type=PlayerGetType(PlayerGroupCurrentGet(players));
    KeyGroupReset(pkeys,KP4,KP6,KP8,KP2,ENTER,BKSP);
    MenuItemEnable(mainmenu,2,1,TRUE);
    MenuItemEnable(mainmenu,2,0,FALSE);
    MenuSubCurrentSet(mainmenu,2,1);
}

void NextPlayer()
{
    if ((PlayerGroupUseGet(players,0))==(PlayerGroupCurrentGet(players)))
        SetP2Current();
    else
        SetP1Current();
}

int MinOf(int a,int b) {if (a<b) return a; else return b;}
int MaxOf(int a,int b) {if (a>b) return a; else return b;}

void GameSize7x8x4(void) {ResetGameSize(7,8,4); GameStatus();}
void GameSize12x13x6(void) {ResetGameSize(12,13,6); GameStatus();}

void GameSizeFit(DIALOG *d)
// only to be used with the boarddialog dialog
{
    int rows=ControlCurrentGet(DialogControlGet(boarddialog,0)),
        cols=ControlCurrentGet(DialogControlGet(boarddialog,1)),
        size=MaxOf(cols,rows);
    CONTROL *gsize=DialogControlGet(boarddialog,2);
    if (ControlCurrentGet(gsize)>size) // put back in range
        ControlCurrentSet(gsize,size);
    ControlReset(gsize,3,"",size,"");
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
    GameSizeFit(boarddialog);
    playgrid=NewGrid(grid_cols,grid_rows);
    entrygrid=NewGrid(grid_cols,1);
    keypos=0;
    InsertTokenAtCR(entrygrid,keypos,0,PlayerGroupCurrentGet(players));
}

void SetupPlayer(PLAYER *player,int puse)
// uses the playerdialog dialog
{
    int pnum=PlayerGroupFind(players,player);
    int ptype=PlayerGetType(player);
    int pcol=PlayerGetColor(player);
    DialogNameSet(playerdialog,puse==0?"Player One":"Player Two");
    ControlCurrentSetTo(DialogControlGet(playerdialog,0),ptype);
    if (ptype==Computer)
    {
        ControlReset(DialogControlGet(playerdialog,1),
                        2,PlayerGetName(PlayerGroupPlayer(players,2)),
                        3,PlayerGetName(PlayerGroupPlayer(players,3)),
                        4,PlayerGetName(PlayerGroupPlayer(players,4)),
                        5,PlayerGetName(PlayerGroupPlayer(players,5)),
                        6,PlayerGetName(PlayerGroupPlayer(players,6)));
    }
    else
    {
        ControlReset(DialogControlGet(playerdialog,1),
                        -1,PlayerGetName(player),
                        -1,"New Player",
                        -1,NULL,
                        -1,NULL,
                        -1,NULL);
    }
    ControlCurrentSetTo(DialogControlGet(playerdialog,2),pcol);
}

void ModifyPlayer(DIALOG *d)
// only to be used with the playerdialog dialog
{
    
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

int KeyUp(void *vp)
{
    KEYGROUP *kg=KeySysGroupGet(ks);
    MenuKeyGroupSet(mainmenu,kg);
    MenuCall(mainmenu);
    return TRUE;
}

int KeyLeft(void *vp)
{
    if (MoveTokenAtCR(entrygrid,keypos,0,T_LEFT))
    {
        --keypos;
        ViewSysWindowChanged(vs,0,CHANGED);
        return DisplayChanged;
    }
    // 0 is entry grid
    else
        return NoDisplay;
}

int KeyRight(void *vp)
{
    if (MoveTokenAtCR(entrygrid,keypos,0,T_RIGHT))
    {
        ++keypos;
        ViewSysWindowChanged(vs,0,CHANGED);
        return DisplayChanged;
    }
    // 0 is entry grid
    else
        return NoDisplay;
}

int KeyDown(void *vp)
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

int KeySelect(void *vp) {return KeyDown(vp);}
int KeyCancel(void *vp) {return KeyUp(vp);}

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
    BorderedBox(200,100,439,249,GREY,LTBLUE);
    BorderedBox(210,140,429,209,BLACK,LTBLUE);
    _setviewport(210,140,429,209);
    DisplayGameName();
    SwapPage();
    GetKey(); // should change so that only current player can do
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

void GameSizeCustom(void)
{
    VIEWPORT vp={50,50,590,300};
    int buffer[4];
    buffer[0]=grid_rows;
    buffer[1]=grid_cols;
    buffer[2]=win->tokens;
    buffer[3]=0;
    DialogStateSet(boarddialog,buffer);
    if (DialogCall(boarddialog,&vp,vs->current,5,8))
    {
        DialogStateGet(boarddialog,buffer);
        ResetGameSize(buffer[1],buffer[0],buffer[2]);
    }
    GameStatus();
}

void PlayerDialog(int pnum)
{
    VIEWPORT vp={50,50,590,300};
    DialogCall(playerdialog,&vp,vs->current,5,8);
}

void P1Dialog(void)
{
    SetupPlayer(PlayerGroupUseGet(players,0),0);
    PlayerDialog(0);
}

void P2Dialog(void)
{
    SetupPlayer(PlayerGroupUseGet(players,1),1);
    PlayerDialog(1);
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

void ToggleSound(void)
{
    char buffer[20];
    sprintf(buffer,"Sound is now %s",ToggleMute()?"Off":"On");
    TextBoxAddLine(msgbox,buffer,ATEND);
    TextBoxScrollEnd(msgbox);
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
    
    msgbox=TextBoxNew(8,RED,WHITE,40);

    players=PlayerGroupNew(2,7, // allows for only two humans
        PlayerNew("Player1",ORANGE,Human,Low),
        PlayerNew("Player2",RED,Human,Low),
        PlayerNew("Ronda Random",LTBLUE,Computer,Low),
        PlayerNew("Alfa B Prune",GREEN,Computer,Medium),
        PlayerNew("Tammy Tree",BLUE,Computer,High),
        PlayerNew("Rodney Rule",ORANGE,Computer,Medium),
        PlayerNew("Minny Maxx",PURPLE,Computer,Medium));
    pkeys=KeyGroupNew(6,'a','d','w','s','e','q');

    TextBoxAddLine(msgbox,"Player1 Arrives",ATEND);
    TextBoxAddLine(msgbox,"Ronda Random Arrives",ATEND);
    TextBoxAddLine(msgbox,"This is the first line",ATSTART);
    TextBoxAddLine(msgbox,"Let The Game Begin",ATEND);
    TextBoxScrollEnd(msgbox);
                
    playgrid=NewGrid(grid_cols,grid_rows);
    entrygrid=NewGrid(grid_cols,1);
    InsertTokenAtCR(entrygrid,keypos,0,PlayerGroupCurrentGet(players));
    playerdialog=DialogNew(pkeys,NULL,"Player One",3,
        "Type",TRUE,TRUE,ControlNew(List,Radio,1,2,10,2,
                        Computer,"Computer",
                        Human,"Human"),
        "Name",TRUE,TRUE,ControlNew(List,Radio,3,2,13,5,
                        2,PlayerGetName(PlayerGroupPlayer(players,2)),
                        3,PlayerGetName(PlayerGroupPlayer(players,3)),
                        4,PlayerGetName(PlayerGroupPlayer(players,4)),
                        5,PlayerGetName(PlayerGroupPlayer(players,5)),
                        6,PlayerGetName(PlayerGroupPlayer(players,6))),
        "Color",TRUE,TRUE,ControlNew(Range,Color,1,10,5,
                        1,"",7,""));
    boarddialog=DialogNew(pkeys,GameSizeFit,"Custom Board Size",3,
        "Rows",TRUE,TRUE,ControlNew(Range,Slider,1,1,10,
                        3,"",50,""),
        "Columns",TRUE,TRUE,ControlNew(Range,Slider,1,1,10,
                        3,"",50,""),
        "Size",TRUE,TRUE,ControlNew(Range,Slider,1,1,10,
                        3,"",MaxOf(grid_rows,grid_cols),""));

              
    mainmenu=MenuNew(4,&(vs->main),ViewSysCurrent(vs),pkeys);
    MenuNewSub(mainmenu,"Game",7,
        "New",      CN, NewGame,    TRUE,
        "End",      CE, EndGame,    FALSE,
        "Status",   CS, GameStatus, TRUE,
        "High Scores",NONE,NULL,    TRUE,
        "Suggest Move",NONE,NULL,   TRUE,
        "Sound",    NONE,ToggleSound,      TRUE,
        "Exit",     CX, ExitGame,   TRUE);
    MenuNewSub(mainmenu,"Board",3,
        "7x8x4",    NONE,GameSize7x8x4,      TRUE,
        "12x13x6",  NONE,GameSize12x13x6,      TRUE, 
        "Custom...",NONE,GameSizeCustom,      TRUE);
    MenuNewSub(mainmenu,"Player",3,
        "One...",   NONE,P1Dialog,      FALSE,
        "Two...",   NONE,P2Dialog,      TRUE,
        "Options",  NONE,NULL,          TRUE);
    MenuNewSub(mainmenu,"Help",2,
        "Contents", NONE,NULL,      TRUE,
        "About",    NONE,HelpAbout, TRUE);

    ks=KeySysNew(pkeys);
    KeySysIFunc(ks,0,KeyLeft);
    KeySysIFunc(ks,1,KeyRight);
    KeySysIFunc(ks,2,KeyUp);
    KeySysIFunc(ks,3,KeyDown);
    KeySysIFunc(ks,4,KeySelect);
    KeySysIFunc(ks,5,KeyCancel);
    SetP1Current();
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
    int keyused=FALSE,key,index,funcret;
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
    
    for (;;)
    {
        if (KeySysKey(ks,GetKey(),NULL,&funcret,&index))
        // key was used
        {
            switch (index)
            {
                case 2 : case 5 : // menu was called
                    ViewSysDisplay(vs,DisplayAll);
                    SwapPage();
                break;
                default :
                    ViewSysDisplay(vs,funcret);
                    if (funcret!=NoDisplay) SwapPage();
                break;
            }
            
        }
    }
}
