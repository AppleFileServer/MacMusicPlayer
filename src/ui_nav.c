#include <Events.h>

#include "ui_nav.h"
#include "ui_lists.h"
#include "ui_menus.h"
#include "ipod_lcd.h"
#include "lcd_draw.h"
#include "lcd_layout.h"
#include "library.h"
#include "playback.h"
#include "pstr.h"
#include "prefs.h"
#include "song_format.h"
#include "volume.h"

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

static Boolean gKeepRow;
static void ClampScroll(Screen *s);

void UI_KeepRowOnRefresh(void)
{
    gKeepRow = true;
}

void UI_Refresh(void)
{
    Screen *s = CurrentScreen();
    short i, row = s->selected;

    for (i = 0; i < gNavDepth; i++)
        RebuildScreen(&gNav[i]);
    if (gKeepRow)
    {
        gKeepRow = false;
        s->selected = row;
        ClampScroll(s);
    }
}

Boolean UI_SelectedPlaylistEntry(short *playlist, long *id, short *nth)
{
    Screen *s = CurrentScreen();
    short song = UIList_ItemSong(s, s->selected), i;

    if (s->kind != kScreenSongs || s->filter.playlist == kNone || song == kNone)
        return false;
    *playlist = s->filter.playlist;
    *id = Library_SongID(song);
    *nth = 0;
    for (i = 0; i < s->selected; i++)
        if (s->items[i] == song)
            (*nth)++;
    return true;
}

short UI_SongToEdit(void)
{
    Screen *s = CurrentScreen();

    if (s->kind == kScreenNowPlaying)
        return Playback_CurrentSong();
    return UIList_ItemSong(s, s->selected);
}

/* ---- Stage 8c: staying on screen after an edit (L4) ------------------- */

/* A screen's choices and highlighted row, by name (or song ID), noted
   before the library's numbers change: an edit renumbers the artists,
   albums, genres and composers; a rescan (8d-2) the songs and playlists
   too. */
typedef struct
{
    Str63 genre, artist, album, composer;   /* empty: not chosen */
    Str63 playlist;
    Str63 selectedName;                     /* a named row (group, playlist) */
    long selectedID;                        /* a song row: its ID */
    short selectedItem;                     /* any other row (menus) */
} ScreenNames;

static ScreenNames gSavedNames[kMaxNavDepth];

static void NoteName(short group, ConstStringPtr (*name)(short), Str63 out)
{
    ConstStringPtr s;

    out[0] = 0;
    if (group == kNone)
        return;
    s = name(group);
    out[0] = s[0] <= 63 ? s[0] : 63;
    BlockMove(s + 1, out + 1, out[0]);
}

/* Lists whose rows are found again by name. */
static Boolean IsNamedList(short kind)
{
    return kind == kScreenGenres || kind == kScreenArtists
        || kind == kScreenAlbums || kind == kScreenComposers
        || kind == kScreenPlaylists;
}

static Boolean IsSongList(short kind)
{
    return kind == kScreenSongs || kind == kScreenAudiobooks;
}

void UI_BeginLibraryEdit(void)
{
    ScreenNames *n;
    Screen *s;
    ConstStringPtr title;
    short i;

    for (i = 0; i < gNavDepth; i++)
    {
        s = &gNav[i];
        n = &gSavedNames[i];
        NoteName(s->filter.genre, Library_GenreName, n->genre);
        NoteName(s->filter.artist, Library_ArtistName, n->artist);
        NoteName(s->filter.album, Library_AlbumName, n->album);
        NoteName(s->filter.composer, Library_ComposerName, n->composer);
        NoteName(s->filter.playlist, Library_PlaylistName, n->playlist);

        n->selectedName[0] = 0;
        n->selectedItem = s->count > 0 ? s->items[s->selected] : kNone;
        n->selectedID = IsSongList(s->kind) && n->selectedItem != kNone
                        ? Library_SongID(n->selectedItem) : 0;
        if (IsNamedList(s->kind) && n->selectedItem != kAllItem
            && n->selectedItem != kNone)
        {
            title = UIList_ItemTitle(s, s->selected);
            n->selectedName[0] = title[0] <= 63 ? title[0] : 63;
            BlockMove(title + 1, n->selectedName + 1, n->selectedName[0]);
        }
    }
}

