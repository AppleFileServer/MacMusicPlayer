#include <Quickdraw.h>
#include <Windows.h>
#include <Events.h>
#include <Fonts.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Devices.h>
#include <ToolUtils.h>
#include <Memory.h>

#include "modal.h"
#include "ipod_shell.h"
#include "ipod_lcd.h"
#include "libsetup.h"
#include "import.h"
#include "export.h"
#include "library.h"
#include "playlists.h"
#include "playback.h"
#include "audio.h"
#include "message.h"
#include "pstr.h"
#include "version.h"
#ifdef MACMUSIC_SMOKE_TEST
#include "smoke.h"
#endif
#include "lcd_draw.h"
#include "print.h"
#include <SegLoad.h>
#include "ui_nav.h"
#include "input.h"
#include "prefs.h"
#include "volume.h"
#include "editsong.h"
#include "settings.h"

/* Top of the window's content area: below the 20-pixel menu bar
   and the rDocProc title bar. */
#define kWindowContentTop   39

/* DECISION (D32): the Apple logo stays up at least 1 second (60 ticks)
   whenever the iPod starts or restarts, however quickly the library
   loads. */
#define kLogoTicks          60

enum
{
    kMenuBar = 128,

    kMenuApple = 128,
    kItemAbout = 1,

    kMenuFile = 129,
    kItemImport = 1,            /* enabled only when a library is set */
    kItemExport = 2,            /* ... and the highlight is on a song */
    kItemChooseLibrary = 4,
    kItemRescan = 5,            /* Stage 8d-2; with a library */
    kItemLibraryInfo = 6,       /* Stage 8d-2; with a library */
    kItemPageSetup = 8,         /* 1.1 (N11-15) */
    kItemPrint = 9,             /* 1.1; with songs to print */
    kItemFirstRecent = 11,      /* Stage 8d-3: recent libraries, then a
                                   divider, then Quit; with none, Quit */

    kMenuEdit = 130,
    kItemEditSong = 8,          /* Stage 8c; items 1-6 are for DAs */

    kMenuPlaylists = 131,

    kAboutAlert = 128
};

static WindowPtr gMainWindow;
static Boolean gQuit = false;
static unsigned long gLogoShownAt;

/* Stage 6b-3 (DECISION S7, issue I10): the stack MultiFinder gives a 68000
   program is 8 KB (measured: ApplLimit to CurStackBase), and the app's
   deepest use was about 6.4 KB, a thin margin since interrupt routines use
   the same stack. Before anything is allocated, ApplLimit is lowered so
   the stack has 24 KB; the heap can't grow into it. Only ever lowered,
   and only if the heap hasn't already grown past the new limit. */
#define kStackSize          (24L * 1024)

static void ReserveStack(void)
{
    Ptr limit = LMGetCurStackBase() - kStackSize;

    if (limit < LMGetApplLimit() && limit > (Ptr)ApplicationZone()->bkLim)
        SetApplLimit(limit);
}

static void InitToolbox(void)
{
    InitGraf(&qd.thePort);
    InitFonts();
    InitWindows();
    InitMenus();
    TEInit();
    InitDialogs(nil);
    FlushEvents(everyEvent, 0);
    InitCursor();
}

static void SetUpMenus(void)
{
    SetMenuBar(GetNewMBar(kMenuBar));
    AppendResMenu(GetMenuHandle(kMenuApple), 'DRVR');
    DrawMenuBar();
}

static void ChooseLibrary(void);
static void ImportSong(void);
static void ImportSongPausingAudio(void);
static void ExportSongPausingAudio(void);
static void UpdateFileMenu(void);
static void PlaylistsChanged(void);
static void EditSong(void);
static void RescanLibrary(void);
static void UseRecentLibrary(short index);
static void PageSetup(void);
static void PrintLibrary(void);

/* Recent libraries listed in the File menu (Stage 8d-3). Quit follows
   them (after a divider), so its item number changes with them. */
