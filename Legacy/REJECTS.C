-----------MOUSE-------------------------------------------

MAPPING **map;      // map of the windows in the viewsystem
struct mappingTag
{
    WINDOW *window;
    int x,y;
};

typedef struct mappingTag MAPPING;

void MSwapPage()
{
    static int mpage=0;
    SwapPage();
    SetMousePage(mpage);
    if (mpage==0) mpage=1; else mpage=0;
}

ViewSysNew
    viewsys->map=malloc(xrange*sizeof(MAPPING *));
    printf("Creating Map...\n");
    for (x=0;x<xrange;x++)
    {
        viewsys->map[x]=malloc(yrange*sizeof(MAPPING));
        for (y=0;y<yrange;y++)
        {
            viewsys->map[x][y].window=NULL;
            viewsys->map[x][y].x=x;
            viewsys->map[x][y].y=y;
        }
    }

MAPPING *ViewSysWindow(VIEWSYS *vs,int x,int y)
{
    if (PointInsideViewport(x,y,&(vs->main)))
        return &(vs->map[x-vs->main.x1][y-vs->main.y1]);
    else return NULL;
}

void ViewSysMapWindow(VIEWSYS *vs,int window)
// fits the window from the viewsys into the viewsys' map
{
    int x,y,
        fromx=vs->window[window].vp.x1-vs->main.x1,
        fromy=vs->window[window].vp.y1-vs->main.y1,
        tox=vs->window[window].vp.x2-vs->main.x1,
        toy=vs->window[window].vp.y2-vs->main.y1;
    for (x=fromx;x<tox;x++)
        for (y=fromy;y<toy;y++)
        {
            vs->map[x][y].window=&(vs->window[window]);
            vs->map[x][y].x=x-fromx;
            vs->map[x][y].y=y-fromy;
        }
}

int ViewSysMouse(VIEWSYS *vs,int *mx,int *my)
// returns TRUE if a button signal was sent else returns FALSE
{
    static int cleared=TRUE; // cleared = mouse click can be done
    int x,y,n;
//    MAPPING *mp;
    MouseStatus ms;
//    char temp[100];

    MouseStat(&ms);
    if (!(ms.left || ms.right)) // no mouse buttons down
    {
//        Buzz(100,200,5,20);
        cleared=TRUE; return FALSE;
    }
    // else a button is down
    if (!cleared) // but cant recieve it
        return FALSE;
    else // process button press
        cleared=FALSE;
    PressInfo(&ms);
    Buzz(100,200,5,20);

    // mp=ViewSysWindow(vs,ms.xpos-23592960,ms.ypos-2097152);
/*
    _setactivepage(0);_setvisualpage(0);sprintf(temp,"%d,%d %s",ms.xpos-23592960,ms.ypos-2097152,mp==NULL?"NULL":"!NULL");
    _setviewport(100,100,600,200);_clearscreen(_GVIEWPORT);_grtext_w(0.5,0.5, temp);
    _setactivepage(1);_setvisualpage(0);
*/
    *mx=ms.xpos-1507328;//-23592960;
    *my=ms.ypos-1507328;
    return TRUE;
    if (PointInsideViewport(*mx,*my,&(vs->main)))
        Buzz(500,600,5,100);

//    if (mp==NULL) return FALSE;
        
    if (ms.left)
    {
        for (n=0;n<vs->windows;n++)
            if (PointInsideViewport(ms.xpos-1507328,ms.ypos-1507328,&(vs->window[n].vp)))
            {
                if (vs->window[n].lmb!=NULL)
                    vs->window[n].lmb(0,0);
                return TRUE;
            }
         return FALSE;
//        if (mp->window->lmb!=NULL) mp->window->lmb(mp->x,mp->y);

    }
    if (ms.right)
    {
//        if (mp->window->rmb!=NULL) mp->window->rmb(mp->x,mp->y);
        return TRUE;
        
    }
}

--------------------------------------------------------------------
