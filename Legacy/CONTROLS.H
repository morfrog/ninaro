#ifndef CONTROLS_H
#define CONTROLS_H

#define CONTROLBACK GREY
#define CONTROLTEXT BLACK
#define CONTROLHILITE RED

enum {Radio,Color,Button,Value};

struct radioTag
{
    int value;      // the value            e.g. Computer // enum = 1
    char *name;    // the associated name  e.g. "Computer"
};

typedef struct radioTag RADIO;

struct radiolistTag
{
    int current;   // the one which is selected
    int rbs;       // how many radio buttons the are
    RADIO *rb;     // array of radio buttons
};

typedef struct radiolistTag RADIOLIST;

struct colorTag
{
    int value;  // the value of the color
};

typedef struct colorTag COLOR;

struct colorlistTag
{
    int current;    // the one which is selected
    int colors;     // the size of the list
    COLOR *color;   // the color list
};

typedef struct colorlistTag COLORLIST;

struct buttonTag
{
    char *name;             // the name of the button
    void (* func)(void);    // the function to run when the button is pressed
};

typedef struct buttonTag BUTTON;

struct buttonlistTag
{
    int current;    // the one which is selected
    int buttons;    // how many buttons are in the list
    BUTTON *button; // the list of buttons
};

typedef struct buttonlistTag BUTTONLIST;

struct valueTag
{
    int value;      // the value
};

typedef struct valueTag VALUE;

struct valuelistTag
{
    int current;    // the one which is selected
    int values;     // how many values there are
    VALUE *value;   // the list of values
    int names;      // how many names there are
    char **name;    // array of the names
};

typedef struct valuelistTag VALUELIST;

struct dialogelemTag
{
    int type; // enum {Radio,Color,Button,Value};
    char *name;
    void *elem; // are actually lists of above enum type
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
                         // automatic buttons OK,Reset,Cancel
};

typedef struct dialogTag DIALOG;

#endif
