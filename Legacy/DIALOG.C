#include <stdlib.h>
#include <conio.h>
#include <stdio.h>
#include <graph.h>
#include <stdarg.h>
#include <string.h>
#include "sys.h"
#include "screen.h"
#include "key.h"
#include "dialog.h"
#include "mempage.h"

void ControlResetDisplay(CONTROL *c,int display,int rows,int cols,int strwidth)
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
//           ControlDisplayReset for these. Can't reset the size of a list.
// returns : TRUE if successful, else FALSE
{
    va_list ap;
    CRANGE *cr; CLIST *cl;
    int x;
    va_start(ap,c);
    switch (c->controltype)
    {
        case Range :
            cr=(CRANGE *)c->store;
            cr->min=va_arg(ap,int);
            strcpy(cr->minname,va_arg(ap,char *));
            cr->max=va_arg(ap,int);
            strcpy(cr->maxname,va_arg(ap,char *));
            break;
        case List :
            cl=(CLIST *)c->store;
            for (x=0;x<cl->elems;x++)
            {
                cl->elem[x].value=va_arg(ap,int);
                cl->elem[x].name=va_arg(ap,char *);
            }
        break;
    }
    va_end(ap);
    return TRUE; // could check and return FALSE but values would have changed
}

CONTROL *ControlNew(int type,int display,int rows,int cols,int strwidth,...)
// list ... is elems,{int value,char *name}*elems           name=name;
// range... is int min,char *minname,int max,int maxname    name=NewString(name);
{
    va_list ap;
    CONTROL *ret=malloc(sizeof(CONTROL));
    CRANGE *rp; CLIST *cl; int x;
    ret->controltype=type;
    ControlResetDisplay(ret,display,rows,cols,strwidth);

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
// purpose : to print the contents of the control
// returns : void
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
            printf("[%d] ",cl->current);
            for (x=0;x<cl->elems;x++)
            {
                printf("<%d.%s>",cl->elem[x].value,cl->elem[x].name);
            }
            printf("\n");
        break;
        case Range :
            cr=(CRANGE *)c->store;
            printf("[%d] ",cr->current);
            printf("Min <%d.%s> Max <%d.%s>\n",cr->min,cr->minname,cr->max,cr->maxname);
        break;
    }
}

int ControlValueGet(CONTROL *c,int pos,int *value)
// purpose : to get the value of the control at position pos.
//           Range pos would be 0 or 1, list 0..size
// returns : if pos invalid FALSE is returned, else TRUE is returned and
//           value set
{
    CLIST *cl; CRANGE *cr;
    switch (c->controltype)
    {
        case List :
            cl=(CLIST *)c->store;
            if ((pos<0) || pos > (cl->elems-1)) return FALSE;
            *value=cl->elem[pos].value;
            return TRUE;
        case Range :
            cr=(CRANGE *)c->store;
            switch (pos)
            {
                case 0 : *value=cr->min; return TRUE;
                case 1 : *value=cr->max; return TRUE;
                default : return FALSE;
            }
    }
    return FALSE;
}

int ControlNameGet(CONTROL *c,int pos,char *name)
// purpose : to get the name of the control at position pos.
//           Range pos would be 0 or 1, list 0..size
// returns : if pos invalid FALSE is returned, else TRUE is returned and
//           name set
{
    CLIST *cl; CRANGE *cr;
    switch (c->controltype)
    {
        case List :
            cl=(CLIST *)c->store;
            if ((pos<0) || pos > (cl->elems-1)) return FALSE;
            name=cl->elem[pos].name;
            return TRUE;
        case Range :
            cr=(CRANGE *)c->store;
            switch (pos)
            {
                case 0 : name=cr->minname; return TRUE;
                case 1 : name=cr->maxname; return TRUE;
                default : return FALSE;
            }
    }
    return FALSE;
}

int MoveTo(int min,int max,int to)
{if ((to<min) || (to>max)) return FALSE; return TRUE;}

int Move(int min,int max,int *current,int howfar)
// purpose : if direction added to current leaves current inside min..max it
// is moved
// returns : TRUE if moved, else FALSE
{
    if (MoveTo(min,max,*current+howfar))
    {
        *current+=howfar;
        return TRUE;
    }
    return FALSE;
}

