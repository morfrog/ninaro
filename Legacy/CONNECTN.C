/****************************************************************
Copyright 1998 the author
Module Name    : CONNECT4
Module Purpose : To provide functions for the game connect 4
Creation Date  : 23 March 1998
Updated        : 4 May 1998         RemoveTokenTarget   CREATED
                 5 May 1998         WinPotential        CREATED
                                    AnglePotential      CREATED 
****************************************************************/
#include <stdlib.h>
#include "sys.h"
#include "grid.h"
#include "player.h"
#include "connectn.h"

int Continuance(GRID *grid,int size)
// removes bottom layers of grid such that size-1 high minimum remains
// ( size-1 because all tokens below this are irrelevant )
// and slides the tokens down to the bottom
// returns TRUE if a continuance was made else FALSE if not.
{
    int x;
    int minimum=GridInCol(grid,0);
    int shift;
    int incol;
    for (x=1;x<grid->width;x++)
    {
        incol=GridInCol(grid,x);
        if (incol<minimum) minimum=incol;
    }
    if (minimum<=size-1) return FALSE;
    shift=minimum-size+1;
    MoveAll(grid,0,shift,0);
    return TRUE;
}

int InsertTokenTarget(GRID *grid,int col)
// returns which row the token will fall to.
{
    return (GridRows(grid))-(GridInCol(grid,col))-1;
}

int RemoveTokenTarget(GRID *grid,int col)
{
    return (GridRows(grid))-(GridInCol(grid,col));
}

int InsertToken(GRID *grid,int col,PLAYER *player)
// returns if token was successfully inserted
{
    return InsertTokenAtCR(grid,col,InsertTokenTarget(grid,col),player);
}

PLAYER *RemoveToken(GRID *grid,int col)
// returns PLAYER * if token was successfully removed, else NULL
{
    return RemoveTokenAtCR(grid,col,RemoveTokenTarget(grid,col));
}

int CheckAngle(GRID *grid,int size,int col,int row,int col_dir,int row_dir,
        WIN *win,PLAYER *player,int (* compare)(PLAYER *,PLAYER *))
{
    int left,right,start_col,start_row,x;
    left=FuncNumInDirection(grid,col,row,col_dir,row_dir,player,compare);
    if (left>=size) left=size-1;    // ensures token is a part of the win
    right=FuncNumInDirection(grid,col,row,-col_dir,-row_dir,player,compare);
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

int CheckForWin(GRID *grid,int size,int col,int row,WIN *win,PLAYER *player)
// checks for win which includes the player token 
// if there isn't one 0 is returned else
// 1 is returned and the tokens which are the win are in the WIN *
{
    if (CheckAngle(grid,size,col,row,0,-1,win,player,FuncNotEqual))
        return TRUE; // check vertical    0  -1
    else if (CheckAngle(grid,size,col,row,-1,0,win,player,FuncNotEqual))
        return TRUE; // check horizontal -1   0
    else if (CheckAngle(grid,size,col,row,1,-1,win,player,FuncNotEqual))
        return TRUE; // check /          +1  -1
    else if (CheckAngle(grid,size,col,row,1,1,win,player,FuncNotEqual))
        return TRUE; // check \          -1  -1
    else return FALSE;
}

int AnglePotential(GRID *grid,int size,int col,int row,int col_dir,int row_dir,
    PLAYER *player,PLAYER *other)
{
    int lp,lnull,rp,rnull;
    int left=NumsInDirection(grid,col,row,col_dir,row_dir,player,&lp,
                NULL,&lnull,other,size-1),
         right=NumsInDirection(grid,col,row,-col_dir,-row_dir,player,&rp,
                NULL,&rnull,other,size-1);
    if (left+right+1>=size)
    {
        return ((5*(lp+rp)) + (lnull+rnull));
        // tokens = 5, nulls = 1
    }
    else return 0; // no potential                
}

int WinPotential(GRID *grid,int size,int col,int row,PLAYER *player,
        PLAYER *other)
// purpose : checks for how much potential for wins are around token at
//           (col,row) and
// returns : a number which is a measure of this. the measure is in the
//           range {0..((size*2)*4)}
{
    
    int ret=0;
    ret+=AnglePotential(grid,size,col,row,0,-1,player,other);
                     // check vertical    0  -1
    ret+=AnglePotential(grid,size,col,row,-1,0,player,other);
                     // check horizontal -1   0
    ret+=AnglePotential(grid,size,col,row,+1,-1,player,other);                     
                     // check /          +1  -1
    ret+=AnglePotential(grid,size,col,row,-1,-1,player,other);                     
                     // check \          -1  -1
    return ret;
}

int InsertTokenInCol(GRID *grid,int col,PLAYER *player,WIN *win)
// inserts player token in col on grid, returns Win and sets the win* to
// be the win if this makes a win, else Success if the token is
// successfully inserted, else Failure if the token could not be inserted
// might also animate the insertion
{
    int row=InsertTokenTarget(grid,col);
    if (InsertToken(grid,col,player))
    {
        if (CheckForWin(grid,win->tokens,col,row,win,player))
            return Win;
        else
            return Success;
    }
    else return Failure;
}

void SetWinSize(WIN *win,int size)
{   
    free(win->win);
    win->tokens=size;
    win->win=malloc(size*sizeof(TOKEN));
}

int GameSave(int p1,int p2,int current,int size,GRID *grid,PLAYERGROUP *pg,
                char *filename)
{
    int ret;
    BINFILE *bf=BinFileNew((4*sizeof(int))+GridSaveSize(grid));
    BinFilePutInt(bf,p1);
    BinFilePutInt(bf,p2);
    BinFilePutInt(bf,current);
    BinFilePutInt(bf,size);
    GridSave(bf,grid,pg);       // save grid to bin file
    ret=BinFileWriteFull(bf,filename);
    BinFileFree(bf);
    return ret;
}

GRID *GameLoad(int *p1,int *p2,int *current,int *size,PLAYERGROUP *pg,char *filename)
{
    BINFILE *bf=BinFileReadFull(filename,0);
    GRID *ret;
    if (bf==NULL) return NULL;
    *p1=BinFileGetInt(bf);
    *p2=BinFileGetInt(bf);
    *current=BinFileGetInt(bf);
    *size=BinFileGetInt(bf);
    ret=GridLoad(bf,pg);
    BinFileFree(bf);
    return ret;
}

