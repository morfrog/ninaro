#include <stdlib.h>
#include "defines.h"
#include "sys.h"
#include "screen.h"
#include "binfile.h"
#include "grid.h"

void GridEmpty(GRID *grid)
// purpose : empties the grid of all tokens
// returns : void
{
    int x,y;
    for (x=0;x<grid->width;x++)
        for (y=0;y<grid->height;y++)
            grid->grid[x][y]=NULL;
    for (x=0;x<grid->width;x++)  grid->numincol[x]=0;
    for (y=0;y<grid->height;y++) grid->numinrow[y]=0;
}

GRID *GridNew(int width,int height)
// purpose : to create a new grid, of width columns and height rows
// returns : the grid
{
    int x;
    GRID *ret=malloc(sizeof(GRID));
    ret->width=width;
    ret->height=height;
    ret->grid=malloc(width*sizeof(PLAYER **));
    for (x=0;x<width;x++) ret->grid[x]=malloc(height*sizeof(PLAYER *));
    ret->numincol=malloc(width*sizeof(int));
    ret->numinrow=malloc(height*sizeof(int));
    GridEmpty(ret);
    return ret;
}

int GridRows(GRID *grid)
// purpose : to get the number of rows in the grid
// returns : void
{
    return grid->height;
}

int GridInRow(GRID *grid,int row)
// purpose : how many tokens are in row of grid, row = 0..
// returns : -1 of row out of range, else the number
{
    if (row>=grid->height) return -1;
    return (grid->numinrow[row]);
}

int GridCols(GRID *grid)
// purpose : to get the number of columns in the grid
// returns : void
{
    return grid->width;
}

int GridInCol(GRID *grid,int col)
// purpose : how many tokens are in col of grid, col = 0..
// returns : -1 of row out of range, else the number
{
    if (col>=grid->width) return -1;
    return (grid->numincol[col]);
}

void GridFree(GRID *grid)
// purpose : to free the memory allocated to the grid
// returns : void
{
    int x;
    for (x=0;x<grid->width;x++) free(grid->grid[x]);
    free(grid->grid);
    free(grid);
}

void DisplayGrid(GRID *grid,double size)
// purpose : to display the grid in the current viewport, tokens being of size
// returns : void
{
    int x,y;
    DrawGrid(grid->width,grid->height);
    for (x=0;x<grid->width;x++)
        for (y=0;y<grid->height;y++)
        {
            if (grid->grid[x][y]!=NULL) // not empty
                DrawDisc(x,grid->width,y,grid->height,PlayerGetColor(grid->grid[x][y]),size);
        }
}

int InsertTokenAtCR(GRID *grid,int col,int row,PLAYER *player)
// purpose : to insert a player token at col,row in the grid
// returns : TRUE if col,row was empty and token inserted, else FALSE
{
    if ((grid->grid[col][row]==NULL) && (player!=NULL))
    {
        grid->grid[col][row]=player;
        grid->numincol[col]+=1;
        grid->numinrow[row]+=1;
        return TRUE;
    }
    else
        return FALSE;
}

int ResetTokenAtCR(GRID *grid,int col,int row,PLAYER *player)
// purpose : set token at cr to player
// returns : FALSE if col,row was empty, else TRUE
{
    if (grid->grid[col][row]==NULL)
    {
        InsertTokenAtCR(grid,col,row,player);
        return FALSE;
    }
    else
    {
        grid->grid[col][row]=player;
        return TRUE;
    }
}

PLAYER *RemoveTokenAtCR(GRID *grid,int col,int row)
// purpose : removes the token at col,row
// returns : the player * who was there, a NULL return means the position
//           was already empty.
{
    PLAYER *ret=grid->grid[col][row];
    if (ret!=NULL) // is a token there;
    {
        grid->grid[col][row]=NULL;
        grid->numincol[col]-=1;
        grid->numinrow[row]-=1;
    }
    return ret;
}

int MoveTokenAtCR(GRID *grid,int col,int row,int direction)
// purpose : moves the grid token at col,row one space in direction if
//           possible to do so without rejecting token from grid,
// returns : TRUE if moved token else FALSE
{
    int targetcol=col,targetrow=row;
    
    switch (direction)
    {
        case T_UP     : --targetrow; break;
        case T_DOWN   : ++targetrow; break;
        case T_LEFT   : --targetcol; break;
        case T_RIGHT  : ++targetcol; break;
    }
    if ((targetrow<0) || (targetrow>grid->height-1) ||
        (targetcol<0) || (targetcol>grid->width-1)) // out of grid
        return FALSE;
    if (grid->grid[targetcol][targetrow]!=NULL)
        return FALSE; // token already there

    grid->grid[targetcol][targetrow]=grid->grid[col][row];
    grid->grid[col][row]=NULL;

    switch (direction)
    {
        case T_UP   : 
        case T_DOWN : grid->numinrow[row]--; grid->numinrow[targetrow]++; break;
        case T_LEFT : 
        case T_RIGHT: grid->numincol[col]--; grid->numincol[targetcol]++; break;
    }

    return TRUE;
}

PLAYER *StatusOfCR(GRID *grid,int col,int row)
// purpose : to find which player is at col,row
// returns : the played token at rc, NULL = empty else = player * 
{return grid->grid[col][row];}

