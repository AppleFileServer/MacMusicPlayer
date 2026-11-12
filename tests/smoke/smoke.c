/* MacMusic's emulator smoke test (1.0 Part B2; see smoke.h). Run by
   tests/run-smoke.sh under LaunchAPPL, in Mini vMac at about 1x: the
   songs are 6 s long and the tour seeks rather than plays them through,
   so the whole run takes about a minute. The log ends with
   "FAILURES: n" and "DONE"; a log without them was cut short. */

#include <Files.h>
#include <OSUtils.h>
#include <Quickdraw.h>
#include <TextUtils.h>
#include <string.h>
#include <Resources.h>
#include <Menus.h>

#include "smoke.h"
#include "version.h"
#include "fileopen.h"
#include "songfile.h"
#include "library.h"
#include "libsetup.h"
#include "playback.h"
#include "audio.h"
#include "settings.h"
#include "prefs.h"
#include "volume.h"
#include "legal.h"
#include "ui_nav.h"
#include "ui_lists.h"
#include "ipod_lcd.h"
#include "lcd_layout.h"

static short gFailures;

/* ---- The log ------------------------------------------------------------ */

static void Log(const char *s)
{
    short ref;
    long n = strlen(s), one = 1;
    char nl = '\n';

    HCreate(0, 0, (ConstStringPtr)"\pout", 'ttxt', 'TEXT');
    if (OpenDataFork(0, 0, (ConstStringPtr)"\pout", fsRdWrPerm, &ref) != noErr)
        return;
    SetFPos(ref, fsFromLEOF, 0);
    FSWrite(ref, &n, s);
    FSWrite(ref, &one, &nl);
    FSClose(ref);
}

static void AppendNum(char *line, long v)
{
    Str255 s;

    NumToString(v, s);
    strncat(line, (const char *)s + 1, s[0]);
}

static void Check(Boolean ok, const char *what)
{
    char line[200];

    strcpy(line, ok ? "  ok   " : "  FAIL ");
    strcat(line, what);
    Log(line);
    if (!ok)
        gFailures++;
}

/* A check of a number, logging the value when it fails. */
static void CheckNum(Boolean ok, const char *what, long value)
{
    char line[200];

    strcpy(line, what);
    if (!ok)
    {
        strcat(line, " (got ");
        AppendNum(line, value);
        strcat(line, ")");
    }
    Check(ok, line);
}

static Boolean Same(ConstStringPtr p, const char *c)
{
    return p != nil && p[0] == strlen(c) && memcmp(p + 1, c, p[0]) == 0;
}

/* ---- The library -------------------------------------------------------- */

#define kSongSamples    (6L * 22050)        /* 6 s */

static unsigned char gBuf[4096];

static void SetText(Str63 out, const char *c)
{
    out[0] = (unsigned char)strlen(c);
    memcpy(out + 1, c, out[0]);
}

/* A song file as Import makes it: a quiet 6 s WAV and its details. */
static void MakeSong(const LibraryFolder *f, short id, const char *title,
                     const char *artist, const char *album, short track,
                     short flags)
{
    Str63 name;
    SongInfo in;
    short ref;
    long c, left, i;
    unsigned char h[44];
    long n = kSongSamples;

    name[0] = 8;
    memcpy(name + 1, "IPOD000", 7);
    name[8] = (unsigned char)('0' + id);
    HCreate(f->vRefNum, f->folderID, name, kAppCreator, kSongFileType);
    if (OpenDataFork(f->vRefNum, f->folderID, name, fsRdWrPerm, &ref) != noErr)
        return;
    memcpy(h, "RIFF\0\0\0\0WAVEfmt \20\0\0\0\1\0\1\0\x22\x56\0\0\x22\x56\0\0"
              "\1\0\x08\0data", 40);
    h[40] = n; h[41] = n >> 8; h[42] = n >> 16; h[43] = 0;
    c = 36 + n;
    h[4] = c; h[5] = c >> 8; h[6] = c >> 16; h[7] = 0;
    c = 44;
    FSWrite(ref, &c, h);
    for (i = 0; i < (long)sizeof gBuf; i++)     /* a soft hum, id-pitched */
        gBuf[i] = (unsigned char)(0x80 + ((i * id / 4) & 0x0F) - 8);
    for (left = n; left > 0; left -= c)
    {
        c = left < (long)sizeof gBuf ? left : (long)sizeof gBuf;
        FSWrite(ref, &c, gBuf);
    }
    memset(&in, 0, sizeof in);
    in.id = id;
    in.flags = flags;
    in.track = track;
    in.seconds = (n + 11025) / 22050;
    in.sampleRate = kSongSampleRate;    /* 1.2: the rate is now stored */
    in.audioOffset = 44;
    in.audioLength = n;
    in.originalLength = 44 + n;
    in.originalRIFFSize = 36 + n;
    SetText(in.text[0], title);
    SetText(in.text[1], artist);
    SetText(in.text[2], album);
    SetText(in.text[5], "smoke.wav");
    SongFile_AddChunk(ref, &in);
    FSClose(ref);
}

