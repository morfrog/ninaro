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

MENU *MenuNew(byte nummenus,int x1,int y1,int x2,int y2,MEMPAGE *background)
// pre : nummenus < 255
// nummenus menus are allocated memory so keep it at the real number
{
    MENU *menu=malloc(sizeof(MENU));
    menu->currentmenu=0;
    menu->nummenus=nummenus;
    menu->submenu=malloc(nummenus*sizeof(SUBMENU));
    menu->vp.x1=x1;
    menu->vp.y1=y1;
    menu->vp.x2=x2;
    menu->vp.y2=y2;
    menu->background=background;
    return menu;
}

// MenuNewSub(menu,
//    "Game",3,
//    "New",CN,New(),ENABLED,
//    SEPARATOR,
//    "Exit",CX,Exit(),ENABLED);

void MenuNewSub(MENU *menu,char *menuname,int items,...)
// ... is char *Name,int Shortcut,void (* Function)(void),byte Enabled,
// Name==SEPARATOR needs no shortcut, function or enabled
{
    va_list ap;
    int n;
    char *name;
    SUBMENU *sm=&(menu->submenu[menu->currentmenu]);
    sm->menuname=malloc((strlen(menuname)+1)*sizeof(char));
    strcpy(sm->menuname,menuname);
    sm->menuitems=items;
    sm->menuitem=malloc(items*sizeof(MENUITEM));
    
    va_start(ap,items);
    for (n=0;n<items;n++)
    {
        name=va_arg(ap,char *);
        if (name==SEPARATOR)
            sm->menuitem[n].name=SEPARATOR;
        else
        {
            sm->menuitem[n].name=malloc((strlen(name)+1)*sizeof(char));
            strcpy(sm->menuitem[n].name,name);
            sm->menuitem[n].shortcut=va_arg(ap,int);
            sm->menuitem[n].function=va_arg(ap,void *);
            sm->menuitem[n].enabled=va_arg(ap,byte);            
        }
    }
    ++(menu->currentmenu);
}

void MenuDisplay(MENU *menu)
{
//    MemPageGet(menu->background);
    _clearscreen(_GCLEARSCREEN);
    _setcharspacing(-0.3);
    _settextalign(_LEFT,_BASE);
    _setcharsize_w(0.1,0.04);
    _setviewport(menu->vp.x1,menu->vp.y1,menu->vp.x2,menu->vp.y2);
// menu bar
    
    _setbkcolor(_GRAY);
    _settextcolor(RED);
    _settextposition(1,1);
    _outtext("Hello");
    _setbkcolor(_BLACK);
    getch();
    Buzz(100,500,20,40);
    SwapPage();
    getch();

    MemPagePut(menu->background);
    SwapPage();
}

void MenuFree(MENU *menu)
{
    int submenu,item;
    for (submenu=0;submenu<menu->nummenus;submenu++)
    {
        free(menu->submenu[submenu].menuname);
        for (item=0;item<menu->submenu[submenu].menuitems;item++)
            if (menu->submenu[submenu].menuitem[item].name!=SEPARATOR)
                free(menu->submenu[submenu].menuitem[item].name);
        free(menu->submenu[submenu].menuitem);
    }
    free(menu->submenu);
    free(menu);
}


