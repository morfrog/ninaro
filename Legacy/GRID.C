#include <stdlib.h>
#include "sys.h"
#include "screen.h"
#include "grid.h"

GRID *NewGrid(int width,int height)
{
    int x,y;
    GRID *ret=malloc(sizeof(GRID));

    ret->width=width;
    ret->height=height;
    ret->grid=malloc(width*sizeof(PLAYER **));
    for (x=0;x<width;x++) ret->grid[x]=malloc(height*sizeof(PLAYER *));
    ret->numincol=malloc(width*sizeof(int));
    ret->numinrow=malloc(height*sizeof(int));
    for (x=0;x<width;x++)
        for (y=0;y<height;y++)
            ret->grid[x][y]=NULL;

    for (x=0;x<width;x++)  ret->numincol[x]=0;
    for (y=0;y<height;y++) ret->numinrow[y]=0;
    return ret;
}

void DisplayGrid(GRID *grid)
{
    int x,y;
    DrawGrid(grid->width,grid->height);
    for (x=0;x<grid->width;x++)
        for (y=0;y<grid->height;y++)
        {
            if (grid->grid[x][y]!=NULL) // not empty
                DrawDisc(x,grid->width,y,grid->height,PlayerGetColor(grid->grid[x][y]),0.60);
        }
}

int InsertTokenAtCR(GRID *grid,int col,int row,PLAYER *player)
{
    if (grid->grid[col][row]==NULL)
    {
        grid->grid[col][row]=player;
        grid->numincol[col]+=1;
        grid->numinrow[row]+=1;
        return 1;
    }
    else
        return 0;
}

PLAYER *RemoveTokenAtCR(GRID *grid,int col,int row)
// removes the token at RC and returns the player number who was there,
// a zero return means the position was already empty.
{
    PLAYER *ret=grid->grid[col][row];
    if (ret!=0) // is a token there;
    {
        grid->grid[col][row]=NULL;
        grid->numincol[col]-=1;
        grid->numinrow[row]-=1;
    }
    return ret;
}

int MoveTokenAtCR(GRID *grid,int col,int row,int direction)
// moves the grid token at col,row one space in direction if possible
// to do so without rejecting token from grid, returns TRUE if moved token
// else FALSE
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

//PLAYER *StatusOfCR(GRID *grid,int col,int row)
// returns the value of the token at rc, NULL = empty else = player * 
//{return grid->grid[col][row];}

int TokensInGrid(GRID *grid)
{
    int x,ret=0;
    
    for (x=0;x<grid->width;x++)
        ret+=grid->numincol[x];
    return ret;
}

int NumInDirection(GRID *grid,int col,int row,int col_mod,int row_mod,
                    PLAYER *player)
// starting at row,col in grid, modifies current position by
// col_mod and row_mod till token of same type not found then returns
// the number of such tokens found 
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
// moves all tokens in the grid across col_dir and down row_dir
// if moving a token in this way causes it to fall off the grid,
// wrap determines if it cycles to the other side(TRUE) or is
// lost entirely(FALSE)
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
