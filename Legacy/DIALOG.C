#include <stdlib.h>
#include <stdio.h>
#include <graph.h>
#include <stdarg.h>
#include <string.h>
#include "sys.h"
#include "screen.h"
#include "dialog.h"

void ControlDisplayReset(CONTROL *c,int display,int rows,int cols,int strwidth)
// purpose : resets the display attributes of a control
// returns : void
{
    c->displaytype=display;
    c->rows=rows;
    c->cols=cols;
    c->strwidth=strwidth;
}

int ControlReset(CONTROL *c,...)
// purpose : resets the values of a control, only sets the list or range
//           values, not the type,display,rows,cols,strwidth, use
//           ControlDisplayReset for these
// returns : TRUE if successful, else FALSE
{

}

CONTROL *ControlNew(int type,int display,int rows,int cols,int strwidth,...)
// list ... is elems,{int value,char *name}*elems           name=name;
// range... is int min,char *minname,int max,int maxname    name=NewString(name);
{
    va_list ap;
    CONTROL *ret=malloc(sizeof(CONTROL));
    CRANGE *rp; CLIST *cl; char *cp; int x;
    ret->controltype=type;
    ret->displaytype=display;
    ret->rows=rows;
    ret->cols=cols;
    ret->strwidth=strwidth;

    va_start(ap,strwidth);
    if (type==List)
    {
        cl=malloc(sizeof(CLIST));
        cl->elems=va_arg(ap,int);
        cl->elem=malloc(cl->elems*sizeof(CLISTELEM));
        cl->current=0;
        for (x=0;x<cl->elems;x++)
        {
            cl->elem[x].value=va_arg(ap,int);
            cl->elem[x].name=va_arg(ap,char *);
        }
        ret->store=cl;
//        _Break(1,"...cl->elems",cl->elems);
    }
    else // type==Range
    {
        rp=malloc(sizeof(CRANGE));
        rp->min=va_arg(ap,int);
        strcpy(rp->minname,va_arg(ap,char *));
        rp->max=va_arg(ap,int);
        strcpy(rp->maxname,va_arg(ap,char *));
        rp->current=rp->min;
        ret->store=rp;
    }
    va_end(ap);
    return ret;
}

void ControlPrint(CONTROL *c)
{
    CRANGE *cr; CLIST *cl; int x;
    switch (c->controltype)
    {
        case List : printf("List ."); break;
        case Range: printf("Range."); break;
    }
    switch (c->displaytype)
    {
        case Button : printf("Button."); break;
        case Radio  : printf("Radio ."); break;        
        case Color  : printf("Color ."); break;
        case Name   : printf("Name  ."); break;
        case Slider : printf("Slider."); break;
        case Dial   : printf("Dial  ."); break;
    }
    printf("Cols(%d).Rows(%d).Str(%d)\n",c->cols,c->rows,c->strwidth);
    switch (c->controltype)
    {
        case List :
            cl=(CLIST *)c->store;
            printf("[%d]\n",cl->current);
            for (x=0;x<cl->elems;x++)
            {
                printf("<%d.%s>\n",cl->elem[x].value,cl->elem[x].name);
            }
        break;
        case Range :
            cr=(CRANGE *)c->store;
            printf("[%d]\n",cr->current);
            printf("Min <%d.%s> Max <%d.%s>\n",cr->min,cr->minname,cr->max,cr->maxname);
        break;
    }
}

int MoveTo(int min,int max,int to)
{if ((to<=max) && (to>=min)) return TRUE; return FALSE;}

int Move(int min,int max,int *current,int direction)
// purpose : if direction added to current leaves current inside min..max it
// is moved
// returns : TRUE if moved, else FALSE
{
    if (MoveTo(min,max,*current+direction))
    {
        *current+=direction;
        return TRUE;
    }
    return FALSE;
}

int ControlMove(CONTROL *c,int howfar)
// purpose : moves current to the next in the control
// returns : TRUE if moves else false;
{
    CRANGE *cr; CLIST *cl;
    if (c->controltype==Range) {
        cr=(CRANGE *)c->store;
        return Move(cr->min,cr->max,&(cr->current),howfar);
    }
    else { // List
        cl=(CLIST *)c->store;
        return Move(0,cl->elems-1,&(cl->current),howfar);
    }
}

int ControlPrev(CONTROL *c) {return ControlMove(c,-1);}
// purpose : moves current to the prev in the control
// returns : TRUE if moves else false;

int ControlNext(CONTROL *c) {return ControlMove(c,1);}
// purpose : moves current to the next in the control
// returns : TRUE if moves else false;

ControlCurrentSet(CONTROL *c,int current)
// purpose :
// returns :
{
    CLIST *cl; CRANGE *cr;
    if (c->controltype==List)
    {
        cl=(CLIST *)c->store;
        if (MoveTo(0,cl->elems-1,current))
        cl->current=current;        
    }
    else
    {
        cr=(CRANGE *)c->store;
        if (MoveTo(cr->min,cr->max,current))
        cl->current=current;        
    }
}

