#ifndef SCREEN_H
#define SCREEN_H

/* color presets */
#define BLACK 0
#define BLUE 1
#define BLACK2 2
#define BLUE2 3
#define RED 4
#define PURPLE 5
#define ORANGE 6
#define WHITE 7
#define GREY 8
#define LTBLUE 9
#define GREEN 10
#define CYAN 11
#define LTRED 12
#define PINK 13
#define YELLOW 14
#define LTWHITE 15

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
void ScreenInit();
void DrawGrid(int x,int y);
void DrawDisc(int col,int cols,int row,int rows,int color,double discsize);
void Exit_Failure(char *message);
void SwapPage();
int ShutDown( void );

#endif
