#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>      // sprintf
#include <graph.h>
#include "sys.h"
#include "screen.h"
#include "controls.h"

int ControlsControlListSet(CONTROLLIST *cl,...)
// if Button ...=char *
//    Value  ...=int
//    Color  ...=int
//    Radio  ...=int
//    Name   ...=char *
{
    int val,x;
    char *charp;
    va_list ap;
    BUTTON *bp; VALUE *vp; RADIO *rp; COLOR *cp; NAME *np;    
    va_start(ap,cl);
    switch (cl->type)
    {
        case Button :
            bp=(BUTTON *)(cl->store);
            charp=va_arg(ap,char *);
            for (x=0;x<cl->size;x++)
            {
                if (strcmp(bp[x].name,charp)==0) // equal
                {
                    cl->current=x;
                    return TRUE;
                }
            }
            return FALSE;
            break;
        case Radio :
            rp=(RADIO *)(cl->store);
            val=va_arg(ap,int);
            for (x=0;x<cl->size;x++)
            {
                if (rp[x].value==val)
                {
                    cl->current=x;
                    return TRUE;
                }
            }
            return FALSE;
            break;
        case Color :
            cp=(COLOR *)(cl->store);
            val=va_arg(ap,int);
            for (x=0;x<cl->size;x++)
            {
                if (cp[x].value==val)
                {
                    cl->current=x;
                    return TRUE;
                }
            }
            return FALSE;
            break;
        case Value :
            vp=(VALUE *)(cl->store);
            val=va_arg(ap,int);
            for (x=0;x<cl->size;x++)
            {
                if (rp[x].value==val)
                {
                    cl->current=x;
                    return TRUE;
                }
            }
            return FALSE;
            break;
        case Name :
            np=(NAME *)(cl->store);
            charp=va_arg(ap,char *);
            for (x=0;x<cl->size;x++)
            {
                if (strcmp(np[x].name,charp)==0) // equal
                {
                    cl->current=x;
                    return TRUE;
                }
            }
            return FALSE;
            break;
    }
    return FALSE;
}

CONTROLLIST *ControlsControlListNew(int type,int copy,int num,...)
// if copy==TRUE the string values are copied else only the pointers
// are copied.
// Radio ...={int value,char *name}*num
// Color ...=FALSE,{int value}*num,... or TRUE,int minimum.
//    FALSE = non scaled, TRUE = num scaled numbers, min..(min+num-1)
// Button...={char *name,void (* func)(void)}*num
// Value ...=FALSE,{int value}*num,... or TRUE,int minimum.
//    FALSE = non scaled, TRUE = num scaled numbers, min..(min+num-1)
// Name  ...={char *name}*num
{
    va_list ap;
    int x,min;
    char *charp;
    CONTROLLIST *cl=malloc(sizeof(CONTROLLIST));
    BUTTON *bp; VALUE *vp; RADIO *rp; COLOR *cp; NAME *np;
    cl->type=type;
    cl->current=0;
    cl->size=num;
    va_start(ap,num);
    switch (type)
    {
        case Radio :
            cl->store=malloc(num*sizeof(RADIO));
            rp=(RADIO *)(cl->store);
            for (x=0;x<num;x++)
            {
                rp[x].value=va_arg(ap,int);
                charp=va_arg(ap,char *);
                if (copy==TRUE)
                {
                    rp[x].name=malloc((strlen(charp)+1)*sizeof(char));
                    strcpy(rp[x].name,charp);
                } else rp[x].name=charp;
            }
            break;
        case Color :
            cl->store=malloc(num*sizeof(COLOR));
            cp=(COLOR *)(cl->store);
            if (va_arg(ap,int)==TRUE)
            {
                min=va_arg(ap,int);
                for (x=0;x<num;x++) cp[x].value=min+x;
            }
            else
            {
                for (x=0;x<num;x++) cp[x].value=va_arg(ap,int);
            }
            break;
        case Button :
            cl->store=malloc(num*sizeof(BUTTON));
            bp=(BUTTON *)(cl->store);
            for (x=0;x<num;x++)
            {
                charp=va_arg(ap,char *);
                if (copy==TRUE)
                {
                    bp[x].name=malloc((strlen(charp)+1)*sizeof(char));
                    strcpy(bp[x].name,charp);
                } else bp[x].name=charp;
                bp[x].func=va_arg(ap,void *);
            }
            break;
        case Value :
            cl->store=malloc(num*sizeof(VALUE));
            vp=(VALUE *)(cl->store);
            if (va_arg(ap,int)==TRUE)
            {   min=va_arg(ap,int);
                for (x=0;x<num;x++) vp[x].value=min+x;
            }
            else {for (x=0;x<num;x++) vp[x].value=va_arg(ap,int);}
            break;
        case Name :
            cl->store=malloc(num*sizeof(NAME));
            np=(NAME *)(cl->store);
            for (x=0;x<num;x++)
            {   charp=va_arg(ap,char *);
                if (copy==TRUE)
                {
                    np[x].name=malloc((strlen(charp)+1)*sizeof(char));
                    strcpy(np[x].name,charp);
                } else np[x].name=charp;
            }
            break;
    }
    return cl;
}