static long MakeFolder(short vRefNum, long parent, ConstStringPtr name)
{
    HParamBlockRec pb;

    memset(&pb, 0, sizeof pb);
    pb.fileParam.ioNamePtr = (StringPtr)name;
    pb.fileParam.ioVRefNum = vRefNum;
    pb.fileParam.ioDirID = parent;
    PBDirCreateSync(&pb);
    return pb.fileParam.ioDirID;
}

/* Songs by title: Five, Four, One, Six, Three, Two; an audiobook. */
static void MakeLibrary(void)
{
    WDPBRec wd;
    Str63 volume;
    short v;
    LibraryFolder f;

    GetVol(volume, &v);
    memset(&wd, 0, sizeof wd);
    wd.ioVRefNum = v;
    PBGetWDInfoSync(&wd);
    f.vRefNum = wd.ioWDVRefNum;
    f.folderID = MakeFolder(f.vRefNum, wd.ioWDDirID, (ConstStringPtr)"\pSmoke Library");
    f.playlistsID = MakeFolder(f.vRefNum, f.folderID, (ConstStringPtr)"\pPlaylists");
    MakeSong(&f, 1, "One", "Artist A", "Alpha", 1, 0);
    MakeSong(&f, 2, "Two", "Artist A", "Alpha", 2, 0);
    MakeSong(&f, 3, "Three", "Artist A", "Alpha", 3, 0);
    MakeSong(&f, 4, "Four", "Artist B", "Beta", 1, 0);
    MakeSong(&f, 5, "Five", "Artist B", "Beta", 2, 0);
    MakeSong(&f, 6, "Six", "Artist C", "", 0, 0);
    MakeSong(&f, 7, "Seven Chapters", "Reader", "", 0, kSongFlagAudiobook);
    LibSetup_TestUseFolder(&f);
    LibSetup_Rescan();
}

/* ---- Driving the iPod --------------------------------------------------- */

static void Send(UIEvent e)
{
    if (UI_HandleEvent(e))
        UI_Render();
}

/* Runs the main loop's playback work for ticks (redrawing as it would). */
static void Wait(long ticks)
{
    unsigned long until = TickCount() + ticks;

    while (TickCount() < until)
        if (UI_Tick())
            UI_Render();
}

static const Screen *Here(void)
{
    return UI_TestScreen();
}

static short RowNamed(const char *title)
{
    const Screen *s = Here();
    short i;

    for (i = 0; i < s->count; i++)
        if (Same(UIList_ItemTitle(s, i), title))
            return i;
    return -1;
}

/* Turns the wheel to the row with this title and presses Select. */
static Boolean Choose(const char *title)
{
    short row = RowNamed(title);

    if (row < 0)
    {
        char line[120] = "  (no row named ";

        strcat(line, title);
        strcat(line, ")");
        Log(line);
        return false;
    }
    while (Here()->selected < row)
        Send(kEvtWheelDown);
    while (Here()->selected > row)
        Send(kEvtWheelUp);
    Send(kEvtSelect);
    return true;
}

/* Menu until the main menu shows. */
static void Home(void)
{
    short i;

    for (i = 0; i < 10 && !(Here()->kind == kScreenMenu && Same(Here()->title, "iPod")); i++)
        Send(kEvtMenu);
}

static Boolean Playing(const char *title)
{
    return Playback_State() == kPlayPlaying
        && Playback_CurrentSong() != kNone
        && Same(Library_SongTitle(Playback_CurrentSong()), title);
}

/* Black pixels in an LCD rectangle. */
static short BlackPixels(short left, short top, short right, short bottom)
{
    const BitMap *b = LCD_TestBits();
    short x, y, n = 0;

    for (y = top; y < bottom; y++)
        for (x = left; x < right; x++)
            if (((const unsigned char *)b->baseAddr)[(long)y * b->rowBytes + (x >> 3)]
                & (0x80 >> (x & 7)))
                n++;
    return n;
}