static short gRecentShown;

static short QuitItem(void)
{
    return gRecentShown > 0 ? kItemFirstRecent + gRecentShown + 1
                            : kItemFirstRecent;
}

static void ShowAboutBox(void)
{
    ParamText(kMacMusicVersionP, P(""), P(""), P(""));
    Alert(kAboutAlert, Modal_DialogFilter);
}

static void HandleMenuChoice(long choice)
{
    short menuID = choice >> 16;
    short menuItem = choice & 0xFFFF;
    Str255 itemName;

    switch (menuID)
    {
        case kMenuApple:
            if (menuItem == kItemAbout)
            {
                ShowAboutBox();
            }
            else
            {
                GetMenuItemText(GetMenuHandle(kMenuApple), menuItem, itemName);
                OpenDeskAcc(itemName);
            }
            break;

        case kMenuFile:
            if (menuItem == kItemImport)
                ImportSongPausingAudio();
            else if (menuItem == kItemExport)
                ExportSongPausingAudio();
            else if (menuItem == kItemChooseLibrary)
                ChooseLibrary();
            else if (menuItem == kItemRescan)
                RescanLibrary();
            else if (menuItem == kItemLibraryInfo)
            {
                HiliteMenu(0);
                LibSetup_ShowInfo();
            }
            else if (menuItem == kItemPageSetup)
                PageSetup();
            else if (menuItem == kItemPrint)
                PrintLibrary();
            else if (menuItem == QuitItem())
                gQuit = true;
            else if (menuItem >= kItemFirstRecent
                     && menuItem < kItemFirstRecent + gRecentShown)
                UseRecentLibrary(menuItem - kItemFirstRecent);
            break;

        case kMenuEdit:
            if (menuItem == kItemEditSong)
                EditSong();
            else
                SystemEdit(menuItem - 1);
            break;

        case kMenuPlaylists:
            HiliteMenu(0);
            if (Playlists_HandleMenu(menuItem))
                PlaylistsChanged();
            break;
    }

    HiliteMenu(0);
}

static void DrawMainWindow(WindowPtr window)
{
    Rect lcdRect;

    SetPort(window);
    EraseRect(&window->portRect);

    DrawIPodShell();

    GetLCDRect(&lcdRect);
    LCD_CopyToWindow(window, &lcdRect);
}

/* Passes an iPod event to the UI and, if the screen changed, redraws the
   LCD and copies it straight to the window (no update event needed). */
static void PresentLCD(void)
{
    Rect lcdRect;

    GetLCDRect(&lcdRect);
    LCD_Present(gMainWindow, &lcdRect);
}

static void HandleUIEvent(UIEvent uiEvent)
{
    if (uiEvent == kEvtNone || !UI_HandleEvent(uiEvent))
        return;

    UI_Render();
    PresentLCD();
}

/* The sink for wheel gestures (Input_TrackWheel): the steps of one mouse
   sample are drawn together when it sends kEvtNone, so fine steps (the
   7c scrubber, 120 a turn) don't each cost a redraw. kEvtNone also gives
   the UI its timed work while the button is held. */
static Boolean gWheelRedraw;

static void HandleWheelEvent(UIEvent uiEvent)
{
    if (uiEvent != kEvtNone)
    {
        if (UI_HandleEvent(uiEvent))
            gWheelRedraw = true;
        return;
    }
    if (UI_Idle())
        gWheelRedraw = true;
    if (!gWheelRedraw)
        return;
    gWheelRedraw = false;
    UI_Render();
    PresentLCD();
}

/* Draws the whole window now (shell and LCD) instead of waiting for an
   update event: used while no events are being handled. */
static void RedrawWindowNow(void)
{
    SetPort(gMainWindow);
    DrawMainWindow(gMainWindow);
    ValidRect(&gMainWindow->portRect);
}

