/****************************************************************
Copyright 1998 the author
Module Name    : TXTBOX
Module Purpose : to provide text boxes
Creation Date  : 14 April 1998
Updated        : 1 May 1998         ADDED TxtBoxFixedAddLine
****************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <graph.h>
#include <string.h>
#include "screen.h"
#include "txtbox.h"

TXTBOX *TextBoxNew(int lines,int bordercol,int *textcol,int *boldcol,int maxwidth)
// purpose : creates a new text box, with lines viewable lines, border color
//           bordercol (can be NOBORDER), the color of the text in the box,
//           with maxwidth being the maximum length of a string in a line of
//           the box text boxes are lined 1..
// returns : the text box
{
    TXTBOX *ret=malloc(sizeof(TXTBOX));
    ret->size=0;
    ret->head=NULL;
    ret->start=NULL;
    ret->lines=lines;
    ret->width=maxwidth;
    ret->bordercol=bordercol;
    ret->textcol=textcol;
    ret->boldcol=boldcol;
    return ret;
}

int TextBoxFixedAddLine(TXTBOX *tb,char *string)
// purpose : to add string line to end of txtbox, without allocating more
//           memory (top line is removed)
// returns : TRUE if successfully added, else FALSE
{
    if (tb->size<tb->lines) return TextBoxAddLine(tb,string,ATEND);
    strncpy(tb->head->string,string,tb->width+1);
    tb->head=tb->head->next;
    tb->start=tb->start->next;
    return TRUE;
}

int TextBoxAddLine(TXTBOX *tb,char *string,int afterline)
// purpose : adds a string line to the text box after line afterline, can be
//           ATEND or ATSTART or any number. the string must be terminated
//           by a '/0' character.
// returns : successful addition returns TRUE, else if the number is
//           greater than the current end the line is inserted at the end
//           and FALSE is returned.
{
    int outside=(afterline>tb->size?TRUE:FALSE),x;
    NODE *look;
    NODE *new=malloc(sizeof(NODE));
    new->string=malloc((tb->width+1)*sizeof(char)); // +1 for '\0'
    strncpy(new->string,string,tb->width+1);
    new->string[tb->width]='\0'; // terminate string
    if (tb->size==0) // the box is empty
    {
        new->next=new; new->prev=new;
        tb->head=new; tb->start=new;
        tb->size=1;
        return TRUE;
    }
    else if ((afterline==ATSTART) ||
              (afterline==ATEND) ||
              (outside)) // outside list so insert at end
              look=tb->head->prev;

    else
    {
        look=tb->head;
        for (x=1;x<afterline;x++)
            look=look->next;
    }

    new->next=look->next;           new->prev=look;
    new->prev->next=new;            new->next->prev=new;

    if (afterline==ATSTART) tb->head=new;
    tb->size++;
    if (outside) return FALSE;
    else         return TRUE;
}

int TextBoxDelLine(TXTBOX *tb,int line)
// purpose : line line is deleted from the textbox,
// returns : FALSE is returned if the line is not in the textbox, TRUE is
//           returned upon successful deletion.
// NOTE    : the lines below scroll up to fill the gap. Use TextBoxSetLine
//           to set a line to "" for emptying without scrolling; 
{
    int x;
    NODE *look;
    if (line>tb->size) return FALSE;
    else
    {
        look=tb->head;
        for (x=1;x<line;x++)
            look=look->next;
        if (look==tb->head) tb->head=tb->head->next;
        if (look==tb->start) tb->start=tb->start->next;
        look->next->prev=look->prev;
        look->prev->next=look->next;
        tb->size-=1;
        look->next=NULL;
        look->prev=NULL;
        free(look->string);
        free(look);
        if (tb->size==0)
        {
            tb->head=NULL;
            tb->start=NULL;
        }
        return TRUE;    
    }
}

char *TextBoxLine(TXTBOX *tb,int line)
// purpose : to get the contents of the text box at line line
// returns : if line>the lines in the box NULL is returned else the string
//           is returned
{
    int x;
    NODE *look=tb->head;
    
    if (line>tb->size) return NULL;
    for (x=1;x<line;x++) look=look->next;
    return look->string;
}

int TextBoxSetLine(TXTBOX *tb,int line,char *string)
// purpose : to set the line of the textbox to string
// returns : if the line is in the TXTBOX it is set to string and TRUE is
//           returned else FALSE is returned
{
    int x;
    NODE *look=tb->head;
    if (line>tb->size) return FALSE;
    for (x=1;x<line;x++)
        look=look->next;
    strcpy(look->string,string);
    return TRUE;
}

int TextBoxEmpty(TXTBOX *tb)
// purpose : to empty the contents of the txtbox
// returns : FALSE if was already empty, else TRUE upon successful emptying
{
    // inefficient but it works
    if (tb->size==0) return FALSE;
    while (TextBoxDelLine(tb,1)) {}
    return TRUE;
}

void TextBoxPrint(TXTBOX *tb)
// purpose : to print the contents of the text box
// returns : void
{
    NODE *look=tb->start;
    int x;
    for (x=0;x<tb->lines;x++)
    {
        printf("%s\n",look->string);
        if (look->next==tb->head) // at end
            x=tb->lines;
        else
            look=look->next;
    }
}

void TextBoxDisplay(TXTBOX *tb,VIEWPORT *vp)
{
    int x;
    NODE *look=tb->start;
    char *cp;
    int width=vp->x2-vp->x1+1,height=vp->y2-vp->y1+1,cheight=height/tb->lines;
    int at=vp->y1;
    _setcharspacing(0);
    _settextalign(_LEFT,_TOP);
    _setcharsize(cheight,width/tb->width);

    for (x=0;x<tb->lines;x++)
    {
        if (look->string[0]=='~') {_setcolor(*(tb->boldcol)); cp=look->string+1;}
        else                      {_setcolor(*(tb->textcol)); cp=look->string;}
        _grtext(vp->x1,at,cp);
        if (look->next==tb->head) // at end
            x=tb->lines;
        else
            look=look->next;
        at+=cheight;
    }

    if (tb->bordercol!=NOBORDER) // border is displayed last
    {
        _setcolor(tb->bordercol);
        _rectangle(_GBORDER,vp->x1,vp->y1,vp->x2,vp->y2);
    }

}

/*
void TextBoxDisplay(TXTBOX *tb)
// purpose : draws the contents of the textbox in the current viewport,
//           drawing a border if required by the textbox
// returns : void
{
    int x;
    NODE *look=tb->start;
    char *cp;
    struct xycoord xyc1=_getviewcoord_w(0.0,0.0);
    struct xycoord xyc2=_getviewcoord_w(1.0,1.0);
    int pixels_line_height=(xyc2.ycoord-xyc1.ycoord)/tb->lines;
    int pixels_line_width=xyc2.xcoord-xyc1.xcoord;
    double char_width=(double)pixels_line_height/(double)pixels_line_width;
    _setcharspacing_w(-0.005);
    _setcharsize_w(0.4*1.0/(double)tb->lines,char_width*0.2);
    _settextalign(_LEFT,_BOTTOM);
     
    _setcolor(*(tb->textcol));
    for (x=0;x<tb->lines;x++)
    {
        if (look->string[0]=='~') {_setcolor(*(tb->boldcol)); cp=look->string+1;}
        else                      {_setcolor(*(tb->textcol)); cp=look->string;}
        _grtext_w(0.0,((x+1)*(1.0/((double)tb->lines)))-0.02,cp);
        if (look->next==tb->head) // at end
            x=tb->lines;
        else
            look=look->next;
    }
    if (tb->bordercol!=NOBORDER) // border is displayed last
    {
        _setcolor(tb->bordercol);
        _rectangle_w(_GBORDER,0.0,0.0,1.0,1.0);
    }
}
*/

