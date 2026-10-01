#ifndef UI_MENUS_H
#define UI_MENUS_H

#include <MacTypes.h>

enum
{
    kMenuMain = 0,
    kMenuMusic,
    kMenuCount
};

/* Item has no submenu: no arrow, Select does nothing yet. */
#define kNoSubmenu          (-1)

/* Item shows an arrow but its submenu is not implemented yet:
   Select does nothing (Stage 1 stub, not iPod behaviour). */
#define kSubmenuStub        (-2)

typedef struct
{
    const char *title;      /* Pascal string, e.g. "\pMusic" */
    short submenu;          /* menu ID, kNoSubmenu or kSubmenuStub */
} MenuItemDef;

typedef struct
{
    const char *title;      /* header title, Pascal string */
    short count;
    const MenuItemDef *items;
} MenuDef;

const MenuDef *UIMenus_Get(short menuID);

#endif
