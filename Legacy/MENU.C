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
    int old=sm->current;
    for (;;)
    {
        sm->current-=1;
        if (sm->current<0) {sm->current=old; return directionbad;}
        if ((sm->item[sm->current].name!=SEPARATOR) &&
            (sm->item[sm->current].enabled!=ItemDisabled))
            return directionok; // acceptable item
        // else keep going
    }
}

int MenuDown(MENUVIEW *mv)
{
    SUBMENU *sm=&(mv->submenu[mv->current]);
    int old=sm->current;
    for (;;)
    {
        sm->current+=1;
        if (sm->current>sm->items-1) {sm->current=old; return directionbad;}
        if ((sm->item[sm->current].name!=SEPARATOR) &&
            (sm->item[sm->current].enabled!=ItemDisabled))
            return directionok; // acceptable item
        // else keep going
    }
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
            if (sm->item[n].enabled==ItemEnabled) sm->current=n;
            // ensures the current is a proper item
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

int MenuSub(MENU *menu) {return menu->view->current;}
int MenuSubs(MENU *menu) {return menu->view->submenus;}
int MenuSubItem(MENU *menu) {return menu->view->submenu[menu->view->current].current;}
int MenuSubItems(MENU *menu) {return menu->view->submenu[menu->view->current].items;}

void MenuDisplayBar(MENU *menu)
// displays the menu bar in the current viewport
{
    int x;
    double pos=0.0;
    double menuwidth=(1.0/(double)MenuSubs(menu));
// the menu bar
    BorderedBox(0.0,0.0,1.0,1.0,MENUBACK,MENUHILITE);
// hilite selected
    _rectangle_w(_GFILLINTERIOR,(double)MenuSub(menu)*menuwidth,0.0,
                                (double)(MenuSub(menu)+1)*menuwidth,1.0);
// write text
    _setcolor(MENUTEXT);
    for (x=0;x<MenuSubs(menu);x++)
    {
        _grtext_w(pos,0.5,menu->view->submenu[x].name);
        pos+=menuwidth;
    }
}

void MenuDisplaySub(SUBMENU *sub)
{
    int x;
    double itemheight=1.0/(double)sub->items;
    double pos=itemheight/2.0;
// the submenu
    BorderedBox(0.0,0.0,1.0,1.0,MENUBACK,MENUHILITE);
// hilite item
    BorderedBox(0.0,itemheight*((double)sub->current),
                1.0,itemheight*((double)(sub->current+1)),
                MENUHILITE,MENUHILITE);

    for (x=0;x<sub->items;x++)
    {
        if (sub->item[x].name==SEPARATOR)
        {
            _setcolor(MENUHILITE);
            _moveto_w(0.0,pos);
            _lineto_w(1.0,pos);
        }
        else
        {
            if (sub->item[x].enabled==ItemDisabled)
                _setcolor(MENUHILITE);
            else
                _setcolor(MENUTEXT);
            _grtext_w(0.0,pos,sub->item[x].name);
        }
        pos+=itemheight;
    }   

}

void MenuDisplay(MENU *menu)
// displays the menu in its current state
{
    int width=menu->view->vp->x2-menu->view->vp->x1+1;
    MENUVIEW *mv=menu->view;

    _setcharspacing_w(-0.01);
    _settextalign(_LEFT,_HALF);
    _setcharsize_w(0.05,0.03);
    MemPagePut(mv->background);
// menu bar
    _setviewport(menu->view->vp->x1,menu->view->vp->y1,
                 menu->view->vp->x2,menu->view->vp->y1+30);
    MenuDisplayBar(menu);
// submenu
    _setviewport(menu->view->vp->x1+(width*MenuSub(menu)/MenuSubs(menu)),
                 menu->view->vp->y1+31,
                 menu->view->vp->x1+(width*(MenuSub(menu)+1)/MenuSubs(menu)),
                 menu->view->vp->y1+31+(MenuSubItems(menu)*30));
    MenuDisplaySub(&(menu->view->submenu[MenuSub(menu)]));
}

int MenuCall(MENU *menu)
// calls the menu
{
    int key,n;

    menu->view->current=0;
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


