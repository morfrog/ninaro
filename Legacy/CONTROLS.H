#ifndef CONTROLS_H
#define CONTROLS_H

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
    int rbs;       // how many buttons the are
    RADIO *rb;     // array of radio buttons
};

typedef struct radiolistTag RADIOLIST;

struct colorTag
{
    int color;
};

typedef struct colorTag COLOR;

struct colorlistTag
{
    int current;
    int colors;
    COLOR *color;
};

typedef struct colorlistTag COLORLIST;

struct buttonTag
{
    char *name;
    void (* func)(void);
};

typedef struct buttonTag BUTTON;

struct buttonlistTag
{
    int current;
    int buttons;
    BUTTON *button;
};

typedef struct buttonlist BUTTONLIST;

struct valueTag
{
    int value;
};

typedef struct valueTag VALUE;

struct valuelistTag
{
    int current;
    int values;
    VALUE *value;
    int names;
    char **name;
};

typedef struct valuelistTag VALUELIST;

struct dialogelemTag
{
    int type; // enum {Radio,Color,Button,Value};
    void *elem;
};

typedef struct dialogelemTag DIALOGELEM;

struct dialogTag
{
    int current;
    int controls;
    DIALOGELEM *control;
};

typedef struct dialogTag DIALOG;

#endif
