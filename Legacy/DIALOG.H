#ifndef DIALOG_H
#define DIALOG_H
#include "screen.h"
#include "mempage.h"

enum {Button=0,Radio,Color,Name,Slider,Dial}; // Display Type

enum {List=0,Range}; // Control Type

#define CONTROLTEXT BLACK
#define CONTROLHILITE RED
#define CONTROLBACK LTGREY

struct CListElemTag
{
    int value;
    char *name;
};
typedef struct CListElemTag CLISTELEM;

struct CListTag
{
    int elems;
    CLISTELEM *elem;
    int current;
};
typedef struct CListTag CLIST;

struct CRangeTag
{
    int min; char minname[10];
    int max; char maxname[10];
    int current; // min..max;
};
typedef struct CRangeTag CRANGE;

struct ControlTag
{
    int controltype; // enum {List,Range} // Control Type
    int displaytype; // enum {Button,Radio,Color,Name,Slider,Dial}; // Display Type
    int rows; int cols; // rows and cols for display
    int strwidth;   // max width of a string to be displayed
    void *store;  // the control
};
typedef struct ControlTag CONTROL;

struct DialogElemTag
{
    char name[20];     // the name of the control
    int enabled; // TRUE,FALSE(not displayed)
    int selectable; // TRUE,FALSE(next & prev skip it,current=-1)
    CONTROL *store;
};
typedef struct DialogElemTag DIALOGELEM;

struct DialogTag
{
    char name[20];      // the name of the dialog
    int current;        // the current control of the dialog
    int elems;          // the number of controls in the dialog
    int *state;         // the state of the dialog, used to save state
    void (* change)(void *,int); // func to run when controls change value
    DIALOGELEM *elem;   // array of controls in the dialog
//  MEMPAGE *background;// the background image
    KEYSYS *ks;         // the key system of the dialog
};
typedef struct DialogTag DIALOG;

/*---------------------------CONTROL-----------------------------------*/
void ControlResetDisplay(CONTROL *c,int display,int rows,int cols,int strwidth);
// purpose : resets the display attributes of a control
// returns : void
int ControlReset(CONTROL *c,...);
// purpose : resets the values of a control, only sets the list or range
//           values, not the type,display,rows,cols,strwidth, use
//           ControlDisplayReset for these. Can't reset the size of a list.
// returns : TRUE if successful, else FALSE
CONTROL *ControlNew(int type,int display,int rows,int cols,int strwidth,...);
// list ... is elems,{int value,char *name}*elems           name=name;
// range... is int min,char *minname,int max,int maxname    name=NewString(name);
void ControlPrint(CONTROL *c);
// purpose : to print the contents of the control
// returns : void
int ControlValueGet(CONTROL *c,int pos,int *value);
// purpose : to get the value of the control at position pos.
//           Range pos would be 0 or 1, list 0..size
// returns : if pos invalid FALSE is returned, else TRUE is returned and
//           value set
int ControlNameGet(CONTROL *c,int pos,char *name);
// purpose : to get the name of the control at position pos.
//           Range pos would be 0 or 1, list 0..size
// returns : if pos invalid FALSE is returned, else TRUE is returned and
//           name set
int ControlMove(CONTROL *c,int howfar);
// purpose : moves howfar in the control
// returns : TRUE if moves else false;
int ControlPrev(CONTROL *c);
// purpose : moves current to the prev in the control
// returns : TRUE if moves else false;
int ControlNext(CONTROL *c);
// purpose : moves current to the next in the control
// returns : TRUE if moves else false;
int ControlCurrentSet(CONTROL *c,int current);
// purpose : sets the controls current to current
// returns : TRUE if successful else FALSE
int ControlCurrentSetTo(CONTROL *c,int value);
// purpose : sets the controls current to the first in the control which has value
// returns : TRUE if successful else FALSE
int ControlCurrentGet(CONTROL *c);
// purpose : gets the controls current
// returns : the controls current
ControlElemSet(CONTROL *c,int elem,int value,char *name);
// if range then if value==min strcpy(minname,name)
//               if value==max strcpy(maxname,name)
// if list then list[elem]=value,name.
void ControlDisplay(CONTROL *c,VIEWPORT *vp);
// purpose : to graphically display the control in the viewport
// returns : void

