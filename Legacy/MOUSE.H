/*****************************************************************************
DOS mouse routines written in C by Matthew Luckman 24/4/1996.
This code was developed to aid in the use of the mouse and may be
freely distributed in its orignal, unaltered form.

Requires: DOS with a mouse driver installed. (ie, mouse.com)

Compiles without error under Borland C, Turbo C.
> Updated 17/10/1996 - Created seperate header file, updated documentation.
> Updated 8/8/1996 - Simplified data structures
> Finished first version 24/4/1996.

If you have any problems/queries/comments, please send them to:-
        
        mluckman@qldnet.com.au
******************************************************************************/
#ifndef MOUSE_H
#define MOUSE_H

#define LEFT 0                                                        /* mouse buttons */
#define CENTER 2
#define RIGHT 1
#define ARROW 0                                                       /* cursor shape */
#define GLOVE 1
#define IBEAM 2
#define CROSS 3
#define CHECK 4
#define _NoInts 0                                                     /* number of interrupts per second */
#define _30Ints 1
#define _50Ints (1 << 1)
#define _100Ints (1 << 2)
#define _200Ints (1 << 3)
#define NoMouse 0                                                     /*type of mouse installed */
#define BusMouse 1
#define SerialMouse 2
#define InPortMouse 3
#define IBMMouse 4
#define HPMouse 5

typedef struct {                                                      /* Mouse sensitivity structure */
    int XSense,                                                       /* The sensitivity in the X direction (left,right) */
        YSense,                                                       /* The sensitivity in the Y direction (up,down) */
        DThreshold;                                                   /* The double speed threshold. */
} MouseSense;

typedef struct {                                                      /* Mouse status information structure */
    unsigned    left   :1,                                               /* Set to 1 if button is on, 0 if it is off. */
                 center :1,                                              /* "                                       " */
                 right  :1;                                              /* "                                       " */
    int         ButtonCount,                                            /* The number of buttons being pressed*/
                 xpos,                                                    /* The current x co-ordinate of the mouse. (in pixels) */
                 ypos;                                                    /* The current y co-ordinate of the mouse. (in pixels) */
} MouseStatus;


typedef struct {                                                      /* Graphical mouse cursor shape structure */
    int cursor[32];
    unsigned int xpos,                                                /* The x co-ordinate of the "hot-spot" */
                 ypos;                                                /* The y co-ordinate of the "hot-spot" */
} g_cursor;


int ResetMouse(void);
/****************************************************************
Description:-    Resets the mouse driver.
Pre Condition:-  Nil
Post Condition:- returns TRUE if mouse is installed
****************************************************************/

int NumButtons(void);
/******************************************************************
Description:-    Resets the mouse & determines how many buttons it
                 has.
Pre Condition:-  Nil
Post Condition:- The number of buttons is returned
****************************************************************/

void ShowMouse(void);
/********************************************************************
Description:-    Displays the mouse cursor on the screen using the
                 current cursor shape.
Pre Condition:-  Nil.
Post Condition:- Mouse cursor is displayed.
****************************************************************/

void HideMouse(void);
/********************************************************************
Description:-    Hides mouse cursor on the screen
Pre Condition:-  Nil.
Post Condition:- Mouse is hidden
****************************************************************/

void MouseStat(MouseStatus *s);
/***************************************************************
Description:-    Checks what buttons are being pressed and the
                 current position of the mouse.
Pre Condition:-  MouseStatus structure is passed in.
Post Condition:- MouseStatus structure is updated.
****************************************************************/

void SetMousePos(int xpos, int ypos);
/***************************************************************
Description:-    Sets the mouse cursor at the x and y
                 co-ordinates
Pre Condition:-  X & Y co-ordinates are passed in.
Post Condition:- Mouse cursor is re-positioned
****************************************************************/

void PressInfo(MouseStatus *p);
/***************************************************************
Description:-    Gets the position of the last button press for
                 a particular button
Pre Condition:-  MouseStatus structure is passed in, button
                 left, right, or center is set to indicate
                 button to query; default is left.
Post Condition:- MousePressInfo structure is updated.
****************************************************************/

void ReleaseInfo(MouseStatus *p);
/***************************************************************
Description:-    Gets the position of the last button release
                 for a particular button.
Pre Condition:-  MousePressInfo structure is passed in, button
                 left, right, or center is set to indicate
                 button to query; default is left.
Post Condition:- MousePressInfo structure is updated.
****************************************************************/

int ButtonDown(MouseStatus *s);
/***************************************************************
Description:-    Checks to see if there is a button being pressed.
Pre Condition:-  You must pass in a MouseStatus that is current.
Post Condition:- returns TRUE or FALSE if a button is being pressed.
****************************************************************/


void SetXLimits(int min, int max);
/***************************************************************
Description:-    Sets the maximum and minimum limits of the
                 mouse cursor along the X.
Pre Condition:-  Max and min values are passed in.
Post Condition:- Mouse limitations are set.
****************************************************************/

