enum {NoSubMenu=255};

struct menuitemTag
{
    char *name;                 // then menu item's name
    int shortcut;               // the shortcut key that runs the function
    void (* function)(void);    // the menu item's function
    byte submenu;               // ==NoSubMenu else = submenu #   
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
    byte maxmenus;      // how many menus are required, including main one
    byte submenus;      // how many submenus - File,Player,Game...
    SUBMENU *submenu;    // array of all submenus, up to 255
};

typedef struct menuTag MENU;

