/****************************************************************
Copyright 1998 the author
Module Name    : NINARO
Module Purpose : the main module for the ninaro project
Creation Date  : 7 March 1998
Updated        : 2 May 1998     CREATED AI's and modified main to fit
****************************************************************/
//  todo :
//  DONE  grid save and load
//  DONE  title screen
//  DONE  player has new attribute (* func)(GRID *grid) which gets move
//        so each player has own function
//        getting a computer move cannot be interrupted, interrupt between
//        turns
//        player_swap could swap keys only???
//        possible mouse inclusion, though is perhaps too late.
//          mouse in main,dialog,menu.
//  DONE  options dialog must do something.
//  DONE  options dialog could use computer speed, so that computers could
//        play at human speed(3 seconds?), or at computer speed(0 secs);
//        that way could watch random vs tree play 500 games in 1 minute.
//  DONE  when two computers battling it out, player can press ESC or M to
//        interrupt them to gain menu access
//  DONE  provide animation for computers
//  DONE  possibly merge player color and type icons to expand score width
//        if score too big - too many characters squash char width to fit.
//  DONE  ERROR found - load saved game and cant drop tokens!!!
//        hoping proper save/load will fix the error
//        save game config at exit and restore when program initialised
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <graph.h>
#include <i86.h>
#include "sys.h"
#include "mouse.h"
#include "mousein.h"
#include "binfile.h"
#include "sound.h"
#include "key.h"
#include "txtbox.h"
#include "help.h"
#include "viewsys.h"
#include "menu.h"
#include "screen.h"
#include "mempage.h"
#include "grid.h"
#include "player.h"
#include "dialog.h"
#include "connectn.h"

enum {GFOne=0,GFTwo,GFLoser,GFWinner,GFSuccessive,GFRandom}; // goes first

// #define TESTING

#define ColAdd 2 // 3 is red/green
// secondary color = primary color + ColAdd

// Rodney Rule
// Ronda Random
// Peter Prune
// Nathan Neuro
// Minny Maxx

char buffer[40];
int win_occured;
WIN *win;
int grid_cols;
int grid_rows;
int keypos;     // the column at which the keys <- & -> place the token
GRID *playgrid;
GRID *entrygrid;
PLAYERGROUP *players;
int player_one,player_two,player_current;
int player_one_score,player_two_score;
int player_winner;
int dialogplayer;
KEYGROUP *pkeys;
KEYGROUP *p1keys;
KEYGROUP *p2keys;
TXTBOX *msgbox;
TXTBOX *statusbox;
VIEWSYS *vs;
MENU *mainmenu;
DIALOG *playerdialog;
DIALOG *boarddialog;
DIALOG *optionsdialog;
HELP *help;
HELP *aboutgame;
MEMPAGE *aboutmp;
KEYSYS *ks;
int ingame;
int isdraw;
int goesfirst;
int wentfirst;
int ai_speed;
int ai_delay; // milliseconds
int hilite,back,text;


int BothPlayerAI(void)
{
    return ((player_one>1) && (player_two>1));
}

void SetP1(int player)
{
    if (player_current==player_one) {player_one=player;player_current=player_one;}
    else player_one=player;
}

void SetP2(int player)
{
    if (player_current==player_two) {player_two=player;player_current=player_two;}
    else player_two=player;
}

void SetP1Current()
{
    player_current=player_one;
    MenuItemEnable(mainmenu,2,0,TRUE);
    MenuItemEnable(mainmenu,2,1,FALSE);
    MenuSubCurrentSet(mainmenu,2,0);
}

void SetP2Current()
{
    player_current=player_two;
    MenuItemEnable(mainmenu,2,1,TRUE);
    MenuItemEnable(mainmenu,2,0,FALSE);
    MenuSubCurrentSet(mainmenu,2,1);
}

void NextPlayer()
{
    if (player_current==player_one) SetP2Current();
    else                            SetP1Current();
}

PLAYER *OtherPlayer(void)
{
    if (player_current==player_one)
        return PlayerGroupPlayer(players,player_two);
    else
        return PlayerGroupPlayer(players,player_one);
}

void ResetScores(void)
{
    player_one_score=0;
    player_two_score=0;
    player_winner=-1;       // no winner
}

void PlayerSwap(void) // not in use
{
    int tmp=player_one;
    player_one=player_two;
    player_two=tmp;
    tmp=player_one_score;
    player_one_score=player_two_score;
    player_two_score=player_one_score;
    NextPlayer();
    ResetTokenAtCR(entrygrid,keypos,0,PlayerGroupPlayer(players,player_current));        
}

int MinOf(int a,int b) {if (a<b) return a; else return b;}
int MaxOf(int a,int b) {if (a>b) return a; else return b;}

void GameSize7x8x4(void) {ResetGameSize(7,8,4); GameStatus();}
void GameSize12x13x6(void) {ResetGameSize(12,13,6); GameStatus();}

void GameSizeFit(DIALOG *d,int last)
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
    playgrid=GridNew(grid_cols,grid_rows);
    entrygrid=GridNew(grid_cols,1);
    InsertTokenAtCR(entrygrid,keypos,0,PlayerGroupPlayer(players,player_current));
}

void SetupPlayer(int player_index)
// uses the playerdialog dialog
{
    PLAYER *player=PlayerGroupPlayer(players,player_index);
    int pcol=PlayerGetColor(player);

    if (player_index==player_one)
        DialogNameSet(playerdialog,"Player One");
    else if (player_index==player_two)
        DialogNameSet(playerdialog,"Player Two");
    else
        DialogNameSet(playerdialog,"New Player"); // should never happen
    if ((player_index==0) || (player_index==1)) // human
        ControlCurrentSetTo(DialogControlGet(playerdialog,0),1); // set player
    else
        ControlCurrentSetTo(DialogControlGet(playerdialog,0),player_index); // set player            
    ControlCurrentSetTo(DialogControlGet(playerdialog,1),pcol); // set color
}

