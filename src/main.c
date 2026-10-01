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
#include "ui_nav.h"
#include "input.h"

/* Top of the window's content area: below the 20-pixel menu bar
   and the rDocProc title bar. */
#define kWindowContentTop   39

enum
{
    kMenuBar = 128,

    kMenuApple = 128,
    kItemAbout = 1,

    kMenuFile = 129,
    kItemQuit = 1,

    kMenuEdit = 130,

    kAboutAlert = 128
};

static WindowPtr gMainWindow;
static Boolean gQuit = false;

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
            if (menuItem == kItemQuit)
                gQuit = true;
            break;

        case kMenuEdit:
            SystemEdit(menuItem - 1);
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
static void HandleUIEvent(UIEvent uiEvent)
{
    Rect lcdRect;

    if (uiEvent == kEvtNone || !UI_HandleEvent(uiEvent))
        return;

    UI_Render();

    GetLCDRect(&lcdRect);
    LCD_Present(gMainWindow, &lcdRect);
}

static void HandleContentClick(WindowPtr window, Point where)
{
    if (window != gMainWindow)
        return;

    SetPort(window);
    GlobalToLocal(&where);
    HandleUIEvent(Input_FromShellZone(IPodShell_HitTest(where)));
}

static void HandleKey(EventRecord *event)
{
    char key = event->message & charCodeMask;

    if (event->modifiers & cmdKey)
    {
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

    if (!LCD_Init())
    {
        SysBeep(10);
        return 0;
    }

    UI_Init();
    UI_Render();

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

    while (!gQuit)
    {
        if (WaitNextEvent(everyEvent, &event, 30, nil))
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
    }

    DisposeWindow(gMainWindow);
    LCD_Dispose();
    return 0;
}
