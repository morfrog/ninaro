#ifndef CONNECT4_H
#define CONNECT4_H
#include "grid.h"
#include "player.h"

enum {Win,Success,Failure};

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
int InsertTokenInCol(GRID *grid,int col,PLAYER *player,WIN *win);
int InsertTokenTarget(GRID *grid,int col);
int RemoveTokenTarget(GRID *grid,int col);
PLAYER *RemoveToken(GRID *grid,int col);
// returns PLAYER * if token was successfully removed, else NULL
int CheckAngle(GRID *grid,int size,int col,int row,int col_dir,int row_dir,WIN *win,PLAYER *player);
int CheckForWin(GRID *grid,int size,int col,int row,WIN *win,PLAYER *player);
// checks for win which includes the player token 
// if there isn't one 0 is returned else
// 1 is returned and the tokens which are the win are in the WIN *

void SetWinSize(WIN *win,int size);
int GameSave(int p1,int p2,int current,int size,GRID *grid,PLAYERGROUP *pg,
                char *filename);
GRID *GameLoad(int *p1,int *p2,int *current,int *size,PLAYERGROUP *pg,char *filename);                
#endif
