/*****************************************************************************
DOS mouse routines written in C by Matthew Luckman 24/4/1996.
This code was developed to aid in the use of the mouse and may be
freely distributed in its orignal, unaltered form.

Requires: DOS with a mouse driver installed. (ie, mouse.com)

Compiles without error under Borland C, Turbo C.
> Updated 17/10/1996 - Created seperate header file
> Updated 8/8/1996 - Simplified data structures
> Finished first version 24/4/1996.

If you have any problems/queries/comments, please send them to:-
        
        mluckman@qldnet.com.au
******************************************************************************/

#include <dos.h>
#include <i86.h>
#ifndef MOUSE_C
    #define MOUSE_C
#endif
#include "mouse.h"

#define TRUE 1
#define FALSE 0

static unsigned short arrow[] = {                                             /* screen mask for an arrow */
    0x1FFF, 0x0FFF, 0x07FF, 0x03FF,
    0x01FF, 0x00FF, 0x007F, 0x003F,
    0x001F, 0x003F, 0x01FF, 0x01FF,
    0xE0FF, 0xF0FF, 0xF8FF, 0xF8FF,
                                                                      /* cursor mask */
    0x0000, 0x4000, 0x6000, 0x7000,
    0x7800, 0x7C00, 0x7E00, 0x7F00,
    0x7F80, 0x7C00, 0x4C00, 0x0600,
    0x0600, 0x0300, 0x0300, 0x0000
};

static unsigned short glove[] = {                                             /* screen mask for a glove */
    0xF3FF, 0xE1FF, 0xE1FF, 0xE1FF,
    0xE1FF, 0xE049, 0xE000, 0x8000,
    0x0000, 0x0000, 0x07FC, 0x07F8,
    0x9FF9, 0x8FF1, 0xC003, 0xE007,
                                                                      /* cursor mask */
    0x0C00, 0x1200, 0x1200, 0x1200,
    0x1200, 0x13B6, 0x1249, 0x7249,
    0x9249, 0x9001, 0x9001, 0x8001,
    0x4002, 0x4002, 0x2004, 0x1FF8
};

static unsigned short check[] = {                                              /* screen mask for a check */
    0xFFF0, 0xFFE0, 0xFFC0, 0xFF81,
    0xFF03, 0x0607, 0x000F, 0x001F,
    0x803F, 0xC07F, 0xE0FF, 0xF1FF,
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
                                                                      /* cursor mask */
    0x0000, 0x0006, 0x000C, 0x0018,
    0x0030, 0x0060, 0x70C0, 0x3980,
    0x1F00, 0x0E00, 0x0400, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000
};

static unsigned short cross[] = {                                             /* screen mask for a cross */
    0xF01F, 0xE00F, 0xC007, 0x8003,
    0x0441, 0x0C61, 0x70C0, 0x0381,
    0x0381, 0x0C61, 0x0441, 0x8003,
    0xC007, 0xE00F, 0xF01F, 0xFFFF,
                                                                      /* cursor mask */
    0x0000, 0x07C0, 0x0920, 0x1110,
    0x2108, 0x4004, 0x4004, 0x783C,
    0x4004, 0x4004, 0x2108, 0x1110,
    0x0920, 0x07C0, 0x0000, 0x0000
};

static unsigned short ibeam[] = {                                              /* screen mask for an I-beam */
    0xF39F, 0xFD7F, 0xFEFF, 0xFEFF,
    0xFEFF, 0xFEFF, 0xFEFF, 0xFEFF,
    0xFEFF, 0xFEFF, 0xFEFF, 0xFEFF,
    0xFEFF, 0xFEFF, 0xFD7F, 0xF39F,
                                                                      /* cursor mask */
    0x0C60, 0x0290, 0x0100, 0x0100,
    0x0100, 0x0100, 0x0100, 0x0100,
    0x0100, 0x0100, 0x0100, 0x0100,
    0x0100, 0x0100, 0x0280, 0x0C60
};

int ResetMouse(void) {
/****************************************************************
Description:-    Resets the mouse driver.
Pre Condition:-  Nil
Post Condition:- returns TRUE if mouse is installed
****************************************************************/
    union  REGS  inreg,  outreg;                                      /* static registers */

    inreg.x.eax = 0;                                                   /* mouse reset function 0 */
    int386(0x33, &inreg, &outreg);
    return outreg.x.eax;
}