int ControlsControlListNext(CONTROLLIST *cl)
{ if (cl->current<cl->size-1) {cl->current+=1; return TRUE;} return FALSE;}

int ControlsControlListPrev(CONTROLLIST *cl)
{ if (cl->current>0) {cl->current-=1; return TRUE;} return FALSE;}

void DisplayCircleFilled(double x1,double y1,double x2,double y2,int filled)
// displays a circle in the current color in the viewwin defined by
// x1,y1-x2,y2, fills in the center if filled==TRUE
{
    double x,y;
    _ellipse_w(_GBORDER,x1,y1,x2,y2);
    if (filled)
    {
        x=(x2-x1)/4.0;
        y=(y2-y1)/4.0;
        _ellipse_w(_GFILLINTERIOR,x1+x,y1+y,x2-x,y2-y);        
    }
}

void ControlsControlListDisplay(CONTROLLIST *cl,int cols,int chars)
// displays the control list in the current viewport, cols and chars,
// if relevant, are used to display in cols columns with chars characters
// in each column. If not relevant chars the value must be set to zero, the
// cols must be set to one.
{
    int x,col=0,rows=(cl->size/cols)+(cl->size%cols?1:0);
    double width=1.0/(double)cols,atx=0.0;
    double height=1.0/(double)rows,aty=0.0;
    double gapx=1.0/((double)(10*cols*(cols/2)));
    double gapy=1.0/((double)(10*rows*rows));
    double text=height/2.0;
    double all;
    char buffer[10];
    BUTTON *bp; VALUE *vp; RADIO *rp; COLOR *cp; NAME *np;

    _setcolor(CONTROLBACK);
    _rectangle_w(_GFILLINTERIOR,0.0,0.0,1.0,1.0);


    switch (cl->type)
    {
        case Value :
            vp=(VALUE *)(cl->store);
            _settextalign(_CENTER,_HALF);
            _setcharsize_w(0.07,0.4*width/(double)chars);
            _setcharspacing_w(-0.02);
            all=0.1+0.8*(double)(cl->current)/(double)(cl->size-1);
            _setcolor(CONTROLHILITE);
            _moveto_w(all,0.5); _lineto_w(all,0.4);
            _rectangle_w(_GBORDER,all-0.1,0.4,all+0.1,0.0);
            _setcolor(CONTROLTEXT);
            _moveto_w(0.1,0.9); _lineto_w(0.9,0.9); // main line;
            sprintf(buffer,"%d",vp[cl->current].value);
            _grtext_w(all,0.2,buffer);
            width=0.8/(double)(cl->size-1);
            atx=0.1;
            for (x=0;x<cl->size;x++)
            {
                _moveto_w(atx,0.9); _lineto_w(atx,0.5);
                atx+=width;                
            }
        break;
        case Radio :
            rp=(RADIO *)(cl->store);
            all=width/10.0; // circle section
            _settextalign(_LEFT,_HALF);
            _setcharsize_w(height*0.15,0.4*width/(double)chars);
            _setcharspacing_w(0.0);
            _setcolor(CONTROLTEXT);
            for (x=0;x<cl->size;x++)
            {
                DisplayCircleFilled(atx,aty,atx+all,aty+height,x==cl->current?TRUE:FALSE);
                _grtext_w(atx+all+gapx,aty+text,rp[x].name);
                if (col>=cols-1) {col=0; atx=0.0; aty+=height;}
                else {atx+=width; ++col;}
            }
            break;
        case Color :
            cp=(COLOR *)(cl->store);
            all=width/4.0;  // select section
            for (x=0;x<cl->size;x++)
            {
                if (x==cl->current)
                {
                    _setcolor(CONTROLHILITE);
                    _rectangle_w(_GFILLINTERIOR,atx,aty,atx+width,aty+height);
                    _setcolor(CONTROLBACK);
                    _rectangle_w(_GFILLINTERIOR,atx+all,aty,atx+width-all,aty+height);
                }
                _setcolor(cp[x].value);
                _rectangle_w(_GFILLINTERIOR,atx+gapx,aty+gapy,atx+width-gapx,aty+height-gapy);
                if (col>=cols-1) {col=0; atx=0.0; aty+=height;}
                else {atx+=width; ++col;}
            }
            break;
        case Name :
            np=(NAME *)(cl->store);
            all=width/2.0;  // text centered
            _settextalign(_CENTER,_HALF);
            _setcharsize_w(height*0.10,0.4*width/(double)chars);
            _setcolor(CONTROLTEXT);
            atx=all;
            aty=text;
            for (x=0;x<cl->size;x++)
            {
                _grtext_w(atx,aty,np[x].name);
                if (col>=cols-1) {col=0; atx=0.0; aty+=height;}
                else {atx+=width; ++col;}
            }
            break;
        case Button :
            bp=(BUTTON *)(cl->store);
            all=width/2.0;  // text centered
            _settextalign(_CENTER,_HALF);
            _setcharsize_w(height*0.15,0.4*width/(double)chars);
            _setcharspacing_w(0.0);
            for (x=0;x<cl->size;x++)
            {
                if (x==cl->current)
                {
                    _setcolor(CONTROLHILITE);
                    _rectangle_w(_GFILLINTERIOR,atx+gapx,aty+gapy,atx+width-gapx,aty+height-gapy);
                }
                _setcolor(CONTROLTEXT);
                _rectangle_w(_GBORDER,atx+gapx,aty+gapy,atx+width-gapx,aty+height-gapy);
                _grtext_w(atx+all,aty+text,bp[x].name);
                if (col>=cols-1) {col=0; atx=0.0; aty+=height;}
                else {atx+=width; ++col;}
            }
            break;
    }
}

