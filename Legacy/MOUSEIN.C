#include "mouse.h"
#include "defines.h"
#include "mousein.h"

int GetMouseInput(int *x,int *y)
// purpose : to get mouse input, *button==LMB or RMB
// returns : RMB if right button input found, LMB if left, NMB if none
{
    static int lmbpress=0;
    static int rmbpress=0;
    MouseStatus gmims={0};
    
    MouseStat(&gmims);
    gmims.left=1;
    PressInfo(&gmims);
    if (gmims.ButtonCount) ++lmbpress;
    gmims.left=0;
    gmims.right=1;
    PressInfo(&gmims);
    if (gmims.ButtonCount) ++rmbpress;

    if (lmbpress)
    {   
        MouseStat(&gmims);
        *x=gmims.xpos-XADJUST;
        *y=gmims.ypos-YADJUST;
        --lmbpress;
        return LMB;
    }
    else if (rmbpress)
    {
        MouseStat(&gmims);
        *x=gmims.xpos-XADJUST;
        *y=gmims.ypos-YADJUST;
        --rmbpress;
        return RMB;
    }
    else
    {
        MouseStat(&gmims);
        *x=gmims.xpos-XADJUST;
        *y=gmims.ypos-YADJUST;
        return NMB;
    }
}

/*
int GetMouseInput(int *x,int *y)
// purpose : to get mouse input, *button==LMB or RMB
// returns : RMB if right button input found, LMB if left, NMB if none
{
    static int lmbpress=0;
    static int lmbavailable=0;
    static int rmbpress=0;
    static int rmbavailable=0;
    MouseStatus gmims={0};
    
    MouseStat(&gmims);
    gmims.left=1;
    PressInfo(&gmims);
    if (gmims.ButtonCount) ++lmbpress;
    gmims.left=1;
    ReleaseInfo(&gmims);
    if (gmims.ButtonCount) {--lmbpress; ++lmbavailable;}
    gmims.left=0;
    gmims.right=1;
    PressInfo(&gmims);
    if (gmims.ButtonCount) ++rmbpress;
    gmims.left=0;
    gmims.right=1;
    ReleaseInfo(&gmims);
    if (gmims.ButtonCount) {--rmbpress; ++rmbavailable;}

    if (lmbavailable)
    {   
        MouseStat(&gmims);
        *x=gmims.xpos-37683200;
        *y=gmims.ypos-8847360;
        --lmbavailable;
        return LMB;
    }
    else if (rmbavailable)
    {
        MouseStat(&gmims);
        *x=gmims.xpos-37683200;
        *y=gmims.ypos-8847360;
        --rmbavailable;
        return RMB;
    }
    else
    {
        MouseStat(&gmims);
        *x=gmims.xpos-37683200;
        *y=gmims.ypos-8847360;
        return NMB;
    }
}
*/