void ModifyPlayer(DIALOG *d,int last)
// only to be used with the playerdialog dialog
{
    CONTROL *c;
    int p;
    int notcolor,thiscolor;

    c=DialogControlGet(d,0); // name
    switch (DialogCurrentGet(d))
    {
        case 0 :  // changing names
            ControlValueGet(c,ControlCurrentGet(c),&p); // which player
            ControlCurrentSetTo(DialogControlGet(d,1),
                PlayerGetColor(PlayerGroupPlayer(players,p))); // set color
        // cascade to changing colors to ensure p1.color!=p2.color
        case 1 : // changing colors
            ControlValueGet(c,0,&p); // which player human is
            if (p==0) // is player one dialog
                notcolor=PlayerGetColor(PlayerGroupPlayer(players,player_two));
            else      // is player two dialog
                notcolor=PlayerGetColor(PlayerGroupPlayer(players,player_one));

            c=DialogControlGet(d,1); // color control
            thiscolor=ControlCurrentGet(c);  // color control current
            if (thiscolor==notcolor)
            {
                switch (last)
                {
                    case 0 : // LEFT
                        if (!(ControlPrev(c))) ControlNext(c); break;
                    case 1 : // RIGHT
                        if (!(ControlNext(c))) ControlPrev(c); break;
                }
            }
        break;
    }
}

void DisplayHuman(int primary,int secondary)
// draws a human in the current viewport
{
    _setcolor(primary);
    _ellipse_w(_GFILLINTERIOR,0.2,0.1,0.8,0.25);  //head
    _moveto_w(0.49,0.25); _lineto_w(0.49,0.7);     // body
    _moveto_w(0.51,0.25); _lineto_w(0.51,0.7);     // body
    _setcolor(secondary);
    _moveto_w(0.49,0.7); _lineto_w(0.8,0.9);     // left leg
    _moveto_w(0.51,0.7); _lineto_w(0.2,0.9);     // right leg
    _moveto_w(0.1,0.4); _lineto_w(0.9,0.4);     // arms
}

void DisplayComputer(int primary,int secondary)
// draws a computer in the current viewport
{
    _setcolor(secondary);
    _rectangle_w(_GBORDER,0.1,0.1,0.9,0.65);
    _rectangle_w(_GBORDER,0.45,0.65,0.55,0.75);
    _rectangle_w(_GBORDER,0.1,0.75,0.9,0.9);
    _setcolor(primary);
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
    _setcharspacing_w(-0.013);
    _setcharsize_w(0.04,0.03);
    _grtext_w(0.0,1.0,PlayerGetName(player));
}

void DisplayPlayerScore(int score)
{
    char sbuf[5];
    _setcolor(GREEN);
    sprintf(sbuf,"%d",score);
    _grtext_w(0.0,1.0,sbuf);
}

void Debug(char *string)
{
    _setviewport(500,100,600,200);
    _clearscreen(_GVIEWPORT);
    _grtext_w(0.5,0.5, string);
}

void DisplayEntryGrid(void) {_setcolor(YELLOW);DisplayGrid(entrygrid,0.6);}

void DisplayPlayGrid(void)
{
    int x;
    int col,row;
    _setcolor(GREEN);
    DisplayGrid(playgrid,0.6);
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
    _setcharspacing_w(-0.01);
    _setcharsize_w(0.1,0.04);
    _setcolor(RED);
    _grtext_w(0.0,0.7,"Ninaro");
    _setcolor(GREEN);
    _grtext_w(0.01,0.7,"Ninaro");
    _setcharspacing_w(-0.001);
    _setcharsize_w(0.03,0.013);
    _setcolor(RED);
    _grtext_w(0.65,0.25,"By the author");
    _grtext_w(0.65,0.55,"");
    _grtext_w(0.65,0.85,"   1998  ");
    _setcolor(GREEN);
    _grtext_w(0.655,0.25,"By the author");
    _grtext_w(0.655,0.55,"");
    _grtext_w(0.655,0.85,"   1998  ");

}

void DisplayMsgBox(void)
{
    _setvieworg(0,0);
    TextBoxDisplay(msgbox,CurrentViewport());
}

void DisplayStatusBox(void)
{
    _setvieworg(0,0);
    TextBoxDisplay(statusbox,CurrentViewport());
}

void DisplayP1Color(void) {DisplayPlayerColor(PlayerGroupPlayer(players,player_one));}
void DisplayP1Type(void) {DisplayPlayerType(PlayerGroupPlayer(players,player_one));}
void DisplayP1Name(void) {DisplayPlayerName(PlayerGroupPlayer(players,player_one));}
void DisplayP1Score(void) {DisplayPlayerScore(player_one_score);}
void DisplayP2Color(void) {DisplayPlayerColor(PlayerGroupPlayer(players,player_two));}
void DisplayP2Type(void) {DisplayPlayerType(PlayerGroupPlayer(players,player_two));}
void DisplayP2Name(void) {DisplayPlayerName(PlayerGroupPlayer(players,player_two));}
void DisplayP2Score(void) {DisplayPlayerScore(player_two_score);}

void ChangePlayerType(PLAYER *player)
{
    if (PlayerGetType(player)==Human) PlayerSetType(player,Computer);
    else                              PlayerSetType(player,Human);
}

void IncPlayerColor(PLAYER *player) {PlayerSetColor(player,PlayerGetColor(player)+1);}
void DecPlayerColor(PLAYER *player) {PlayerSetColor(player,PlayerGetColor(player)-1);}

void ChangeP1Type(void) {ChangePlayerType(PlayerGroupPlayer(players,player_one));}
void ChangeP2Type(void) {ChangePlayerType(PlayerGroupPlayer(players,player_two));}
void IncP1Color(void) {IncPlayerColor(PlayerGroupPlayer(players,player_one));}
void IncP2Color(void) {IncPlayerColor(PlayerGroupPlayer(players,player_two));}
void DecP1Color(void) {DecPlayerColor(PlayerGroupPlayer(players,player_one));}
void DecP2Color(void) {DecPlayerColor(PlayerGroupPlayer(players,player_two));}

void ClearScreen(void) {_clearscreen(_GCLEARSCREEN);}

void HelpContents(void)
{
    VIEWPORT vp={10,10,629,339};
    HelpCall(help,&vp,ViewSysCurrent(vs));
}

void HelpContentsFunc(void)
{
    HelpHome(help);
    HelpContents();
}