void SetYLimits(int min, int max);
/***************************************************************
Description:-    Sets the maximum and minimum limits of the
                 mouse cursor along the Y.
Pre Condition:-  Max and min values are passed in.
Post Condition:- Mouse limitations are set.
****************************************************************/

void SetGfxCurShape(int xpos, int ypos, int cursor[32]);
/***************************************************************
Description:-    Sets the graphic mouse cursor to a new shape.
Pre Condition:-  The hot spot offsets are passed in. An array
                 of 32 integers describing the cursor shape is
                 passed in.
Post Condition:- Mouse cursor shape is changed.
****************************************************************/

void SetTxtCurShape( int cursor_type,int val1,int val2);
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

int NetXMotion(void);
/***************************************************************
Description:-    Gets the distance traveled in the x direction
                 since it was last checked.
Pre Condition:-  Nil.
Post Condition:- Net X motion is returned.
****************************************************************/

int NetYMotion(void);
/***************************************************************
Description:-    Gets the distance traveled in the y direction
                 since it was last checked.
Pre Condition:-  Nil.
Post Condition:- Net Y motion is returned.
*************************************************************/

int MouseMoved(void);
/***************************************************************
Description:-    Checks if the mouse has moved since last check.
Pre Condition:-  Nil.
Post Condition:- returns TRUE or FALSE is mouse has moved.
****************************************************************/

void EnablePen(void);
/***************************************************************
Description:-    Enables light-pen emulation
Pre Condition:-  Nil.
Post Condition:- Mouse now emulates a light-pen
****************************************************************/

void DisablePen(void);
/***************************************************************
Description:-    Disables light-pen emulation
Pre Condition:-  Nil.
Post Condition:- Emulation is turned off.
****************************************************************/

void MotionRatio(int x, int y);
/***************************************************************
Description:-    Specifies the number of mickeys of mouse movement
                 required for eight pixels of mouse cursor movement.
Pre Condition:-  Ratio factors are passed in.
Post Condition:- Mouse motion ration is changed.
****************************************************************/

void CursorExclude(int x1, int y1, int x2, int y2);
/***************************************************************
Description:-    Excludes the mouse cursor from this rectangle.
Pre Condition:-  Top left & bottom right corners of a rectangle
                 are passed in.
Post Condition:- Mouse cursor is excluded from this rectangle.
****************************************************************/

void DoubleThresh(int mickeys);
/***************************************************************
Description:-    Specifies the mouse movement speed at which
                 cursor movement is doubled
Pre Condition:-  Pass in threshold speed in mickeys per second.
Post Condition:- Mouse is set.
****************************************************************/

void Sensitivity(int x, int y, int dspeed);
/***************************************************************
Description:-    Specifies the mickeys-to-cursor movement
                 sensitivity & also the threshold for double
                 speed movement
Pre Condition:-  Horizontal & vertical mouse sensitivity, and
                 double speed threshold are passed in.
Post Condition:- Mouse is set.
****************************************************************/

void GetSense(MouseSense *s);
/***************************************************************
Description:-    Gets the current mouse sensitivity settings.
Pre Condition:-  MouseSense structure is passed in.
Post Condition:- MouseSense structure is updated.
****************************************************************/

void IntRate(int rate);
/***************************************************************
Description:-    Sets the rate at which the mouse driver queries
                 the mouse hardware. (in interrupts per second)
Pre Condition:-  The interrupt amount is passed in.
Post Condition:- Mouse is set.
****************************************************************/

void SetMousePage(int page);
/***************************************************************
Description:-    Specifies the video page for the mouse cursor.
Pre Condition:-  The video page number is passed in.
Post Condition:- Mouse is displayed on the new video page.
****************************************************************/

int GetMousePage(void);
/***************************************************************
Description:-    Gets the current video page in which the mouse
                 is shown.
Pre Condition:-  Nil.
Post Condition:- Video page number is returned
****************************************************************/

int MouseType(void);
/***************************************************************
Description:-    Gets the type of mouse installed.
Pre Condition:-  Nil.
Post Condition:- Mouse type is returned.
****************************************************************/

int XHotSpot(void);
/***************************************************************
Description:-    Gets the X position of the hot spot.
Pre Condition:-  Nil.
Post Condition:- X position is returned
****************************************************************/

int YHotSpot(void);
/***************************************************************
Description:-    Gets the Y position of the hot spot.
Pre Condition:-  Nil.
Post Condition:- Y position is returned
****************************************************************/

int UseMouseCur(int CurType);
/***************************************************************
Description:-    Sets the graphic cursor to a pre-determined
                 shape. The shapes are defined at the top.
                 eg. UseMouseCur(ARROW);
Pre Condition:-  The type of shape of the cursor is passed in.
Post Condition:- Mouse cursor shape is changed.
****************************************************************/


#endif