int ControlsDialogSet(DIALOG *dl,int control,...)
{
    va_list ap;

    va_start(ap,control);
    switch (dl->control[control].cl->type)
    {
        case Button : case Name :
            return ControlsControlListSet(dl->control[control].cl,va_arg(ap,char *));
            break;
        case Value : case Color : case Radio :
            return ControlsControlListSet(dl->control[control].cl,va_arg(ap,int));
            break;
    }
    va_end(ap);
    return FALSE;
}

DIALOG *ControlsDialogNew(double fract,int controls,...)
// fract is fraction (0.0<=fract<=1.0) if viewport for default buttons
// ... is enabled,name,displaycols,displaychars,double displayfract,list,...
// dialogs automatically get OK,Reset,Cancel Buttons.
{
    va_list ap;
    char *cp;
    int x;
    DIALOG *dl=malloc(sizeof(DIALOG));
    dl->controls=controls+1;
    dl->control=malloc(dl->controls*sizeof(DIALOGELEM));
    dl->current=0;
    dl->control[controls].enabled=TRUE;
    dl->control[controls].name=NULL;
    dl->control[controls].cl=ControlsControlListNew(Button,FALSE,3,"OK",NULL,"Reset",NULL,"Cancel",NULL);
    dl->control[controls].displaycols=3;
    dl->control[controls].displaychars=5;
    dl->control[controls].displayfract=fract;

    va_start(ap,controls);
    for (x=0;x<controls;x++)
    {
        dl->control[x].enabled=va_arg(ap,int);
        cp=va_arg(ap,char *);
        dl->control[x].name=malloc((strlen(cp)+1)*sizeof(char));
        strcpy(dl->control[x].name,cp);
        dl->control[x].displaycols=va_arg(ap,int);
        dl->control[x].displaychars=va_arg(ap,int);
        dl->control[x].displayfract=va_arg(ap,double);
        dl->control[x].cl=va_arg(ap,CONTROLLIST *);
    }
    va_end(ap);
    return dl;
}