void HelpAbout(void)
{
    VIEWPORT vp={190,181,449,259}; // help viewport
    ViewSysDisplay(vs,DisplayCurrent);
    BorderedBox(180,100,459,269,GREY,LTBLUE);
    BorderedBox(190,110,449,179,BLACK,LTBLUE); // game name
    _setviewport(190,110,449,179);
    DisplayGameName();
    MemPageGet(ViewSysCurrent(vs));
    SwapPage();
    MemPagePut(ViewSysCurrent(vs));     // both pages same
    HelpCall(aboutgame,&vp,ViewSysCurrent(vs));
//    ViewSysDisplay(vs,DisplayCurrent);      // refesh to normal
}

void HelpPlayerDialog(void)
{HelpScrollString(help,"Player Dialog"); HelpContents();}
void HelpBoardDialog(void)
{HelpScrollString(help,"Board Dialog"); HelpContents();}
void HelpOptionsDialog(void)
{HelpScrollString(help,"Options Dialog"); HelpContents();}

void HelpMenu(void)
{
    switch (MenuSub(mainmenu))
    {
        case 0 : HelpScrollString(help,"Game Menu"); HelpContents(); break;
        case 1 : HelpScrollString(help,"Board Menu"); HelpContents(); break;
        case 2 : HelpScrollString(help,"Player Menu"); HelpContents(); break;
        case 3 : HelpScrollString(help,"Help Menu"); HelpContents(); break;
    }
}

int KeyUp(void *vp)
{
    KEYGROUP *kg=KeySysGroupGet(ks);
    MenuKeyGroupSet(mainmenu,kg);
    MenuCall(mainmenu,HelpMenu);
    return DisplayAll; // none or all could have changed in menucall
}

int KeyLeft(void *vp)
{
    if (MoveTokenAtCR(entrygrid,keypos,0,T_LEFT))
    {
        --keypos;
        ViewSysWindowChanged(vs,0,TRUE); // entrygrid
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
        ViewSysWindowChanged(vs,0,TRUE);
        return DisplayChanged;
    }
    // 0 is entry grid
    else
        return NoDisplay;
}

int KeyDown(void *vp)
{
    return TRUE;
}

int DropToken()
{
    int ret=InsertTokenInCol(playgrid,keypos,PlayerGroupPlayer(players,player_current),win);
    switch (ret)
    {
        case Success :
            RemoveTokenAtCR(entrygrid,keypos,0);
            NextPlayer();
            InsertTokenAtCR(entrygrid,keypos,0,PlayerGroupPlayer(players,player_current));
            ViewSysWindowChanged(vs,0,TRUE); ViewSysWindowChanged(vs,1,TRUE);
            if (TokensInGrid(playgrid)==MaxTokensInGrid(playgrid)) // draw
            {
                TextBoxFixedAddLine(msgbox,"It's A Draw...");
                isdraw=TRUE; 
            }
            break;
        case Win :
            PlayTune(4,100,50,300,50,500,100,400,5);
            win_occured=TRUE;
            sprintf(buffer,"%s Wins",PlayerGetName(PlayerGroupPlayer(players,player_current)));
            if (player_current==player_one)
                {++player_one_score; player_winner=player_one;}
            else
                {++player_two_score; player_winner=player_two;}
            TextBoxFixedAddLine(msgbox,buffer);
            RemoveTokenAtCR(entrygrid,keypos,0);
            break;
        case Failure :
            Buzz(200,400,20,100);
            break;
    }
    return ret;
}

int KeySelect(void *vp) {return KeyDown(vp);}
int KeyCancel(void *vp) {return KeyUp(vp);}

void NewGame(void)
{
    switch (goesfirst)
    {
        case GFOne : SetP1Current();
            TextBoxFixedAddLine(msgbox,"Player One Goes First");
        break;
        case GFTwo : SetP2Current();
            TextBoxFixedAddLine(msgbox,"Player Two Goes First");
        break;
        case GFLoser :
            if (player_winner==player_one) SetP2Current();
            else if (player_winner==player_two) SetP1Current();
            else if (Random(2)==0) SetP1Current();
            else SetP2Current;
            TextBoxFixedAddLine(msgbox,"Loser Goes First");
            break;
        case GFWinner :
            if (player_winner==player_one) SetP1Current();
            else if (player_winner==player_two) SetP2Current();
            else if (Random(2)==0) SetP1Current();
            else SetP2Current();
            TextBoxFixedAddLine(msgbox,"Winner Goes First");
            break;
        case GFSuccessive :
            if (wentfirst==player_two) SetP1Current();
            else                        SetP2Current();
            TextBoxFixedAddLine(msgbox,"Successive Player Goes First");
            break;
        case GFRandom :
            if (Random(2)==0) SetP1Current(); else SetP2Current();
            TextBoxFixedAddLine(msgbox,"Random Player Goes First");
            break;
    }
    wentfirst=player_current;
    GridEmpty(playgrid); GridEmpty(entrygrid);
    ViewSysWindowChanged(vs,0,TRUE); ViewSysWindowChanged(vs,1,TRUE);
    InsertTokenAtCR(entrygrid,keypos,0,PlayerGroupPlayer(players,player_current));
    win_occured=FALSE; 
    isdraw=FALSE;
    MenuItemEnable(mainmenu,0,3,TRUE); // enable end game
    MenuItemEnable(mainmenu,0,0,FALSE); // disable new game
    MenuSubCurrentSet(mainmenu,0,3);
    if (!ingame)
    {
        TextBoxFixedAddLine(msgbox,"Currently Playing Game.");
        ViewSysWindowChanged(vs,3,TRUE); 
    }
    ingame=TRUE;
}

void EndGame(void)
{
    MenuItemEnable(mainmenu,2,0,TRUE); // enable players one and two
    MenuItemEnable(mainmenu,2,1,TRUE);
    MenuItemEnable(mainmenu,0,3,FALSE); // disable end game
    MenuItemEnable(mainmenu,0,0,TRUE); // enable new game
    MenuSubCurrentSet(mainmenu,0,0);   // current is new game
    TextBoxFixedAddLine(msgbox,"Currently Not Playing.");
    ingame=FALSE;
    GridEmpty(playgrid);
}

