#ifndef UI_MENUS_H
#define UI_MENUS_H

#include <MacTypes.h>

enum
{
    kMenuMain = 0,
    kMenuMusic,
    kMenuSettings,      /* Stage 9a */
    kMenuReset,         /* Stage 9a: Reset All Settings' Cancel / Reset */
    kMenuCount
};

/* What Select does on a static menu item. */
enum
{
    kActNone = 0,       /* no arrow; nothing yet (playback etc.) */
    kActStub,           /* arrow shown, screen not implemented yet */
    kActMenu,           /* open static menu `arg` */
    kActScreen,         /* open library list screen kind `arg` */
    kActShuffle,        /* Shuffle Songs: play everything, random order */
    kActNowPlaying,     /* open Now Playing */
    kActSetting,        /* Stage 9a: Select steps setting `arg`'s value */
    kActReset,          /* Stage 9a: reset all settings, then back */
    kActBack,           /* Stage 9a: back (Cancel) */
    kActBacklight       /* Stage 9a: invert the LCD, or back (S-9) */
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
