// ESC brings up the menu, could also use Ctrl-menuletter if no conflicts
// if a menu item's name is NULL then that menuitem is a separator
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <graph.h>
#include <conio.h>
#include "defines.h"
#include "sound.h"
#include "screen.h"
#include "key.h"
#include "menu.h"

int MenuLeft(MENU *m)
// purpose : sets the current menu as the one to the left of the current
// returns : TRUE if moved else FALSE
{
    if (m->current>0) {m->current-=1; return TRUE;}
    return FALSE;
}

int MenuRight(MENU *m)
// purpose : sets the current menu as the one to the right of the current
// returns : TRUE if moved else FALSE
{
    if (m->current<m->submenus-1) {m->current+=1; return TRUE;}
    return FALSE;
}

int MenuUp(MENU *m)
// purpose : sets the current item of the current menu as the one above it
// returns : TRUE if moved else FALSE
{
    SUBMENU *sm=&(m->submenu[m->current]);
    int old=sm->current;
    for (;;)
    {
        sm->current-=1;
        if (sm->current<0) {sm->current=old; return FALSE;}
        if ((sm->item[sm->current].name!=SEPARATOR) &&
            (sm->item[sm->current].enabled==TRUE))
            return TRUE; // acceptable item
        // else keep going
    }
}

int MenuDown(MENU *m)
// purpose : sets the current item of the current menu as the one below it
// returns : TRUE if moved else FALSE
{
    SUBMENU *sm=&(m->submenu[m->current]);
    int old=sm->current;
    for (;;)
    {
        sm->current+=1;
        if (sm->current>sm->items-1) {sm->current=old; return FALSE;}
        if ((sm->item[sm->current].name!=SEPARATOR) &&
            (sm->item[sm->current].enabled==TRUE))
            return TRUE; // acceptable item
        // else keep going
    }
}

int MenuSelect(MENU *m)
// purpose : sends a select signal to the menu
// returns : TRUE
{
    m->submenu[m->current].item[m->submenu[m->current].current].function();
    return TRUE;
}

int MenuCancel(MENU *m)
// purpose : sends a cancel signal to the menu
// returns : TRUE 
{
    return TRUE;
}

MENU *MenuNew(byte nummenus,VIEWPORT *vp,MEMPAGE *background,KEYGROUP *kg)
// purpose : to create a new menu, nummenus menus are allocated memory
//           so keep it at the real number,
// returns : the menu
{
    MENU *menu=malloc(sizeof(MENU));
    menu->current=0;
    menu->submenus=nummenus;
    menu->submenu=malloc(nummenus*sizeof(SUBMENU));
    menu->vp=vp;
    menu->background=background;
    menu->ks=KeySysNew(kg);
    KeySysIFunc(menu->ks,0,MenuLeft);   // LEFT
    KeySysIFunc(menu->ks,1,MenuRight);  // RIGHT
    KeySysIFunc(menu->ks,2,MenuUp);     // UP
    KeySysIFunc(menu->ks,3,MenuDown);   // DOWN
    KeySysIFunc(menu->ks,4,MenuSelect); // SELECT
    KeySysIFunc(menu->ks,5,MenuCancel); // CANCEL
    return menu;
}

// MenuNewSub(menu,
//    "Game",3,
//    "New",CN,New(),ENABLED,
//    SEPARATOR,
//    "Exit",CX,Exit(),ENABLED);

// possibly add shortcut string, CN's might be "CTRL-N"
void MenuNewSub(MENU *menu,char *menuname,int items,...)
// purpose : to initialise a submenu for menu.
//           ... is char *Name,int Shortcut,void (* Function)(void),byte Enabled,
//           Name==SEPARATOR needs no shortcut, function or enabled
// returns : void
{
    va_list ap;
    int n;
    char *name;
    SUBMENU *sm=&(menu->submenu[menu->current]);
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
            if (sm->item[n].enabled==TRUE) sm->current=n;
            // ensures the current is a proper item
        }
    }
    va_end(ap);
    ++(menu->current);
}

