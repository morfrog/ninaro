#ifndef MOUSEIN_H
#define MOUSEIN_H

enum {NMB=0,LMB,RMB}; // Left, Right, None

#define YADJUST 9371648
#define XADJUST 37683200

int GetMouseInput(int *x,int *y);
// purpose : to get mouse input, *button==LMB or RMB
// returns : RMB if right button input found, LMB if left, NMB if none

int MouseCol(int col,int cols);
int MouseRow(int cols,int rows);

#endif
