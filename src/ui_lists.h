#ifndef UI_LISTS_H
#define UI_LISTS_H

#include <MacTypes.h>

#include "library.h"

/* Kinds of list screen. kScreenMenu shows a static menu (ui_menus.c);
   the rest are generated from the library. */
typedef enum
{
    kScreenMenu = 0,
    kScreenPlaylists,
    kScreenGenres,
    kScreenArtists,
    kScreenAlbums,
    kScreenSongs,
    kScreenComposers,
    kScreenAudiobooks
} ScreenKind;

/* Item value standing for the "All" row (D9). */
#define kAllItem        (-1)

/* The longest list: every song. */
#define kMaxListItems   (kMaxSongs + 1)

typedef struct
{
    short kind;             /* ScreenKind */
    short menuID;           /* kScreenMenu only */
    LibFilter filter;       /* choices made on the way here */
    ConstStringPtr title;   /* header title */
    short count;
    short *items;           /* count entries; buffer owned by ui_nav.c */
    short selected;
    short top;
} Screen;

/* Fills screen->items and screen->count from its kind and filter. */
void UIList_Build(Screen *screen);

ConstStringPtr UIList_ItemTitle(const Screen *screen, short index);
Boolean UIList_ItemHasArrow(const Screen *screen, short index);

/* Sets kind, menuID, filter and title of the screen that Select on item
   index opens. Returns false if the item opens nothing (yet). */
Boolean UIList_ItemTarget(const Screen *screen, short index, Screen *next);

#endif
