#ifndef UI_NAV_H
#define UI_NAV_H

#include <MacTypes.h>

/* Abstract iPod input events (Stage 1 subset). */
typedef enum
{
    kEvtNone = 0,
    kEvtWheelUp,        /* counter-clockwise: highlight moves up */
    kEvtWheelDown,      /* clockwise: highlight moves down */
    kEvtSelect,
    kEvtMenu
} UIEvent;

void UI_Init(void);

/* Returns true if the LCD needs redrawing. */
Boolean UI_HandleEvent(UIEvent event);

/* Draws the current screen into the offscreen LCD. */
void UI_Render(void);

/* The song highlighted on the iPod, or kNone if the highlighted row is not
   a song (DECISION P7: only song rows can be added to playlists). */
short UI_SelectedSong(void);

/* Rebuilds every screen on the navigation stack from the library (after a
   playlist change), keeping each highlight on the same item where it
   still exists. Does not draw. */
void UI_Refresh(void);

#endif