/*---------------------------DIALOG-----------------------------------*/
DIALOG *DialogNew(KEYGROUP *kg,void (* change)(DIALOG *,int),char *name,int controls,...);
// purpose : creates a new dialog, ...= {name,enabled,selectable,CONTROL *}
//           dialogs have a default control of buttons, 'OK,Reset,Cancel'
// returns : the dialog
int DialogMove(DIALOG *d,int howfar);
// purpose : to move the dialog's current howfar
// returns : FALSE it cant move, else TRUE
int DialogPrev(DIALOG *d);
// purpose : moves current to the prev in the dialog
// returns : TRUE if successful, else FALSE
int DialogNext(DIALOG *d);
// purpose : moves current to the next in the dialog
// returns : TRUE if successful, else FALSE
int DialogControlNext(DIALOG *d);
// purpose : moves current of the current control in the dialog next
// returns : TRUE if successful, else FALSE
int DialogControlPrev(DIALOG *d);
// purpose : moves current of the current control in the dialog next
// returns : TRUE if successful, else FALSE
int DialogCurrentSet(DIALOG *d,int current);
// purpose : sets the dialog current to current
// returns : TRUE if successful, else FALSE
int DialogCurrentGet(DIALOG *d);
// purpose : to get the current
// returns : the current
int DialogDefaultGet(DIALOG *d);
// purpose : gets which of the default (OK,Reset,Cancel) buttons is selected
// returns : the number of the button
void DialogDefaultSet(DIALOG *d,int select);
// purpose : sets which of the default (OK,Reset,Cancel) buttons is selected
// returns : void
CONTROL *DialogControlGet(DIALOG *d,int control);
// purpose : gets the control from the dialog, for use with control functions
//           assumes 0<=control<=d->elems-1
// returns : the control 
void DialogNameSet(DIALOG *d,char *string);
// purpose : sets the name of the dialog
// returns : void
void DialogKeyGroupSet(DIALOG *d,KEYGROUP *kg);
// purpose : sets the dialogs keygroup
// returns : void
KEYGROUP *DialogKeyGroupGet(DIALOG *d);
// purpose : gets the dialogs keygroup
// returns : the keygroup
int DialogStateSame(DIALOG *d,int *state);
// purpose : to see if a state is the same as a dialog's state
// returns : TRUE if same, else FALSE is returned
int DialogStateGet(DIALOG *d,int *buffer);
// purpose : gets the current of each control in the dialog, into the int array buffer
// returns : TRUE if all gotten successfully, else FALSE
int DialogStateSet(DIALOG *d,int *buffer);
// purpose : sets the current of each control in the dialog, ...{int}*d->elems
// returns : TRUE if all successfully set, else FALSE
void DialogDisplay(DIALOG *d,VIEWPORT *vp,MEMPAGE *mp,int namefract,int chars);
// purpose : displays the dialog in the viewport with the mempage background
//           1/namefract is the name of each control, where up to chars characters are displayed
// returns : void
void DialogPrint(DIALOG *dl);
// purpose : to print the contents of the dialog
// returns : void
int DialogCancel(DIALOG *d);
// purpose : sends a cancel signal to the dialog
// returns : FALSE, as dialog finished
int DialogSelect(DIALOG *d);
// purpose : sends a select signal to the dialog
// returns : TRUE if OK, FALSE if cancel, UNKNOWN if reset
int DialogCall(DIALOG *d,VIEWPORT *vp,MEMPAGE *mp,int namefract,int chars,void (* help)(void));
// purpose : runs a dialog, as is, in the viewport with the mempage as
//           background, running function change whenever a controls current
//           changes, using chars for control names, which are in left
//           1/namefract section of viewport. help function is run via F1 or H
//           kg keys are left,right,up,down,select,cancel
// returns : FALSE if cancelled, TRUE if OK'd. if exit state is same as entry
//           state, cancel is assumed and FALSE is returned

#endif
