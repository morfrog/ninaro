#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>      // sprintf
#include <graph.h>
#include "sys.h"
#include "screen.h"
#include "controls.h"

int ControlsControlListSetCurrent(CONTROLLIST *cl,...)
// if Button ...=char *
//    Value  ...=int
//    Color  ...=int
//    Radio  ...=int
//    Name   ...=char *
{
    int val,x;
    char *charp;
    va_list ap;
    BUTTON *bp; VALUE *vp; RADIO *rp; COLOR *cp; NAME *np; RANGE *rnp;   
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
        case Range :
            rnp=(RANGE *)(cl->store);
            val=va_arg(ap,int);
            if ((val<rnp[0].min) || (val>rnp[0].max)) return FALSE;
            cl->current=val; return TRUE;
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

ControlsControlListChange(CONTROLLIST *cl,...)
// Range  ...=min,max.
{
    va_list ap;
    BUTTON *bp; VALUE *vp; RADIO *rp; COLOR *cp; NAME *np; RANGE *rnp;   

    va_start(ap,cl);
    switch (cl->type)
    {
        case Range :
            rnp=(RANGE *)(cl->store);
            rnp[0].min=va_arg(ap,int);
            rnp[0].max=va_arg(ap,int);
            if (cl->current>rnp[0].max) cl->current=rnp[0].max;
            if (cl->current<rnp[0].min) cl->current=rnp[0].min;
        break;
    }
    va_end(ap);
}

CONTROLLIST *ControlsControlListNew(int type,int copy,int num,...)
// if copy==TRUE the string values are copied else only the pointers
// are copied.
// Radio ...={int value,char *name}*num
// Color ...=FALSE,{int value}*num,... or TRUE,int minimum.
//    FALSE = non scaled, TRUE = num scaled numbers, min..(min+num-1)
// Button...={char *name}*num
// Value ...=FALSE,{int value}*num,... or TRUE,int minimum.
//    FALSE = non scaled, TRUE = num scaled numbers, min..(min+num-1)
// Range ...=int min. max is calculated off num
// Name  ...={char *name}*num
{
    va_list ap;
    int x,min;
    char *charp;
    CONTROLLIST *cl=malloc(sizeof(CONTROLLIST));
    BUTTON *bp; VALUE *vp; RADIO *rp; COLOR *cp; NAME *np;RANGE *rnp;
    cl->type=type;
    cl->current=0;
    cl->size=num;
    va_start(ap,num);
    switch (type)
    {
        case Radio :
            rp=malloc(num*sizeof(RADIO));
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
            cl->store=rp;
            break;
        case Color :
            cp=malloc(num*sizeof(COLOR));
            if (va_arg(ap,int)==TRUE)
            {
                min=va_arg(ap,int);
                for (x=0;x<num;x++) cp[x].value=min+x;
            }
            else
            {
                for (x=0;x<num;x++) cp[x].value=va_arg(ap,int);
            }
            cl->store=cp;
            break;
        case Button :
            bp=malloc(num*sizeof(BUTTON));
            for (x=0;x<num;x++)
            {
                charp=va_arg(ap,char *);
                if (copy==TRUE)
                {
                    bp[x].name=malloc((strlen(charp)+1)*sizeof(char));
                    strcpy(bp[x].name,charp);
                } else bp[x].name=charp;
            }
            cl->store=bp;
            break;
        case Value :
            vp=malloc(num*sizeof(VALUE));
            if (va_arg(ap,int)==TRUE)
            {   min=va_arg(ap,int);
                for (x=0;x<num;x++) vp[x].value=min+x;
            }
            else {for (x=0;x<num;x++) vp[x].value=va_arg(ap,int);}
            cl->store=vp;
            break;
        case Range :
            rnp=malloc(sizeof(RANGE));
            rnp[0].min=va_arg(ap,int);
            rnp[0].max=num;
            cl->current=rnp[0].min;
            cl->store=rnp;
            break;
        case Name :
            np=malloc(num*sizeof(NAME));
            for (x=0;x<num;x++)
            {   charp=va_arg(ap,char *);
                if (copy==TRUE)
                {
                    np[x].name=malloc((strlen(charp)+1)*sizeof(char));
                    strcpy(np[x].name,charp);
                } else np[x].name=charp;
            }
            cl->store=np;
            break;
    }
    return cl;
}

int ControlsControlListNext(CONTROLLIST *cl)
{
    RANGE *rnp;
    switch (cl->type)
    {   case Range :
            rnp=(RANGE *)(cl->store);
            if (cl->current<rnp[0].max) {cl->current+=1; return TRUE;}
            return FALSE;
        break;
        default:        
            if (cl->current<cl->size-1) {cl->current+=1; return TRUE;}
            return FALSE;
        break;
    }
}

int ControlsControlListPrev(CONTROLLIST *cl)
{
    RANGE *rnp;
    switch (cl->type)
    {   case Range :
            rnp=(RANGE *)(cl->store);
            if (cl->current>rnp[0].min) {cl->current-=1; return TRUE;}
            return FALSE;
        break;
        default:        
            if (cl->current>0) {cl->current-=1; return TRUE;}
            return FALSE;
        break;
    }
}

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

void DisplayScale(VIEWPORT *vp,int range,int pos,int value,int chars)
// displays the scale in the vp viewport
{
    int x;
    double width=0.8/(double)(range-1), 
            all=0.1+0.8*(double)(pos)/(double)(range-1),
            at=0.1;
    char buffer[10];
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
    sprintf(buffer,"%d",value);
    _grtext_w(all,0.2,buffer);
    for (x=0;x<range;x++)
    {
        _moveto_w(at,0.9); _lineto_w(at,0.5);
        at+=width;                
    }
}

// must change to use a viewport parameter and setcharsize() instead of _w
// for the algorithm to be correct on any sized viewport
void ControlsControlListDisplay(VIEWPORT *vpt,CONTROLLIST *cl,int cols,int rows,int chars)
// displays the control list in the current viewport, cols and chars,
// if relevant, are used to display in cols columns with chars characters
// in each column. If not relevant chars the value must be set to zero, the
// cols must be set to one.
{
    int x,col=0;
    double width=1.0/(double)cols,atx=0.0;
    double height=1.0/(double)rows,aty=0.0;
    double gapx=1.0/((double)(10*cols*(cols/2)));
    double gapy=1.0/((double)(10*rows*rows));
    double text=height/2.0;
    double all;
    char buffer[10];
    BUTTON *bp; VALUE *vp; RADIO *rp; COLOR *cp; NAME *np; RANGE *rnp;

    SetViewport(vpt);
    _setcolor(CONTROLBACK);
    _rectangle_w(_GFILLINTERIOR,0.0,0.0,1.0,1.0);

    switch (cl->type)
    {
        case Value :
            vp=(VALUE *)(cl->store);
            DisplayScale(vpt,cl->size,cl->current,vp[cl->current].value,4);
        break;
        case Range :
            rnp=(RANGE *)(cl->store);
            DisplayScale(vpt,rnp[0].max-rnp[0].min+1,cl->current-rnp[0].min,cl->current,4);
        break;
        case Radio :
            rp=(RADIO *)(cl->store);
            all=width/10.0; // circle section
            _settextalign(_LEFT,_HALF);
            _setcharsize(3*(vpt->y2-vpt->y1)/(rows*4),9*(vpt->x2-vpt->x1)/(10*cols*chars));
            _setcharspacing(0);
            _setcolor(CONTROLTEXT);
            SetViewport(vpt);
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
            SetViewport(vpt);
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
            _setcharsize(3*(vpt->y2-vpt->y1)/(rows*4),(vpt->x2-vpt->x1)/(2*cols*chars));
            _setcharspacing(0);
            _setcolor(CONTROLTEXT);
            SetViewport(vpt);
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
            _setcharsize(2*(vpt->y2-vpt->y1)/(rows*4),(vpt->x2-vpt->x1)/(2*cols*chars));
            _setcharspacing(0);
            SetViewport(vpt);
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

int ControlsDialogGet(DIALOG *dl,int control)
// returns the current of the current control list of dialog dl
{
    return dl->control[control].cl->current;
}

int ControlsDialogDefaultButton(DIALOG *dl)
{
    return dl->control[dl->controls-1].cl->current;
}

int ControlsDialogSet(DIALOG *dl,int control,...)
{
    va_list ap;

    va_start(ap,control);
    switch (dl->control[control].cl->type)
    {
        case Button : case Name :
            return ControlsControlListSetCurrent(dl->control[control].cl,va_arg(ap,char *));
            break;
        case Value : case Color : case Radio : case Range :
            return ControlsControlListSetCurrent(dl->control[control].cl,va_arg(ap,int));
            break;
    }
    va_end(ap);
    return FALSE;
}

CONTROLLIST *ControlsDialogList(DIALOG *dl,int control)
{
    return dl->control[control].cl;
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
    dl->current=controls;
    dl->control[controls].enabled=TRUE;
    dl->control[controls].name=NULL;
    dl->control[controls].cl=ControlsControlListNew(Button,FALSE,3,"OK","Reset","Cancel");
    dl->control[controls].displaycols=3;
    dl->control[controls].displayrows=1;
    dl->control[controls].displaychars=5;
    dl->control[controls].displayfract=fract;

    va_start(ap,controls);
    for (x=0;x<controls;x++)
    {
        dl->control[x].enabled=va_arg(ap,int);
        cp=va_arg(ap,char *);
        if (cp==NULL) dl->control[x].name=NULL;
        else
        {
            dl->control[x].name=malloc((strlen(cp)+1)*sizeof(char));
            strcpy(dl->control[x].name,cp);
        }
        dl->control[x].displaycols=va_arg(ap,int);
        dl->control[x].displayrows=va_arg(ap,int);
        dl->control[x].displaychars=va_arg(ap,int);
        dl->control[x].displayfract=va_arg(ap,double);
        dl->control[x].cl=va_arg(ap,CONTROLLIST *);
    }
    va_end(ap);
    return dl;
}

int ControlsDialogControlNext(DIALOG *dl)
{
    return ControlsControlListNext(dl->control[dl->current].cl);
}

int ControlsDialogNext(DIALOG *dl)
{
    int old=dl->current;
    for (;;)
    {
        dl->current+=1;
        if (dl->current>=dl->controls) // out of range
            {dl->current=old; return FALSE;}
        if ((dl->control[dl->current].enabled==TRUE)
            && (dl->control[dl->current].cl->type!=Name)) // name not selectable
            // ok elem
            return TRUE;
        // else continue
    }
}

int ControlsDialogControlPrev(DIALOG *dl)
{
    return ControlsControlListPrev(dl->control[dl->current].cl);
}

int ControlsDialogPrev(DIALOG *dl)
{
    int old=dl->current;
    for (;;)
    {
        dl->current-=1;
        if (dl->current<0) // out of range
            {dl->current=old; return FALSE;}
        if ((dl->control[dl->current].enabled==TRUE)
            && (dl->control[dl->current].cl->type!=Name)) // name not selectable
            // ok elem
            return TRUE;
        // else continue
    }
}

void ControlsDialogDisplay(VIEWPORT *vp,DIALOG *dl,int gap)
// displays the dialog in the viewport vp
// gap is amount to leave out of each window to separate them, is vertical gap only
{
    int x;
    double fract,at=0.0;
    double height=(double)(vp->y2-vp->y1);
    int names=100;
    int wintop,winbot;
    VIEWPORT i;
    
    SetViewport(vp);
    _setcolor(CONTROLTEXT);
    _rectangle_w(_GFILLINTERIOR,0.0,0.0,1.0,1.0);
    for (x=0;x<dl->controls;x++)
    {
        if (dl->control[x].enabled==TRUE)
        {
            fract=dl->control[x].displayfract;
            wintop=vp->y1+(int)(at*height);
            winbot=vp->y1+(int)((at+fract)*height);
            if (x==dl->current)
            {
                SetViewport(vp);
                _setvieworg(0,0); 
                _setcolor(CONTROLHILITE);
                _rectangle(_GFILLINTERIOR,vp->x1,wintop,vp->x2,winbot);
            }
            if (dl->control[x].name==NULL)
            {
                ResetViewport(&i,vp->x1+gap,wintop+gap,vp->x2-2,winbot-gap);
            }
            else // display control name
            {
                SetViewport(vp);
                _setvieworg(0,0);
                _setcolor(CONTROLBACK);
                _rectangle(_GFILLINTERIOR,vp->x1+gap,wintop+gap,vp->x1+names+gap,winbot-gap);
                _settextalign(_LEFT,_TOP);
                _setcharsize(15,names/6);
                _setcharspacing(-6);
                _setcolor(CONTROLTEXT);
                _grtext(vp->x1+gap+1,wintop+gap,dl->control[x].name);
                ResetViewport(&i,vp->x1+gap+names,wintop+gap,vp->x2-2,winbot-gap);
            }
            ControlsControlListDisplay(&i,dl->control[x].cl,dl->control[x].displaycols,dl->control[x].displayrows,dl->control[x].displaychars);
            at+=fract;
        }
    }
}

int ControlsDialogCall(VIEWPORT *vp,DIALOG *dl,int gap)
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

void ControlsDialogControlEnable(DIALOG *dl,int control,int enable)
{
    dl->control[control].enabled=enable;
}

/*
void main()
{
    VIEWPORT vp={100,0,300,300};

    DIALOG *dl=ControlsDialogNew(0.2,4,
        TRUE,NULL,      1,  1, 15,  0.1,
            ControlsControlListNew(Name,TRUE,1,"Player Setup"),
        TRUE,"Type",    2,  1,  10, 0.1,
            ControlsControlListNew(Radio,FALSE,2,0,"Computer",1,"Human"),
        TRUE,"Color",   10,  1,  0,  0.1,
            ControlsControlListNew(Color,TRUE,10,TRUE,1),
        TRUE,"Skill",   1,  1, 6,  0.2,
            ControlsControlListNew(Range,TRUE,10,1));

// name,displaycols,displaychars,type,controllist,...
    ScreenInit();
    ControlsDialogDisplay(&vp,dl,2);
    ResetViewport(&vp,0,0,100,100);
    ControlsControlListDisplay(&vp,ControlsControlListNew(Range,TRUE,10,1),1,1,4);
    ResetViewport(&vp,100,100,200,200);
    ControlsControlListDisplay(&vp,ControlsControlListNew(Range,TRUE,20,1),1,1,4);

    SwapPage();
    GetKey();
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