int NumButtons(void) {
/******************************************************************
Description:-    Resets the mouse & determinse how many buttons it
                 has.
Pre Condition:-  Nil
Post Condition:- The number of buttons is returned
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax = 0;
    int386(0x33, &inreg, &outreg);
    if (outreg.x.eax) return outreg.x.ebx;
    return FALSE;
}

void ShowMouse(void) {
/********************************************************************
Description:-    Displays the mouse cursor on the screen using the
                 current cursor shape.
Pre Condition:-  Nil.
Post Condition:- Mouse cursor is displayed.
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax=1;                                                     /* mouse show, function 1 */
    int386(0x33, &inreg, &outreg);
}

void HideMouse(void) {
/********************************************************************
Description:-    Hides mouse cursor on the screen
Pre Condition:-  Nil.
Post Condition:- Mouse is hidden
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax=2;                                                     /* mouse hide, function 2 */
    int386(0x33, &inreg, &outreg);
}

void MouseStat(MouseStatus *s) {
/***************************************************************
Description:-    Checks what buttons are being pressed and the
                 current position of the mouse.
Pre Condition:-  MouseStatus structure is passed in.
Post Condition:- MouseStatus structure is updated.
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax=3;                                                     /*mouse status, function 3 */
    int386(0x33, &inreg, &outreg);
    s->left=outreg.x.ebx & 1;
    s->right=((outreg.x.ebx & (1 << 1)) >>1);
    s->center=((outreg.x.ebx & (1 << 2)) >>2);
    s->xpos=outreg.x.ecx;
    s->ypos=outreg.x.edx;
    s->ButtonCount=s->left + s->center + s->right;                    /* number of buttons being pressed */
}

void SetMousePos(int xpos, int ypos) {
/***************************************************************
Description:-    Sets the mouse cursor at the x and y
                 co-ordinates
Pre Condition:-  X & Y co-ordinates are passed in.
Post Condition:- Mouse cursor is re-positioned
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax=4;                                                     /* set mouse, function 4 */
    inreg.x.ecx=xpos;
    inreg.x.edx=ypos;
    int386(0x33, &inreg, &outreg);
}

void PressInfo(MouseStatus *p) {
/***************************************************************
Description:-    Gets the position of the last button press for
                 a particular button
Pre Condition:-  MouseStatus structure is passed in, button
                 left, right, or center is set to indicate
                 button to query; default is left.
Post Condition:- MousePressInfo structure is updated.
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax=5;                                                     /*press information, function 5 */
    if (p->left) inreg.x.ebx=LEFT;
    else if (p->right) inreg.x.ebx=RIGHT;
    else if (p->center) inreg.x.ebx=CENTER;
    else inreg.x.ebx=LEFT;
    int386(0x33, &inreg, &outreg);
    p->left=outreg.x.eax & 1;
    p->right=((outreg.x.eax & (1 << 1)) >>1);
    p->center=((outreg.x.eax & (1 << 2)) >>2);
    p->ButtonCount=outreg.x.ebx;                                      /* button press counter */
    p->xpos=outreg.x.ecx;
    p->ypos=outreg.x.edx;
}

void ReleaseInfo(MouseStatus *p) {
/***************************************************************
Description:-    Gets the position of the last button release
                 for a particular button.
Pre Condition:-  MousePressInfo structure is passed in, button
                 left, right, or center is set to indicate
                 button to query; default is left.
Post Condition:- MousePressInfo structure is updated.
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax=6;                                                     /* release information, function 6*/
    if (p->left) inreg.x.ebx=LEFT;
    else if (p->right) inreg.x.ebx=RIGHT;
    else if (p->center) inreg.x.ebx=CENTER;
    else inreg.x.ebx=LEFT;
    int386(0x33, &inreg, &outreg);
    p->left=outreg.x.eax & 1;
    p->right=((outreg.x.eax & (1 << 1)) >>1);
    p->center=((outreg.x.eax & (1 << 2)) >>2);
    p->ButtonCount=outreg.x.ebx;
    p->xpos=outreg.x.ecx;
    p->ypos=outreg.x.edx;
}

int ButtonDown(MouseStatus *s) {
/***************************************************************
Description:-    Checks to see if there is a button being pressed.
Pre Condition:-  You must pass in a MouseStatus that is current.
Post Condition:- returns TRUE or FALSE if a button is being pressed.
****************************************************************/
return (s->left + s->center + s->right);
}

