#ifndef MOUSEIN_H
#define MOUSEIN_H

enum {NMB=0,LMB,RMB};

// #define YADJUST 9371648
// #define XADJUST 37683200
int MouseX(void);
int MouseY(void);
void SetMouseX(int x);
void SetMouseY(int y);
int InitMouse(void);
int GetMouseInput(int *x,int *y);
// purpose : to get mouse input, *button==LMB or RMB
// returns : RMB if right button input found, LMB if left, NMB if none

int MouseCol(int col,int cols);
int MouseRow(int cols,int rows);

#endif