void LoadGame(void)
{
    int p1,p2,current,size;
    GRID *tmp=GameLoad(&p1,&p2,&current,&size,players,"NINARO.SAV");
    if (tmp==NULL) // failed to load
    {
        TextBoxFixedAddLine(msgbox,"~Load Error!");
    }
    else // loaded OK
    {
        GridFree(playgrid);
        GridFree(entrygrid);
        grid_cols=GridCols(tmp);
        keypos=grid_cols/2;
        grid_rows=GridRows(tmp);
        SetWinSize(win,size);
        playgrid=tmp;
        entrygrid=GridNew(grid_cols,1);
        SetP1(p1);
        SetP2(p2);
        player_current=current;
        TextBoxFixedAddLine(msgbox,"Game Loaded");
        win_occured=FALSE; 
        ResetTokenAtCR(entrygrid,keypos,0,PlayerGroupPlayer(players,player_current));
        GameStatus();
        if (ingame)
        {
            if (player_current==player_one)
            {
                MenuItemEnable(mainmenu,2,0,TRUE); // player one
                MenuItemEnable(mainmenu,2,1,FALSE); // player two
                MenuSubCurrentSet(mainmenu,2,0);
            }
            else
            {
                MenuItemEnable(mainmenu,2,0,FALSE); // player one
                MenuItemEnable(mainmenu,2,1,TRUE); // player two
                MenuSubCurrentSet(mainmenu,2,1);
            }
        }
        else
        {
            MenuItemEnable(mainmenu,2,0,TRUE); // player one
            MenuItemEnable(mainmenu,2,1,TRUE); // player two
        }
    }
}

void SaveGame(void)
{
    if (GameSave(player_one,player_two,player_current,win->tokens,playgrid,players,"NINARO.SAV"))
    {
        TextBoxFixedAddLine(msgbox,"Game Saved");
    }
    else // can't save - file error
    {
        TextBoxFixedAddLine(msgbox,"~Save Error!");
    }
}

int SaveCfg()
{
    int ret,x;
    PLAYER *pl;
    BINFILE *bf=BinFileNew((10+players->players)*sizeof(int));
    BinFilePutInt(bf,grid_cols);
    BinFilePutInt(bf,grid_rows);
    BinFilePutInt(bf,win->tokens);
    BinFilePutInt(bf,goesfirst);
    BinFilePutInt(bf,wentfirst);
    BinFilePutInt(bf,ai_speed);
    BinFilePutInt(bf,ai_delay);
    BinFilePutInt(bf,player_one);
    BinFilePutInt(bf,player_two);
    BinFilePutInt(bf,IsMute()?TRUE:FALSE);
    for (x=0;x<players->players;x++)
    {
        pl=PlayerGroupPlayer(players,x);
        BinFilePutInt(bf,PlayerGetColor(pl));
    }
    ret=BinFileWriteFull(bf,"NINARO.CFG");
    BinFileFree(bf);
    return ret;
}

int LoadCfg()
{
    int ret,x,col;
    PLAYER *pl;
    BINFILE *bf=BinFileReadFull("NINARO.CFG",0);
    if (bf==NULL) return FALSE;
    grid_cols=BinFileGetInt(bf);
    grid_rows=BinFileGetInt(bf);
    win->tokens=BinFileGetInt(bf);
    goesfirst=BinFileGetInt(bf);
    wentfirst=BinFileGetInt(bf);
    ai_speed=BinFileGetInt(bf);
    ai_delay=BinFileGetInt(bf);
    SetP1(BinFileGetInt(bf));
    SetP2(BinFileGetInt(bf));
    Mute(BinFileGetInt(bf));
    for (x=0;x<players->players;x++)
    {
        pl=PlayerGroupPlayer(players,x);
        PlayerSetColor(pl,BinFileGetInt(bf));
    }
    BinFileFree(bf);
    if (player_two==7)
    {
        col=PlayerGetColor(PlayerGroupPlayer(players,7));
        PlayerCopy(PlayerGroupPlayer(players,7),PlayerGroupPlayer(players,player_one));
    }
    else if (player_one==7)
    {
        col=PlayerGetColor(PlayerGroupPlayer(players,7));
        PlayerCopy(PlayerGroupPlayer(players,7),PlayerGroupPlayer(players,player_two));
        PlayerSetColor(PlayerGroupPlayer(players,7),col);
    }
    return TRUE;
}

void ExitGame(void) {SaveCfg(); ProgFree(); ShutDown(); exit(0);}

void GameStatus(void)
{
    char gbuf[30];
    TextBoxSetLine(statusbox,1,"Columns  Rows  Connect  Sound");
    sprintf(gbuf,"~%5d  %4d  %6d      %s",grid_cols,grid_rows,win->tokens,IsMute()?"Off":"On");
    TextBoxSetLine(statusbox,2,gbuf);
    TextBoxSetLine(statusbox,3,"Type    Player         Score");
}

void GameSizeCustom(void)
{
    VIEWPORT vp={50,50,590,300};
    int cbuf[4];
    cbuf[0]=grid_rows; cbuf[1]=grid_cols;
    cbuf[2]=win->tokens; cbuf[3]=0;
    DialogStateSet(boarddialog,cbuf);
    GameSizeFit(boarddialog,6);
    if (DialogCall(boarddialog,&vp,vs->current,5,8,HelpBoardDialog))
    {
        DialogStateGet(boarddialog,cbuf);
        ResetGameSize(cbuf[1],cbuf[0],cbuf[2]);
        GameStatus();
    }
}

int PlayerDialog(int pnum)
// returns -1 if same player, else the number of the selected player
{
    VIEWPORT vp={50,50,590,300};
    CONTROL *c=DialogControlGet(playerdialog,0); // names
    CONTROL *c2=DialogControlGet(playerdialog,1);// colors
    int p;
    SetupPlayer(pnum);
    if (DialogCall(playerdialog,&vp,vs->current,5,8,HelpPlayerDialog))
    {
        ControlValueGet(c,ControlCurrentGet(c),&p); // which player
        if (((pnum==player_two) && (p==player_one))
        || ((pnum==player_one) && (p==player_two)))
        {
            PlayerCopy(PlayerGroupPlayer(players,7),PlayerGroupPlayer(players,p));
            p=7;
        }
        PlayerSetColor(PlayerGroupPlayer(players,p),ControlCurrentGet(c2));
        if (p==pnum) return -1; // no change
        return p;
    }
    else return -1;
}