int ControlsDialogNext(DIALOG *dl)
{
    int old=dl->current;
    for (;;)
    {
        dl->current+=1;
        if (dl->current>=dl->controls) // out of range
            {dl->current=old; return FALSE;}
        if (dl->control[dl->current].enabled==TRUE) // ok elem
            return TRUE;
        // else continue
    }
}

int ControlsDialogPrev(DIALOG *dl)
{
    int old=dl->current;
    for (;;)
    {
        dl->current-=1;
        if (dl->current<0) // out of range
            {dl->current=old; return FALSE;}
        if (dl->control[dl->current].enabled==TRUE) // ok elem
            return TRUE;
        // else continue
    }
}

void ControlsDialogReset(void) {}
// used to reset values to original, if func== it

void ControlsDialogDisplay(VIEWPORT *vp,DIALOG *dl,double gap)
// displays the dialog in the viewport vp
// gap is amount to leave out of each window to separate them, is vertical gap only
{
    int x;
    double fract;
    double at=0.0;
    double height=(double)(vp->y2-vp->y1);
    
    for (x=0;x<dl->controls;x++)
    {
        if (dl->control[x].enabled==TRUE)
        {
            fract=dl->control[x].displayfract;
            if (x==dl->current)
            {
                _setcolor(CONTROLHILITE);
                _setviewport(vp->x1,(int)(at*height),vp->x2,(int)((at+fract)*height));
                _rectangle_w(_GFILLINTERIOR,0.0,0.0,1.0,1.0);
            }
            _setviewport(vp->x1+2,(int)((at+gap)*height),vp->x2-2,(int)((at+fract-gap)*height));
            ControlsControlListDisplay(dl->control[x].cl,dl->control[x].displaycols,dl->control[x].displaychars);
            at+=fract;
        }
    }
}

int ControlsDialogCall(VIEWPORT *vp,DIALOG *dl,double gap)
// ... is int *value,int *temp
// if OK is pressed the values are changed, else the values 
{
    int x;
/*
    int **value=malloc(dl->controls*sizeof(int *));
    va_list ap;
    for (x=0;x<dl->controls;x++) // read values and temps
    {
        dl->control[x]
        value[x]=va_arg(ap,int *);
        dl->control[x].temp=*(value[x]);
        dl->control[x].temp=*(value[x]);
    }
    va_end(ap);
*/    
    for (;;)
    {
        _clearscreen(_GCLEARSCREEN);
        ControlsDialogDisplay(vp,dl,gap);    
        SwapPage();
        switch(GetKey())
        {
            case KP4 : ControlsControlListPrev(dl->control[dl->current].cl); break;
            case KP6 : ControlsControlListNext(dl->control[dl->current].cl); break;
            case KP8 : ControlsDialogPrev(dl); break;
            case KP2 : ControlsDialogNext(dl); break;
            case ESC : return TRUE; 
        }
    }
}

/*
void main()
{
    VIEWPORT vp={100,0,500,300};

    DIALOG *dl=ControlsDialogNew(0.2,3,
        "Name",     2,  10, 0.2,
            ControlsControlListNew(Radio,4,0,"Matthew",1,"Mark",2,"Luke",3,"John"),
        "Color",    5,  0,  0.4,
            ControlsControlListNew(Color,15,TRUE,1),
        "Skill",    1,  6,  0.2,
            ControlsControlListNew(Value,100,TRUE,1));
    
// name,displaycols,displaychars,type,controllist,...
    ScreenInit();
    ControlsDialogCall(&vp,dl,0.01);
    ShutDown();
}
*/
/*
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
*/