void SetXLimits(int min, int max) {
/***************************************************************
Description:-    Sets the maximum and minimum limits of the
                 mouse cursor along the X.
Pre Condition:-  Max and min values are passed in.
Post Condition:- Mouse limitations are set.
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax=7;
    inreg.x.ecx=min;
    inreg.x.edx=max;
    int386(0x33, &inreg, &outreg);
}

void SetYLimits(int min, int max) {
/***************************************************************
Description:-    Sets the maximum and minimum limits of the
                 mouse cursor along the Y.
Pre Condition:-  Max and min values are passed in.
Post Condition:- Mouse limitations are set.
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax=8;
    inreg.x.ecx=min;
    inreg.x.edx=max;
    int386(0x33, &inreg, &outreg);
}

void SetGfxCurShape(int xpos, int ypos, int cursor[32]) {
/***************************************************************
Description:-    Sets the graphic mouse cursor to a new shape.
Pre Condition:-  The hot spot offsets are passed in. An array
                 of 32 integers describing the cursor shape is
                 passed in.
Post Condition:- Mouse cursor shape is changed.
****************************************************************/
    union REGS inreg, outreg;
    struct SREGS   seg;

    inreg.x.eax = 9;
    inreg.x.ebx = xpos;                                                /* X position hotspot */
    inreg.x.ecx = ypos;                                                /* Y position hotspot */
    inreg.x.edx = (int)cursor;
    segread(&seg);
    seg.es = seg.ds;
    int386x(0x33,&inreg,&outreg,&seg);
}

void SetTxtCurShape( int cursor_type,int val1,int val2) {
/***************************************************************
Description:-    Defines the shape and attributes of the text
                 mode mouse cursor.
Pre Condition:-  cursor_type (hardware or software) is passed in.
                 If cursor_type is 0 (software), val1 is the
                 screen mask, & val2 is cursor mask.
                 If cursor_type is 1 (hardward), val1 is the
                 starting scan line for the cursor, & val2 is the
                 ending scan line for the cursor.
Post Condition:- Mouse cursor shape is changed.
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax = 10;                                                  // mouse function 10 //
    inreg.x.ebx = cursor_type;
    inreg.x.ecx = val1;
    inreg.x.edx = val2;
    int386( 0x33, &inreg, &outreg);
}

int NetXMotion(void) {
/***************************************************************
Description:-    Gets the distance traveled in the x direction
                 since it was last checked.
Pre Condition:-  Nil.
Post Condition:- Net X motion is returned.
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax=11;
    int386(0x33, &inreg, &outreg);
    return outreg.x.ecx;
}

int NetYMotion(void) {
/***************************************************************
Description:-    Gets the distance traveled in the y direction
                 since it was last checked.
Pre Condition:-  Nil.
Post Condition:- Net Y motion is returned.
*************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax=11;
    int386(0x33, &inreg, &outreg);
    return outreg.x.edx;
}


int MouseMoved(void) {
/***************************************************************
Description:-    Checks if the mouse has moved since last check.
Pre Condition:-  Nil.
Post Condition:- returns TRUE or FALSE is mouse has moved.
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax=11;
    int386(0x33, &inreg, &outreg);
    if ((outreg.x.ecx) || (outreg.x.edx)) return TRUE;
    return FALSE;
}

void EnablePen(void) {
/***************************************************************
Description:-    Enables light-pen emulation
Pre Condition:-  Nil.
Post Condition:- Mouse now emulates a light-pen
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax=13;
    int386(0x33, &inreg, &outreg);
}

void DisablePen(void) {
/***************************************************************
Description:-    Disables light-pen emulation
Pre Condition:-  Nil.
Post Condition:- Emulation is turned off.
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax=14;
    int386(0x33, &inreg, &outreg);
}

void MotionRatio(int x, int y) {
/***************************************************************
Description:-    Specifies the number of mickeys of mouse movement
                 required for eight pixels of mouse cursor movement.
Pre Condition:-  Ratio factors are passed in.
Post Condition:- Mouse motion ration is changed.
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax=15;
    inreg.x.ecx=x;
    inreg.x.edx=y;
    int386(0x33, &inreg, &outreg);
}

void CursorExclude(int x1, int y1, int x2, int y2) {
/***************************************************************
Description:-    Excludes the mouse cursor from this rectangle.
Pre Condition:-  Top left & bottom right corners of a rectangle
                 are passed in.
Post Condition:- Mouse cursor is excluded from this rectangle.
****************************************************************/
    union REGS inreg, outreg;
    inreg.x.eax=16;
    inreg.x.ecx=x1;
    inreg.x.edx=y1;
    inreg.x.esi=x2;
    inreg.x.edi=y2;
    int386(0x33, &inreg, &outreg);
}

