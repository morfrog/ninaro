#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "controls.h"

RADIO *ControlsRadioNew(int value,char *name)
{
    RADIO *ret=malloc(sizeof(RADIO));
    ret->value=value;
    ret->name=malloc(strlen(name)+1);
    strcpy(ret->name,name);
    return ret;
}

RADIOLIST *ControlsRadioListNew(int size, ...)
{
    va_list ap;
    int x;
    char *cp;
    RADIOLIST *rl=malloc(sizeof(RADIOLIST));
    rl->current=0;
    rl->rbs=size;
    rl->rb=malloc(size*sizeof(RADIO));
    va_start(ap,size);
    for (x=0;x<size;x++)
    {
        rl->rb[x].value=va_arg(ap,int);
        cp=va_arg(ap,char *);
        rl->rb[x].name=malloc((strlen(cp)+1)*sizeof(char));
        strcpy(rl->rb[x].name,cp);
    }
    va_end(ap);
    return rl;
}

COLOR *ControlsColorNew(int color)
{
    COLOR *ret=malloc(sizeof(COLOR));
    ret->color=color;
    return ret;
}

BUTTON *ControlsButtonNew(char *name,void (* func)(void))
{
    BUTTON *ret=malloc(sizeof(BUTTON));
    ret->name=malloc((strlen(name)+1)*sizeof(char));
    strcpy(ret->name,name);
    ret->func=func;
    return ret;
}

VALUE *ControlsValueNew(int value)
{
    VALUE *ret=malloc(sizeof(VALUE));
    ret->value=value;
    return ret;
}