void P1Dialog(void)
{
    int player;
    CONTROL *c=DialogControlGet(playerdialog,0); // names
    ControlElemSet(c,0,0,"Human");
    player=PlayerDialog(player_one);
    if (player!=-1) // player has changed
    {
        SetP1(player);
        player_one_score=0;
        player_two_score=0;
        if (ingame) NewGame();
    }
}

void P2Dialog(void)
{
    int player;
    CONTROL *c=DialogControlGet(playerdialog,0); // names
    ControlElemSet(c,0,1,"Human");
    player=PlayerDialog(player_two);
    if (player!=-1)// player has changed
    {
        SetP2(player);
        player_one_score=0;
        player_two_score=0;
        if (ingame) NewGame();
    }
}

void PlayerOptions(void)
{
    VIEWPORT vp={50,50,590,300};
    CONTROL *c;
    ControlCurrentSetTo(DialogControlGet(optionsdialog,0),goesfirst);
    ControlCurrentSetTo(DialogControlGet(optionsdialog,1),ai_speed);
    if (DialogCall(optionsdialog,&vp,vs->current,5,11,HelpOptionsDialog))
    {
        goesfirst=ControlCurrentGet(DialogControlGet(optionsdialog,0));
        c=DialogControlGet(optionsdialog,1);
        ControlValueGet(c,ControlCurrentGet(c),&ai_speed);
    }
}

int RandomMoveFromList(int size,int *list)
// returns the index of the random'th maximum in list of size where there
// are n TRUE's.
{
    int max=list[0],howmany=1,first=0,x;
    int move,found;
    for (x=1;x<size;x++)
    {
        if (list[x]>max) {max=list[x]; howmany=1; first=x;}
        else if (list[x]==max) {++howmany;} // else ignore
    }
    move=Random(howmany);

    found=0;
    for (x=first;x<size;x++)
    {
        if (list[x]==max)
        {
            ++found;
            if (found==move+1) return x;
        }
    }
    return -1;
}

int RuleMove(GRID *grid,int level,PLAYER *thisplayer,PLAYER *otherplayer)
// level is how many rules to invoke
// returns column, -1 if could not make a move
{
    char tmp[40];
    int cols=GridCols(grid),rows=GridRows(grid);
    int *col=malloc(sizeof(int)*cols),x;
    int atlevel,canuse=FALSE,ret;
    int triggered=FALSE;
    
    for (x=0;x<cols;x++) // find how many can columns still get tokens
    {
        col[x]=(GridInCol(grid,x)<rows?100:0);
        if (col[x]) canuse=TRUE;
    }
    if (!canuse) {free(col); return -1;}

    for (atlevel=0;atlevel<=level;atlevel++)
    {
        switch (atlevel)
        {
            case 0 : break; // invoke zero rules is just random
            case 1 : // take win rule
                for (x=0;x<cols;x++)
                {
                    if (col[x])
                    {
                        if (CheckForWin(playgrid,win->tokens,x,InsertTokenTarget(playgrid,x),win,thisplayer))
                        {   
                            col[x]+=1;
                            triggered=TRUE;
                        }
                    }
                }
                if (triggered) atlevel=level+1; // get out of loop
            break; 
            case 2 : // block opponent rule
                for (x=0;x<cols;x++)
                {
                    if (col[x])
                    {
                        if (CheckForWin(playgrid,win->tokens,x,InsertTokenTarget(playgrid,x),win,otherplayer))
                        {
                            col[x]+=50;
                        }
                    }
                }
            break;
            case 3 : // other above rule, requires at least 2 empties in col
                for (x=0;x<cols;x++)
                {
                    if (col[x])
                    {
                        if (rows-GridInCol(playgrid,x)>1) // rule applicable
                        {
                            if (CheckForWin(playgrid,win->tokens,x,InsertTokenTarget(playgrid,x)-1,win,otherplayer))
                            {   // drop here and opponent drops here and wins
                                col[x]-=90;
                            }
                            else if (CheckForWin(playgrid,win->tokens,x,InsertTokenTarget(playgrid,x)-1,win,thisplayer))
                            {   // drop here and opponent drops here and blocks me
                                col[x]-=20;
                            }
                        }

                        // also if opponent can make a move which gives them
                        // size-1 in a row with a gap either side block that
                        // move
                    }
                }
            break;
            case 4 :
                for (x=0;x<cols;x++)
                {
                    if (col[x])
                    {
                        col[x]+=WinPotential(playgrid,win->tokens,x,InsertTokenTarget(playgrid,x),thisplayer,otherplayer,40);
                    }
                }
            break;
        }
    }
    ret=RandomMoveFromList(cols,col);
    sprintf(tmp,"%d %d %d %d %d %d %d",col[0],col[1],col[2],col[3],col[4],col[5],col[6]);
    TextBoxFixedAddLine(msgbox,tmp);
    ViewSysWindowChanged(vs,3,TRUE);    
    free(col);
    return ret;
}

int AIRandomMove(GRID *grid)
// purpose : find a column which has space for a token to be inserted
// returns : -1 if unable to make a move, else the moves column is returned
{
    return RuleMove(grid,0,PlayerGroupPlayer(players,player_current),OtherPlayer());
}

