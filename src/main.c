#include <Quickdraw.h>
#include <Windows.h>
#include <Events.h>
#include <Fonts.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Devices.h>
#include <ToolUtils.h>

enum
{
    kMenuBar = 128,

    kMenuApple = 128,

    kMenuFile = 129,
    kItemQuit = 1,

    kMenuEdit = 130
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

static void HandleMenuChoice(long choice)
{
    short menuID = choice >> 16;
    short menuItem = choice & 0xFFFF;
    Str255 itemName;

    switch (menuID)
    {
        case kMenuApple:
            GetMenuItemText(GetMenuHandle(kMenuApple), menuItem, itemName);
            OpenDeskAcc(itemName);
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
    SetPort(window);
    EraseRect(&window->portRect);

    MoveTo(80, 100);
    DrawString("\pMacMusicPlayer");

    MoveTo(80, 125);
    DrawString("\pYour Macintosh music player");
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
            break;
    }
}

int main(void)
{
    EventRecord event;
    Rect windowRect;

    InitToolbox();
    SetUpMenus();

    SetRect(&windowRect, 40, 40, 440, 240);

    gMainWindow = NewWindow(
        nil,
        &windowRect,
        "\pMacMusicPlayer",
        true,
        documentProc,
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
                    if (event.modifiers & cmdKey)
                        HandleMenuChoice(MenuKey(event.message & charCodeMask));
                    break;

                case updateEvt:
                    HandleUpdate(&event);
                    break;
            }
        }
    }

    DisposeWindow(gMainWindow);
    return 0;
}
