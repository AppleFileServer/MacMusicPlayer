#include "ui_nav.h"
#include "ui_lists.h"
#include "ui_menus.h"
#include "ipod_lcd.h"
#include "lcd_draw.h"
#include "lcd_layout.h"
#include "library.h"
#include "playback.h"
#include "pstr.h"

/* Deepest planned path is 7 screens (Main > Music > Genres > Genre >
   Artist > Album > Now Playing). */
#define kMaxNavDepth    8

static Screen gNav[kMaxNavDepth];
static short gNavItems[kMaxNavDepth][kMaxListItems];
static short gNavDepth;

/* Songs a row would play (UIList_RowSongs), or a shuffled library. */
static short gPlayList[kMaxListItems];

static Screen *CurrentScreen(void)
{
    return &gNav[gNavDepth - 1];
}

/* Pushes a screen whose kind, menuID, filter and title are set. */
static void PushScreen(const Screen *target)
{
    Screen *s;

    if (gNavDepth >= kMaxNavDepth)
        return;

    s = &gNav[gNavDepth];
    *s = *target;
    s->items = gNavItems[gNavDepth];
    gNavDepth++;

    UIList_Build(s);

    /* PROVISIONAL (D7): entering a menu highlights its first item.
       Inferred from the screenshots, not documented. */
    s->selected = 0;
    s->top = 0;
}

void UI_Init(void)
{
    Screen main;

    main.kind = kScreenMenu;
    main.menuID = kMenuMain;
    LibFilter_Clear(&main.filter);
    main.title = (ConstStringPtr)UIMenus_Get(kMenuMain)->title;

    gNavDepth = 0;
    PushScreen(&main);
}

static Boolean MoveSelection(short delta)
{
    Screen *s = CurrentScreen();
    short next = s->selected + delta;

    /* PROVISIONAL (D1, spec unknown #3): the highlight stops at the first
       and last items rather than wrapping. Real 4G behaviour unknown. */
    if (next < 0 || next >= s->count)
        return false;

    s->selected = next;

    /* Inferred: the list scrolls one row only when the highlight
       would leave the visible rows. */
    if (s->selected < s->top)
        s->top = s->selected;
    else if (s->selected >= s->top + kVisibleRows)
        s->top = s->selected - kVisibleRows + 1;

    return true;
}

static Boolean SelectItem(void)
{
    Screen *s = CurrentScreen();
    Screen next;

    /* Items with nothing to open yet (stubs, songs) do nothing. */
    if (s->count == 0 || !UIList_ItemTarget(s, s->selected, &next))
        return false;

    PushScreen(&next);
    return true;
}

static Boolean GoBack(void)
{
    /* PROVISIONAL (D2, spec: unknown): Menu on the main menu does nothing. */
    if (gNavDepth <= 1)
        return false;

    /* PROVISIONAL (D6, Inferred): the previous screen keeps its saved
       highlight and scroll position. */
    gNavDepth--;
    return true;
}

short UI_SelectedSong(void)
{
    Screen *s = CurrentScreen();

    return UIList_ItemSong(s, s->selected);
}

/* Rebuilds one screen; the highlight follows its item if still listed,
   otherwise stays at the same row (clamped). Scrolling keeps it visible. */
static void RebuildScreen(Screen *s)
{
    short oldItem = s->count > 0 ? s->items[s->selected] : kNone;
    short i;

    UIList_Build(s);

    for (i = 0; i < s->count; i++)
        if (s->items[i] == oldItem)
        {
            s->selected = i;
            break;
        }
    if (s->selected >= s->count)
        s->selected = s->count > 0 ? s->count - 1 : 0;

    if (s->top > s->selected)
        s->top = s->selected;
    if (s->selected >= s->top + kVisibleRows)
        s->top = s->selected - kVisibleRows + 1;
    if (s->top > 0 && s->top + kVisibleRows > s->count)
        s->top = s->count > kVisibleRows ? s->count - kVisibleRows : 0;
}

void UI_Refresh(void)
{
    short i;

    for (i = 0; i < gNavDepth; i++)
        RebuildScreen(&gNav[i]);
}

/* The main menu lists Now Playing only while something plays (N1):
   rebuild it wherever it is on the stack when that changes. */
static void RefreshMainMenus(void)
{
    short i;

    for (i = 0; i < gNavDepth; i++)
        if (gNav[i].kind == kScreenMenu && gNav[i].menuID == kMenuMain)
            RebuildScreen(&gNav[i]);
}

static void PushNowPlaying(void)
{
    Screen np;

    np.kind = kScreenNowPlaying;
    np.menuID = 0;
    LibFilter_Clear(&np.filter);
    np.title = P("Now Playing");
    PushScreen(&np);
}

/* Plays songs[start..] and shows Now Playing (manual p. 15). */
static Boolean PlayAndShow(const short *songs, short count, short start)
{
    if (count <= 0)
        return false;
    Playback_Start(songs, count, start);
    RefreshMainMenus();
    if (Playback_IsActive())            /* R8: nothing playable -> stay */
        PushNowPlaying();
    return true;
}

/* Shuffle Songs: the whole library in random order, audiobooks skipped
   (manual p. 32; Library_ListSongs already leaves them out). */