void TitleScreen(void)
// uses global variables
{
    VIEWPORT vp;
    PLAYER *plname=PlayerNew("",RED,Human,NULL);
    PLAYER *plback=PlayerNew("",YELLOW,Human,NULL);
    PLAYER *pl;
    GRID *filegrid=GridLoadFull(players,"NINARO.TTL");
    int x,tokens,scol; // source col
    int up=1,add=0;
    long sety,adds[3]={0x010100L,0x000101L,0x010001L};
    long bottom=0x010000L,top=bottom+0x030303L;

    if (filegrid==NULL) return;
    ResetViewport(&vp,vs->main.x1,vs->main.y1,vs->main.x2,vs->main.y2-30);
    tokens=MaxTokensInGrid(filegrid);
   _remappalette(YELLOW,0x020202L); _remappalette(GREEN,0x040404L);
    StartRandom();
    ResetGameSize(GridCols(filegrid),GridRows(filegrid),4);
    _setcolor(WHITE); _setcharsize(20,10); _setcharspacing(0);
    _settextalign(_CENTER,_TOP);
    _grtext(vp.x1+((vp.x2-vp.x1)/2),vp.y2+5,"Hit Any Key To Continue...");
    SwapPage();
    _grtext(vp.x1+((vp.x2-vp.x1)/2),vp.y2+5,"Hit Any Key To Continue...");
    SwapPage();
    SetViewport(&(vp));
    _setcolor(GREEN); DisplayGrid(playgrid,0.95);
    sety=bottom;
    x=0;
    do
    {
        if (x<tokens)
        {
            scol=AIRandomMove(playgrid);
            pl=RemoveToken(filegrid,scol);
            InsertTokenInCol(playgrid,scol,PlayerGroupFind(players,pl)==0?plback:plname,win);
            _setcolor(GREEN); DisplayGrid(playgrid,0.95); SwapPage();
            x++;
        }
        if (up)
        {
            sety+=adds[add]; if (add==2) add=0; else ++add;
            if (sety>top) up=0;
        }
        else // down
        {
            sety-=adds[add]; if (add==2) add=0; else ++add;
            if (sety<bottom) up=1;
        }
        _remappalette(YELLOW,sety);
    }
    while (!kbhit());
    _setcolor(GREEN); DisplayGrid(playgrid,0.95); SwapPage();
    GetKey();
    GridFree(filegrid);
    PlayerFree(plname);
    PlayerFree(plback);
}

void Test(void)
{
    Buzz(800,900,50,400);
}

void ToggleSound(void)
{
    ToggleMute();
    GameStatus();
}

int ExtraFunctionality(int key)
// returns : TRUE if key was used else FALSE
{
    switch (key)
    {
        case 'h' : case F1  :
            KeyGroupCopy(pkeys,p2keys);
            // F1 to help defaults to numeric keys
            HelpContentsFunc(); 
            ViewSysDisplay(vs,DisplayCurrent); SwapPage(); break;
        case 'm' : case ESC :
            KeyGroupCopy(pkeys,p2keys);
            // ESC to menu defaults to numeric keys
            MenuCall(mainmenu,HelpMenu);
            ViewSysDisplay(vs,DisplayAll); // none or all could have changed
            SwapPage();
            break;
        case AF4 : case CX : case CQ :
        case F10 : ExitGame(); break;
        default : return FALSE; break;
    } // switch
    return TRUE;
}

void AnimateMove(int col)
// post : keypos=col
{
    int goright=col>keypos;
    int dist=(goright?col-keypos:keypos-col);
    int del=0;
    if (dist==0) return;
    del=(ai_speed*ai_delay)/(dist);   // ai_speed is a half second delay
    do
    {
        if (goright) KeyRight(NULL); else KeyLeft(NULL);
#ifndef TESTING
        ViewSysWindowChanged(vs,0,TRUE); // entry grid
        ViewSysDisplay(vs,DisplayChanged); SwapPage();
        delay(del); // delay appropriate amount of time as per ai_speed 
#endif
    } while (keypos!=col);
}

void MouseMove(int x,int y,int width,int height)
{
    PLAYER *pl=RemoveToken(entrygrid,keypos);
    keypos=x*grid_cols/width;
    InsertTokenAtCR(entrygrid,keypos,0,pl);
    ViewSysWindowChanged(vs,0,TRUE); // entrygrid
}

int GetMouseCol(int x)
// returns the column the mouse is in, -1 if invalid x
{
    int width;
    if (x<vs->main.x1) return -1;
    if (x>vs->main.x2) return -1;
    width=(vs->main.x2-vs->main.x1+1);
    return (x*grid_cols/width);
}

void SetMouseCol(int col)
// sets the mouse pos to the middle of the col
{
    int width=(vs->main.x2-vs->main.x1+1)/grid_cols; // col width
    SetMousePos(vs->main.x1+(col*width)+(width/2),vs->main.y1);
}

int HumanMove(GRID *grid)
// returns column, returns -1 if anything other than a move occurs
{
    int key,index=0,funcret,mx,my,newpos,btn;
//    char bfg[20];

    SetMouseCol(keypos);
    
//  Buzz(400,500,50,200);
    for (;;)
    {
        funcret=FALSE;
        if (kbhit())
        {
            key=GetKey();
            if (KeySysKey(ks,key,NULL,&funcret,&index)) // key was used
            {
                switch (index)
                {
                    case 0 : case 1 : // left or right
                        SetMouseCol(keypos);
                        ViewSysDisplay(vs,funcret);
                        if (funcret!=NoDisplay) SwapPage(); break;
                    case 2 : case 5 : // up or cancel = menu
                        ViewSysDisplay(vs,DisplayCurrent);
                        SwapPage();
                        return -1; // menu call
                    case 3 : case 4 : // down or select
                        return keypos;
                }
            }
            else // provide extra functionality
            {
                if (ExtraFunctionality(key)) return -1;
                // else key does nothing
            }
        }

        btn=GetMouseInput(&mx,&my);
        mx=mx%640;
        if (mx<0) mx+=640;
        switch (btn)
        {
            case LMB :
                return keypos;
            case RMB :
                funcret=KeyUp(NULL);
                ViewSysDisplay(vs,funcret);
                if (funcret!=NoDisplay) SwapPage();
                return -1; // menu call
                break;
            case NMB :
                newpos=GetMouseCol(mx);
                if (newpos>=0) // valid column
                {
                    if (newpos!=keypos)
                    {
                        InsertTokenAtCR(entrygrid,newpos,0,
                            RemoveTokenAtCR(entrygrid,keypos,0));
                        keypos=newpos;
                        ViewSysWindowChanged(vs,0,TRUE);
//                        sprintf(bfg,"%d",mx);
//                        TextBoxFixedAddLine(msgbox,bfg);
//                        ViewSysWindowChanged(vs,3,TRUE);
                        ViewSysDisplay(vs,DisplayChanged);
                        SwapPage();
                    }
                }
//                else Buzz(200,300,4,20);
                // else mouse pos bad
                break;
        } // switch
    }
}

int P1Move(GRID *grid)
{
    KeyGroupCopy(pkeys,p1keys);
    return HumanMove(grid);
}

int P2Move(GRID *grid)
{
    KeyGroupCopy(pkeys,p2keys);
    return HumanMove(grid);
}