int TokensInGrid(GRID *grid)
// purpose : to get the number of tokens in the grid
// returns : how many tokens are in the grid (0..MaxTokensInGrid)
{
    int x,ret=0;
    
    for (x=0;x<grid->width;x++)
        ret+=grid->numincol[x];
    return ret;
}

int MaxTokensInGrid(GRID *grid)
// purpose : to get how many tokens the grid can contain
// returns : the number
{
    return (grid->width*grid->height);
}

int NumInDirection(GRID *grid,int col,int row,int col_mod,int row_mod,
                    PLAYER *player)
// purpose : to find how many player tokens lie in direction(col_mod,row_mod)
//           starting at row,col in grid, modifies current position by
//           col_mod and row_mod till token of same type not found then
// returns : the number of such tokens found 
{
    int cur_col=col;
    int cur_row=row;
    int number=0;

//    if (col_mod==row_mod==0) return 0;
    for (;;)
    {
        cur_col+=col_mod;
        cur_row+=row_mod;
        if ((cur_col>grid->width-1) || (cur_col<0) ||
            (cur_row>grid->height-1) || (cur_row<0)) // then out of range
            return number;
        else if (grid->grid[cur_col][cur_row]!=player) // then another player found
            return number;
        else // in range and same player
            ++number;        
    }
}

void MoveAll(GRID *grid,int col_dir,int row_dir,int wrap)
// purpose : moves all tokens in the grid across col_dir and down row_dir
//           if moving a token in this way causes it to fall off the grid,
//           wrap determines if it cycles to the other side(TRUE) or is
//           lost entirely(FALSE)
// returns : void
{
    int x,y,t_col,t_row,outside;
    PLAYER ***copygrid=malloc(grid->width*sizeof(PLAYER **));
    for (x=0;x<grid->width;x++) copygrid[x]=malloc(grid->height*sizeof(PLAYER *));

    for (x=0;x<grid->width;x++)
        for (y=0;y<grid->height;y++)
            copygrid[x][y]=NULL;
    for (x=0;x<grid->width;x++)
        for (y=0;y<grid->height;y++)
        {   
            outside=0;
            t_col=x+col_dir;
            t_row=y+row_dir;
            if (t_col>grid->width-1) {t_col-=grid->width; outside=1;}
            else if (t_col<0) {t_col+=grid->width; outside=1;}
            if (t_row>grid->height-1) {t_row-=grid->height; outside=1;}
            else if (t_row<0) {t_row+=grid->height; outside=1;}
            
            if ((!outside) || (wrap)) copygrid[t_col][t_row]=grid->grid[x][y];
        }
    for (x=0;x<grid->width;x++)
        for (y=0;y<grid->height;y++)
            grid->grid[x][y]=copygrid[x][y];
    
    for (x=0;x<grid->width;x++) grid->numincol[x]=0;
    for (y=0;y<grid->height;y++) grid->numinrow[y]=0;

    for (x=0;x<grid->width;x++) 
        for (y=0;y<grid->height;y++)
            if (grid->grid[x][y]!=NULL)
            {
                grid->numincol[x]+=1;
                grid->numinrow[y]+=1;
            }
                                
    free(copygrid);
}

int GridSaveSize(GRID *grid)
// purpose : to find how many bytes a grid requires to be saved
// returns : the number
{
    return (sizeof(int)*(2+MaxTokensInGrid(grid)));
}

void GridSave(BINFILE *bf,GRID *grid,PLAYERGROUP *pg)
// purpose : saves the grid to the binfile, assumes all players have
//           unique ID's
// returns : void
{
    int x,y;
    PLAYER *player;

    BinFilePutInt(bf,grid->width);
    BinFilePutInt(bf,grid->height);

    for (x=0;x<grid->width;x++)
        for (y=0;y<grid->height;y++)
        {
            player=grid->grid[x][y];
            BinFilePutInt(bf,player==NULL?0:PlayerGroupFind(pg,player)+1);
        }
}

void GridSaveFull(GRID *grid,PLAYERGROUP *pg,char *gridname)
// purpose : to save the grid to gridname file
// returns : void
{
    BINFILE *bf=BinFileNew(GridSaveSize(grid));
    GridSave(bf,grid,pg);
    BinFileWriteFull(bf,gridname);
    BinFileFree(bf);
}

GRID *GridLoad(BINFILE *bf,PLAYERGROUP *pg)
// purpose : load a grid
// returns : NULL if load unsuccessful, else the grid
{
    int player;
    GRID *ret;
    int width,height,x,y;

    width=BinFileGetInt(bf);
    height=BinFileGetInt(bf);
    ret=GridNew(width,height);
    for (x=0;x<width;x++)
        for (y=0;y<height;y++)
        {
            player=BinFileGetInt(bf);
            if (player!=0) InsertTokenAtCR(ret,x,y,PlayerGroupPlayer(pg,player-1));
        }
    return ret;
}

GRID *GridLoadFull(PLAYERGROUP *pg,char *gridname)
// purpose : to load grid from file gridname
// returns : NULL if couldn't read file, else the grid is returned
{
    BINFILE *bf=BinFileReadFull(gridname,0);
    GRID *ret;
    if (bf==NULL) return NULL; // failed to load file
    ret=GridLoad(bf,pg);
    BinFileFree(bf);
    return ret;
}
