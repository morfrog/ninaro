#include <stdlib.h>
#include <graph.h>
#include "sys.h"
#include "screen.h"
#include "binfile.h"
#include "mempage.h"
#include "txtbox.h"
#include "key.h"
#include "help.h"

HELP *HelpNew(char *filename,KEYGROUP *kg,int lines,int bordercol,int textcol,int boldcol,int maxwidth)
// purpose : to create a new help box
// returns : the help box
{
    char buffer[80]; // can read up to 80 characters
    int left,got,x;
    BINFILE *bf=BinFileReadFull(filename,0);
    HELP *help=malloc(sizeof(HELP));
    help->tb=TextBoxNew(lines,bordercol,textcol,boldcol,maxwidth);
    help->ks=KeySysNew(kg);
    KeySysIFunc(help->ks,1,HelpPageUp);
    KeySysIFunc(help->ks,2,HelpPageDown);
    KeySysIFunc(help->ks,2,HelpLineUp);
    KeySysIFunc(help->ks,3,HelpLineDown);
    KeySysIFunc(help->ks,4,HelpCancel);
    KeySysIFunc(help->ks,5,HelpCancel);

    left=30;
    while (left>0)
    {
        got=BinFileGetTill(bf,80,buffer,(char)ENTER);
        buffer[got]='/0';   // terminate string
        TextBoxAddLine(help->tb,buffer,ATEND);
        left-=got;
    }
    return help;
}

void HelpFree(HELP *help)
// purpose : to free the memory allocated to a help box
// returns : void
{
    TextBoxFree(help->tb);
    free(help);
}

void HelpDisplay(HELP *help,VIEWPORT *vp)
// purpose : to display the help box in the viewport
// returns : void
{
    SetViewport(vp);
    _clearscreen(_GVIEWPORT);
    TextBoxShow(help->tb);
}

void HelpCall(HELP *help,VIEWPORT *vp,MEMPAGE *mp)
// purpose : runs the help system, in the viewport, with the mempage
//           as background
// returns : void
{
    int key,used,funcret,index;
    TextBoxScrollHome(help->tb);
    MemPagePut(mp);
    SwapPage();
    MemPagePut(mp);
    HelpDisplay(help,vp);
    SwapPage();
    for (;;)
    {
        key=GetKey();
        used=KeySysKey(help->ks,key,help,&funcret,&index);
        if (!used) // provide extra functionality
        {
            switch (key)
            {
                case ESC :
                case F1 : index=4; used=KeySysIKey(help->ks,index,help,&funcret);
            }
        }
        if (used)
        {
            switch (index)
            {
                case 4 : // cancel
                case 5 : return;
            }
            if (funcret) // something has changed
            {
                HelpDisplay(help,vp);
                SwapPage();
            }
        }
    }
}

int HelpCancel(HELP *help)
// purpose : to end the help box call
// returns : FALSE, as help finished
{
    return FALSE;
}

int HelpLineUp(HELP *help)
// purpose : scrolls the help box up one line
// returns : TRUE if scrolled else FALSE
{
    return TextBoxScroll(help->tb,-1);
}

int HelpLineDown(HELP *help)
// purpose : scrolls the help box down one line
// returns : TRUE if scrolled else FALSE
{
    return TextBoxScroll(help->tb,1);
}

int HelpPageUp(HELP *help)
// purpose : scrolls the help box up one page
// returns : TRUE if scrolled else FALSE
{
    return TextBoxScroll(help->tb,-(TextBoxLines(help->tb)));
}

int HelpPageDown(HELP *help)
// purpose : scrolls the help box down one page
// returns : TRUE if scrolled else FALSE
{
    return TextBoxScroll(help->tb,TextBoxLines(help->tb));
}

void HelpHome(HELP *help)
// purpose : scrolls the help box to the top
// returns : void
{
    TextBoxScrollHome(help->tb);
}

void HelpEnd(HELP *help)
// purpose : scrolls the help box to the end
// returns : void
{
    TextBoxScrollEnd(help->tb);
}