int AINoviceMove(GRID *grid)
// purpose : find a column which has space for a token to be inserted
// returns : -1 if unable to make a move, else the moves column is returned
{
    return RuleMove(grid,1,PlayerGroupPlayer(players,player_current),OtherPlayer());
}

int AINormalMove(GRID *grid)
// purpose : find a column which has space for a token to be inserted
// returns : -1 if unable to make a move, else the moves column is returned
{
    return RuleMove(grid,2,PlayerGroupPlayer(players,player_current),OtherPlayer());
}

int AIExpertMove(GRID *grid)
// purpose : find a column which has space for a token to be inserted
// returns : -1 if unable to make a move, else the moves column is returned
{
    return RuleMove(grid,3,PlayerGroupPlayer(players,player_current),OtherPlayer());
}

int AIGeniusMove(GRID *grid)
// purpose : find a column which has space for a token to be inserted
// returns : -1 if unable to make a move, else the moves column is returned
{
    return RuleMove(grid,4,PlayerGroupPlayer(players,player_current),OtherPlayer());
}

void SuggestMove(void)
{
    PLAYER *pl=PlayerGroupPlayer(players,6); // AI genius
    int col;
    char old_mask[ 8 ];
    char new_mask[ 8 ] = { 0xa2, 0x15, 0xa2, 0x10,
                       0xa2, 0x10, 0xa2, 0x15 };

    _getfillmask( old_mask );
    _setfillmask( new_mask );

    col=pl->getmove(playgrid);
    ViewSysDisplay(vs,DisplayCurrent); SwapPage();
    if (keypos==col)
    {
        ViewSysWindowChanged(vs,0,TRUE); // entry grid
        ViewSysDisplay(vs,DisplayChanged); SwapPage();
        delay(ai_speed*ai_delay);
    }
    else AnimateMove(col);
    _setfillmask( old_mask );
}

VIEWSYS *ViewSysInit(int x1,int y1,int x2,int y2,int maxwindows)
{
    VIEWSYS *vs=ViewSysNew(x1,y1,x2,y2,maxwindows,ClearViewport);
    ViewSysWindowNew(vs,0.0,0.0,0.6,0.08,TRUE,
        DisplayEntryGrid,MouseMove,"Enter Your Token Here");      // entrygrid is 0
    ViewSysWindowNew(vs,0.0,0.09,0.6,1.0,TRUE,
        DisplayPlayGrid,MouseMove,"Enter Your Token Here");       // playgrid is 1
    ViewSysWindowNew(vs,0.61,0.0,1.0,0.1,TRUE,
        DisplayGameName,NULL,"Ninaro - By the author 1998");
    ViewSysWindowNew(vs,0.61,0.52,1.0,1.0,TRUE,
        DisplayMsgBox,NULL,"Messages appear here");
    ViewSysWindowNew(vs,0.61,0.1,1.0,0.37,TRUE,
        DisplayStatusBox,NULL,"The Game Status");
    ViewSysWindowNew(vs,0.61,0.4,0.65,0.448,TRUE,
        DisplayP1Type,NULL,"Player One's Type");
    ViewSysWindowNew(vs,0.66,0.4,0.90,0.448,TRUE,
        DisplayP1Name,NULL,"Player One's Name");
    ViewSysWindowNew(vs,0.91,0.4,1.0,0.448,TRUE,
        DisplayP1Score,NULL,"Player One's Score");
    ViewSysWindowNew(vs,0.61,0.452,0.65,0.5,TRUE,
        DisplayP2Type,NULL,"Player Two's Type");
    ViewSysWindowNew(vs,0.66,0.452,0.90,0.5,TRUE,
        DisplayP2Name,NULL,"Player Two's Name");
    ViewSysWindowNew(vs,0.91,0.452,1.0,0.5,TRUE,
        DisplayP2Score,NULL,"Player Two's Score");
    return vs;
}

