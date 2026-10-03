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
    kScreenAudiobooks,
    kScreenNowPlaying       /* not a list: drawn by LCDDraw_NowPlaying */
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

/* The song shown in row index, for Songs and Audiobooks lists; kNone for
   every other kind of row. */
short UIList_ItemSong(const Screen *screen, short index);

/* The songs that playing row index would play, in order, written to out
   (at most max); *start is the song to begin with. 0 if the row plays
   nothing. Song rows play their list from that song (N2); list rows play
   every song that row leads to, from the first (N3). */
short UIList_RowSongs(const Screen *screen, short index,
                      short *out, short max, short *start);

/* Sets kind, menuID, filter and title of the screen that Select on item
   index opens. Returns false if the item opens nothing (yet). */
Boolean UIList_ItemTarget(const Screen *screen, short index, Screen *next);

#endif