/* DECISION A8: without sound, one alert per session, then silent
   playback on the clock. DECISION R8: a song whose file can't be read is
   skipped, with a message. */
static void ReportPlaybackProblems(void)
{
    static Boolean toldNoSound = false;

    if (!toldNoSound && Playback_IsActive() && !Audio_IsAvailable())
    {
        toldNoSound = true;
        Message_Show(P("Sound playback isn\xD5t available; songs will play "
                       "silently."), noErr);
    }
    if (Playback_TakeFileError())
        Message_Show(P("A song\xD5s file couldn\xD5t be read, so it was "
                       "skipped."), noErr);
    if (UI_TakeRatingError())
        Message_Show(P("The song\xD5s rating couldn\xD5t be saved."), noErr);
}

/* Brings menus up to date with the library and the iPod's highlighted
   row; called just before a menu is pulled down or a key equivalent is
   looked up. */
static void AdjustMenus(void)
{
    MenuHandle editMenu = GetMenuHandle(kMenuEdit);

    UpdateFileMenu();
    if (LibSetup_HasLibrary() && UI_SongToEdit() != kNone)
        EnableItem(editMenu, kItemEditSong);
    else
        DisableItem(editMenu, kItemEditSong);
    Playlists_AdjustMenu(GetMenuHandle(kMenuPlaylists),
                         LibSetup_HasLibrary());
}

/* A playlist was created or added to: rebuild the iPod's screens (the
   current one may be the Playlists list or that playlist) and redraw.
   Not an iPod restart, so no logo (P11). */
static void PlaylistsChanged(void)
{
    UI_Refresh();
    UI_Render();
    PresentLCD();
}

/* Lists the recent libraries between Library Info... and Quit: the
   current one ticked, any whose disk isn't mounted dimmed. The items are
   set with SetMenuItemText so that characters such as ";" or "(" in a
   folder name are shown, not taken as menu commands. */
static void UpdateRecentLibraries(MenuHandle fileMenu)
{
    Str255 text;
    Boolean available, current;
    short i, n = LibSetup_RecentCount();

    while (CountMItems(fileMenu) >= kItemFirstRecent)
        DeleteMenuItem(fileMenu, kItemFirstRecent);
    for (i = 0; i < n; i++)
    {
        LibSetup_RecentItem(i, text, &available, &current);
        AppendMenu(fileMenu, P(" "));
        SetMenuItemText(fileMenu, kItemFirstRecent + i, text);
        if (!available)
            DisableItem(fileMenu, kItemFirstRecent + i);
        CheckItem(fileMenu, kItemFirstRecent + i, current);
    }
    if (n > 0)
        AppendMenu(fileMenu, P("(-"));
    AppendMenu(fileMenu, P("Quit/Q"));
    gRecentShown = n;
}

static void UpdateFileMenu(void)
{
    MenuHandle fileMenu = GetMenuHandle(kMenuFile);

    UpdateRecentLibraries(fileMenu);

    if (LibSetup_HasLibrary())
    {
        EnableItem(fileMenu, kItemImport);
        EnableItem(fileMenu, kItemRescan);
        EnableItem(fileMenu, kItemLibraryInfo);
    }
    else
    {
        DisableItem(fileMenu, kItemImport);
        DisableItem(fileMenu, kItemRescan);
        DisableItem(fileMenu, kItemLibraryInfo);
    }

    /* 1.1: Print Library needs songs to print. */
    if (LibSetup_HasLibrary() && Library_SongCount() > 0)
        EnableItem(fileMenu, kItemPrint);
    else
        DisableItem(fileMenu, kItemPrint);

    /* Plan item 3: Export acts only on a highlighted song. */
    if (LibSetup_HasLibrary() && UI_SelectedSong() != kNone)
        EnableItem(fileMenu, kItemExport);
    else
        DisableItem(fileMenu, kItemExport);
}

