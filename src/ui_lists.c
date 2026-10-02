#include "ui_lists.h"
#include "ui_menus.h"

/* Music hierarchy screens. Levels below each Music item are Inferred
   (see "iPod 4G UI Specification", Music hierarchy).

   DECISION (D9, project decision, not confirmed 4G behaviour): Genres,
   Artists and Albums lists start with "All", which opens the next level
   down (Genres > Artists > Albums > Songs) keeping the choices made so far.

   PROVISIONAL (D12): a screen's title is the name of the item chosen to
   reach it; an "All" screen is titled with its category name. */

static const unsigned char kAllTitle[] = "\pAll";
static const unsigned char kArtistsTitle[] = "\pArtists";
static const unsigned char kAlbumsTitle[] = "\pAlbums";
static const unsigned char kSongsTitle[] = "\pSongs";

/* Writes "All" followed by a library list into screen->items. */
static void BuildWithAll(Screen *s, short n)
{
    s->items[0] = kAllItem;
    s->count = n + 1;
}

void UIList_Build(Screen *s)
{
    short i;
    short *rest = s->items + 1;
    short max = kMaxListItems - 1;

    switch (s->kind)
    {
        case kScreenMenu:
            s->count = UIMenus_Get(s->menuID)->count;
            for (i = 0; i < s->count; i++)
                s->items[i] = i;
            break;

        case kScreenPlaylists:
            s->count = Library_ListPlaylists(s->items, kMaxListItems);
            break;

        case kScreenGenres:
            BuildWithAll(s, Library_ListGenres(rest, max));
            break;

        case kScreenArtists:
            BuildWithAll(s, Library_ListArtists(&s->filter, rest, max));
            break;

        case kScreenAlbums:
            BuildWithAll(s, Library_ListAlbums(&s->filter, rest, max));
            break;

        case kScreenSongs:
            s->count = Library_ListSongs(&s->filter, s->items, kMaxListItems);
            break;

        case kScreenComposers:
            s->count = Library_ListComposers(s->items, kMaxListItems);
            break;

        case kScreenAudiobooks:
            s->count = Library_ListAudiobooks(s->items, kMaxListItems);
            break;

        default:
            s->count = 0;
            break;
    }
}

ConstStringPtr UIList_ItemTitle(const Screen *s, short index)
{
    short item = s->items[index];

    if (item == kAllItem)
        return kAllTitle;

    switch (s->kind)
    {
        case kScreenMenu:
            return (ConstStringPtr)UIMenus_Get(s->menuID)->items[item].title;
        case kScreenPlaylists:  return Library_PlaylistName(item);
        case kScreenGenres:     return Library_GenreName(item);
        case kScreenArtists:    return Library_ArtistName(item);
        case kScreenAlbums:     return Library_AlbumName(item);
        case kScreenComposers:  return Library_ComposerName(item);
        default:                return Library_SongTitle(item);
    }
}

Boolean UIList_ItemHasArrow(const Screen *s, short index)
{
    switch (s->kind)
    {
        case kScreenMenu:
            return UIMenus_Get(s->menuID)->items[s->items[index]].action
                   != kActNone;

        case kScreenSongs:
        case kScreenAudiobooks:
            return false;       /* songs are leaves (Observed) */

        default:
            return true;
    }
}

Boolean UIList_ItemTarget(const Screen *s, short index, Screen *next)
{
    short item = s->items[index];
    const MenuItemDef *def;

    next->menuID = 0;
    next->filter = s->filter;

    switch (s->kind)
    {
        case kScreenMenu:
            def = &UIMenus_Get(s->menuID)->items[item];
            next->title = (ConstStringPtr)def->title;
            if (def->action == kActMenu)
            {
                next->kind = kScreenMenu;
                next->menuID = def->arg;
                next->title = (ConstStringPtr)UIMenus_Get(def->arg)->title;
                return true;
            }
            if (def->action == kActScreen)
            {
                next->kind = def->arg;
                LibFilter_Clear(&next->filter);
                return true;
            }
            return false;       /* kActNone, kActStub: not yet */

        case kScreenPlaylists:
            next->kind = kScreenSongs;
            next->filter.playlist = item;
            next->title = Library_PlaylistName(item);
            return true;

        case kScreenGenres:
            next->kind = kScreenArtists;
            if (item == kAllItem)
                next->title = kArtistsTitle;
            else
            {
                next->filter.genre = item;
                next->title = Library_GenreName(item);
            }
            return true;

        case kScreenArtists:
            next->kind = kScreenAlbums;
            if (item == kAllItem)
                next->title = kAlbumsTitle;
            else
            {
                next->filter.artist = item;
                next->title = Library_ArtistName(item);
            }
            return true;

        case kScreenAlbums:
            next->kind = kScreenSongs;
            if (item == kAllItem)
                next->title = kSongsTitle;
            else
            {
                next->filter.album = item;
                next->title = Library_AlbumName(item);
            }
            return true;

        case kScreenComposers:
            next->kind = kScreenSongs;
            next->filter.composer = item;
            next->title = Library_ComposerName(item);
            return true;

        default:
            return false;       /* songs, audiobooks: playback is Stage 4 */
    }
}
