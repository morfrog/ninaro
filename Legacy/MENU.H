#ifndef MENU_H
#define MENU_H
#include "sys.h"
#include "set.h"
#include "mempage.h"
#include "screen.h"

#define SEPARATOR NULL
#define MENUBACK GREY
#define MENUHILITE RED
#define MENUTEXT BLACK

#define NONE -1

enum {ItemDisabled=0,ItemEnabled};

enum {left=0,right,up,down,select,end};

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
    char *name;     // The name of the submenu
    byte items;      // up to 256 menuitems
    MENUITEM *item; // array of menuitem
    byte current;   // in range (0..menuitems-1)
};

typedef struct submenuTag SUBMENU;

struct menuviewTag
{
    byte current;   // which menu is the current one
    byte submenus;      // how many menus are required
    SUBMENU *submenu;   // array of all submenus
    VIEWPORT *vp;        // the viewport of the menu system
    MEMPAGE *background;// the screen background behind the menu
};

typedef struct menuviewTag MENUVIEW;

struct menukeyTag
{
    int key;
    int (* func)(MENUVIEW *);
};

typedef struct menukeyTag MENUKEY;

struct menuTag
{
    MENUVIEW *view;
    MENUKEY key[6];     // the keys and associated functions
};

typedef struct menuTag MENU;

MENU *MenuNew(byte nummenus,VIEWPORT *vp,MEMPAGE *background);
void MenuNewSub(MENU *menu,char *menuname,int items,...);
int MenuCall(MENU *menu,MEMPAGE *mp);
void MenuKeysSet(MENU *menu,int l,int r,int u,int d,int s,int e);
void MenuFree(MENU *menu);
#endif