/* An iPod start or restart: BeginRestart shows the Apple logo; the caller
   then (re)loads the library, during which alerts may appear; EndRestart
   keeps the logo up for at least kLogoTicks, discards clicks and keys
   made meanwhile, and shows the main menu. Rescanning invalidates list
   positions, so every restart returns to the main menu (D26). */
/* 1.1 (N11-8, N11-29): the scan's notice, as it begins: "Loading the
   library..." under the logo (a start, an import, or a change of
   library), or across the foot of the screen (Rescan Library). */
static void ShowLoading(Boolean withLogo)
{
    LCD_BeginDraw();
    LCDDraw_Loading(P("Loading the library\xC9"), withLogo);
    LCD_EndDraw();
    PresentLCD();
    SetCursor(*GetCursor(watchCursor));
}

static void LoadingAtStart(void)        { ShowLoading(true); }
static void LoadingOverScreen(void)     { ShowLoading(false); }

static void BeginRestart(void)
{
    LCD_BeginDraw();
    LCDDraw_Logo();
    LCD_EndDraw();
    RedrawWindowNow();
    gLogoShownAt = TickCount();
    Library_SetReadingNotice(LoadingAtStart);
}

static void EndRestart(void)
{
    Library_SetReadingNotice(nil);
    InitCursor();
    RedrawWindowNow();          /* alerts may have covered the window */
    while (TickCount() - gLogoShownAt < kLogoTicks)
        ;
    FlushEvents(mDownMask | mUpMask | keyDownMask | keyUpMask | autoKeyMask, 0);

    UI_Init();
    UI_Render();
    PresentLCD();
    UpdateFileMenu();
}

/* File > Choose Library... */
static void ChooseLibrary(void)
{
    HiliteMenu(0);
    if (!LibSetup_ChooseLibrary())
        return;

    /* A different library: whatever was playing belongs to the old one. */
    Playback_Stop();
    BeginRestart();
    LibSetup_Rescan();
    EndRestart();
}

/* File > a recent library (Stage 8d-3): switches to it, as Choose
   Library... does. */
static void UseRecentLibrary(short index)
{
    HiliteMenu(0);
    if (!LibSetup_UseRecent(index))
        return;

    Playback_Stop();
    BeginRestart();
    LibSetup_Rescan();
    EndRestart();
}

/* File > Import Song... */
static void ImportSong(void)
{
    HiliteMenu(0);
    if (!LibSetup_HasLibrary() || !Import_Run(LibSetup_Folder()))
        return;

    /* Same library, rescanned: the queue (song IDs) carries on. */
    BeginRestart();
    LibSetup_Rescan();
    Playback_LibraryChanged();
    EndRestart();
}

/* File > Import Song...: DECISION A5, audio pauses for the import (its
   dialogs and disk copy would starve the ring) and resumes at the same
   song and sample afterwards, found again by its ID. */
/* 1.1, File > Page Setup... and Print Library... (Part C). The printing
   code is a segment of its own (N11-24): unloaded after each use, so it
   takes no memory between. The music pauses while it prints (N11-17):
   an ImageWriter keeps the Plus too busy to keep the sound fed, as
   Import does. */
static void PageSetup(void)
{
    HiliteMenu(0);
    Print_PageSetup();
    UnloadSeg((Ptr)Print_PageSetup);
    RedrawWindowNow();          /* the dialog may have covered the window */
}

static void PrintLibrary(void)
{
    Boolean wasPlaying = Playback_State() == kPlayPlaying;

    HiliteMenu(0);
    if (!LibSetup_HasLibrary() || Library_SongCount() == 0)
        return;
    if (wasPlaying)
        Playback_TogglePause();
    Print_Library();
    UnloadSeg((Ptr)Print_Library);
    if (wasPlaying && Playback_State() == kPlayPaused)
        Playback_TogglePause();
    RedrawWindowNow();
}

