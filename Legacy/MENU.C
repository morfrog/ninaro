// ESC brings up the menu, could also use Ctrl-menuletter if no conflicts
// if a menu item's name is NULL then that menuitem is a separator
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <graph.h>
#include <conio.h>
#include "sys.h"
#include "sound.h"
#include "screen.h"
#include "menu.h"

enum {menuend=-2,selected,directionbad,directionok};

int MenuLeft(MENUVIEW *mv)
{
    if (mv->current>0)
    {
        mv->current-=1; return directionok;
    }
    return directionbad;
}

int MenuRight(MENUVIEW *mv)
{
    if (mv->current<mv->submenus-1)
    {
        mv->current+=1; return directionok;
    }
    return directionbad;
}

int MenuUp(MENUVIEW *mv)
{
    SUBMENU *sm=&(mv->submenu[mv->current]);
    if (sm->current>0)
    {
        do {sm->current-=1;}
        while (sm->item[sm->current].name==SEPARATOR);
        return directionok;
    }
    return directionbad;
}

int MenuDown(MENUVIEW *mv)
{
    SUBMENU *sm=&(mv->submenu[mv->current]);
    if (sm->current<sm->items-1)
    {
        do {sm->current+=1;}
        while (sm->item[sm->current].name==SEPARATOR);
        return directionok;
    }
    return directionbad;
}

int MenuSelect(MENUVIEW *mv)
{
    mv->submenu[mv->current].item[mv->submenu[mv->current].current].function();
    return selected;
}

int MenuEnd(MENUVIEW *mv)
{
    return menuend;
}

MENU *MenuNew(byte nummenus,VIEWPORT *vp,MEMPAGE *background)
// pre : nummenus < 255
// nummenus menus are allocated memory so keep it at the real number
{
    MENU *menu=malloc(sizeof(MENU));
    menu->view=malloc(sizeof(MENUVIEW));
    menu->view->current=0;
    menu->view->submenus=nummenus;
    menu->view->submenu=malloc(nummenus*sizeof(SUBMENU));
    menu->view->vp=vp;
    menu->view->background=background;
    menu->key[left].func=MenuLeft;
    menu->key[right].func=MenuRight;
    menu->key[up].func=MenuUp;
    menu->key[down].func=MenuDown;
    menu->key[select].func=MenuSelect;
    menu->key[end].func=MenuEnd;
    return menu;
}

// MenuNewSub(menu,
//    "Game",3,
//    "New",CN,New(),ENABLED,
//    SEPARATOR,
//    "Exit",CX,Exit(),ENABLED);

// possibly add shortcut string, CN's might be "CTRL-N"
void MenuNewSub(MENU *menu,char *menuname,int items,...)
// ... is char *Name,int Shortcut,void (* Function)(void),byte Enabled,
// Name==SEPARATOR needs no shortcut, function or enabled
{
    va_list ap;
    int n;
    char *name;
    SUBMENU *sm=&(menu->view->submenu[menu->view->current]);
    sm->name=malloc((strlen(menuname)+1)*sizeof(char));
    strcpy(sm->name,menuname);
    sm->items=items;
    sm->item=malloc(items*sizeof(MENUITEM));
    sm->current=0;
    
    va_start(ap,items);
    for (n=0;n<items;n++)
    {
        name=va_arg(ap,char *);
        if (name==SEPARATOR)
            sm->item[n].name=SEPARATOR;
        else
        {
            sm->item[n].name=malloc((strlen(name)+1)*sizeof(char));
            strcpy(sm->item[n].name,name);
            sm->item[n].shortcut=va_arg(ap,int);
            sm->item[n].function=va_arg(ap,void *);
            sm->item[n].enabled=va_arg(ap,byte);            
        }
    }
    ++(menu->view->current);
}

void MenuKeysSet(MENU *menu,int l,int r,int u,int d,int s,int e)
{
    menu->key[left].key=l;
    menu->key[right].key=r;
    menu->key[up].key=u;
    menu->key[down].key=d;
    menu->key[select].key=s;
    menu->key[end].key=e;
}

