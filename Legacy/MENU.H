#ifndef MENU_H
#define MENU_H
#include "sys.h"
#include "mempage.h"
#include "screen.h"

#define SEPARATOR NULL
#define MENUBACK GREY
#define MENUHILITE RED
#define MENUTEXT BLACK


enum {ItemDisabled=0,ItemEnabled};

struct menuitemTag
{
    char *name;                 // then menu item's name, can be SEPARATOR
    int shortcut;               // the shortcut key that runs the function
    void (* function)(void);    // the menu item's function
    byte enabled;               // is the menuitem enabled
};

typedef struct menuitemTag MENUITEM;

struct submenuTag
{
    char *menuname;     // The name of the submenu
    byte menuitems;      // up to 256 menuitems
    MENUITEM *menuitem; // array of menuitem
    byte currentitem;   // in range (0..menuitems-1)
};

typedef struct submenuTag SUBMENU;

struct menuTag
{
    byte currentmenu;   // which menu is the current one
    byte nummenus;      // how many menus are required
    SUBMENU *submenu;   // array of all submenus
    VIEWPORT vp;        // the viewport of the menu system
    MEMPAGE *background;// the screen background behind the menu
    SET *validkeys;     // list of the keys which are valid to have
                        // have the menu finish, one chosen is returned
};

typedef struct menuTag MENU;

MENU *MenuNew(byte nummenus,int x1,int y1,int x2,int y2,MEMPAGE *background);
void MenuNewSub(MENU *menu,char *menuname,int items,...);
void MenuDisplay(MENU *menu);
void MenuFree(MENU *menu);
#endif
