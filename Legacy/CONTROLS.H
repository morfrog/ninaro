#ifndef CONTROLS_H
#define CONTROLS_H
#include "sys.h"
#include "screen.h"

#define CONTROLBACK GREY
#define CONTROLTEXT BLACK
#define CONTROLHILITE RED

enum {Radio,Color,Button,Value,Name};
// Name does not recieve input

struct radioTag
{
    int value;      // the value            e.g. Computer // enum = 1
    char *name;    // the associated name  e.g. "Computer"
};

typedef struct radioTag RADIO;

struct colorTag
{
    int value;  // the value of the color
};

typedef struct colorTag COLOR;

struct buttonTag
{
    char *name;             // the name of the button
    void (* func)(void);    // the function to run when the button is pressed
};

typedef struct buttonTag BUTTON;

struct valueTag
{
    int value;      // the value
};

typedef struct valueTag VALUE;

struct nameTag
{
    char *name;
};

typedef struct nameTag NAME;

struct controllistTag
{
    int type;       // enum {Radio,Color,Button,Value,Name};
    int current;    // which one is selected
    int size;       // how many controls there are
    void *store;    // the storage of the controls
};

typedef struct controllistTag CONTROLLIST;

struct dialogelemTag
{
    int enabled;    // only enabled elems displayed
    char *name;
    CONTROLLIST *cl;
    int temp;   // saves current for reset if required
    int displaycols; // columns for display
    int displaychars; // max chars to display
    double displayfract; // fraction of 1.0 of viewport in which it is displayed
};

typedef struct dialogelemTag DIALOGELEM;

struct dialogTag
{
    int current;        // which control is currently selected
    int controls;       // how many controls there are
    DIALOGELEM *control; // array of the controls, includes 
                         // automatic buttons - OK,Reset,Cancel
};

typedef struct dialogTag DIALOG;

CONTROLLIST *ControlsControlListNew(int type,int copy,int num,...);
int ControlsControlListNext(CONTROLLIST *cl);
int ControlsControlListPrev(CONTROLLIST *cl);
void ControlsControlListDisplay(CONTROLLIST *cl,int cols,int chars);
int ControlsControlListSet(CONTROLLIST *cl,...);

DIALOG *ControlsDialogNew(double fract,int controls,...);
int ControlsDialogNext(DIALOG *dl);
int ControlsDialogPrev(DIALOG *dl);
void ControlsDialogDisplay(VIEWPORT *vp,DIALOG *dl,double gap);
int ControlsDialogCall(VIEWPORT *vp,DIALOG *dl,double gap);
int ControlsDialogSet(DIALOG *dl,int control,...);
#endif
