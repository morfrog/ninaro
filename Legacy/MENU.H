#ifndef MENU_H
#define MENU_H
#include "sys.h"
#include "mempage.h"
#include "key.h"
#include "screen.h"

#define SEPARATOR NULL
#define MENUBACK LTGREY
#define MENUHILITE RED
#define MENUTEXT BLACK

#define NONE -1

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
    short current;   // in range (0..menuitems-1)
};

typedef struct submenuTag SUBMENU;

struct menuTag
{
    byte current;   // which menu is the current one
    byte submenus;      // how many menus are required
    SUBMENU *submenu;   // array of all submenus
    VIEWPORT *vp;        // the viewport of the menu system
    MEMPAGE *background;// the screen background behind the menu
    KEYSYS *ks;         // the menus key system
};
typedef struct menuTag MENU;

int MenuLeft(MENU *m);
// purpose : sets the current menu as the one to the left of the current
// returns : TRUE if moved else FALSE
int MenuRight(MENU *m);
// purpose : sets the current menu as the one to the right of the current
// returns : TRUE if moved else FALSE
int MenuUp(MENU *m);
// purpose : sets the current item of the current menu as the one above it
// returns : TRUE if moved else FALSE
int MenuDown(MENU *m);
// purpose : sets the current item of the current menu as the one below it
// returns : TRUE if moved else FALSE
int MenuSelect(MENU *m);
// purpose : sends a select signal to the menu
// returns : TRUE
int MenuCancel(MENU *m);
// purpose : sends a cancel signal to the menu
// returns : TRUE 
MENU *MenuNew(byte nummenus,VIEWPORT *vp,MEMPAGE *background,KEYGROUP *kg);
// purpose : to create a new menu, nummenus menus are allocated memory
//           so keep it at the real number,
// returns : the menu
void MenuNewSub(MENU *menu,char *menuname,int items,...);
// purpose : to initialise a submenu for menu.
//           ... is char *Name,int Shortcut,void (* Function)(void),byte Enabled,
//           Name==SEPARATOR needs no shortcut, function or enabled
// returns : void
KEYGROUP *MenuKeyGroupGet(MENU *menu);
// purpose : gets the menus keygroup
// returns : the keygroup
void MenuKeyGroupSet(MENU *menu,KEYGROUP *kg);
// purpose : sets the dialogs keygroup
// returns : void
int MenuSub(MENU *menu);
// purpose : to get the index of the current submenu
// returns : the index 
int MenuSubs(MENU *menu);
// purpose : to get the number of submenus in the menu
// returns : the number 
int MenuSubItem(MENU *menu);
// purpose : to get the index of the current item in the current submenu
// returns : the index 
int MenuSubItems(MENU *menu);
// purpose : to get the number of items in the current submenu
// returns : the number
void MenuDisplayBar(MENU *menu,VIEWPORT *vp,int dropped);
// purpose : displays the menu bar in the current viewport
//           if dropped the menu name is not hilited
// returns : void
void MenuDisplaySub(SUBMENU *sub,VIEWPORT *vp);
// purpose : displays the submenu in the viewport
// returns : void
void MenuDisplay(MENU *menu);
// purpose : displays the menu in its current state
// returns : void
void MenuCall(MENU *menu,void (* help)(void));
// purpose : calls the menu
// returns : void
int MenuItemEnable(MENU *menu,int sub,int item,int enable);
// purpose : to set the enable state of an item in the submenu in the menu
// returns : TRUE if successfully set else FALSE
int MenuSubCurrentSet(MENU *menu,int sub,int current);
// purpose : to set the current in the submenu
// returns : TRUE if set else FALSE
void MenuFree(MENU *menu);
// purpose : to free the memory allocated to the menu
// returns : void

#endif