/* Finds a noted choice again by its name; false if it has gone. */
static Boolean FindAgain(ConstStringPtr name,
                         short (*find)(ConstStringPtr), short *group)
{
    if (*group == kNone)
        return true;
    *group = find(name);
    return *group != kNone;
}

/* True if the screen was opened by choosing an artist, album, genre or
   composer (its filter adds one to its parent's); its title is then that
   name, which has moved. */
static Boolean RetitleFromChoice(Screen *s, const Screen *parent)
{
    if (s->filter.genre != kNone && s->filter.genre != parent->filter.genre)
        s->title = Library_GenreName(s->filter.genre);
    else if (s->filter.artist != kNone
             && s->filter.artist != parent->filter.artist)
        s->title = Library_ArtistName(s->filter.artist);
    else if (s->filter.album != kNone
             && s->filter.album != parent->filter.album)
        s->title = Library_AlbumName(s->filter.album);
    else if (s->filter.composer != kNone
             && s->filter.composer != parent->filter.composer)
        s->title = Library_ComposerName(s->filter.composer);
    else if (s->filter.playlist != kNone
             && s->filter.playlist != parent->filter.playlist)
        s->title = Library_PlaylistName(s->filter.playlist);
    else
        return false;
    return true;
}

/* Keeps the highlight visible, as RebuildScreen does. */
static void ClampScroll(Screen *s)
{
    if (s->selected >= s->count)
        s->selected = s->count > 0 ? s->count - 1 : 0;
    if (s->top > s->selected)
        s->top = s->selected;
    if (s->selected >= s->top + kVisibleRows)
        s->top = s->selected - kVisibleRows + 1;
    if (s->top > 0 && s->top + kVisibleRows > s->count)
        s->top = s->count > kVisibleRows ? s->count - kVisibleRows : 0;
}

/* Rebuilds one screen after an edit; false if it no longer exists. */
static Boolean FindScreenAgain(Screen *s, const Screen *parent,
                               const ScreenNames *n)
{
    Boolean byChoice;
    short i;

    if (!FindAgain(n->genre, Library_FindGenre, &s->filter.genre)
        || !FindAgain(n->artist, Library_FindArtist, &s->filter.artist)
        || !FindAgain(n->album, Library_FindAlbum, &s->filter.album)
        || !FindAgain(n->composer, Library_FindComposer, &s->filter.composer)
        || !FindAgain(n->playlist, Library_FindPlaylist, &s->filter.playlist))
        return false;
    byChoice = parent != nil && RetitleFromChoice(s, parent);

    UIList_Build(s);
    if (byChoice && s->filter.playlist == kNone
        && (s->count == 0 || (s->count == 1 && s->items[0] == kAllItem)))
        return false;               /* nothing left under that name */

    for (i = 0; i < s->count; i++)
    {
        Boolean same;

        if (IsNamedList(s->kind) && n->selectedName[0] != 0)
            same = s->items[i] != kAllItem
                && EqualString(UIList_ItemTitle(s, i), n->selectedName,
                               true, true);
        else if (IsSongList(s->kind))
            same = n->selectedID != 0
                && Library_SongID(s->items[i]) == n->selectedID;
        else
            same = s->items[i] == n->selectedItem;
        if (same)
        {
            s->selected = i;
            break;
        }
    }
    ClampScroll(s);
    return true;
}

