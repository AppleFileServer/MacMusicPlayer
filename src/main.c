#include <Quickdraw.h>
#include <Windows.h>
#include <Events.h>
#include <Fonts.h>
#include <ToolUtils.h>

int main(void)
{
    WindowPtr window;
    EventRecord event;
    Rect windowRect;

    InitGraf(&qd.thePort);
    InitFonts();
    InitWindows();
    InitCursor();

    SetRect(&windowRect, 40, 40, 440, 240);

    window = NewWindow(
        nil,
        &windowRect,
        "\pMacMusicPlayer",
        true,
        documentProc,
        (WindowPtr)-1,
        true,
        0
    );

    SetPort(window);

    MoveTo(80, 100);
    DrawString("\pMacMusicPlayer");

    MoveTo(80, 125);
    DrawString("\pYour Macintosh music player");

    while (true)
    {
        if (WaitNextEvent(everyEvent, &event, 30, nil))
        {
            if (event.what == mouseDown)
            {
                Point where;
                WindowPtr clickedWindow;

                where = event.where;

                if (FindWindow(where, &clickedWindow) == inContent)
                {
                    SelectWindow(clickedWindow);
                }
            }
        }
    }

    return 0;
}
