#ifndef MOUSEIN_H
#define MOUSEIN_H

enum {NMB=0,LMB,RMB}; // Left, Right, None

int GetMouseInput(int *x,int *y);
// purpose : to get mouse input, *button==LMB or RMB
// returns : RMB if right button input found, LMB if left, NMB if none

#endif