void UI_EndLibraryEdit(void)
{
    /* A rescan may have stopped playback (its songs gone): Now Playing
       then goes, as at the end of the last song (N4). */
    Boolean nowPlayingOnTop = CurrentScreen()->kind == kScreenNowPlaying
                              && Playback_IsActive();
    short i, kept;

    if (CurrentScreen()->kind == kScreenNowPlaying && !Playback_IsActive())
        gNavDepth--;

    for (kept = 0; kept < gNavDepth; kept++)
    {
        if (gNav[kept].kind == kScreenNowPlaying)
            continue;               /* stays (drawn from the song itself) */
        if (!FindScreenAgain(&gNav[kept], kept > 0 ? &gNav[kept - 1] : nil,
                             &gSavedNames[kept]))
            break;
    }
    if (kept >= gNavDepth)
        return;

    /* Screens from the first that has gone are closed; Now Playing, if
       it was showing, stays on top of what remains. */
    if (nowPlayingOnTop)
    {
        gNav[kept] = gNav[gNavDepth - 1];
        gNav[kept].items = gNavItems[kept];
        kept++;
    }
    gNavDepth = kept;
    for (i = 0; i < gNavDepth; i++)
        ClampScroll(&gNav[i]);
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

/* ---- Stage 7: Now Playing modes ----------------------------------------

   From Apple's 4G guide: turning the wheel on Now Playing changes the
   volume; Select shows the scrubber, and again the rating. Here Select
   steps normal -> scrubber -> rating -> normal (from volume, Select goes
   to the scrubber). PROVISIONAL (N5): each mode returns to normal after
   kVolumeTimeout / kModeTimeout ticks without input. PROVISIONAL: Next,
   Previous and a change of song return to normal (the rating and
   scrubber belong to the song that was showing). 7a: the modes and their
   screens; the wheel's action in each (volume, seeking, rating) comes in
   7b-7d. */
#define kVolumeTimeout      120         /* 2 s */
#define kModeTimeout        300         /* 5 s */

/* PROVISIONAL (E3, spec unknown #6): one list step per 15 degrees of
   wheel rotation (24 per turn), no acceleration. The 4G's real step size
   is unknown; this is the single sensitivity setting. */
#define kWheelStepDegrees   15

/* 7c (N3): one turn of the wheel is the whole song, in 3-degree steps
   (120 a turn); the audio jumps once turning has stopped for this long.
   Both PROVISIONAL. */
#define kScrubStepDegrees   3
#define kScrubSteps         (360 / kScrubStepDegrees)
#define kScrubSettle        30          /* 0.5 s */

/* 8e-1: one turn of the wheel takes the volume from silent to full, in
   the same 3-degree steps (120 a turn) while the volume bar shows. The
   first turn from the normal screen keeps the 15-degree step, so a small
   wobble while pressing a wheel button isn't taken as a turn; it counts
   as 5 fine steps. A keyboard arrow is a whole level, or 1/24 of a song
   in the scrubber (as a 15-degree step). */
#define kFineSteps          (kWheelStepDegrees / kScrubStepDegrees)
#define kVolumeSteps        kScrubSteps

static NowPlayingMode gMode;
static unsigned long gModeInput;        /* TickCount of the last input */

static Boolean gVolumeChanged;          /* to be saved in the prefs */

/* 7d (N4): the rating being set, for the song with this ID (kept by ID,
   so a song change or rescan can't send it to another song). Written to
   the song file only when rating mode ends, never on each wheel step. */
static short gRating;
static long gRatingID;
static Boolean gRatingChanged;
static Boolean gRatingError;

static void BeginRating(void)
{
    short song = Playback_CurrentSong();

    gRatingChanged = false;
    gRatingID = song == kNone ? 0 : Library_SongID(song);
    gRating = song == kNone ? 0 : Library_SongRating(song);
}

static void SaveRating(void)
{
    short song;

    if (!gRatingChanged)
        return;
    gRatingChanged = false;
    song = Library_SongForID(gRatingID);
    if (song != kNone && Library_SetSongRating(song, gRating) != noErr)
        gRatingError = true;
}

Boolean UI_TakeRatingError(void)
{
    Boolean error = gRatingError;

    gRatingError = false;
    return error;
}

/* 7b (N2): the level is saved when volume mode ends, not on every step:
   a save is several disk accesses, which made the wheel lag. */
void UI_SaveSettings(void)
{
    SaveRating();
    if (!gVolumeChanged)
        return;
    gVolumeChanged = false;
    Prefs_SaveVolume(Volume_Get());
}

/* 7c: the scrubber's position while it is being moved, not yet sent to
   playback (N3: the audio jumps only once turning stops). */
static Boolean gScrubPending;
static long gScrubTicks;
static unsigned long gScrubMoved;       /* TickCount of the last step */

/* Sends a pending scrubber position to playback. */
static void ApplyScrub(void)
{
    if (!gScrubPending)
        return;
    gScrubPending = false;
    Playback_Seek(gScrubTicks);
}

/* Drops a pending position (the song is changing). */
static void CancelScrub(void)
{
    gScrubPending = false;
}

static void ScrubWheel(short delta)
{
    long length = Playback_LengthTicks();

    if (!gScrubPending)
        gScrubTicks = Playback_ElapsedTicks();
    gScrubTicks += (long)delta * length / kScrubSteps;
    if (gScrubTicks < 0)
        gScrubTicks = 0;
    if (gScrubTicks > length)
        gScrubTicks = length;
    gScrubPending = true;
    gScrubMoved = TickCount();
}

short UI_WheelStepDegrees(void)
{
    if (CurrentScreen()->kind == kScreenNowPlaying
        && (gMode == kNowScrubber || gMode == kNowVolume))
        return kScrubStepDegrees;
    return kWheelStepDegrees;
}

Boolean UI_Idle(void)
{
    if (!gScrubPending || TickCount() - gScrubMoved < kScrubSettle)
        return false;
    ApplyScrub();
    return true;
}

static void SetMode(NowPlayingMode mode)
{
    if (gMode == kNowVolume && mode != kNowVolume)
        UI_SaveSettings();
    if (gMode == kNowScrubber && mode != kNowScrubber)
        ApplyScrub();
    if (gMode == kNowRating && mode != kNowRating)
        SaveRating();
    if (mode == kNowRating && gMode != kNowRating)
        BeginRating();
    gMode = mode;
    gModeInput = TickCount();
}

/* Back to normal, from whatever mode, without a new input time. */
static void NormalMode(void)
{
    if (gMode == kNowVolume)
        UI_SaveSettings();
    if (gMode == kNowScrubber)
        ApplyScrub();
    if (gMode == kNowRating)
        SaveRating();
    gMode = kNowNormal;
}

NowPlayingMode UI_NowPlayingMode(void)
{
    return gMode;
}

/* The wheel on Now Playing. Normal or volume mode: changes the volume one
   level per wheel step (N1: the Plus's 8 levels), the first step opening
   volume mode, as on the 4G. Scrubber: moves the position (7c, N3).
   Rating: one star per step, 0-5 (7d). */
/* 8e-1: the volume's place along one turn of the wheel, 0 (silent) to
   kVolumeSteps (full); the level is that place scaled to 0-7, rounded. */
static short gVolumePosition;

static short LevelAt(short position)
{
    return (short)(((long)position * kVolumeMax + kVolumeSteps / 2)
                   / kVolumeSteps);
}

static short PositionOf(short level)
{
    return (short)(((long)level * kVolumeSteps + kVolumeMax / 2) / kVolumeMax);
}

/* Moves the volume by fine steps along the turn. */
static void TurnVolume(short steps)
{
    short level;

    /* Starts from the level as it is (it may have been changed in the
       Control Panel), so the bar never jumps. */
    if (gMode != kNowVolume || LevelAt(gVolumePosition) != Volume_Get())
        gVolumePosition = PositionOf(Volume_Get());
    gVolumePosition += steps;
    if (gVolumePosition < 0)
        gVolumePosition = 0;
    if (gVolumePosition > kVolumeSteps)
        gVolumePosition = kVolumeSteps;

    level = LevelAt(gVolumePosition);
    if (level != Volume_Get())
    {
        Volume_Set(level);
        gVolumeChanged = true;
    }
}

/* A keyboard arrow: a whole volume level (8e-1). */
static void StepVolume(short delta)
{
    short level = Volume_Get() + delta;

    if (level >= 0 && level <= kVolumeMax)
    {
        Volume_Set(level);
        gVolumeChanged = true;
    }
    gVolumePosition = PositionOf(Volume_Get());
}

static Boolean NowPlayingWheel(short delta)
{
    if (gMode == kNowNormal || gMode == kNowVolume)
    {
        /* From the normal screen the step was 15 degrees. */
        TurnVolume(gMode == kNowNormal ? delta * kFineSteps : delta);
        SetMode(kNowVolume);
        return true;
    }
    if (gMode == kNowScrubber)
        ScrubWheel(delta);
    else if (gMode == kNowRating && gRating + delta >= 0
             && gRating + delta <= kSongMaxRating)
    {
        gRating += delta;
        gRatingChanged = true;
    }
    SetMode(gMode);                     /* keeps the mode up */
    return true;
}

/* A keyboard arrow on Now Playing: a whole step (8e-1). */
static Boolean NowPlayingArrow(short delta)
{
    if (gMode == kNowNormal || gMode == kNowVolume)
    {
        StepVolume(delta);
        SetMode(kNowVolume);
        return true;
    }
    if (gMode == kNowScrubber)
    {
        ScrubWheel(delta * kFineSteps);
        SetMode(gMode);
        return true;
    }
    return NowPlayingWheel(delta);      /* rating: a star, as the wheel */
}

static Boolean NowPlayingSelect(void)
{
    switch (gMode)
    {
        case kNowScrubber:  SetMode(kNowRating);    break;
        case kNowRating:    SetMode(kNowNormal);    break;
        default:            SetMode(kNowScrubber);  break;  /* normal, volume */
    }
    return true;
}

/* True if the mode timed out (the screen needs redrawing). */
static Boolean ModeTimedOut(void)
{
    unsigned long limit = gMode == kNowVolume ? kVolumeTimeout : kModeTimeout;

    if (gMode == kNowNormal || TickCount() - gModeInput < limit)
        return false;
    NormalMode();
    return true;
}

static void PushNowPlaying(void)
{
    Screen np;

    NormalMode();
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

    ApplyScrub();                       /* 7c: play/pause from there */

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
    CancelScrub();                      /* that position was this song's */
    NormalMode();
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
        case kEvtWheelUp:   return onNowPlaying ? NowPlayingWheel(-1)
                                            : MoveSelection(-1);
        case kEvtWheelDown: return onNowPlaying ? NowPlayingWheel(1)
                                            : MoveSelection(1);
        case kEvtArrowUp:   return onNowPlaying ? NowPlayingArrow(-1)
                                            : MoveSelection(-1);
        case kEvtArrowDown: return onNowPlaying ? NowPlayingArrow(1)
                                            : MoveSelection(1);
        case kEvtSelect:    return onNowPlaying ? NowPlayingSelect()
                                            : HandleSelect();
        case kEvtMenu:      NormalMode();
                            return GoBack();
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
    if (result & kTickSongChanged)
    {
        CancelScrub();
        NormalMode();
    }
    if (UI_Idle() && onNowPlaying)
        return true;
    if (onNowPlaying && ModeTimedOut())
        return true;
    return onNowPlaying && (result & (kTickTimeChanged | kTickSongChanged));
}

/* The level or rating a mode shows. */
static short ModeValue(void)
{
    if (gMode == kNowVolume)
        return Volume_Get();
    if (gMode == kNowRating)
        return gRating;
    return 0;
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
                       Playback_QueueLength(),
                       gScrubPending ? gScrubTicks : Playback_ElapsedTicks(),
                       Playback_LengthTicks(),
                       Playback_State() == kPlayPaused,
                       gMode, ModeValue());
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
