#ifndef VIEWSYS_H
#define VIEWSYS_H

enum {DisplayAll,DisplayChanged,Key=-1};

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
    void (* lmb)(int,int);        // run when the lmb sent here
    void (* rmb)(int,int);        // run when the rmb sent here
    char *help;                    // single line help string - used when
                                    // cursor over window
};

typedef struct windowTag WINDOW;

struct keyinTag
{
    char key;       // the keypress
    char extend;    // only used if key=0, for extended keys
    int window;     // the number of the window
    int lmb;        // lmb==true send lmb signal to window else send rmb signal
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
};

typedef struct viewsysTag VIEWSYS;

VIEWSYS *ViewSysNew(int x1,int y1,int x2,int y2,int maxwindows,int maxkeys);
void ViewSysNewWindow(VIEWSYS *vs,double x1,double y1,double x2,double y2,
                int active,int (* display)(void),void (* lmb)(int,int),
                void (* rmb)(int,int),char *help);
void ViewSysAddKey(VIEWSYS *vs,char key,char extend,int window,int lmb);
void ViewSysDisplay(VIEWSYS *vs,int mode);
int ViewSysKey(VIEWSYS *vs,char key,char extend);
void ViewSysActivate(VIEWSYS *vs,int window,int activity);
char *ViewSysHelp(VIEWSYS *vs,int window);

#endif
