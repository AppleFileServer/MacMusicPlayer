#include <Windows.h>
#include <Menus.h>

#include "modal.h"

#define kMenuHook       0x0A30      /* low memory: called during MenuSelect */
#define kDragHook       0x09F6      /* ... during DragWindow, TrackGoAway */
#define kReturnKey      0x0D
#define kEnterKey       0x03

static void (*gService)(void);

void Modal_SetService(void (*service)(void))
{
    gService = service;
}

void Modal_Service(void)
{
    if (gService != nil)
        gService();
}

pascal Boolean Modal_DialogFilter(DialogPtr dialog, EventRecord *event,
                                  short *item)
{
    char key;

    (void)dialog;
    Modal_Service();
    if (event->what == keyDown || event->what == autoKey)
    {
        key = event->message & charCodeMask;
        if (key == kReturnKey || key == kEnterKey)
        {
            *item = 1;
            return true;
        }
    }
    return false;
}

pascal Boolean Modal_FileFilter(DialogPtr dialog, EventRecord *event,
                                short *item)
{
    (void)dialog;
    (void)event;
    (void)item;
    Modal_Service();
    return false;
}

/* The Toolbox calls MenuHook and DragHook directly. This stub saves every
   register around the C service, so nothing the Toolbox relies on can be
   disturbed. */
void Modal_HookStub(void);
__asm__(
    "    .text\n"
    "    .globl Modal_HookStub\n"
    "Modal_HookStub:\n"
    "    movem.l %d0-%d7/%a0-%a6,-(%sp)\n"
    "    jsr Modal_Service\n"
    "    movem.l (%sp)+,%d0-%d7/%a0-%a6\n"
    "    rts\n");

long Modal_MenuSelect(Point where)
{
    long saved = *(long *)kMenuHook, choice;

    *(long *)kMenuHook = (long)Modal_HookStub;
    choice = MenuSelect(where);
    *(long *)kMenuHook = saved;
    return choice;
}

void Modal_DragWindow(WindowPtr window, Point where, const Rect *bounds)
{
    long saved = *(long *)kDragHook;

    *(long *)kDragHook = (long)Modal_HookStub;
    DragWindow(window, where, bounds);
    *(long *)kDragHook = saved;
}

Boolean Modal_TrackGoAway(WindowPtr window, Point where)
{
    long saved = *(long *)kDragHook;
    Boolean inside;

    *(long *)kDragHook = (long)Modal_HookStub;
    inside = TrackGoAway(window, where);
    *(long *)kDragHook = saved;
    return inside;
}
