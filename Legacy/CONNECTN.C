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

int RemoveTokenTarget(GRID *grid,int col)
{
    return (grid->height)-(grid->numincol[col]);
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

int CheckAngle(GRID *grid,int size,int col,int row,int col_dir,int row_dir,WIN *win,PLAYER *player)
{
    int left,right,start_col,start_row,x;
    left=NumInDirection(grid,col,row,col_dir,row_dir,player);
    if (left>=size) left=size-1;    // ensures token is a part of the win
    right=NumInDirection(grid,col,row,-col_dir,-row_dir,player);
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
    // check vertical    0  -1
    if (CheckAngle(grid,size,col,row,0,-1,win,player)) return TRUE;
    // check horizontal -1   0
    else if (CheckAngle(grid,size,col,row,-1,0,win,player)) return TRUE;
    // check /          +1  -1
    else if (CheckAngle(grid,size,col,row,1,-1,win,player)) return TRUE;
    // check \          -1  -1
    else if (CheckAngle(grid,size,col,row,1,1,win,player)) return TRUE;
    else return FALSE;
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