void ProgInit() // initialises global variables
{
    vs=ViewSysInit(0,0,639,349,20);
        
    players=PlayerGroupNew(8, // allows for only two humans
        PlayerNew("Player One",YELLOW,Human,P1Move),
        PlayerNew("Player Two",RED,Human,P2Move),
        PlayerNew("AI Random",ORANGE,Computer,AIRandomMove),
        PlayerNew("AI Novice",GREEN,Computer,AINoviceMove),
        PlayerNew("AI Normal",BLUE,Computer,AINormalMove),
        PlayerNew("AI Expert",YELLOW,Computer,AIExpertMove),
        PlayerNew("AI Genius",PURPLE,Computer,AIGeniusMove),
        PlayerNew("Empty AI",GREEN,Computer,NULL));
    pkeys=KeyGroupNew(6,0,0,0,0,0,0); //KP4,KP6,KP8,KP2,ENTER,BKSP);
    p1keys=KeyGroupNew(6,'a','d','w','s','e','q');
    p2keys=KeyGroupNew(6,KP4,KP6,KP8,KP2,ENTER,BKSP);
    ResetScores();

        hilite=GREEN;
        text=WHITE;
        back=BLACK;

    win=malloc(sizeof(WIN));
    if (!LoadCfg()) // no cfg file
    {
        win->tokens=4;
        grid_rows=8;
        grid_cols=7;
        SetP1(0);
        SetP2(1);
        goesfirst=GFSuccessive;
        wentfirst=player_one;
        ai_speed=2; // normal
        ai_delay=500; // half a second
    }
    win->win=malloc(win->tokens*sizeof(TOKEN));
    win_occured=FALSE;
                
    msgbox=TextBoxNew(7,RED,&text,&hilite,30);
    TextBoxFixedAddLine(msgbox,"Press F1 for HELP");
    TextBoxFixedAddLine(msgbox,"Press Esc for MENU");
    TextBoxFixedAddLine(msgbox,"Which uses arrow keys");
    TextBoxFixedAddLine(msgbox,"Press F10 to QUIT");
    TextBoxFixedAddLine(msgbox,"~Or begin playing...");
    
    statusbox=TextBoxNew(3,NOBORDER,&text,&hilite,30);
    TextBoxAddLine(statusbox," ",ATEND);
    TextBoxAddLine(statusbox," ",ATEND);
    TextBoxAddLine(statusbox," ",ATEND);
    
    playgrid=GridNew(grid_cols,grid_rows);
    entrygrid=GridNew(grid_cols,1);
    playerdialog=DialogNew(pkeys,ModifyPlayer,"Player One",2,
        "Player",TRUE,TRUE,ControlNew(List,Radio,3,2,13,6,
                        1,"Human",
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
        "Connect",TRUE,TRUE,ControlNew(Range,Slider,1,1,10,
                        3,"",MaxOf(grid_rows,grid_cols),""));
/*
    gameoptionsdialog=DialogNew(pkeys,ModifyGameOptions,"Game Options",3,
        "Background",TRUE,TRUE,ControlNew(List,Color,1,2,11,
                        0,
//         black or white
        "Hilite"
//         red,green,blue,yellow,white
        
        hilite color red
        text color black
*/
    optionsdialog=DialogNew(pkeys,NULL,"Player Options",2,
        "Goes First",TRUE,TRUE,ControlNew(List,Radio,3,2,13,6,
                        0,"Player One",
                        1,"Player Two",
                        2,"Loser",
                        3,"Winner",
                        4,"Successive",
                        5,"Random"),
        "AI Speed",TRUE,TRUE,ControlNew(List,Slider,1,1,7,5,                        
                        4,"Slowest",
                        3,"Slow",
                        2,"Normal",
                        1,"Fast",
                        0,"Fastest"));
                        
/*        "Best Of",TRUE,TRUE,ControlNew(List,Radio,3,2,13,6,
0,"Infinity",3,"Three",5,"Five",11,"Eleven",25,"Twenty-Five",51,"Fifty-One"));
*/            
    mainmenu=MenuNew(4,&(vs->main),ViewSysCurrent(vs),pkeys,&hilite);
    MenuNewSub(mainmenu,"Game",7,
        "New",      CN, NewGame,FALSE,
        "Load",   NONE, LoadGame,TRUE,
        "Save",   NONE, SaveGame,   TRUE,
        "End",      CE, EndGame,    TRUE,
        "Suggest Move",NONE,SuggestMove,TRUE,
        "Toggle Sound",NONE,ToggleSound,TRUE,
//        "Options",NONE,GameOptions,TRUE,
        "Exit",     CX, ExitGame,   TRUE);
    MenuNewSub(mainmenu,"Board",3,
        "7x8x4",    NONE,GameSize7x8x4,      TRUE,
        "12x13x6",  NONE,GameSize12x13x6,     TRUE, 
        "Custom...",NONE,GameSizeCustom,      TRUE);
    MenuNewSub(mainmenu,"Player",4,
        "One...",   NONE,P1Dialog,      FALSE,
        "Two...",   NONE,P2Dialog,      TRUE,
        "Options...",NONE,PlayerOptions,TRUE,
        "Reset Scores",NONE,ResetScores,TRUE);
//        "Swap",     NONE,PlayerSwap,  TRUE);
    MenuNewSub(mainmenu,"Help",2,
        "Contents", NONE,HelpContentsFunc,TRUE,
        "About",    NONE,HelpAbout, TRUE);

    ks=KeySysNew(pkeys);
    KeySysIFunc(ks,0,KeyLeft);
    KeySysIFunc(ks,1,KeyRight);
    KeySysIFunc(ks,2,KeyUp);
    KeySysIFunc(ks,3,KeyDown);
    KeySysIFunc(ks,4,KeySelect);
    KeySysIFunc(ks,5,KeyCancel);

    help=HelpNew("NINARO.HLP",pkeys,20,RED,&text,&hilite,60);

    aboutgame=HelpNew(NULL,pkeys,3,LTBLUE,&text,&hilite,29);
    HelpAddLine(aboutgame,"~Scroll Up/Down, Any Key OK's");
    HelpAddLine(aboutgame,"Project");
    HelpAddLine(aboutgame,"By the author");
    HelpAddLine(aboutgame,"");
    HelpAddLine(aboutgame,"      Semester One 1998");
    HelpAddLine(aboutgame,"");
    HelpAddLine(aboutgame,"       Copyright 1998");
    HelpAddLine(aboutgame,"        Version 0.9b");
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
    PLAYER *pl;
    int waitesc,col,ccols,crows,csize,mb,mx,my;

    ScreenInit();
    _setcolor(RED);
    _setcharsize_w(0.02,0.02);
    _setcharspacing_w(-(0.01));
    _registerfonts( "*.fon" );
    _setfont("t'Modern'");
    _settextalign(_CENTER,_HALF);
    ProgInit(); // some parts rely on graphics mode
 
    ccols=grid_cols; crows=grid_rows; csize=win->tokens;
    TitleScreen();
    ResetGameSize(ccols,crows,csize);
    NewGame();
    GameStatus();
    ViewSysDisplay(vs,DisplayAll);  // initialises viewsys current
    SwapPage();
    ResetPalette();
    GetMouseInput(&mx,&my);            
    for (;;)
    {
        while (ingame)
        {
            pl=PlayerGroupPlayer(players,player_current);
            col=pl->getmove(playgrid);
            if (col==-1)
            {

            }
            if (kbhit())
            {
                ExtraFunctionality(GetKey());
            }
            else if (col>=0) // valid move
            {
                if (col!=keypos) // animate move
                {
                    AnimateMove(col);
                }   // now col==keypos;
                keypos=col;
                if (DropToken()!=Failure)
                {
                    if ((win_occured) || (isdraw))
                    {
                        waitesc=((ai_speed) || (!BothPlayerAI()));
                        if (waitesc)
                            TextBoxFixedAddLine(msgbox,"~Press Cancel To Continue");
                        ViewSysDisplay(vs,DisplayAll); SwapPage();
                        NewGame();
                        if (waitesc)
                        {
                            for (;;)
                            {
                                if (kbhit())
                                {
                                    // find relevant cancel key/s
                                    // or esc or RMB if p1=human
                                    GetKey(); break;
                                }
                            }
                        }
#ifndef TESTING                        
                        ViewSysDisplay(vs,DisplayAll); SwapPage();
#endif
                    }
                    else // just a normal move
                    {
#ifndef TESTING                        
                        ViewSysDisplay(vs,DisplayChanged); SwapPage();
#endif
                    }
                }
            }
        } // while (ingame)
        ExtraFunctionality(GetKey());
    }
}