static short StatusIconPixels(void)
{
    UI_Render();
    return BlackPixels(kStatusIconRight - 2 * kStatusIconWidth - kStatusIconGap,
                       kStatusIconTop, kStatusIconRight,
                       kStatusIconTop + kStatusIconHeight);
}

/* ---- The tour ----------------------------------------------------------- */

void Smoke_Run(void)
{
    char line[120];
    long e1, e2, length;
    short level, song;
    unsigned char saved[kMaxSettings];
    unsigned long started = TickCount();

    Log("MacMusic smoke test");
    Settings_ResetAll();
    Volume_Set(3);
    MakeLibrary();
    UI_Refresh();
    UI_Render();

    Log("library");
    CheckNum(Library_SongCount() == 7, "7 songs scanned", Library_SongCount());
    CheckNum(Library_SkippedCount() == 0, "none skipped", Library_SkippedCount());

    Log("browsing");
    Check(Here()->kind == kScreenMenu && Same(Here()->title, "iPod"), "the main menu first");
    CheckNum(Here()->count == 5, "five items, no Now Playing", Here()->count);
    Check(Choose("Music") && Same(Here()->title, "Music"), "Music");
    Check(Choose("Songs") && Here()->kind == kScreenSongs, "Songs");
    CheckNum(Here()->count == 6, "six songs, the audiobook apart", Here()->count);
    Check(Same(UIList_ItemTitle(Here(), 0), "Five")
          && Same(UIList_ItemTitle(Here(), 5), "Two"), "in title order");
    Send(kEvtMenu);
    Check(Choose("Albums") && Here()->count == 3 && RowNamed("All") == 0
          && RowNamed("Alpha") == 1 && RowNamed("Beta") == 2,
          "All, then the two albums (D9)");
    Check(Choose("Alpha") && Same(UIList_ItemTitle(Here(), 0), "One")
          && Same(UIList_ItemTitle(Here(), 2), "Three"), "an album in track order");
    Send(kEvtMenu);
    Send(kEvtMenu);
    Check(Choose("Audiobooks") && Here()->count == 1, "one audiobook");
    Send(kEvtMenu);

    Log("playing");
    Choose("Songs");
    Check(Choose("One") && Here()->kind == kScreenNowPlaying, "Select plays, Now Playing shows");
    Check(Playing("One"), "One is playing");
    CheckNum(Playback_QueueLength() == 6 && Playback_Position() == 2,
             "the Songs list queued, from One", Playback_Position());
    Wait(90);
    CheckNum(Playback_ElapsedTicks() >= 50, "the time moves", Playback_ElapsedTicks());

    Send(kEvtPlayPause);
    Check(Playback_State() == kPlayPaused, "Play/Pause pauses");
    e1 = Playback_ElapsedTicks();
    Wait(30);
    CheckNum(Playback_ElapsedTicks() - e1 <= 2, "paused: the time stands", Playback_ElapsedTicks() - e1);
    Send(kEvtPlayPause);
    Check(Playback_State() == kPlayPlaying, "and resumes");

    Send(kEvtNext);
    Check(Playing("Six"), "Next: the next song in the list");
    Send(kEvtPrevious);
    Check(Playing("One"), "Previous at the start: the song before");

    length = Playback_LengthTicks();
    CheckNum(length >= 355 && length <= 365, "the song is 6 s", length);
    Playback_Seek(length - 20);
    Wait(90);
    Check(Playing("Six"), "at its end, the next song plays on");

    Log("Now Playing modes");
    Send(kEvtSelect);
    CheckNum(UI_NowPlayingMode() == kNowScrubber, "Select: the scrubber", UI_NowPlayingMode());
    Playback_Seek(0);
    Wait(10);
    e1 = Playback_ElapsedTicks();
    Send(kEvtArrowDown);
    Send(kEvtArrowDown);
    Send(kEvtArrowDown);
    Wait(90);
    CheckNum(Playback_ElapsedTicks() >= e1 + 3 * length / 24 - 5,
             "scrubbing moves the song on", Playback_ElapsedTicks() - e1);
    Send(kEvtSelect);
    CheckNum(UI_NowPlayingMode() == kNowRating, "Select again: the rating", UI_NowPlayingMode());
    song = Playback_CurrentSong();
    Send(kEvtWheelDown);
    Send(kEvtWheelDown);
    Send(kEvtWheelDown);
    Send(kEvtSelect);
    CheckNum(UI_NowPlayingMode() == kNowNormal, "and again: back to normal", UI_NowPlayingMode());
    CheckNum(Library_SongRating(song) == 3, "three stars saved", Library_SongRating(song));

    level = Volume_Get();
    Send(kEvtArrowDown);
    CheckNum(UI_NowPlayingMode() == kNowVolume, "an arrow: the volume", UI_NowPlayingMode());
    CheckNum(Volume_Get() == level + 1, "one level up", Volume_Get());
    Send(kEvtArrowUp);
    Send(kEvtArrowUp);
    CheckNum(Volume_Get() == level - 1, "two levels down", Volume_Get());
    UI_SaveSettings();
    CheckNum(Prefs_LoadVolume() == level - 1, "the volume saved", Prefs_LoadVolume());
    Wait(200);                                  /* the volume bar times out */
    CheckNum(UI_NowPlayingMode() == kNowNormal, "the volume bar goes", UI_NowPlayingMode());

    Log("fast-forward and rewind");
    Playback_Seek(0);
    Wait(10);
    e1 = Playback_ElapsedTicks();
    Send(kEvtHoldNext);
    Wait(60);
    Send(kEvtHoldEnd);
    Wait(10);
    e2 = Playback_ElapsedTicks();
    CheckNum(e2 - e1 >= 150, "holding Next: about 4x", e2 - e1);
    Check(Playback_State() == kPlayPlaying && Playback_CurrentSong() == song,
          "still playing the same song");
    Send(kEvtHoldPrevious);
    Wait(30);
    Send(kEvtHoldEnd);
    Wait(5);
    CheckNum(Playback_ElapsedTicks() < e2, "holding Previous goes back", Playback_ElapsedTicks());

    Log("the main menu while playing");
    Home();
    CheckNum(Here()->count == 6 && Same(UIList_ItemTitle(Here(), 5), "Now Playing"),
             "Now Playing is listed", Here()->count);
    Check(Choose("Now Playing") && Here()->kind == kScreenNowPlaying, "and opens");
    Home();

    Log("Settings");
    Choose("Settings");
    Check(Choose("About") && Here()->kind == kScreenAbout, "About");
    Check(Same(UIList_ItemValue(Here(), 0), "7 of 500"), "About: 7 of 500 songs");
    Check(EqualString(UIList_ItemValue(Here(), 2), (ConstStringPtr)kMacMusicVersionP,
                      true, true),
          "About: the version, as CMakeLists.txt sets it");
    Send(kEvtMenu);
    Choose("Shuffle");
    Check(Same(UIList_ItemValue(Here(), RowNamed("Shuffle")), "Songs"), "Shuffle: Songs");
    Choose("Repeat");
    Choose("Repeat");
    Check(Same(UIList_ItemValue(Here(), RowNamed("Repeat")), "All"), "Repeat: All");
    Prefs_LoadSettings(saved, kSettingCount);
    Check(saved[kSettingShuffle] == kShuffleSongs && saved[kSettingRepeat] == kRepeatAll,
          "both saved in the prefs");
    Check(Choose("Legal") && Here()->kind == kScreenLegal, "Legal");
    CheckNum(Legal_LineCount() > 20, "the legal text is there", Legal_LineCount());
    Send(kEvtWheelDown);
    Send(kEvtWheelDown);
    CheckNum(Here()->top > 0, "and scrolls", Here()->top);
    Send(kEvtMenu);

    Log("Shuffle and Repeat");
    Home();
    Choose("Music");
    Choose("Songs");
    Check(Choose("One") && Playing("One"), "a song row plays that song first");
    CheckNum(Playback_Position() == 0, "at the top of the shuffled queue", Playback_Position());
    Check(Playback_IsShuffled(), "shuffled");
    CheckNum(StatusIconPixels() > 20, "the shuffle and repeat icons show", StatusIconPixels());
    Settings_Cycle(kSettingRepeat);             /* All -> Off -> One */
    Settings_Cycle(kSettingRepeat);
    song = Playback_CurrentSong();
    Playback_Seek(Playback_LengthTicks() - 20);
    Wait(90);
    Check(Playback_CurrentSong() == song && Playback_State() == kPlayPlaying,
          "Repeat One: the song plays again");

    Log("Reset All Settings");
    Home();
    Choose("Settings");
    Choose("Reset All Settings");
    Check(Same(UIList_ItemTitle(Here(), 0), "Cancel"), "Cancel comes first");
    Choose("Reset");
    Check(Settings_Get(kSettingShuffle) == kShuffleOff
          && Settings_Get(kSettingRepeat) == kRepeatOff, "Shuffle and Repeat off");
    Check(Same(Here()->title, "Settings"), "back on Settings");
    Prefs_LoadSettings(saved, kSettingCount);
    Check(saved[kSettingShuffle] == kShuffleOff && saved[kSettingRepeat] == kRepeatOff,
          "saved");

    Log("Shuffle Songs and Backlight");
    Home();
    Check(Choose("Shuffle Songs") && Playback_State() == kPlayPlaying, "Shuffle Songs plays");
    CheckNum(Playback_QueueLength() == 6, "every song, no audiobook", Playback_QueueLength());
    Check(Playback_IsShuffled(), "shuffled");
    CheckNum(StatusIconPixels() > 10, "the shuffle icon shows", StatusIconPixels());
    Home();
    Choose("Backlight");
    Check(LCD_IsInverted(), "Backlight inverts the screen");
    Send(kEvtSelect);
    Check(!LCD_IsInverted(), "and back");

    Log("Extras > Clock");
    Home();
    Check(Choose("Extras") && Same(Here()->title, "Extras") && Here()->count == 1,
          "Extras holds one item");
    Check(Choose("Clock") && Here()->kind == kScreenClock && Same(Here()->title, "Clock"),
          "the Clock opens");
    {
        Str255 time, date, expectTime, expectDate;
        unsigned long now;
        short tries;

        for (tries = 0; tries < 2; tries++)     /* again if a minute turned */
        {
            UI_Render();
            GetDateTime(&now);
            IUTimeString((long)now, false, expectTime);
            IUDateString((long)now, abbrevDate, expectDate);
            UI_TestClockText(time, date);
            if (EqualString(time, expectTime, true, true))
                break;
        }
        Check(EqualString(time, expectTime, true, true), "the system's time shown");
        Check(EqualString(date, expectDate, true, true), "and its date");
        strcpy(line, "  (");
        strncat(line, (const char *)time + 1, time[0]);
        strcat(line, ", ");
        strncat(line, (const char *)date + 1, date[0]);
        strcat(line, ")");
        Log(line);
    }
    CheckNum(BlackPixels(0, kClockTimeBaseline - 12, 160, kClockTimeBaseline + 3) > 30,
             "the time is drawn", BlackPixels(0, kClockTimeBaseline - 12, 160, kClockTimeBaseline + 3));
    CheckNum(BlackPixels(0, kClockDateBaseline - 12, 160, kClockDateBaseline + 3) > 30,
             "the date is drawn", BlackPixels(0, kClockDateBaseline - 12, 160, kClockDateBaseline + 3));
    Send(kEvtSelect);
    Send(kEvtWheelDown);
    Check(Here()->kind == kScreenClock, "Select and the wheel do nothing there");
    Check(Playback_State() == kPlayPlaying, "the music plays on");
    Send(kEvtMenu);
    Check(Same(Here()->title, "Extras"), "Menu goes back to Extras");

    /* 1.1: the File menu's items that are always enabled (an enable
       flags mistake once dimmed Choose Library and Page Setup). */
    Log("the File menu");
    {
        MenuHandle file = GetMenuHandle(129);
        Check(file != nil && ((**file).enableFlags & (1L << 4)), "Choose Library... enabled");
        Check(file != nil && ((**file).enableFlags & (1L << 8)), "Page Setup... enabled");
        Check(file != nil && !((**file).enableFlags & (1L << 3)), "the first divider disabled");
        Check(file != nil && !((**file).enableFlags & (1L << 10)), "the divider before Quit disabled");
    }

    /* 1.1 (N11-24): the Printing segment is loaded only to print. */
    Log("printing segment");
    {
        Handle code;
        SetResLoad(false);
        code = Get1Resource('CODE', 3);
        SetResLoad(true);
        Check(code != nil, "a Printing segment (CODE 3)");
        Check(code != nil && *code == nil, "not in memory while nothing prints");
    }

    Log("stopping");
    Playback_Stop();
    Wait(5);
    Check(!Playback_IsActive(), "stopped");
    CheckNum(Audio_Underruns() == 0, "no underruns", Audio_Underruns());
    Check(!Playback_TakeFileError(), "every song file played");

    strcpy(line, "seconds: ");
    AppendNum(line, (long)((TickCount() - started) / 60));
    Log(line);
    strcpy(line, "FAILURES: ");
    AppendNum(line, gFailures);
    Log(line);
    Log("DONE");
}
