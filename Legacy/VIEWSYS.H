#ifndef VIEWSYS_H
#define VIEWSYS_H
#include "screen.h"
#include "mempage.h"

enum {NoDisplay=0,DisplayAll,DisplayChanged};

enum {DISABLE=0,ENABLE=1};

enum {NOCHANGE=0,CHANGED};

enum {INACTIVE=0,ACTIVE=1};

struct windowTag
{
    int active;                     // is the window active
    int changed;                    // if true, update window in display
    VIEWPORT vp;                    // the physical viewport
    void (* display)(void);        // the function which displays the window
    char *help;                    // single line help string - used when
                                   // cursor over window
};

typedef struct windowTag WINDOW;

struct keyinTag
{
    int key;       // the keypress - key enum in sys.h
    int enable;
    int (* keyfunc)(void); // the function associated with the key
};

typedef struct keyinTag KEYIN;

struct viewsysTag
{
    VIEWPORT main;      // the main viewport, usually set off the screen res
    int maxwindows;     // the maximum number of windows
    int windows;        // the number of windows in use
    WINDOW *window;     // storage of the windows;
    int maxkeys;        // the maximum number of keys
    int keys;           // the number of keys in use
    KEYIN *key;         // storage of the keys
    void (* background)(VIEWPORT *); // function which draws the background
                                    // in the viewport
    MEMPAGE *current;   // the current screen
};
// DisplayChanged sets screen to current then updates changed windows, then
// updates current. It runs the background function for the changed window
// then updates the window.
// DisplayAll runs the background function for the main viewport, then
// updates all windows, after which it also updates current to reflect changes.

                                      
typedef struct viewsysTag VIEWSYS;

void ViewSysClear(VIEWPORT *vp);
MEMPAGE *ViewSysCurrent(VIEWSYS *vs);
VIEWSYS *ViewSysNew(int x1,int y1,int x2,int y2,int maxwindows,int maxkeys,
        void (* background)(VIEWPORT *));
void ViewSysWindowNew(VIEWSYS *vs,double x1,double y1,double x2,double y2,
                int active,int (* display)(void),char *help);
void ViewSysWindowChanged(VIEWSYS *vs,int window,int changed);
void ViewSysWindowActivate(VIEWSYS *vs,int window,int activity);
char *ViewSysWindowHelp(VIEWSYS *vs,int window);
int ViewSysDisplay(VIEWSYS *vs,int mode);
int ViewSysKey(VIEWSYS *vs,int key,int *used);
void ViewSysKeyAdd(VIEWSYS *vs,int key,int (* func)(void),int enable);
void ViewSysKeyEnable(VIEWSYS *vs,int enable,int n_key);
void ViewSysKeyEnableAll(VIEWSYS *vs,int enable,int key);
void ViewSysKeyEnableGroup(VIEWSYS *vs,int enable,int num,...);
void ViewSysFree(VIEWSYS *vs);
#endif

