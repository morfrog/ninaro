#ifndef GRID_H
#define GRID_H

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
};

typedef struct gridTag GRID;

GRID *NewGrid(int width,int height);
void GridFree(GRID *grid);
void DisplayGrid(GRID *grid);
int InsertTokenAtCR(GRID *grid,int col,int row,PLAYER *player);
PLAYER *RemoveTokenAtCR(GRID *grid,int row,int col);
int MoveTokenAtCR(GRID *grid,int col,int row,int direction);
int StatusOfCR(GRID *grid,int col,int row);
int TokensInGrid(GRID *grid);
int NumInDirection(GRID *grid,int col,int row,int col_mod,int row_mod,PLAYER *player);
void MoveAll(GRID *grid,int col_dir,int row_dir,int wrap);
void GridEmpty(GRID *grid);

#endif
