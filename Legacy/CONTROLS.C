#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <graph.h>
#include "sys.h"
#include "screen.h"
#include "controls.h"

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

int ControlsRadioListNext(RADIOLIST *rl)
{ if (rl->current<rl->rbs-1) {rl->current+=1; return TRUE;} return FALSE;}

int ControlsRadioListPrev(RADIOLIST *rl)
{ if (rl->current>=0) {rl->current-=1; return TRUE;} return FALSE;}

void ControlsRadioListDisplay(RADIOLIST *rl,int cols,int chars)
// displays the radio list in the current viewport using cols columns
{
    int col=0;
    int rows=(rl->rbs/cols)+(rl->rbs%cols?1:0);
    double width=1.0/(double)cols,atx=0.0,circlex=width/10.0,fillx=circlex/4.0;
    double height=1.0/(double)rows,aty=0.0,circley=height,filly=circley/4.0;
    double gap=height/10.0;
    double text=height/2.0; // half way down
    int n;
    _setcolor(CONTROLBACK);
    _rectangle_w(_GFILLINTERIOR,0.0,0.0,1.0,1.0);
    _setcolor(CONTROLTEXT);
    _settextalign(_LEFT,_HALF);
    _setcharsize_w(height*0.25,0.4*width/(double)chars);
    for (n=0;n<rl->rbs;n++)
    {
        _ellipse_w(_GBORDER,atx,aty+gap,atx+circlex,aty+circley-gap);
        if (n==rl->current)
            _ellipse_w(_GFILLINTERIOR,atx+fillx,aty+filly+gap,
                       atx+circlex-fillx,aty+circley-filly-gap);
        _grtext_w(atx+circlex+fillx,aty+text,rl->rb[n].name);
        if (col>=cols-1) {col=0; atx=0.0; aty+=height;}
        else {atx+=width; ++col;}
    }
}

COLORLIST *ControlsColorListNew(int size,int scale,...)
// if scale is true the next value is the minimum
// else size other args are the values
{
    int x,min;
    va_list ap;
    COLORLIST *cl=malloc(sizeof(COLORLIST));
    cl->current=0;
    cl->colors=size;
    cl->color=malloc(size*sizeof(COLOR));
    if (scale==TRUE)
    {
        va_start(ap,size);
        min=va_arg(ap,int);
        for (x=0;x<size;x++)
            cl->color[x].value=min+x;
    }
    else // not specifically scaled
    {
        va_start(ap,size);
        for (x=0;x<size;x++)
            cl->color[x].value=va_arg(ap,int);
    }
    va_end(ap);
    return cl;
}

int ControlsColorListNext(COLORLIST *cl)
{ if (cl->current<cl->colors-1) {cl->current+=1; return TRUE;} return FALSE;}

int ControlsColorListPrev(COLORLIST *cl)
{ if (cl->current>=0) {cl->current-=1; return TRUE;} return FALSE;}

void ControlsColorListDisplay(COLORLIST *cl,int cols)
// displays the color list in the current viewport
{
    int x,col=0,rows=(cl->colors/cols)+(cl->colors%cols?1:0);
    double width=1.0/cols;
    double height=1.0/rows;
    double gapx=0.05/cols,gapy=0.05/rows;
    double posx=0.0,posy=0.0;
    
    _setcolor(CONTROLBACK);
    _rectangle_w(_GFILLINTERIOR,0.0,0.0,1.0,1.0);
    for (x=0;x<cl->colors;x++)
    {
        if (x==cl->current)
        {   _setcolor(CONTROLHILITE);
            _rectangle_w(_GFILLINTERIOR,posx,posy,posx+width,posy+height);
            _setcolor(CONTROLBACK);
            _rectangle_w(_GFILLINTERIOR,posx+(width/4.0),posy,posx+(3.0*width/4.0),posy+height);
        }
        _setcolor(cl->color[x].value);
        _rectangle_w(_GFILLINTERIOR,posx+gapx,posy+gapy,posx+width-gapx,posy+height-gapy);
        if (col>=cols-1) {col=0; posx=0.0; posy+=height;}
        else {posx+=width; ++col;}
    }
}

BUTTONLIST *ControlsButtonListNew(int buttons, ...)
{
    int x;
    char *cp;
    va_list ap;
    BUTTONLIST *bl=malloc(sizeof(BUTTONLIST));
    bl->current=0;
    bl->buttons=buttons;
    bl->button=malloc(buttons*sizeof(BUTTON));
    va_start(ap,buttons);
    {
        for (x=0;x<buttons;x++)
        {
            cp=va_arg(ap,char *);
            bl->button[x].name=malloc((strlen(cp)+1)*sizeof(char));
            strcpy(bl->button[x].name,cp);
            bl->button[x].func=va_arg(ap,void *);
        }
    }
    va_end(ap);
    return bl;
}

int ControlsButtonListNext(BUTTONLIST *bl)
{ if (bl->current<bl->buttons-1) {bl->current+=1; return TRUE;} return FALSE;}

int ControlsButtonListPrev(BUTTONLIST *bl)
{ if (bl->current>=0) {bl->current-=1; return TRUE;} return FALSE;}

