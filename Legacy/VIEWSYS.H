#ifndef VIEWSYS_H
#define VIEWSYS_H
#include "screen.h"
#include "mempage.h"

enum {NoDisplay=0,DisplayAll,DisplayChanged,DisplayCurrent};

struct windowTag
{
    int active;                     // is the window active
    int changed;                    // if true, update window in display
    VIEWPORT vp;                    // the physical viewport
    void (* display)(void);        // the function which displays the window
    void (* mouse)(int,int,int,int); // the func to run when mouse signal sent,
                                    // params are x,y,width,height
    char *help;                    // single line help string - used when
                                   // cursor over window
};

typedef struct windowTag WINDOW;

struct viewsysTag
{
    VIEWPORT main;      // the main viewport, usually set off the screen res
    int maxwindows;     // the maximum number of windows
    int windows;        // the number of windows in use
    WINDOW *window;     // storage of the windows;
    void (* background)(VIEWPORT *); // function which draws the background
                                    // in the viewport
    MEMPAGE *current;   // the current screen
    int changed;        // how many windows have changed
};
// DisplayChanged sets screen to current then updates changed windows, then
// updates current. It runs the background function for the changed window
// then updates the window, for all windows which have changed
// DisplayAll runs the background function for the main viewport, then
// updates all windows, after which it also updates current to reflect changes.

                                      
typedef struct viewsysTag VIEWSYS;

VIEWSYS *ViewSysNew(int x1,int y1,int x2,int y2,int maxwindows,
        void (* background)(VIEWPORT *));
// purpose : creates a new view system, which can contain up to maxwindows
//           which uses background function for each windows background
// returns : the viewsystem
MEMPAGE *ViewSysCurrent(VIEWSYS *vs);
// purpose : to get the viewsys current view
// returns : the current view of the viewsystem, a MEMPAGE
void ViewSysWindowNew(VIEWSYS *vs,double x1,double y1,double x2,double y2,
                int active,int (* display)(void),
                int (* mouse)(int,int,int,int),char *help);
// purpose : to add a window to the view system
// returns : void
int ViewSysDisplay(VIEWSYS *vs,int mode);
// purpose : to display the viewsys, using mode
//           mode==DisplayAll displays all active windows
//           mode==DisplayChanged displays only windows which have changed
//           mode==NoDisplay makes function have no effect
// returns : TRUE if display updated else FALSE
void ViewSysWindowChanged(VIEWSYS *vs,int window,int changed);
// purpose : to register window from viewsys as changed
// returns : void
void ViewSysWindowActivate(VIEWSYS *vs,int window,int active);
// purpose : to register window from viewsys as active
// returns : void
// NOTE    : only active windows can be displayed or recieve input
char *ViewSysWindowHelp(VIEWSYS *vs,int window);
// purpose : to get the help string associated with window in viewsys
// returns : the string
int ViewSysMouse(VIEWSYS *vs,int x,int y);
// purpose : to send a mouse signal to the first relevant window in the viewsys
// returns : FALSE if no window found at x,y
//           TRUE if a window received the signal and ran its mouse func
//           UNKNOWN if a window received the signal but had no mouse func
void ViewSysFree(VIEWSYS *vs);
// purpose : to free memory allocated to the viewsys
// returns : void

#endif

