#include "ui_lists.h"
#include "ui_menus.h"
#include "playback.h"
#include "settings.h"
#include "legal.h"
#include "libsetup.h"
#include "fileops.h"
#include "pstr.h"
#include "message.h"

/* Stage 9b: the version About shows (also in the About box, DITL 128). */
static const unsigned char kAppVersion[] = "\p0.1";

/* Stage 9b: About's rows (S-6). */
enum { kAboutSongs = 0, kAboutAvailable, kAboutVersion, kAboutRows };
static ConstStringPtr const kAboutTitles[kAboutRows] = {
    (ConstStringPtr)"\pSongs", (ConstStringPtr)"\pAvailable",
    (ConstStringPtr)"\pVersion"
};

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
            /* PROVISIONAL (N1): the main menu's last item, Now Playing,
               only while a song is playing or paused. */
            if (s->menuID == kMenuMain && !Playback_IsActive())
                s->count--;
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

        case kScreenAbout:
            s->count = kAboutRows;
            for (i = 0; i < s->count; i++)
                s->items[i] = i;
            break;

        case kScreenLegal:
            s->count = Legal_LineCount();
            if (s->count > kMaxListItems)
                s->count = kMaxListItems;
            for (i = 0; i < s->count; i++)
                s->items[i] = i;
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
        case kScreenAbout:      return kAboutTitles[item];
        case kScreenLegal:
        {
            static Str255 line;

            Legal_Line(item, line);
            return line;
        }
        default:                return Library_SongTitle(item);
    }
}

Boolean UIList_ItemHasArrow(const Screen *s, short index)
{
    switch (s->kind)
    {
        case kScreenMenu:
        {
            short action = UIMenus_Get(s->menuID)->items[s->items[index]].action;
            return action == kActStub || action == kActMenu
                || action == kActScreen || action == kActNowPlaying;
        }

        case kScreenSongs:
        case kScreenAudiobooks:
        case kScreenAbout:
        case kScreenLegal:
            return false;       /* songs are leaves (Observed) */

        default:
            return true;
    }
}

ConstStringPtr UIList_ItemValue(const Screen *s, short index)
{
    const MenuItemDef *def;
    static Str255 value;

    if (s->kind == kScreenAbout)
    {
        value[0] = 0;
        switch (s->items[index])
        {
            case kAboutSongs:       /* S-6: of the 500-song limit */
            {
                Str255 number;

                NumToString(Library_SongCount(), number);
                PStrAppend(value, number);
                PStrAppend(value, P(" of 500"));
                break;
            }
            case kAboutAvailable:   /* free on the library's disk */
                if (LibSetup_HasLibrary())
                    LibSetup_AppendMB(value,
                                      FreeBytes(LibSetup_Folder()->vRefNum));
                else
                    PStrAppend(value, P("\xD0"));
                break;
            default:
                PStrAppend(value, kAppVersion);
                break;
        }
        return value;
    }
    if (s->kind != kScreenMenu)
        return nil;
    def = &UIMenus_Get(s->menuID)->items[s->items[index]];
    return def->action == kActSetting ? Settings_ValueText(def->arg) : nil;
}

short UIList_ItemSong(const Screen *s, short index)
{
    if (index < 0 || index >= s->count)
        return kNone;
    if (s->kind == kScreenSongs || s->kind == kScreenAudiobooks)
        return s->items[index];
    return kNone;
}

short UIList_RowSongs(const Screen *s, short index,
                      short *out, short max, short *start)
{
    Screen next;
    short i, n;

    *start = 0;
    if (index < 0 || index >= s->count)
        return 0;

    if (s->kind == kScreenSongs || s->kind == kScreenAudiobooks)
    {
        n = s->count < max ? s->count : max;
        for (i = 0; i < n; i++)
            out[i] = s->items[i];
        *start = index < n ? index : 0;
        return n;
    }

    /* A list row: what it opens decides the songs. Menus, Now Playing
       and the Playlists list itself play nothing. */
    if (!UIList_ItemTarget(s, index, &next))
        return 0;
    switch (next.kind)
    {
        case kScreenAudiobooks:
            return Library_ListAudiobooks(out, max);
        case kScreenSongs:
        case kScreenArtists:
        case kScreenAlbums:
        case kScreenGenres:
        case kScreenComposers:
            /* DECISION N3: the songs the row leads to, in the order the
               screens show them (album: track order; otherwise D14). */
            return Library_ListSongs(&next.filter, out, max);
        default:
            return 0;
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
            if (def->action == kActNowPlaying)
            {
                next->kind = kScreenNowPlaying;
                return true;
            }
            return false;       /* kActNone, kActStub, kActShuffle */

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
