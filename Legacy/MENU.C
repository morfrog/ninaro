// ESC brings up the menu, could also use Ctrl-menuletter if no conflicts
// if a menu item's name is NULL then that menuitem is a separator
#include "sys.h"
#include "viewsys.h"

struct menuitemTag
{
    char *name;                 // then menu item's name
    int shortcut;               // the shortcut key that runs the function
    void (* function)(void);    // the menu item's function
};

typedef struct menuitemTag MENUITEM;

struct submenuTag
{
    byte menitems;      // up to 256 menuitems
    MENUITEM *menuitem; // array of menuitem
    byte currentitem;   // in range (0..menuitems-1)
};

typedef struct submenuTag SUBMENU;

struct menuTag
{
    byte submenus;      // how many submenus - File,Player,Game...
    SUBMENU submenu;    // functions are probably NULL
};


