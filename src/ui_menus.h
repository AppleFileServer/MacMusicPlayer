#ifndef UI_MENUS_H
#define UI_MENUS_H

#include <MacTypes.h>

enum
{
    kMenuMain = 0,
    kMenuMusic,
    kMenuCount
};

/* What Select does on a static menu item. */
enum
{
    kActNone = 0,       /* no arrow; nothing yet (playback etc.) */
    kActStub,           /* arrow shown, screen not implemented yet */
    kActMenu,           /* open static menu `arg` */
    kActScreen          /* open library list screen kind `arg` */
};

typedef struct
{
    const char *title;  /* Pascal string, e.g. "\pMusic" */
    short action;
    short arg;
} MenuItemDef;

typedef struct
{
    const char *title;  /* header title, Pascal string */
    short count;
    const MenuItemDef *items;
} MenuDef;

const MenuDef *UIMenus_Get(short menuID);

#endif
