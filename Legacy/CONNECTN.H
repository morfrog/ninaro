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
int CheckAngle(GRID *grid,int size,int col,int row,int col_dir,int row_dir,WIN *win);
int CheckForWin(GRID *grid,int size,int col,int row,WIN *win);
void SetWinSize(WIN *win,int size);
int GameSave(int p1,int p2,int current,GRID *grid,PLAYERGROUP *pg,
                char *filename);
GRID *GameLoad(int *p1,int *p2,int *current,PLAYERGROUP *pg,char *filename);                
#endif
