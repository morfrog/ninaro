/****************************************************************
Copyright 1998 the author
Module Name    : HELP
Module Purpose : to provide help facilities
Creation Date  : 23 April 1998
Updated        : 24 April 1998  CREATED PageUp,PageDown,End,Home
****************************************************************/
#ifndef HELP_H
#define HELP_H

#include "key.h"
#include "txtbox.h"
#include "mempage.h"

struct helpTag
{
    TXTBOX *tb;
    KEYSYS *ks;
};
typedef struct helpTag HELP;

HELP *HelpNew(char *filename,KEYGROUP *kg,int lines,int bordercol,int *textcol,int *boldcol,int maxwidth);
// purpose : to create a new help box
// returns : the help box
void HelpAddLine(HELP *help,char *string);
// purpose : adds a line to the end of the help
// returns : void
void HelpDisplay(HELP *help,VIEWPORT *vp);
// purpose : to display the help box in the viewport
// returns : void
void HelpCall(HELP *help,VIEWPORT *vp,MEMPAGE *mp);
// purpose : runs the help system, in the viewport, with the mempage
//           as background
// returns : void
int HelpCancel(HELP *help);
// purpose : to end the help box call
// returns : FALSE, as help finished
int HelpLineUp(HELP *help);
// purpose : scrolls the help box up one line
// returns : TRUE if scrolled else FALSE
int HelpLineDown(HELP *help);
// purpose : scrolls the help box down one line
// returns : TRUE if scrolled else FALSE
int HelpPageUp(HELP *help);
// purpose : scrolls the help box up one page
// returns : TRUE if scrolled else FALSE
int HelpPageDown(HELP *help);
// purpose : scrolls the help box down one page
// returns : TRUE if scrolled else FALSE
int HelpHome(HELP *help);
// purpose : scrolls the help box to the top
// returns : void
int HelpEnd(HELP *help);
// purpose : scrolls the help box to the end
// returns : void
int HelpScrollString(HELP *help,char *string);
// purpose : scrolls to the next line in the help which contains string,
//           loops over
// returns : TRUE if scrolled, else FALSE
int HelpScrollLine(HELP *help,int line);
// purpose : to scroll the help to line
// returns : TRUE if scrolled, else FALSE
void HelpFree(HELP *help);
// purpose : to free the memory allocated to a help box
// returns : void

#endif