int ControlMove(CONTROL *c,int howfar)
// purpose : moves howfar in the control
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

int ControlCurrentSet(CONTROL *c,int current)
// purpose : sets the controls current to current
// returns : TRUE if successful else FALSE
{
    CLIST *cl; CRANGE *cr;
    if (c->controltype==List)
    {
        cl=(CLIST *)c->store;
        if (MoveTo(0,cl->elems-1,current))
        {
            cl->current=current; return TRUE;
        }
    }
    else
    {
        cr=(CRANGE *)c->store;
        if (MoveTo(cr->min,cr->max,current))
        {
            cr->current=current; return TRUE;
        }
    }
    return FALSE;
}

int ControlCurrentSetTo(CONTROL *c,int value)
// purpose : sets the controls current to the first in the control which has value
// returns : TRUE if successful else FALSE
{
    CLIST *cl; CRANGE *cr;
    int x;
    if (c->controltype==List)
    {
        cl=(CLIST *)c->store;
        for (x=0;x<cl->elems;x++)
            if (cl->elem[x].value==value)
            {
                cl->current=x;
                return TRUE;                
            }
        return FALSE; // not found
    }
    else
    {
        cr=(CRANGE *)c->store;
        if (MoveTo(cr->min,cr->max,value))
        {
            cr->current=value; return TRUE;
        }
    }
    return FALSE;
}