void ControlsButtonListDisplay(BUTTONLIST *bl,int cols,int chars)
// displays the buttons list in the current viewport
{
    int x,col=0,rows=(bl->buttons/cols)+(bl->buttons%cols?1:0);
    double width=1.0/(double)cols,height=1.0/(double)rows;
    double posx=0.0,posy=0.0,gapx=0.05/(double)cols,gapy=0.05/(double)rows;
    double textx=width/2.0; // half way down
    double texty=height/2.0; // half way across

    _setcolor(CONTROLBACK);
    _rectangle_w(_GFILLINTERIOR,0.0,0.0,1.0,1.0);
    _setcolor(CONTROLTEXT);
    _settextalign(_CENTER,_HALF);
    _setcharsize_w(height*0.25,0.4*width/(double)chars);

    for (x=0;x<bl->buttons;x++)
    {
        if (x==bl->current)
        {
            _setcolor(CONTROLHILITE);
            _rectangle_w(_GFILLINTERIOR,posx+gapx,posy+gapy,posx+width-gapx,posy+height-gapy);
        }
        _setcolor(CONTROLTEXT);
        _rectangle_w(_GBORDER,posx+gapx,posy+gapy,posx+width-gapx,posy+height-gapy);

        _grtext_w(posx+textx,posy+texty,bl->button[x].name);
        if (col>=cols-1) {col=0; posx=0.0; posy+=height;}
        else {posx+=width; ++col;}
    }
}

VALUELIST *ControlsValueListNew(int scale,int names,int size,...)
// if scale is true the next value is the minimum, followed by the names
// else size other args are the values then the names
{
    int x,min;
    char *cp;
    va_list ap;
    VALUELIST *vl=malloc(sizeof(VALUELIST));
    vl->current=0;
    vl->values=size;
    vl->names=names;
    vl->value=malloc(size*sizeof(VALUE));
    vl->name=malloc(names*sizeof(char *));
    va_start(ap,size);
    if (scale==TRUE)
    {
        min=va_arg(ap,int);
        for (x=0;x<size;x++)
            vl->value[x].value=min+x;
    }
    else // not specifically scaled
    {
        for (x=0;x<size;x++)
            vl->value[x].value=va_arg(ap,int);
    }
    for (x=0;x<names;x++)
    {
        cp=va_arg(ap,char *);
        vl->name[x]=malloc((strlen(cp)+1)*sizeof(char));
        strcpy(vl->name[x],cp);
    }
    va_end(ap);
    return vl;
}

int ControlsValueListNext(VALUELIST *vl)
{ if (vl->current<vl->values-1) {vl->current+=1; return TRUE;} return FALSE;}

int ControlsValueListPrev(VALUELIST *vl)
{ if (vl->current>=0) {vl->current-=1; return TRUE;} return FALSE;}

void ControlsValueListDisplay(VALUELIST *vl,int chars)
// Displays the value list in the current viewport
{
    int x,shorts=vl->values-2;
    double width=1.0/(vl->values-1);
    double height=(vl->names==0?1.0:0.5);
    double heightshort=height/4.0;
    double heightlong=height/2.0;
    double pos=width;
    double arrow=(double)(vl->current)*width;

    _setcharsize_w(0.5*0.25,0.4/(double)(chars*vl->names));
    _setcolor(CONTROLBACK);
    _rectangle_w(_GFILLINTERIOR,0.0,0.0,1.0,1.0);
    _setcolor(CONTROLTEXT);
    _moveto_w(0.0,height); _lineto_w(1.0,height); // main line
    for (x=0;x<shorts;x++)
    {
        _moveto_w(pos,height);
        _lineto_w(pos,height-heightshort); // short line
        pos+=width;
    }    
    _moveto_w(0.0,height); _lineto_w(0.0,height-heightlong);   // left long line
    _moveto_w(1.0,height); _lineto_w(1.0,height-heightlong);   // right long line            

    if (vl->names==1)
    {
        _settextalign(_CENTER,_TOP);
        _grtext_w(0.5,height,vl->name[0]);
    }
    else if (vl->names>1)
    {
        _settextalign(_LEFT,_TOP);
        _grtext_w(0.0,height,vl->name[0]);
        _settextalign(_RIGHT,_TOP);
        _grtext_w(1.0,height,vl->name[vl->names-1]);
        width=1.0/(double)(vl->names-2);
        _settextalign(_CENTER,_TOP);
        pos=width/2;
        for (x=0;x<(vl->names-2);x++)
        {
            _grtext_w(pos,height,vl->name[1+x]);            
            pos+=width;
        }
    }
    // else zero names so do nothing
        
    
    _setcolor(CONTROLHILITE);
    _moveto_w(arrow,0.0); _lineto_w(arrow,height-heightlong); // arrow line
    _moveto_w(arrow,height-heightlong);
    _lineto_w(arrow-0.02,height-heightlong-heightshort);  // left head
    _moveto_w(arrow,height-heightlong);
    _lineto_w(arrow+0.02,height-heightlong-heightshort);    // right head
}

void main()
{
    RADIOLIST *rl=ControlsRadioListNew(4,0,"Matthew",1,"Mark",
                                         2,"Luke",3,"John");
    COLORLIST *cl=ControlsColorListNew(16,TRUE,0);
    BUTTONLIST *bl=ControlsButtonListNew(3,"OK",NULL,"Reset",NULL,"Cancel",NULL);
    VALUELIST *vl=ControlsValueListNew(TRUE,3,10,1,"Low","Med","High");
    ScreenInit();
    _setviewport(100,0,400,100);
    ControlsButtonListDisplay(bl,3,8);
    _setviewport(100,100,400,200);
    ControlsRadioListDisplay(rl,2,8);
    _setviewport(100,200,400,250);
    cl->current=7;
    ControlsColorListDisplay(cl,8);
    _setviewport(100,250,400,350);
    vl->current=4;
    ControlsValueListDisplay(vl,5);
    SwapPage();
    GetKey();
}


