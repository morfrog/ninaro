#ifndef TXTBOX_H
#define TXTBOX_H

#include "defines.h"
#include "screen.h"

// lines are addressed as 1..tb->lines, thus enabling the use of ATSTART
// which has a value of 0

struct nodeTag
{
    struct nodeTag *next;
    struct nodeTag *prev;
    char *string;
};

typedef struct nodeTag NODE;

#define NOBORDER -1
#define ATSTART 0
#define ATEND -1

struct txtboxTag    // circular double linked list
{
    int size;       // the size of the list     :INTERNAL:
    NODE *head;     // head of the list         :INTERNAL:
    NODE *start;    // the first viewable line  :INTERNAL:
    int lines;      // the number of viewable lines 
    int width;      // the size in chars of a string in the text box :INTERNAL:
    int bordercol;  // the color of the text box's border, can be NOBORDER
    int textcol;     // the color of the text in the box
    int boldcol; // the color 'bold' text is displayed in.
    // if the first character of a line is ~ the line is displayed in
    // boldtextcol
};

typedef struct txtboxTag TXTBOX;

TXTBOX *TextBoxNew(int lines,int bordercol,int textcol,int boldcol,int maxwidth);
// purpose : creates a new text box, with lines viewable lines, border color
//           bordercol (can be NOBORDER), the color of the text in the box,
//           with maxwidth being the maximum length of a string in a line of
//           the box text boxes are lined 1..
// returns : the text box
int TextBoxAddLine(TXTBOX *tb,char *string,int afterline);
// purpose : adds a string line to the text box after line afterline, can be
//           ATEND or ATSTART or any number. the string must be terminated
//           by a '/0' character.
// returns : successful addition returns TRUE, else if the number is
//           greater than the current end the line is inserted at the end
//           and FALSE is returned.
int TextBoxDelLine(TXTBOX *tb,int line);
// purpose : line line is deleted from the textbox,
// returns : FALSE is returned if the line is not in the textbox, TRUE is
//           returned upon successful deletion.
// NOTE    : the lines below scroll up to fill the gap. Use TextBoxSetLine
//           to set a line to "" for emptying without scrolling; 
char *TextBoxLine(TXTBOX *tb,int line);
// purpose : to get the contents of the text box at line line
// returns : if line>the lines in the box NULL is returned else the string
//           is returned
int TextBoxSetLine(TXTBOX *tb,int line,char *string);
// purpose : to set the line of the textbox to string
// returns : if the line is in the TXTBOX it is set to string and TRUE is
//           returned else FALSE is returned
int TextBoxEmpty(TXTBOX *tb);
// purpose : to empty the contents of the txtbox
// returns : FALSE if was already empty, else TRUE upon successful emptying
void TextBoxPrint(TXTBOX *tb);
// purpose : to print the contents of the text box
// returns : void
void TextBoxShow(TXTBOX *tb,VIEWPORT *vp);
// purpose : draws the contents of the textbox in the current viewport,
//           drawing a border if required by the textbox
// returns : void
void TextBoxScrollHome(TXTBOX *tb);
void TextBoxScrollEnd(TXTBOX *tb);
// purpose : scrolls textbox so that TextBoxShow shows last tb->lines lines
//           of textbox
// returns : void
int TextBoxScroll(TXTBOX *tb,int amount);
// purpose : scrolls the textbox by amount. amount can be positive (scroll
//           down) or negative (scroll up). can scroll only to top or
//           bottom of box, not outside, amount==0 = scroll to top
// returns : TRUE if scrolled, else FALSE
int TextBoxAtTop(TXTBOX *tb);
// purpose : is the textbox at the top
// returns : TRUE if the first displayable line is the first in the box
//           else returns FALSE
int TextBoxAtBottom(TXTBOX *tb);
// purpose : is the textbox at the bottom
// returns : TRUE if the last displayable line is the first in the box
//           else returns FALSE
int TextBoxLines(TXTBOX *tb);
// purpose : to get the number of lines in the textbox
// returns : the number
int TextBoxBorderCol(TXTBOX *tb);
// purpose : to get the textbox's border color
// returns : the color
int TextBoxTextCol(TXTBOX *tb);
// purpose : to get the textbox's text color
// returns : the color
int TextBoxBoldCol(TXTBOX *tb);
// purpose : to get the textbox's bold color
// returns : the color
void TextBoxChangeLines(TXTBOX *tb,int lines);
// purpose : to set the text box's number of viewable lines to lines
// returns : void
void TextBoxChangeBorderCol(TXTBOX *tb,int bordercol);
// purpose : to set the text box's border color to bordercol
// returns : void
void TextBoxChangeTextCol(TXTBOX *tb,int textcol);
// purpose : to set the text box's text color to textcol
// returns : void
void TextBoxChangeBoldCol(TXTBOX *tb,int boldcol);
// purpose : to set the text box's bold color to boldcol
// returns : void
void TextBoxFree(TXTBOX *tb);
// purpose : to free the memory allocated to a text box
// returns : void

#endif
