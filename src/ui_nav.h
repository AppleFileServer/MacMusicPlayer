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

#endif