static Boolean ShuffleSongs(void)
{
    LibFilter all;
    short n, i, j, t;

    LibFilter_Clear(&all);
    n = Library_ListSongs(&all, gPlayList, kMaxListItems);
    for (i = n - 1; i > 0; i--)
    {
        j = (short)((unsigned short)Random() % (i + 1));
        t = gPlayList[i];
        gPlayList[i] = gPlayList[j];
        gPlayList[j] = t;
    }
    return PlayAndShow(gPlayList, n, 0);
}

static Boolean IsShuffleRow(const Screen *s)
{
    return s->kind == kScreenMenu && s->count > 0
        && UIMenus_Get(s->menuID)->items[s->items[s->selected]].action
           == kActShuffle;
}

/* Select: a song row plays (N2); Shuffle Songs shuffles; on Now Playing
   it does nothing yet (scrubber: Stage 7); otherwise open the item. */
static Boolean HandleSelect(void)
{
    Screen *s = CurrentScreen();
    short start, n;

    if (s->kind == kScreenNowPlaying)
        return false;
    if (IsShuffleRow(s))
        return ShuffleSongs();
    if (UIList_ItemSong(s, s->selected) != kNone)
    {
        n = UIList_RowSongs(s, s->selected, gPlayList, kMaxListItems, &start);
        return PlayAndShow(gPlayList, n, start);
    }
    return SelectItem();
}

/* Play/Pause: plays the highlighted song or list (N2, N3); otherwise, or
   on Now Playing, pauses or resumes (N7). */
static Boolean HandlePlayPause(void)
{
    Screen *s = CurrentScreen();
    short start, n;

    if (s->kind != kScreenNowPlaying)
    {
        if (IsShuffleRow(s))
            return ShuffleSongs();
        n = UIList_RowSongs(s, s->selected, gPlayList, kMaxListItems, &start);
        if (n > 0)
            return PlayAndShow(gPlayList, n, start);
    }
    if (!Playback_IsActive())
        return false;
    Playback_TogglePause();
    return s->kind == kScreenNowPlaying;
}

/* Playback has just stopped (DECISION N4: the natural end of the last
   song, or the queue emptied by a rescan): return to the list underneath
   and drop the main menu's Now Playing item. */
static void PlaybackStopped(void)
{
    if (CurrentScreen()->kind == kScreenNowPlaying)
        GoBack();
    RefreshMainMenus();
}

static Boolean HandleSkip(Boolean forward)
{
    if (!Playback_IsActive())
        return false;
    if (forward)
        Playback_Next();
    else
        Playback_Previous();

    if (!Playback_IsActive())
    {
        PlaybackStopped();
        return true;
    }
    return CurrentScreen()->kind == kScreenNowPlaying;
}

Boolean UI_HandleEvent(UIEvent event)
{
    Boolean onNowPlaying = CurrentScreen()->kind == kScreenNowPlaying;

    switch (event)
    {
        /* The wheel does nothing on Now Playing until volume (Stage 7). */
        case kEvtWheelUp:   return !onNowPlaying && MoveSelection(-1);
        case kEvtWheelDown: return !onNowPlaying && MoveSelection(1);
        case kEvtSelect:    return HandleSelect();
        case kEvtMenu:      return GoBack();
        case kEvtPlayPause: return HandlePlayPause();
        case kEvtNext:      return HandleSkip(true);
        case kEvtPrevious:  return HandleSkip(false);
        default:            return false;
    }
}

Boolean UI_Tick(void)
{
    short result = Playback_Tick();
    Boolean onNowPlaying = CurrentScreen()->kind == kScreenNowPlaying;

    if (result & kTickStopped)
    {
        PlaybackStopped();
        return true;
    }
    return onNowPlaying && (result & (kTickTimeChanged | kTickSongChanged));
}

static void RenderNowPlaying(void)
{
    short song = Playback_CurrentSong();

    if (song == kNone)
    {
        LCDDraw_Clear();
        LCDDraw_Header(P("Now Playing"));
        return;
    }
    LCDDraw_NowPlaying(Library_SongTitle(song), Library_SongArtistName(song),
                       Library_SongAlbumName(song), Playback_Position(),
                       Playback_QueueLength(), Playback_ElapsedTicks(),
                       Playback_LengthTicks(),
                       Playback_State() == kPlayPaused);
}

void UI_Render(void)
{
    Screen *s = CurrentScreen();

    if (s->kind == kScreenNowPlaying)
    {
        LCD_BeginDraw();
        RenderNowPlaying();
        LCD_EndDraw();
        return;
    }

    Boolean hasScrollBar = s->count > kVisibleRows;
    short position;
    short index;

    LCD_BeginDraw();

    LCDDraw_Clear();
    LCDDraw_Header(s->title);

    for (position = 0; position < kVisibleRows; position++)
    {
        index = s->top + position;
        if (index >= s->count)
            break;

        LCDDraw_Row(position, UIList_ItemTitle(s, index),
                    index == s->selected,
                    UIList_ItemHasArrow(s, index),
                    hasScrollBar);
    }

    if (hasScrollBar)
        LCDDraw_ScrollBar(s->count, s->top);

    LCD_EndDraw();
}
