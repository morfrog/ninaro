#ifndef GRID_H
#define GRID_H

#include "binfile.h"
#include "player.h"

#define T_UP 0
#define T_DOWN 1
#define T_LEFT 2
#define T_RIGHT 3

struct gridTag
{
    int width;
    int height;
    PLAYER ***grid; // set as NULL = empty, else = player *
    int *numincol; // how many tokens are in the columns
    int *numinrow; // how many tokens are in the rows
    int tokensingrid;
};

typedef struct gridTag GRID;

GRID *GridNew(int width,int height);
// purpose : to create a new grid, of width columns and height rows
// returns : the grid
int GridRows(GRID *grid);
// purpose : to get the number of rows in the grid
// returns : void
int GridInRow(GRID *grid,int row);
// purpose : how many tokens are in row of grid, row = 0..
// returns : -1 of row out of range, else the number
int GridCols(GRID *grid);
// purpose : to get the number of columns in the grid
// returns : void
int GridInCol(GRID *grid,int col);
// purpose : how many tokens are in col of grid, col = 0..
// returns : -1 of row out of range, else the number
void GridEmpty(GRID *grid);
// purpose : empties the grid of all tokens
// returns : void
void GridFree(GRID *grid);
// purpose : to free the memory allocated to the grid
// returns : void
void DisplayGrid(GRID *grid,double size);
// purpose : to display the grid in the current viewport, tokens being of size
// returns : void
int InsertTokenAtCR(GRID *grid,int col,int row,PLAYER *player);
// purpose : to insert a player token at col,row in the grid
// returns : TRUE if col,row was empty and token inserted, else FALSE
int ResetTokenAtCR(GRID *grid,int col,int row,PLAYER *player);
// purpose : set token at cr to player
// returns : FALSE if col,row was empty, else TRUE
PLAYER *RemoveTokenAtCR(GRID *grid,int col,int row);
// purpose : removes the token at col,row
// returns : the player * who was there, a NULL return means the position
//           was already empty.
int MoveTokenAtCR(GRID *grid,int col,int row,int direction);
// purpose : moves the grid token at col,row one space in direction if
//           possible to do so without rejecting token from grid,
// returns : TRUE if moved token else FALSE
PLAYER *StatusOfCR(GRID *grid,int col,int row);
// purpose : to find which player is at col,row
// returns : the played token at rc, NULL = empty else = player * 
int TokensInGrid(GRID *grid);
// purpose : to get the number of tokens in the grid
// returns : how many tokens are in the grid (0..MaxTokensInGrid)
int MaxTokensInGrid(GRID *grid);
// purpose : to get how many tokens the grid can contain
// returns : the number
int NumInDirection(GRID *grid,int col,int row,int col_mod,int row_mod,
                    PLAYER *player);
// purpose : to find how many player tokens lie in direction(col_mod,row_mod)
//           starting at row,col in grid, modifies current position by
//           col_mod and row_mod till token of same type not found then
// returns : the number of such tokens found 
void MoveAll(GRID *grid,int col_dir,int row_dir,int wrap);
// purpose : moves all tokens in the grid across col_dir and down row_dir
//           if moving a token in this way causes it to fall off the grid,
//           wrap determines if it cycles to the other side(TRUE) or is
//           lost entirely(FALSE)
// returns : void
int GridSaveSize(GRID *grid);
// purpose : to find how many bytes a grid requires to be saved
// returns : the number
void GridSave(BINFILE *bf,GRID *grid,PLAYERGROUP *pg);
// purpose : saves the grid to the binfile, assumes all players have
//           unique ID's
// returns : void
void GridSaveFull(GRID *grid,PLAYERGROUP *pg,char *gridname);
// purpose : to save the grid to gridname file
// returns : void
GRID *GridLoad(BINFILE *bf,PLAYERGROUP *pg);
// purpose : load a grid
// returns : NULL if load unsuccessful, else the grid
GRID *GridLoadFull(PLAYERGROUP *pg,char *gridname);
// purpose : to load grid from file gridname
// returns : NULL if couldn't read file, else the grid is returned

#endif
