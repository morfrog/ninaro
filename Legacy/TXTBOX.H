#ifndef TXTBOX_H
#define TXTBOX_H

// lines are addressed as 1..tb->lines, thus enabling the use of ATSTART
// which has a value of 0

struct nodeTag
{
    struct nodeTag *next;
    struct nodeTag *prev;
    char *string;
};

typedef struct nodeTag NODE;

#define TRUE 1
#define FALSE 0
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
};

typedef struct txtboxTag TXTBOX;

TXTBOX *TextBoxNew(int lines,int bordercol,int textcol,int maxwidth);
void TextBoxFree(TXTBOX *tb);
int TextBoxAddLine(TXTBOX *tb,char *string,int afterline);
int TextBoxDelLine(TXTBOX *tb,int line);
char *TextBoxLine(TXTBOX *tb,int line);
int TextBoxSetLine(TXTBOX *tb,int line,char *string);
int TextBoxEmpty(TXTBOX *tb);
void TextBoxPrint(TXTBOX *tb);
void TextBoxShow(TXTBOX *tb);
int TextBoxScrollEnd(TXTBOX *tb);
int TextBoxScroll(TXTBOX *tb,int amount);
int TextBoxAtTop(TXTBOX *tb);
int TextBoxAtBottom(TXTBOX *tb);
int TextBoxLines(TXTBOX *tb);
int TextBoxBorderCol(TXTBOX *tb);
int TextBoxTextCol(TXTBOX *tb);
void TextBoxChangeLines(TXTBOX *tb,int lines);
void TextBoxChangeBorderCol(TXTBOX *tb,int bordercol);
void TextBoxChangeTextCol(TXTBOX *tb,int textcol);
void TextBoxFree(TXTBOX *tb);

#endif
