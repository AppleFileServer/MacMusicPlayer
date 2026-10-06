#ifndef MODAL_H
#define MODAL_H

#include <Dialogs.h>
#include <Events.h>

/* Stage 6b-2: keeping the audio fed while the Toolbox runs a modal loop
   (a menu held down, a window dragged, a dialog, alert or Standard File
   dialog open). The main loop doesn't run then, so after about 6.6 s
   the ring would run out. Each loop calls the service set here,
   repeatedly, in the app's normal context (not at interrupt time): the
   hook test measured gaps of at most 16 ticks under MultiFinder at 1x. */

/* The routine to call from modal loops; it must not draw. */
void Modal_SetService(void (*service)(void));

/* Calls the service (for the app's own loops, such as the wheel drag). */
void Modal_Service(void);

/* Filter for ModalDialog and Alert: calls the service, and maps Return
   and Enter to item 1 (every dialog and alert here has its default button
   as item 1; with a filter the Toolbox no longer does this itself). */
pascal Boolean Modal_DialogFilter(DialogPtr dialog, EventRecord *event,
                                  short *item);

/* Filter for SFPGetFile/SFPPutFile: calls the service only, leaving
   Standard File's own keyboard handling alone. */
pascal Boolean Modal_FileFilter(DialogPtr dialog, EventRecord *event,
                                short *item);

/* MenuSelect, DragWindow and TrackGoAway with the service called
   throughout, through the MenuHook and DragHook low-memory hooks, set only
   for the duration of the call. */
long Modal_MenuSelect(Point where);
void Modal_DragWindow(WindowPtr window, Point where, const Rect *bounds);
Boolean Modal_TrackGoAway(WindowPtr window, Point where);

#endif