int ControlCurrentGet(CONTROL *c)
// purpose :
// returns :
{
    CLIST *cl; CRANGE *cr;
    if (c->controltype==List)
    {
        cl=(CLIST *)c->store; return cl->current;
    }
    else
    {
        cr=(CRANGE *)c->store; return cl->current;
    }
}

ControlElemSet(CONTROL *c,int elem,int value,char *name)
// if range then if value==min strcpy(minname,name)
//               if value==max strcpy(maxname,name)
// if list then list[elem]=value,name.
{
    CRANGE *cr; CLIST *cl;
    if (c->controltype==Range)
    {
        cr=(CRANGE *)c->store;
        if (elem==cr->min) {strcpy(cr->minname,name); cr->min=value;}
        if (elem==cr->max) {strcpy(cr->maxname,name); cr->max=value;}
        // else do nothing            
    }
    else // c->controltype==List
    {
        cl=(CLIST *)c->store;
        cl->elem[elem].value=value;
        cl->elem[elem].name=name;
    }
}

void DisplayCircleFilled(int x1,int y1,int x2,int y2,int filled)
// displays a circle in the current color in the viewport defined by
// x1,y1-x2,y2, fills in the center if filled==TRUE
{
    int x,y;
    _ellipse(_GBORDER,x1,y1,x2,y2);
    if (filled)
    {
        x=(x2-x1)/4;
        y=(y2-y1)/4;
        _ellipse(_GFILLINTERIOR,x1+x,y1+y,x2-x,y2-y);        
    }
}

void DisplayColor(int x1,int y1,int x2,int y2,int color,int selected)
{
    if (selected)
    {
        _setcolor(CONTROLHILITE);
        _setlinestyle(0xff00);
        _rectangle(_GBORDER,x1,y1,x2,y2);
        _setlinestyle(0xffff);
    }
    _setcolor(color);
    _rectangle(_GFILLINTERIOR,x1+2,y1+2,x2-2,y2-2);
}

void DisplayScale(VIEWPORT *vp,int range,int pos,char *value,int chars)
// displays the scale in the vp viewport
{
    int x;
    double width=0.8/(double)(range-1), 
            all=0.1+0.8*(double)(pos)/(double)(range-1),
            at=0.1;
    _settextalign(_CENTER,_HALF);
    _setcharsize(2*(vp->y2-vp->y1)/5,(vp->x2-vp->x1)/(5*chars));
    _setcharspacing(0);
    if (range==1) {at+=0.4; all=0.5;}
    SetViewport(vp);
    _setcolor(CONTROLHILITE);
    _moveto_w(all,0.5); _lineto_w(all,0.4);
    _rectangle_w(_GBORDER,all-0.1,0.4,all+0.1,0.0);
    _setcolor(CONTROLTEXT);
    _moveto_w(0.1,0.9); _lineto_w(0.9,0.9); // main line;
    _grtext_w(all,0.2,value);
    for (x=0;x<range;x++)
    {
        _moveto_w(at,0.9); _lineto_w(at,0.5);
        at+=width;                
    }
    _setvieworg(0,0);
}

void DisplayRangeColor(CRANGE *c,VIEWPORT *vp)
{
    int width=(vp->x2-vp->x1+1)/(c->max-c->min+1);
    int at=vp->x1,x;

    for (x=c->min;x<=c->max;x++)
    {
        DisplayColor(at,vp->y1,at+width,vp->y2,x,x==c->current);
        at+=width;
    }
}

void DisplayRange(CRANGE *r,VIEWPORT *vp,int display,int rows,int cols,int strwidth)
{
    int height=((rows-1)*(vp->y2-vp->y1))/rows;
    VIEWPORT scale;
    int names=FALSE;
    char buffer[10];

    scale.x1=vp->x1;
    scale.y1=vp->y1;
    scale.x2=vp->x2;
    scale.y2=vp->y1+height;
    _setvieworg(0,0);
    _setcharsize(vp->y2-vp->y1-height,(vp->x2-vp->x1)/(4*strwidth));
    _setcharspacing(0);
    if ((r->minname[0]!='/0') && (r->maxname[0]!='/0'))
    {
        _settextalign(_LEFT,_TOP);  _grtext(vp->x1,vp->y1+height,r->minname);
        _settextalign(_RIGHT,_TOP); _grtext(vp->x2,vp->y1+height,r->maxname);
        names=TRUE;
    }
    else if (r->minname[0]!='/0')
    {
        _settextalign(_CENTER,_TOP);  _grtext(vp->x1+((vp->x2-vp->x1)/2),vp->y1+height,r->minname);
        names=TRUE;
    }
    // else display no names.

    switch (display)
    {
        case Slider :
            sprintf(buffer,"%d",r->current);
            DisplayScale(names?&scale:vp,r->max-r->min+1,r->current-r->min,buffer,3);
        break;
        case Color :
            DisplayRangeColor(r,names?&scale:vp);            
        break;
    }
}