void BorderedBox(double x1,double y1,double x2,double y2,int p,int s)
{
    _setcolor(p);
    _rectangle_w(_GFILLINTERIOR,x1,y1,x2,y2);
    _setcolor(s);
    _rectangle_w(_GBORDER,x1,y1,x2,y2);
}

int MenuSub(MENU *menu) {return menu->view->current;}
int MenuSubs(MENU *menu) {return menu->view->submenus;}
int MenuSubItem(MENU *menu) {return menu->view->submenu[menu->view->current].current;}
int MenuSubItems(MENU *menu) {return menu->view->submenu[menu->view->current].items;}

void MenuDisplay(MENU *menu)
// displays the menu in its current state
{
    int x;
    double pos=0.0;
    MENUVIEW *mv=menu->view;
    double menuwidth=(1.0/(double)MenuSubs(menu));
    double menuheight=MenuSubItems(menu)*0.1;

    _setcharspacing(-0.3);
    _settextalign(_LEFT,_HALF);
    _setcharsize_w(0.05,0.03);
    SetViewport(menu->view->vp);
    _clearscreen(_GCLEARSCREEN);
    MemPagePut(mv->background);
// menu bar
    BorderedBox(0.0,0.0,1.0,0.05,MENUBACK,MENUHILITE);
    _rectangle_w(_GFILLINTERIOR,(double)MenuSub(menu)*menuwidth,0.0,
                                (double)(MenuSub(menu)+1)*menuwidth,0.05);
    _setcolor(MENUTEXT);
    for (x=0;x<mv->submenus;x++)
    {
        _grtext_w(pos,0.025,mv->submenu[x].name);
        pos+=menuwidth;
    }
// submenu
    BorderedBox((double)(MenuSub(menu))*menuwidth,0.05,
                (double)(MenuSub(menu)+1)*menuwidth,
                0.05+menuheight,MENUBACK,MENUHILITE);

// hilite item
    BorderedBox((double)(MenuSub(menu))*menuwidth,
                0.05+menuheight*((double)MenuSubItem(menu)/(double)MenuSubItems(menu)),
                (double)(MenuSub(menu)+1)*menuwidth,
                0.05+menuheight*((double)(MenuSubItem(menu)+1)/(double)MenuSubItems(menu)),
                MENUHILITE,MENUHILITE);

    pos=0.05+0.05;
    _setcolor(MENUTEXT);
    for (x=0;x<MenuSubItems(menu);x++)
    {
        if (mv->submenu[MenuSub(menu)].item[x].name==SEPARATOR)
        {
            _setcolor(MENUHILITE);
            _moveto((double)(MenuSub(menu))*menuwidth,pos);
            _lineto((double)(MenuSub(menu)+1)*menuwidth,pos);
            _setcolor(MENUTEXT);    
        }
        else
        {
            _grtext_w((double)(MenuSub(menu))*menuwidth,pos,
                      mv->submenu[MenuSub(menu)].item[x].name);
        }
        pos+=0.1;
    }   

}

int MenuCall(MENU *menu,MEMPAGE *mp)
// calls the menu
{
    int key,n;

    menu->view->current=0;
    menu->view->background=mp;
    MenuDisplay(menu);
    SwapPage();

    for (;;)
    {
        if (kbhit())
        {
            key=GetKey();
            for (n=0;n<6;n++)
                if (key==menu->key[n].key)
                    switch (menu->key[n].func(menu->view))
                    {
                        case menuend : return FALSE; break;
                        case selected : return TRUE; break;
                        case directionok:
                            MenuDisplay(menu);
                            SwapPage();
                            break;
                    }
        }
    }
}

void MenuFree(MENU *menu)
{
    int submenu,item;
    for (submenu=0;submenu<menu->view->submenus;submenu++)
    {
        free(menu->view->submenu[submenu].name);
        for (item=0;item<menu->view->submenu[submenu].items;item++)
            if (menu->view->submenu[submenu].item[item].name!=SEPARATOR)
                free(menu->view->submenu[submenu].item[item].name);
        free(menu->view->submenu[submenu].item);
    }
    free(menu->view->submenu);
    free(menu);
}


