#include <stdlib.h>
#include <stdio.h>
#include <graph.h>
#include <string.h>
#include "txtbox.h"

TXTBOX *TextBoxNew(int lines,int bordercol,int textcol,int maxwidth)
// creates a new text box, with lines viewable lines, border color border
// col (can be NOBORDER), the color of the text in the box, with maxwidth
// being the maximum length of a string in a line of the box
// text boxes are lined 1..
{
    TXTBOX *ret=malloc(sizeof(TXTBOX));
    ret->size=0;
    ret->head=NULL;
    ret->start=NULL;
    ret->lines=lines;
    ret->width=maxwidth;
    ret->bordercol=bordercol;
    ret->textcol=textcol;
    return ret;
}

int TextBoxAddLine(TXTBOX *tb,char *string,int afterline)
// adds a string line to the text box after line afterline, can be
// ATEND or ATSTART or any number. successful addition returns TRUE
// else if the number is greater than the current end the line is
// inserted at the end and FALSE is returned.
{
    int outside=(afterline>tb->size?TRUE:FALSE),x;
    NODE *look;
    NODE *new=malloc(sizeof(NODE));
    new->string=malloc((tb->width)*sizeof(char));
    strcpy(new->string,string);
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
// line line is deleted from the textbox, FALSE is returned if the line
//  is not in the textbox, TRUE is returned upon successful deletion.
// NOTE: the lines below scroll up to fill the gap. Use TextBoxSetLine to
//       set a line to "" for emptying without scrolling; 
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
// the contents of the text box at line line are returned. This is a copy
// of the line, not the line itself, so should be freed after use.
// if line>the lines in the box NULL is returned else the string is.
{
    int x;
    NODE *look=tb->head;
    char *ret=malloc((tb->width)*sizeof(char));
    
    if (line>tb->size) return NULL;
    for (x=1;x<line;x++)
        look=look->next;
    strcpy(ret,look->string);
    return ret;
}

int TextBoxSetLine(TXTBOX *tb,int line,char *string)
// if the line is in the TXTBOX it is set to string and TRUE is returned
// else FALSE is returned
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
// empties the contents of the txtbox, returns FALSE if was already empty
// else TRUE upon successful emptying
{
    // inefficient but it works
    if (tb->size==0) return FALSE;
    while (TextBoxDelLine(tb,1)) {}
    return TRUE;
}

void TextBoxPrint(TXTBOX *tb)
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

void TextBoxShow(TXTBOX *tb)
// draws the contents of the textbox in the current viewport, drawing
// a border if required.
{
    int x;
    NODE *look=tb->start;
    struct xycoord xyc1=_getviewcoord_w(0.0,0.0);
    struct xycoord xyc2=_getviewcoord_w(1.0,1.0);
    int pixels_line_height=(xyc2.ycoord-xyc1.ycoord)/tb->lines;
    int pixels_line_width=xyc2.xcoord-xyc1.xcoord;
    double char_width=(double)pixels_line_height/(double)pixels_line_width;
    _setcharspacing(-0.05);
    _setcharsize_w(0.2*1.0/(double)tb->lines,char_width*0.2);
    _settextalign(_LEFT,_BOTTOM);
    if (tb->bordercol!=NOBORDER)
    {
        _setcolor(tb->bordercol);
        _rectangle_w(_GBORDER,0.0,0.0,1.0,1.0);
    }
        
    _setcolor(tb->textcol);
    for (x=0;x<tb->lines;x++)
    {
        _grtext_w(0.0,((x+1)*(1.0/((double)tb->lines)))-0.02,look->string);
        if (look->next==tb->head) // at end
            x=tb->lines;
        else
            look=look->next;
    }
}

int TextBoxScrollEnd(TXTBOX *tb)
// scrolls textbox so that TextBoxShow shows last tb->lines lines of textbox
{
    int x;
    tb->start=tb->head->prev; //last;
    for (x=1;x<tb->lines;x++)
        if (tb->start==tb->head)    
            x=tb->lines;
        else
            tb->start=tb->start->prev;            
    return 1;
}

int TextBoxScroll(TXTBOX *tb,int amount)
// scrolls the textbox by amount. amount can be positive (scroll down)
// or negative (scroll up). can scroll only to top or bottom of box,
// not outside, amount==0 = scroll to top
// returns TRUE if scrolled normally else FALSE if tried to scroll too far
{
    int left=amount<0?amount*-1:amount; // a positive number
    if (amount==0)
    {
        tb->start=tb->head;
        return TRUE;
    }
    if (amount<0)
        do
        {
            if (tb->start==tb->head) return FALSE;
            tb->start=tb->start->prev;
            --left;
        } while (left>0);
    else // amount > 0
        do
        {
            if (tb->start==tb->head->prev) return FALSE;
            tb->start=tb->start->next;
            --left;
        } while (left>0);
    return TRUE;    
}

int TextBoxAtTop(TXTBOX *tb)
// returns TRUE if the first displayable line is the first in the box
// else returns FALSE
{
    return (tb->start==tb->head?TRUE:FALSE);
}

int TextBoxAtBottom(TXTBOX *tb)
// returns TRUE is the last displayable line is the first in the box
// else returns FALSE
{
    return (tb->start==tb->head->prev?TRUE:FALSE);
}

int TextBoxLines(TXTBOX *tb)
// returns the number of viewable lines in the text box
{
    return tb->size;
}

int TextBoxBorderCol(TXTBOX *tb)
{
    return tb->bordercol;
}

int TextBoxTextCol(TXTBOX *tb)
{
    return tb->textcol;
}

void TextBoxChangeLines(TXTBOX *tb,int lines)
// sets the text box's number of viewable lines to lines
{
    tb->lines=lines;
}

void TextBoxChangeBorderCol(TXTBOX *tb,int bordercol)
{
    tb->bordercol=bordercol;
}

void TextBoxChangeTextCol(TXTBOX *tb,int textcol)
{
    tb->textcol=textcol;
}

void TextBoxFree(TXTBOX *tb)
// frees the memory allocated to a text box
{
    TextBoxEmpty(tb);
    free(tb);        
}