KEYGROUP *MenuKeyGroupGet(MENU *menu) {return KeySysGroupGet(menu->ks);}
// purpose : gets the menus keygroup
// returns : the keygroup

void MenuKeyGroupSet(MENU *menu,KEYGROUP *kg) {KeySysGroupSet(menu->ks,kg);}
// purpose : sets the dialogs keygroup
// returns : void

int MenuSub(MENU *menu) {return menu->current;}
// purpose : to get the index of the current submenu
// returns : the index 

int MenuSubs(MENU *menu) {return menu->submenus;}
// purpose : to get the number of submenus in the menu
// returns : the number 

int MenuSubItem(MENU *menu) {return menu->submenu[menu->current].current;}
// purpose : to get the index of the current item in the current submenu
// returns : the index 

int MenuSubItems(MENU *menu) {return menu->submenu[menu->current].items;}
// purpose : to get the number of items in the current submenu
// returns : the number

void MenuDisplayBar(MENU *menu,VIEWPORT *vp,int dropped)
// purpose : displays the menu bar in the current viewport
//           if dropped the menu name is not hilited
// returns : void
{
    int x,pos=vp->x1;
    int width=vp->x2-vp->x1+1,height=vp->y2-vp->y1+1;
    int menuwidth=width/MenuSubs(menu);
// the menu bar
    BorderedBox(vp->x1,vp->y1,vp->x2,vp->y2,MENUBACK,MENUHILITE);
// hilite selected
    _rectangle(dropped?_GBORDER:_GFILLINTERIOR,
        vp->x1+(MenuSub(menu)*menuwidth),vp->y1,
        vp->x1+((MenuSub(menu)+1)*menuwidth),vp->y2);
    _settextalign(_LEFT,_TOP);
    _setcharsize(height-4,menuwidth/12);
    _setcharspacing(-1);
// write text
    _setcolor(MENUTEXT);
    for (x=0;x<MenuSubs(menu);x++)
    {
        _grtext(pos,vp->y1+1,menu->submenu[x].name);
        pos+=menuwidth;
    }
}

void MenuDisplaySub(SUBMENU *sub,VIEWPORT *vp)
// purpose : displays the submenu in the viewport
// returns : void
{
    int x,pos=vp->y1;
    int width=vp->x2-vp->x1+1,height=vp->y2-vp->y1+1;
    int itemheight=height/sub->items;
// the submenu
    BorderedBox(vp->x1,vp->y1,vp->x2,vp->y2,MENUBACK,MENUHILITE);
// hilite item
    _settextalign(_LEFT,_TOP);
    _rectangle(_GFILLINTERIOR,
                vp->x1,vp->y1+(itemheight*sub->current),
                vp->x2,vp->y1+(itemheight*(sub->current+1)));
                
    for (x=0;x<sub->items;x++)
    {
        if (sub->item[x].name==SEPARATOR)
        {
            _setcolor(MENUHILITE);
            _moveto(vp->x1,pos);
            _lineto(vp->x2,pos);
        }
        else
        {
            if (sub->item[x].enabled==FALSE) _setcolor(MENUHILITE);
            else                                    _setcolor(MENUTEXT);
            _grtext(vp->x1+2,pos,sub->item[x].name);
        }
        pos+=itemheight;
    }   
}

void MenuDisplay(MENU *menu)
// purpose : displays the menu in its current state
// returns : void
{
    int width=menu->vp->x2-menu->vp->x1+1;
    VIEWPORT vp;

    MemPagePut(menu->background);

// menu bar
    ResetViewport(&vp,menu->vp->x1,menu->vp->y1,
                      menu->vp->x2,menu->vp->y1+30);
    MenuDisplayBar(menu,&vp,TRUE);
// submenu
    ResetViewport(&vp,
        menu->vp->x1+(width*MenuSub(menu)/MenuSubs(menu)),menu->vp->y1+31,
        menu->vp->x1+(width*(MenuSub(menu)+1)/MenuSubs(menu)),
        menu->vp->y1+31+(MenuSubItems(menu)*30));
    MenuDisplaySub(&(menu->submenu[MenuSub(menu)]),&vp);
}

