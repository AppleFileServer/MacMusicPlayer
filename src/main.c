#include <Quickdraw.h>
#include <Windows.h>
#include <Events.h>
#include <Fonts.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Devices.h>
#include <ToolUtils.h>

#include "ipod_shell.h"
#include "ipod_lcd.h"
#include "libsetup.h"
#include "import.h"
#include "playlists.h"
#include "playback.h"
#include "audio.h"
#include "message.h"
#include "pstr.h"
#include "lcd_draw.h"
#include "ui_nav.h"
#include "input.h"

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
    kItemChooseLibrary = 2,
    kItemQuit = 4,

    kMenuEdit = 130,

    kMenuPlaylists = 131,

    kAboutAlert = 128
};

static WindowPtr gMainWindow;
static Boolean gQuit = false;
static unsigned long gLogoShownAt;

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
static void PlaylistsChanged(void);

static void ShowAboutBox(void)
{
    Alert(kAboutAlert, nil);
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
            else if (menuItem == kItemChooseLibrary)
                ChooseLibrary();
            else if (menuItem == kItemQuit)
                gQuit = true;
            break;

        case kMenuEdit:
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
}

/* Brings menus up to date with the library and the iPod's highlighted
   row; called just before a menu is pulled down or a key equivalent is
   looked up. */
static void AdjustMenus(void)
{
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

static void UpdateFileMenu(void)
{
    MenuHandle fileMenu = GetMenuHandle(kMenuFile);

    if (LibSetup_HasLibrary())
        EnableItem(fileMenu, kItemImport);
    else
        DisableItem(fileMenu, kItemImport);
}

/* An iPod start or restart: BeginRestart shows the Apple logo; the caller
   then (re)loads the library, during which alerts may appear; EndRestart
   keeps the logo up for at least kLogoTicks, discards clicks and keys
   made meanwhile, and shows the main menu. Rescanning invalidates list
   positions, so every restart returns to the main menu (D26). */
static void BeginRestart(void)
{
    LCD_BeginDraw();
    LCDDraw_Logo();
    LCD_EndDraw();
    RedrawWindowNow();
    gLogoShownAt = TickCount();
}

static void EndRestart(void)
{
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
static void ImportSongPausingAudio(void)
{
    Boolean wasPlaying = Playback_State() == kPlayPlaying;

    if (wasPlaying)
        Playback_TogglePause();
    ImportSong();
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
        Input_TrackWheel(zone, HandleUIEvent);
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
        HandleUIEvent(Input_FromKey(key, event->what == autoKey));
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
            HandleMenuChoice(MenuSelect(event->where));
            break;

        case inSysWindow:
            SystemClick(event, window);
            break;

        case inDrag:
            dragBounds = qd.screenBits.bounds;
            InsetRect(&dragBounds, 4, 4);
            DragWindow(window, event->where, &dragBounds);
            break;

        case inGoAway:
            if (TrackGoAway(window, event->where))
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

    InitToolbox();
    SetUpMenus();
    qd.randSeed = TickCount();      /* for Shuffle Songs */
    Audio_Init();                   /* if it fails, playback is silent (A8) */

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
        (ConstStringPtr)"\pMacMusicPlayer",
        true,
        rDocProc,
        (WindowPtr)-1,
        true,
        0
    );

    /* Start-up is an iPod start: the logo is shown while the library is
       found (asking for one if needed) and scanned. */
    BeginRestart();
    LibSetup_Startup();
    EndRestart();

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

    Audio_Shutdown();
    DisposeWindow(gMainWindow);
    LCD_Dispose();
    return 0;
}
