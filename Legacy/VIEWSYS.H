#ifndef VIEWSYS_H

struct viewportTag
{
    double x1,y1,x2,y2;
//    int (* lmb)(double,double);
//    int (* rmb)(double,double);
};

typedef struct viewportTag VIEWPORT;

struct viewsysTag
{
    int size;
    int at;     // 0<=at<=size-1;
    VIEWPORT *views;
};

typedef struct viewsysTag VIEWSYSTEM;

VIEWSYSTEM *ViewSysInit(int size,int x1,int y1,int x2,int y2);
void ViewSysFree(VIEWSYSTEM *vs);
int ViewSysSubLevel(VIEWSYSTEM *vs,double x1,double y1,double x2,double y2);
int ViewSysUpLevel(VIEWSYSTEM *vs);
int ViewSysAcrossLevel(VIEWSYSTEM *vs,double x1,double y1,double x2,double y2);
void ViewSysHome(VIEWSYSTEM *vs);
double ViewSysPixelsH(VIEWSYSTEM *vs,int num);
double ViewSysPixelsW(VIEWSYSTEM *vs,int num);
void ViewSysSetCurrent(VIEWSYSTEM *vs);

#endif