static void ImportSongPausingAudio(void)
{
    Boolean wasPlaying = Playback_State() == kPlayPlaying;

    if (wasPlaying)
        Playback_TogglePause();
    ImportSong();
    if (wasPlaying && Playback_State() == kPlayPaused)
        Playback_TogglePause();
}

/* Edit > Edit Song... (Stage 8c). Not an iPod restart: the screens are
   found again by name and stay where they were (L4); playback carries on.
   Only if the library can't be updated in memory is it rescanned, with
   the usual restart. */
static void EditSong(void)
{
    Boolean rescan;

    HiliteMenu(0);
    if (!LibSetup_HasLibrary())
        return;
    UI_BeginLibraryEdit();
    if (!EditSong_Run(UI_SongToEdit(), &rescan))
        return;
    if (rescan)
    {
        BeginRestart();
        LibSetup_Rescan();
        Playback_LibraryChanged();
        EndRestart();
        return;
    }
    UI_EndLibraryEdit();
    UI_Render();
    PresentLCD();
}

/* File > Rescan Library (Stage 8d-2): reads the library folder again,
   for files added or changed in the Finder. Not an iPod restart: the
   screens are found again by name and stay where they were (L4). The
   audio pauses during the scan, as for import (A5): a large library
   takes longer to read than the ring lasts. */
static void RescanLibrary(void)
{
    Boolean wasPlaying = Playback_State() == kPlayPlaying;

    HiliteMenu(0);
    if (!LibSetup_HasLibrary())
        return;
    if (wasPlaying)
        Playback_TogglePause();
    UI_BeginLibraryEdit();
    SetCursor(*GetCursor(watchCursor));
    Library_SetReadingNotice(LoadingOverScreen);
    LibSetup_RescanAll();
    Library_SetReadingNotice(nil);
    InitCursor();
    Playback_LibraryChanged();
    UI_EndLibraryEdit();
    if (wasPlaying && Playback_State() == kPlayPaused)
        Playback_TogglePause();
    RedrawWindowNow();          /* alerts may have covered the window */
    UI_Render();
    PresentLCD();
}

/* File > Export Song... */
static void ExportSong(void)
{
    HiliteMenu(0);
    if (!LibSetup_HasLibrary()
        || !Export_Run(LibSetup_Folder(), UI_SelectedSong()))
        return;

    /* The song has left the library. If it was the current song,
       playback moves on to the next (E7), as for any song that goes. */
    BeginRestart();
    LibSetup_Rescan();
    Playback_LibraryChanged();
    EndRestart();
}

/* File > Export Song...: audio pauses, as for import (A5), which also
   closes the current song's file before export changes it. */
static void ExportSongPausingAudio(void)
{
    Boolean wasPlaying = Playback_State() == kPlayPlaying;

    if (wasPlaying)
        Playback_TogglePause();
    ExportSong();
    if (wasPlaying && Playback_State() == kPlayPaused)
        Playback_TogglePause();
}

static void HandleContentClick(WindowPtr window, Point where)
{
    ShellZone zone;

    if (window != gMainWindow)
        return;

    SetPort(window);
    GlobalToLocal(&where);
    zone = IPodShell_HitTest(where);

    /* Select is a separate physical button: it acts on mouse-down and
       never starts a wheel gesture. The ring tracks rotation and only
       acts as a button press if it was not dragged. */
    if (zone == kZoneSelect)
        HandleUIEvent(kEvtSelect);
    else if (zone != kZoneNone)
        Input_TrackWheel(zone, HandleWheelEvent);
}

static void HandleKey(EventRecord *event)
{
    char key = event->message & charCodeMask;

    if (event->modifiers & cmdKey)
    {
        AdjustMenus();
        HandleMenuChoice(MenuKey(key));
        return;
    }

    if (FrontWindow() == gMainWindow)
        HandleUIEvent(Input_FromKey(key, event->what == autoKey,
                                    (event->modifiers & optionKey) != 0));
}