int TextBoxScrollHome(TXTBOX *tb)
// purpose : scrolls textbox so that TextBoxDisplay shows lines from the top
// returns : TRUE if scrolled, else FALSE
{
    if (tb->start==tb->head) return FALSE;
    tb->start=tb->head;
    return TRUE;
}

int TextBoxScrollEnd(TXTBOX *tb)
// purpose : scrolls textbox so that TextBoxDisplay shows last tb->lines lines
//           of textbox
// returns : TRUE if scrolled, else FALSE
{
    NODE *look=tb->start;
    int x;
    tb->start=tb->head->prev; //last;
    for (x=1;x<tb->lines;x++)
        if (tb->start==tb->head)    
            x=tb->lines;
        else
            tb->start=tb->start->prev;
    if (tb->start==look) return FALSE;
    else return TRUE;
}

int TextBoxScroll(TXTBOX *tb,int amount)
// purpose : scrolls the textbox by amount. amount can be positive (scroll
//           down) or negative (scroll up). can scroll only to top or
//           bottom of box, not outside, amount==0 = scroll to top
// returns : TRUE if scrolled, else FALSE
{
    int left=amount<0?amount*-1:amount; // a positive number
    if (amount<0)   // scroll up
    {
        if (tb->start==tb->head) return FALSE; 
        do
        {
            if (tb->start==tb->head) return TRUE;
            tb->start=tb->start->prev;
            --left;
        } while (left>0);
    }
    else if (amount>0)  // scroll down
    {
        if (tb->start==tb->head->prev) return FALSE; // cant scroll down
        do
        {
            if (tb->start==tb->head->prev) return TRUE; // cant scroll down
            tb->start=tb->start->next;
            --left;
        } while (left>0);
    }
    else // (amount==0)
    {
        tb->start=tb->head;
        return FALSE;
    }

    return TRUE;    
}

