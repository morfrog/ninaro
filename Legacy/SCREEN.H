#ifndef SCREEN_H
#define SCREEN_H

/* color presets */
enum {BLACK=0, RED,    YELLOW,   BLUE,   GREEN,  PURPLE,   ORANGE, WHITE,
       CYAN,    LTRED,  LTYELLOW, LTBLUE, LTGREEN,LTPURPLE, GREY, LTGREY};

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

void SetViewport(VIEWPORT *vp);
void ResetViewport(VIEWPORT *vp,int x1,int y1,int x2,int y2);
void BorderedBox(int x1,int y1,int x2,int y2,int p,int s);
void ScreenInit();
void DrawGrid(int x,int y);
void DrawDisc(int col,int cols,int row,int rows,int color,double discsize);
void Exit_Failure(char *message);
void SwapPage();
int ShutDown( void );

#endif
