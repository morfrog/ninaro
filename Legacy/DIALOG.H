#ifndef DIALOG_H
#define DIALOG_H
#include "screen.h"

enum {Button=0,Radio,Color,Name,Slider,Dial}; // Display Type

enum {List=0,Range}; // Control Type

#define CONTROLTEXT BLACK
#define CONTROLHILITE RED
#define CONTROLBACK GREY

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
    char name[20];
    int current;
    int elems;
    DIALOGELEM *elem;
//    MEMPAGE *background;
};
typedef struct DialogTag DIALOG;

#endif
