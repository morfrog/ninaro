#include <stdlib.h>
#include <graph.h>
#include "sys.h"
#include "screen.h"
#include "binfile.h"
#include "mempage.h"
#include "txtbox.h"
#include "key.h"
#include "help.h"

// #define DEBUG_HELP

HELP *HelpNew(char *filename,KEYGROUP *kg,int lines,int bordercol,int textcol,int boldcol,int maxwidth)
// purpose : to create a new help box
// returns : the help box
{
    char buffer[80]; // can read up to 80 characters
    int left,got,x;
    HELP *help=malloc(sizeof(HELP));
    BINFILE *bf;

    help->ks=KeySysNew(kg);
    KeySysIFunc(help->ks,0,HelpPageUp);
    KeySysIFunc(help->ks,1,HelpPageDown);
    KeySysIFunc(help->ks,2,HelpLineUp);
    KeySysIFunc(help->ks,3,HelpLineDown);
    KeySysIFunc(help->ks,4,HelpCancel);
    KeySysIFunc(help->ks,5,HelpCancel);

    help->tb=TextBoxNew(lines,bordercol,textcol,boldcol,maxwidth);
    if (filename!=NULL)
    {
        bf=BinFileReadFull(filename,0);
        left=BinFileRemain(bf);
#ifdef DEBUG_HELP
        BinFilePrint(bf);
        TextBoxAddLine(help->tb,"This is the top of it",ATEND);
        printf("Start=%d",left);
        GetKey();
#endif
        while (left>0)
        {
            got=BinFileGetTill(bf,79,buffer,13);
            if (got==0) // finished
            {
                left=0;
            }   
            else
            {
                buffer[got]='\0';   // terminate string
                left-=got;
#ifdef DEBUG_HELP
                printf("Left=%d_%s\n",left,buffer); GetKey();
#endif
                TextBoxAddLine(help->tb,buffer,ATEND);
            }
            BinFileGet(bf,1,buffer); // get the line feed character
        }
#ifdef DEBUG_HELP
        TextBoxAddLine(help->tb,"This is the bottom of it",ATEND);
#endif
    }
    return help;
}

void HelpAddLine(HELP *help,char *string)
// purpose : adds a line to the end of the help
// returns : void
{
    TextBoxAddLine(help->tb,string,ATEND);
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
    _setcolor(BLACK);
    _rectangle(_GFILLINTERIOR,vp->x1,vp->y1,vp->x2,vp->y2);
    TextBoxShow(help->tb,vp);
}

void HelpCall(HELP *help,VIEWPORT *vp,MEMPAGE *mp)
// purpose : runs the help system, in the viewport, with the mempage
//           as background
// returns : void
{
    int key,used,funcret,index;
//    TextBoxScrollHome(help->tb);
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
    return TextBoxScroll(help->tb,-1*(TextBoxLines(help->tb)));
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

#ifdef DEBUG_HELP            
void main()
{
    VIEWPORT vp={50,50,600,300};
    KEYGROUP *pkeys=KeyGroupNew(6,KP4,KP6,KP8,KP2,ENTER,BKSP);
    HELP *help=HelpNew("NINARO.HLP",pkeys,10,RED,WHITE,GREEN,60);
    printf("--------------------------\n");
    TextBoxPrint(help->tb);
    GetKey();
    ScreenInit();
    TextBoxShow(help->tb,&vp);
    SwapPage();
    GetKey();
    ShutDown();

}
#endif
