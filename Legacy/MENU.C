// ESC brings up the menu, could also use Ctrl-menuletter if no conflicts
// if a menu item's name is NULL then that menuitem is a separator
#include <stdlib.h>
#include <string.h>
#include "sys.h"
#include "viewsys.h"

MENU *MenuNew(byte maxmenus)
// pre : maxmenus < 255
// maxmenus menus are allocated memory so keep it at the real max
{
    MENU *menu=malloc(sizeof(MENU));
    menu->maxmenus=maxmenus;
    menu->submenus=0;
    menu->submenu=malloc(maxmenus*sizeof(SUBMENU));
    return menu;
}

void MenuAddMenuItem(MENU *menu,byte submenu,
    char *name,int shortcut,void (* function)(void))
{
    strcpy(menu->submenu[submenu].name,name);
    menu->submenu[submenu].shortcut=shortcut;
    menu->submenu[submenu].function=function;
    menu->submenu[submenu].submenu=NoSubMenu;
}

void MenuAddSubMenu(MENU *menu,byte offmenu,byte offitem,byte submenu)
// Adds a submenu to the menu, off offitem in offmenu.
{
    menu->submenu[offmenu]
}