void DisplayList(CLIST *cl,VIEWPORT *vp,int display,int rows,int cols,int strwidth)
{
    int atx=vp->x1,aty=vp->y1,width=(vp->x2-vp->x1+1)/cols;
    int height=(vp->y2-vp->y1+1)/rows,_3hon4=(3*height)/4,x;
    int col=1,row=0,texty=height/2,textx=width/2,elements=rows*cols;
    
//    _Break(7,"width",width,"height",height,"atx",atx,"aty",aty,
//            "display",display,"elements",elements,"cl->elems",cl->elems);

    if (cl->elems<elements) elements=cl->elems;

    if (display==Slider)
        DisplayScale(vp,cl->elems,cl->current,cl->elem[cl->current].name,3);            
    else
    {
        switch (display)
        {
            case Button : case Name :
                _settextalign(_CENTER,_HALF);
                _setcharsize(height/2,width/strwidth);
            break;
            case Color :
                _settextalign(_CENTER,_TOP);
                _setcharsize(height/5,width/strwidth);
            break;
            case Radio :
                _settextalign(_LEFT,_HALF);
                _setcharsize(height/2,(7*width)/(8*strwidth));
            break;
        }

        for (x=0;x<elements;x++)
        {
            switch (display)
            {
                case Radio :
                    _setcolor(CONTROLTEXT);
                    DisplayCircleFilled(atx,aty,atx+(width/8),aty+height,x==cl->current);
                    _grtext(atx+(width/8),aty+texty,cl->elem[x].name);                        
                break;
                case Button :
                    if (x==cl->current)
                    {
                        _setcolor(CONTROLHILITE);
                        _rectangle(_GFILLINTERIOR,atx+5,aty+5,atx+width-5,aty+height-5);
                    }
                    _setcolor(CONTROLTEXT);
                    _rectangle(_GBORDER,atx+3,aty+3,atx+width-3,aty+height-3);
                    _grtext(atx+textx,aty+texty,cl->elem[x].name);
                break;
                case Name :
                    if (x==cl->current)
                        _setcolor(CONTROLHILITE);
                    else
                        _setcolor(CONTROLTEXT);
                    _grtext(atx+textx,aty+texty,cl->elem[x].name);
                break;
                case Color :
                    DisplayColor(atx,aty,atx+width,
                        aty+(cl->elem[x].name==NULL?height:_3hon4),
                        cl->elem[x].value,x==cl->current);
                    _setcolor(CONTROLTEXT);
                    _grtext(atx,aty+_3hon4,cl->elem[x].name);
                break;
                default:
                    exit(0);
                break;
            }
            if (col==cols) {col=1; aty+=height; atx=vp->x1;}
            else {atx+=width; ++col;}
        }
    }        
}

void ControlDisplay(CONTROL *c,VIEWPORT *vp)
{
    switch (c->controltype)
    {
        case Range :
            DisplayRange((CRANGE *)c->store,vp,c->displaytype,c->rows,c->cols,c->strwidth);
        break;
        case List :
            DisplayList((CLIST *)c->store,vp,c->displaytype,c->rows,c->cols,c->strwidth);
        break;
        default:
            exit(0);
        break;
    }            
}

/*
DIALOG *DialogNew(char *name,int controls,...= {name,enabled,selectable,CONTROL *}
DialogPrev(DIALOG *d)
DialogNext(DIALOG *d)
DialogCurrentSet(DIALOG *d,int current)
int DialogCurrentGet(DIALOG *d)
CONTROL *DialogControlGet(DIALOG *d,int control)
DialogControlSet(DIALOG *d,CONTROL *c)
DialogStateGet(DIALOG *d,...) // ...={int *}
DialogStateSet(DIALOG *d,...) // ...={int}
DialogDisplay(DIALOG *d,VIEWPORT *vp,MEMPAGE *mp)
DialogCall(DIALOG *d,VIEWPORT *vp,MEMPAGE *mp)

// purpose :
// returns :
*/
void main()
{
    char *a=NewString("Hellor");
    CONTROL *c;
    VIEWPORT main={0,0,400,50};

    c=ControlNew(List,Name,1,3,10,3,1,a,2,a,3,a);

    ScreenInit();
    _setcharsize(10,5);
    _setcolor(CONTROLBACK);
    _rectangle(_GFILLINTERIOR,main.x1,main.y1,main.x2,main.y2);
    _setcolor(CONTROLHILITE);
    _rectangle(_GBORDER,main.x1,main.y1,main.x2,main.y2);
    _setcolor(CONTROLTEXT);
    ControlDisplay(c,&main);
    SwapPage();
    GetKey();
    ShutDown();
}