int TextBoxScrollLine(TXTBOX *tb,int line)
// purpose : to scroll the textbox to line
// returns : FALSE if line out of range, else TRUE
{
    if ((line>tb->size) || (line<1)) return FALSE;
    TextBoxScrollHome(tb);
    TextBoxScroll(tb,line-1); // no move if line==1
    return TRUE;
}

int TextBoxScrollString(TXTBOX *tb,char *string)
// purpose : to scroll to the next line which contains string, loops over
// returns : TRUE if scrolled else FALSE
{
    NODE *look=tb->start->next;
    while (look!=tb->start)
    {
        if (strstr(look->string,string)!=NULL) // found substring
        {
            tb->start=look;
            return TRUE;
        }
        look=look->next;
    }
    return FALSE;
}

int TextBoxAtTop(TXTBOX *tb)
// purpose : is the textbox at the top
// returns : TRUE if the first displayable line is the first in the box
//           else returns FALSE
{
    return (tb->start==tb->head?TRUE:FALSE);
}

int TextBoxAtBottom(TXTBOX *tb)
// purpose : is the textbox at the bottom
// returns : TRUE if the last displayable line is the first in the box
//           else returns FALSE
{
    return (tb->start==tb->head->prev?TRUE:FALSE);
}

int TextBoxLines(TXTBOX *tb)
// purpose : to get the number of lines in the textbox
// returns : the number
{
    return tb->lines;
}

int TextBoxBorderCol(TXTBOX *tb)
// purpose : to get the textbox's border color
// returns : the color
{
    return tb->bordercol;
}

int TextBoxTextCol(TXTBOX *tb)
// purpose : to get the textbox's text color
// returns : the color
{
    return *(tb->textcol);
}

int TextBoxBoldCol(TXTBOX *tb)
// purpose : to get the textbox's bold color
// returns : the color
{
    return *(tb->boldcol);
}

void TextBoxChangeLines(TXTBOX *tb,int lines)
// purpose : to set the text box's number of viewable lines to lines
// returns : void
{
    tb->lines=lines;
}

void TextBoxChangeBorderCol(TXTBOX *tb,int bordercol)
// purpose : to set the text box's border color to bordercol
// returns : void
{
    tb->bordercol=bordercol;
}

void TextBoxChangeTextCol(TXTBOX *tb,int *textcol)
// purpose : to set the text box's text color to textcol *
// returns : void
{
    tb->textcol=textcol;
}

void TextBoxChangeBoldCol(TXTBOX *tb,int *boldcol)
// purpose : to set the text box's bold color to boldcol *
// returns : void
{
    tb->boldcol=boldcol;
}

void TextBoxFree(TXTBOX *tb)
// purpose : to free the memory allocated to a text box
// returns : void
{
    TextBoxEmpty(tb);
    free(tb);        
}