int ControlCurrentGet(CONTROL *c)
// purpose : gets the controls current
// returns : the controls current
{
    CLIST *cl; CRANGE *cr;
    if (c->controltype==List)
    {
        cl=(CLIST *)c->store; return cl->current;
    }
    else
    {
        cr=(CRANGE *)c->store; return cr->current;
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
{
    int width=vp->x2-vp->x1+1;
    int height=vp->y2-vp->y1+1;
    int edge=width/10;
    int left=vp->x1+edge;
    int section=(width-edge-edge);
    int x,at;

    _setcharsize(height/2,(edge+edge)/chars);
    _settextalign(_CENTER,_HALF);
    _setcolor(CONTROLTEXT);
    _moveto(left,vp->y2); _lineto(vp->x2-edge,vp->y2);
    _moveto(left,vp->y1); _lineto(vp->x2-edge,vp->y1);
    if (range==1)
    {
        at=left+(section/2);
        _moveto(at,vp->y1);
        _lineto(at,vp->y2);
    }
    else
    {
        for (x=0;x<range;x++)
        {
            at=left+((x*section)/(range-1));
            _moveto(at,vp->y1);
            _lineto(at,vp->y2);
        }
        at=left+(pos*section/(range-1));
    }
    BorderedBox(at-edge,vp->y1+(height/4),at+edge,vp->y2-(height/4),CONTROLBACK,CONTROLHILITE);
    _setcolor(CONTROLTEXT);
    _grtext(at,vp->y1+height/2,value);
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
    int height=(vp->y2-vp->y1)/rows;
    VIEWPORT scale;
    int names=FALSE;
    char buffer[10];

    scale.x1=vp->x1;
    scale.y1=vp->y1;
    scale.x2=vp->x2;
    scale.y2=vp->y2-height;

    _setcharsize(height,(vp->x2-vp->x1)/(4*strwidth));
    _setcharspacing(0);
    if ((r->minname[0]!='\0') && (r->maxname[0]!='\0'))
    {
        _settextalign(_LEFT,_TOP);  _grtext(vp->x1,vp->y2-height,r->minname);
        _settextalign(_RIGHT,_TOP); _grtext(vp->x2,vp->y2-height,r->maxname);
        names=TRUE;
    }
    else if (r->minname[0]!='\0')
    {
        _settextalign(_CENTER,_TOP);  _grtext(vp->x1+((vp->x2-vp->x1)/2),vp->y2-height,r->minname);
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
    
    if (cl->elems<elements) elements=cl->elems;

    if (display==Slider)
        DisplayScale(vp,cl->elems,cl->current,cl->elem[cl->current].name,strwidth);
    else
    {
        switch (display)
        {
            case Button :
            case Name :
                _settextalign(_CENTER,_HALF);
                _setcharsize(height/2,width/(strwidth+1));
            break;
            case Color :
                _settextalign(_CENTER,_TOP);
                _setcharsize(height/5,width/(strwidth+1));
            break;
            case Radio :
                _settextalign(_LEFT,_HALF);
                _setcharsize(height/2,(7*width)/(8*(strwidth+1)));
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
                    if (cl->elem[x].name==NULL)
                    {
                        DisplayColor(atx,aty,atx+width,aty+height,
                            cl->elem[x].value,x==cl->current);
                    }
                    else
                    {
                        DisplayColor(atx,aty,atx+width,aty+_3hon4,
                            cl->elem[x].value,x==cl->current);
                        _setcolor(CONTROLTEXT);
                        _grtext(atx+textx,aty+_3hon4,cl->elem[x].name);
                    }
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
// purpose : to graphically display the control in the viewport
// returns : void
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

DIALOG *DialogNew(KEYGROUP *kg,void (* change)(DIALOG *d),char *name,int controls,...)
// purpose : creates a new dialog, ...= {name,enabled,selectable,CONTROL *}
//           dialogs have a default control of buttons, 'OK,Reset,Cancel'
// returns : the dialog
{
    va_list ap;
    DIALOG *ret=malloc(sizeof(DIALOG));
    int x;
    strcpy(ret->name,name);
    ret->elems=controls+1;
    ret->elem=malloc((controls+1)*sizeof(DIALOGELEM));
    ret->state=malloc(ret->elems*sizeof(int));
    ret->change=change;
    va_start(ap,controls);
    for (x=0;x<controls;x++)
    {
        strcpy(ret->elem[x].name,va_arg(ap,char *));
        ret->elem[x].enabled=va_arg(ap,int);
        ret->elem[x].selectable=va_arg(ap,int);
        ret->elem[x].store=va_arg(ap,CONTROL *);
    }
    va_end(ap);
    ret->elem[controls].name[0]='\0'; // no name
    ret->elem[controls].enabled=TRUE;
    ret->elem[controls].selectable=TRUE;
    ret->elem[controls].store=ControlNew(List,Button,1,3,6,3,0,NewString("OK"),
                                                             1,NewString("Reset"),
                                                             2,NewString("Cancel"));
    ret->current=controls; // only one which is definately selectable;
    ret->ks=KeySysNew(kg);
    KeySysIFunc(ret->ks,0,DialogControlPrev); // LEFT
    KeySysIFunc(ret->ks,1,DialogControlNext); // RIGHT
    KeySysIFunc(ret->ks,2,DialogPrev);        // UP
    KeySysIFunc(ret->ks,3,DialogNext);        // DOWN
    KeySysIFunc(ret->ks,4,DialogSelect);        // SELECT
    KeySysIFunc(ret->ks,5,DialogCancel);        // CANCEL
    return ret;
}

int DialogMove(DIALOG *d,int howfar)
// purpose : to move the dialog's current howfar
// returns : FALSE it cant move, else TRUE
{
    
    int was=d->current;                       // save current
    if (MoveTo(0,d->elems-1,d->current+howfar)) // in range
    {
        d->current+=howfar;
        if ((d->elem[d->current].selectable) && (d->elem[d->current].enabled))
            return TRUE;
    }
    else
    {
        d->current=was;
    }
    return FALSE;
}

int DialogPrev(DIALOG *d) {return DialogMove(d,-1);}
// purpose : moves current to the prev in the dialog
// returns : TRUE if successful, else FALSE

int DialogNext(DIALOG *d) {return DialogMove(d,1);}
// purpose : moves current to the next in the dialog
// returns : TRUE if successful, else FALSE

int DialogControlNext(DIALOG *d) {return ControlNext(d->elem[d->current].store);}
// purpose : moves current of the current control in the dialog next
// returns : TRUE if successful, else FALSE

int DialogControlPrev(DIALOG *d) {return ControlPrev(d->elem[d->current].store);}
// purpose : moves current of the current control in the dialog next
// returns : TRUE if successful, else FALSE

int DialogCurrentSet(DIALOG *d,int current)
// purpose : sets the dialog current to current
// returns : TRUE if successful, else FALSE
{
    if (MoveTo(0,d->elems-1,current)) // in range
    {
        if ((d->elem[current].selectable) && (d->elem[current].enabled))
            d->current=current; return TRUE;
    }
    return FALSE;
}

int DialogCurrentGet(DIALOG *d) {return d->current;}
// purpose : to get the current
// returns : the current

int DialogDefaultGet(DIALOG *d)
// purpose : gets which of the default (OK,Reset,Cancel) buttons is selected
// returns : the number of the button
{
    return ControlCurrentGet(d->elem[d->elems-1].store);
}

void DialogDefaultSet(DIALOG *d,int select)
// purpose : sets which of the default (OK,Reset,Cancel) buttons is selected
// returns : void
{
    ControlCurrentSet(d->elem[d->elems-1].store,select);
}

CONTROL *DialogControlGet(DIALOG *d,int control)
// purpose : gets the control from the dialog, for use with control functions
//           assumes 0<=control<=d->elems-1
// returns : the control 
{
    return d->elem[control].store;
}

void DialogNameSet(DIALOG *d,char *string)
// purpose : sets the name of the dialog
// returns : void
{
    strcpy(d->name,string);
}

void DialogKeyGroupSet(DIALOG *d,KEYGROUP *kg)
// purpose : sets the dialogs keygroup
// returns : void
{
    KeySysGroupSet(d->ks,kg);
}

KEYGROUP *DialogKeyGroupGet(DIALOG *d)
// purpose : gets the dialogs keygroup
// returns : the keygroup
{
    return KeySysGroupGet(d->ks);
}

int DialogStateGet(DIALOG *d,int *buffer) 
// purpose : gets the current of each control in the dialog, into the int array buffer
// returns : TRUE if all gotten successfully, else FALSE
{
    int x;
    for (x=0;x<d->elems;x++) buffer[x]=ControlCurrentGet(d->elem[x].store);
    return TRUE;
}

int DialogStateSet(DIALOG *d,int *buffer)
// purpose : sets the current of each control in the dialog, ...{int}*d->elems
// returns : TRUE if all successfully set, else FALSE
{
    int x,ok=TRUE;
    for (x=0;x<d->elems;x++)
    {
        if (ControlCurrentSet(d->elem[x].store,buffer[x])==FALSE)
            ok=FALSE;
    }
    return ok;
}

void DialogDisplay(DIALOG *d,VIEWPORT *vp,MEMPAGE *mp,int namefract,int chars)
// purpose : displays the dialog in the viewport with the mempage background
//           1/namefract is the name of each control, where up to chars characters are displayed
// returns : void
{
    int x,totalrows=1;
    int height=vp->y2-vp->y1+1,width=vp->x2-vp->x1+1;
    int rowheight,names=width/namefract;
    int aty,atx=vp->x1+names;
    VIEWPORT control;

    for (x=0;x<d->elems;x++) totalrows+=d->elem[x].store->rows;
    rowheight=height/totalrows;
    
    MemPagePut(mp);
    _setcolor(CONTROLBACK);               // the back of the dialog
    _rectangle(_GFILLINTERIOR,vp->x1,vp->y1,vp->x2,vp->y2);
    _setcharsize(4*rowheight/5,width/20);
    _setcharspacing(0);
    _settextalign(_CENTER,_HALF);         // the name of the dialog
    _setcolor(CONTROLTEXT);
    _grtext(vp->x1+(width/2),vp->y1+(rowheight/2),d->name);
    _grtext(vp->x1+(width/2)+1,vp->y1+(rowheight/2),d->name);
    _setcharsize(rowheight/2,names/chars);
    _settextalign(_LEFT,_TOP);
    aty=vp->y1+(rowheight);
    for (x=0;x<d->elems;x++) // control names
    {
        _grtext(vp->x1+2,aty+2,d->elem[x].name);
        aty+=(d->elem[x].store->rows*rowheight);
    }

    control.x2=vp->x2;
    aty=vp->y1+(rowheight);
    for (x=0;x<d->elems;x++)
    {
        control.y1=aty+1;
        control.y2=aty+(d->elem[x].store->rows*rowheight)-1;
        if (x==d->current)
        {
            _setcolor(CONTROLHILITE);
            _setlinestyle(0xff00);        // dashed line
            _rectangle(_GBORDER,vp->x1,control.y1,vp->x2,control.y2);
            _setlinestyle(0xffff);
        }
        else
        {
            _setlinestyle(0x5555);      // dotted line
            _setcolor(CONTROLTEXT);
            _rectangle(_GBORDER,vp->x1+1,control.y1,vp->x2-1,control.y2);
            _setlinestyle(0xffff);            
        }
        if (d->elem[x].name[0]=='\0') control.x1=vp->x1;
        else                          control.x1=vp->x1+names;
        control.y1+=1;
        control.y2-=1;
        ControlDisplay(d->elem[x].store,&control);
        aty+=(d->elem[x].store->rows)*rowheight;
    }
}

void DialogPrint(DIALOG *dl)
// purpose : to print the contents of the dialog
// returns : void
{
    int x;
    printf("%s   [%d]\n",dl->name,dl->current);
    for (x=0;x<dl->elems;x++)
    {
        printf("%s [%s] [%s]\n",
             dl->elem[x].name[0]=='\0'?"No Name":dl->elem[x].name,
            (dl->elem[x].enabled)?"Enabled":"Disabled",
            (dl->elem[x].selectable)?"Selectable":"UnSelectable");
        ControlPrint(dl->elem[x].store);
    }
}

int DialogCancel(DIALOG *d)
// purpose : sends a select signal to the dialog
// returns : FALSE, as dialog is finished
{
    DialogStateSet(d,d->state);
    return FALSE;
}

int DialogSelect(DIALOG *d)
// purpose : sends a select signal to the dialog
// returns : TRUE if OK, FALSE if cancel, UNKNOWN if reset
{
    switch (DialogDefaultGet(d))
    {
        case 0 : // OK
            return TRUE;
        case 1 : // Reset
            DialogStateSet(d,d->state);
            if (d->change!=NULL) d->change(d);
            return UNKNOWN;
        case 2 : // Cancel
            return DialogCancel(d);
        break;
    }
    return TRUE; // should never happen
}

int DialogCall(DIALOG *d,VIEWPORT *vp,MEMPAGE *mp,int namefract,int chars)
// purpose : runs a dialog, as is, in the viewport with the mempage as
//           background, running function change whenever a controls current
//           changes, using chars for control names, which are in left
//           1/namefract section of viewport
//           kg keys are left,right,up,down,select,cancel
// returns : FALSE if cancelled, TRUE if OK'd
{
    int index,funcret,changed=FALSE;
    
    DialogStateGet(d,d->state);
    MemPagePut(mp);
    DialogDisplay(d,vp,mp,namefract,chars);
    SwapPage();

    for (;;)
    {
        if (KeySysKey(d->ks,GetKey(),d,&funcret,&index))
        // key was used
        {
            switch (index)
            {
                case 0 : case 1 : // LEFT or RIGHT
                    if (d->change!=NULL) d->change(d);
                    break;
                case 4 : // SELECT
                    if (funcret==TRUE) // dialog finished via ok
                    {
                        return TRUE;
                    }
                    else if (funcret==UNKNOWN) break;    // reset
                case 5 : // CANCEL
                    return FALSE;     // dialog finished via cancel
            }
            if (funcret) // something has changed
            {
                MemPagePut(mp);
                DialogDisplay(d,vp,mp,namefract,chars);
                SwapPage();
            }
        }
    }
}
/*
void main()
{
    char *a=NewString("Hellor");
    DIALOG *dl;
    VIEWPORT main={0,0,400,50};
    VIEWPORT main2={50,50,500,310};
    KEYGROUP *kg=KeyGroupNew(6,KP4,KP6,KP8,KP2,ENTER,ESC);
    
    dl=DialogNew(kg,"Player One Setup",2,
        "Range",TRUE,TRUE,ControlNew(Range,Slider,1,3,10,3,"",27,""),
        "List",TRUE,TRUE,ControlNew(List,Color,1,3,10,3,1,a,2,a,3,a));
    

//    ShutDown(); DialogPrint(dl); GetKey(); exit(0);

    ScreenInit();

//    DialogDisplay(dl,&main2,NULL,8,7);
//    SwapPage();
//    GetKey();
//    DialogNameSet(dl,"Player Two Setup");
//    DialogDisplay(dl,&main2,NULL,8,7);
//    SwapPage();
//    GetKey();

    DialogCall(dl,&main2,NULL,NULL,8,7);
    ShutDown();
}
*/