void MenuCall(MENU *menu,void (* help)(void))
// purpose : calls the menu, runs help via F1 or H
// returns : void
{
    int funcret,index,key,used;

    if (menu->current>=menu->submenus) menu->current=0;
    MenuDisplay(menu);
    SwapPage();

    for (;;)
    {
        key=GetKey();
        used=KeySysKey(menu->ks,key,menu,&funcret,&index);
        if (!used) // provide extra functionality
        {
            switch (key)
            {
                case 'm' :    // cancel
                case ESC : return;
                case F1  :
                case 'h' : help(); used=TRUE; funcret=TRUE; index=6; break;
            }
        }    
        if (used)
        // key is used, via extra functionality or keysys
        {
            switch (index)
            {
                case 4 : case 5 : // SELECT or CANCEL
                    if (funcret) return; // menu finished
                    break;
            }
            if (funcret)
            {
                MenuDisplay(menu);
                SwapPage();
            }
        }
    }
}

int MenuItemEnable(MENU *menu,int sub,int item,int enable)
// purpose : to set the enable state of an item in the submenu in the menu
// returns : TRUE if successfully set else FALSE
{
    SUBMENU *sm;
    if (sub>=menu->submenus) return FALSE;
    sm=&(menu->submenu[sub]);
    if (item>=sm->items) return FALSE;
    sm->item[item].enabled=enable;
    return TRUE;
}

int MenuSubCurrentSet(MENU *menu,int sub,int current)
// purpose : to set the current in the submenu
// returns : TRUE if set else FALSE
{
    SUBMENU *sm;
    if (sub>=menu->submenus) return FALSE;
    sm=&(menu->submenu[sub]);
    if (current>=sm->items) return FALSE;
    if (sm->item[current].enabled==FALSE) return FALSE;
    sm->current=current;
    return TRUE;
   
}

void MenuFree(MENU *menu)
// purpose : to free the memory allocated to the menu
// returns : void
{
    int submenu,item;
    for (submenu=0;submenu<menu->submenus;submenu++)
    {
        free(menu->submenu[submenu].name);
        for (item=0;item<menu->submenu[submenu].items;item++)
            if (menu->submenu[submenu].item[item].name!=SEPARATOR)
                free(menu->submenu[submenu].item[item].name);
        free(menu->submenu[submenu].item);
    }
    free(menu->submenu);
    KeySysFree(menu->ks);
    free(menu);
}

/*
void main()
{
    VIEWPORT main;
    MENU *mainmenu;
    KEYGROUP *kg=KeyGroupNew(6,KP4,KP6,KP8,KP2,ENTER,ESC);
    KEYGROUP *kg2=KeyGroupNew(6,'a','d','w','s','e','q');
    ResetViewport(&main,0,0,639,349);
    mainmenu=MenuNew(4,&main,NULL,kg);
    MenuNewSub(mainmenu,"Game",7,
        "New",      CN, NULL,    TRUE,
        "End",      CE, NULL,    TRUE,
        "Status",   CS, NULL,    TRUE,
        "High Scores",NONE,NULL, TRUE,
        "Suggest Move",NONE,NULL,TRUE,
        "Sound",    NONE,NULL,   TRUE,
        "Exit",     CX, NULL,    TRUE);
    MenuNewSub(mainmenu,"Board",3,
        "7x8x4",    NONE,NULL,   TRUE,
        "12x13x6",  NONE,NULL,   TRUE,
        "Custom...",NONE,NULL,   TRUE);
    MenuNewSub(mainmenu,"Player",3,
        "One...",   NONE,NULL,   TRUE,
        "Two...",   NONE,NULL,   TRUE,
        "Options",  NONE,NULL,   TRUE);
    MenuNewSub(mainmenu,"Help",2,
        "Contents", NONE,NULL,   TRUE,
        "About",    NONE,NULL,   TRUE);
    ScreenInit();
    MenuCall(mainmenu);
    MenuKeyGroupSet(mainmenu,kg2);
    MenuCall(mainmenu);
    ShutDown();
}
*/
/*
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
*/

