#ifndef CONNECT4_H
#define CONNECT4_H

#include "grid.h"
#include "player.h"

struct TokenTag
{
    int row;
    int col;
};

typedef struct TokenTag TOKEN;

struct winTag
{
    int tokens;
    TOKEN *win; // array of tokens which make up the win
};

typedef struct winTag WIN;

int Continuance(GRID *grid,int size);
int InsertTokenTarget(GRID *grid,int col);
int InsertToken(GRID *grid,int col,PLAYER *player);
int CheckAngle(GRID *grid,int size,int col,int row,int col_dir,int row_dir,WIN *win);
int CheckForWin(GRID *grid,int size,int col,int row,WIN *win);

#endif
