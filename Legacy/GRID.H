#ifndef GRID_H

#define T_UP 0
#define T_DOWN 1
#define T_LEFT 2
#define T_RIGHT 3

struct gridTag
{
    int width;
    int height;
    int **grid; // set as 0=empty, n=player n token here
    int *numincol; // how many tokens are in the columns
    int *numinrow; // how many tokens are in the rows
    int players;   // hwo many players there are
    int *player;   // the players
};

typedef struct gridTag GRID;

GRID *NewGrid(int width,int height,int players);
int SetPlayerColor(GRID *grid,int playernum,int color);
void DisplayGrid(GRID *grid);
int InsertTokenAtCR(GRID *grid,int col,int row,int player);
int RemoveTokenAtCR(GRID *grid,int row,int col);
int MoveTokenAtCR(GRID *grid,int col,int row,int direction);
int StatusOfCR(GRID *grid,int row,int col);
int TokensInGrid(GRID *grid);
int NumInDirection(GRID *grid,int col,int row,int col_mod,int row_mod);
void MoveAll(GRID *grid,int col_dir,int row_dir,int wrap);
#endif