void DoubleThresh(int mickeys) {
/***************************************************************
Description:-    Specifies the mouse movement speed at which
                 cursor movement is doubled
Pre Condition:-  Pass in threshold speed in mickeys per second.
Post Condition:- Mouse is set.
****************************************************************/
    union REGS inreg, outreg;
    inreg.x.eax=19;
    inreg.x.edx=mickeys;
    int386(0x33, &inreg, &outreg);
}

void Sensitivity(int x, int y, int dspeed) {
/***************************************************************
Description:-    Specifies the mickeys-to-cursor movement
                 sensitivity & also the threshold for double
                 speed movement
Pre Condition:-  Horizontal & vertical mouse sensitivity, and
                 double speed threshold are passed in.
Post Condition:- Mouse is set.
****************************************************************/
    union REGS inreg, outreg;
    inreg.x.eax=26;
    inreg.x.ebx=x;
    inreg.x.ecx=y;
    inreg.x.edx=dspeed;
    int386(0x33, &inreg, &outreg);
}

void GetSense(MouseSense *s) {
/***************************************************************
Description:-    Gets the current mouse sensitivity settings.
Pre Condition:-  MouseSense structure is passed in.
Post Condition:- MouseSense structure is updated.
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax=27;
    int386(0x33, &inreg, &outreg);
    s->XSense=outreg.x.ebx;
    s->YSense=outreg.x.ecx;
    s->DThreshold=outreg.x.edx;
}

void IntRate(int rate) {
/***************************************************************
Description:-    Sets the rate at which the mouse driver queries
                 the mouse hardware. (in interrupts per second)
Pre Condition:-  The interrupt amount is passed in.
Post Condition:- Mouse is set.
****************************************************************/
    union REGS inreg, outreg;
    inreg.x.eax=28;
    inreg.x.ebx=rate;
    int386(0x33, &inreg, &outreg);
}

void SetMousePage(int page) {
/***************************************************************
Description:-    Specifies the video page for the mouse cursor.
Pre Condition:-  The video page number is passed in.
Post Condition:- Mouse is displayed on the new video page.
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax = 29;
    inreg.x.ebx = page;
    int386(0x33, &inreg, &outreg);
}

int GetMousePage(void) {
/***************************************************************
Description:-    Gets the current video page in which the mouse
                 is shown.
Pre Condition:-  Nil.
Post Condition:- Video page number is returned
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax = 30;
    int386(0x33, &inreg, &outreg);
    return outreg.x.ebx;
}

int MouseType(void) {
/***************************************************************
Description:-    Gets the type of mouse installed.
Pre Condition:-  Nil.
Post Condition:- Mouse type is returned.
****************************************************************/
    union REGS inreg, outreg;
    inreg.x.eax = 42;
    int386(0x33, &inreg, &outreg);
    return outreg.x.edx;
}

int XHotSpot(void) {
/***************************************************************
Description:-    Gets the X position of the hot spot.
Pre Condition:-  Nil.
Post Condition:- X position is returned
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax = 42;
    int386(0x33, &inreg, &outreg);
    return outreg.x.ebx;
}

int YHotSpot(void) {
/***************************************************************
Description:-    Gets the Y position of the hot spot.
Pre Condition:-  Nil.
Post Condition:- Y position is returned
****************************************************************/
    union REGS inreg, outreg;

    inreg.x.eax = 42;
    int386(0x33, &inreg, &outreg);
    return outreg.x.ecx;
}

int UseMouseCur(int CurType) {
/***************************************************************
Description:-    Sets the graphic cursor to a pre-determined
                 shape.
Pre Condition:-  The type of shape of the cursor is passed in.
Post Condition:- Mouse cursor shape is changed.
****************************************************************/
    HideMouse();
    switch (CurType) {
        case ARROW : SetGfxCurShape(0x0001, 0x0001,arrow); break;
        case IBEAM : SetGfxCurShape(0x0007, 0x0008,ibeam); break;
        case CROSS : SetGfxCurShape(0x0007, 0x0007,cross); break;
        case GLOVE : SetGfxCurShape(0x0004, 0x0000,glove); break;
        case CHECK : SetGfxCurShape(0x0005, 0x000A,check); break;
        default : ShowMouse(); return FALSE;
    }
    ShowMouse();
    return TRUE;
}