static void HandleUpdate(EventRecord *event)
{
    WindowPtr window = (WindowPtr)event->message;

    BeginUpdate(window);
    DrawMainWindow(window);
    EndUpdate(window);
}

static void HandleMouseDown(EventRecord *event)
{
    WindowPtr window;
    short part = FindWindow(event->where, &window);
    Rect dragBounds;

    switch (part)
    {
        case inMenuBar:
            AdjustMenus();
            HandleMenuChoice(Modal_MenuSelect(event->where));
            break;

        case inSysWindow:
            SystemClick(event, window);
            break;

        case inDrag:
            dragBounds = qd.screenBits.bounds;
            InsetRect(&dragBounds, 4, 4);
            Modal_DragWindow(window, event->where, &dragBounds);
            break;

        case inGoAway:
            if (Modal_TrackGoAway(window, event->where))
                gQuit = true;
            break;

        case inContent:
            if (window != FrontWindow())
                SelectWindow(window);
            else
                HandleContentClick(window, event->where);
            break;
    }
}

int main(void)
{
    EventRecord event;
    Rect windowRect;

    ReserveStack();                 /* first: before anything is allocated */
    InitToolbox();
    SetUpMenus();
    qd.randSeed = TickCount();      /* for Shuffle Songs */
    Audio_Init();                   /* if it fails, playback is silent (A8) */
    Modal_SetService(Playback_ServiceAudio);   /* 6b-2: fed in modal loops */
    Settings_Load();                /* Stage 9a: Shuffle, Repeat */
    {   /* 7b (N2): MacMusic opens at the volume chosen last time. */
        short level = Prefs_LoadVolume();

        if (level >= 0)
            Volume_Set(level);
    }

    if (!LCD_Init())
    {
        SysBeep(10);
        return 0;
    }

    windowRect.left = (qd.screenBits.bounds.right - kShellWidth) / 2;
    windowRect.top = kWindowContentTop;
    windowRect.right = windowRect.left + kShellWidth;
    windowRect.bottom = windowRect.top + kShellHeight;

    gMainWindow = NewWindow(
        nil,
        &windowRect,
        (ConstStringPtr)"\pMacMusic",
        true,
        rDocProc,
        (WindowPtr)-1,
        true,
        0
    );

    /* 1.1 (N11-28): under MultiFinder a program's windows come in front of
       the Finder's only once it asks for events, and the library is read
       before the event loop. A few calls to EventAvail first, as Apple
       advised for a program with work to do at start-up. */
    {
        short i;

        for (i = 0; i < 3; i++)
            EventAvail(everyEvent, &event);
    }

    /* Start-up is an iPod start: the logo is shown while the library is
       found (asking for one if needed) and scanned. */
    BeginRestart();
#ifdef MACMUSIC_SMOKE_TEST
    EndRestart();
    Smoke_Run();                    /* 1.0 Part B2: the test, then quit */
    gQuit = true;
#else
    LibSetup_Startup();
    EndRestart();
#endif

    while (!gQuit)
    {
        /* While a song plays, come back every tick to keep the audio ring
           filled (R7) and the time updated. */
        if (WaitNextEvent(everyEvent, &event,
                          Playback_State() == kPlayPlaying ? 1 : 30, nil))
        {
            switch (event.what)
            {
                case mouseDown:
                    HandleMouseDown(&event);
                    break;

                case keyDown:
                case autoKey:
                    HandleKey(&event);
                    break;

                case updateEvt:
                    HandleUpdate(&event);
                    break;
            }
        }

        /* Playback clock: redraw when Now Playing's time or song changes,
           or when playback stops. */
        if (UI_Tick())
        {
            UI_Render();
            PresentLCD();
        }

        ReportPlaybackProblems();
    }

    UI_SaveSettings();
    Audio_Shutdown();
    DisposeWindow(gMainWindow);
    LCD_Dispose();
    return 0;
}
