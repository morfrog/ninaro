#ifndef VIEWSYS_H
#define VIEWSYS_H

enum {NoDisplay=0,DisplayAll,DisplayChanged};

enum {DISABLE=0,ENABLE=1};

enum {NOCHANGE=0,CHANGED};

enum {INACTIVE=0,ACTIVE=1};

struct viewwinTag
{
    double x1,y1,x2,y2;
};

typedef struct viewwinTag VIEWWIN;

struct viewportTag
{
    int x1,y1,x2,y2;
};

typedef struct viewportTag VIEWPORT;

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
    void (* background)(void); // function which displays the background in 
                               // the current viewport 
    int maxwindows;     // the maximum number of windows
    int windows;        // the number of windows in use
    WINDOW *window;     // storage of the windows;
    int maxkeys;        // the maximum number of keys
    int keys;           // the number of keys in use
    KEYIN *key;         // storage of the keys
};

typedef struct viewsysTag VIEWSYS;

VIEWSYS *ViewSysNew(int x1,int y1,int x2,int y2,int maxwindows,int maxkeys,
                    void (* background)(void));
void ViewSysNewWindow(VIEWSYS *vs,double x1,double y1,double x2,double y2,
                int active,int (* display)(void),char *help);
void ViewSysAddKey(VIEWSYS *vs,int key,int (* func)(void),int enable);
int ViewSysDisplay(VIEWSYS *vs,int mode);
int ViewSysKey(VIEWSYS *vs,int key,int *used);
void ViewSysEnable(VIEWSYS *vs,int enable,int n_key);
void ViewSysEnableAll(VIEWSYS *vs,int enable,int key);
void ViewSysEnableGroup(VIEWSYS *vs,int enable,int num,...);
void ViewSysChanged(VIEWSYS *vs,int window,int changed);
void ViewSysActivate(VIEWSYS *vs,int window,int activity);
char *ViewSysHelp(VIEWSYS *vs,int window);
void ViewSysFree(VIEWSYS *vs);
#endif
