#ifndef IPOD_SHELL_H
#define IPOD_SHELL_H

#include <Quickdraw.h>

#include "ipod_lcd.h"

/* iPod body geometry, sized to fit the 512x342 Macintosh Plus screen.
   All positions below are in body coordinates: (0,0) = body top-left. */
#define kBodyWidth          200
#define kBodyHeight         296
#define kBodyCornerRadius   28

/* Space between the window edge and the body. */
#define kShellMargin        2
#define kShellWidth         (kBodyWidth + 2 * kShellMargin)
#define kShellHeight        (kBodyHeight + 2 * kShellMargin)

/* LCD placement (its size comes from ipod_lcd.h). */
#define kLCDTop             10
#define kLCDLeft            ((kBodyWidth - kLCDWidth) / 2)
#define kBezelInset         4
#define kBezelCornerRadius  8

/* Click Wheel and centre Select button. */
#define kWheelDiameter      140
#define kWheelTop           150
#define kWheelLeft          ((kBodyWidth - kWheelDiameter) / 2)
#define kSelectDiameter     48

/* Draws the body, bezel, wheel and Select button into the current port,
   which must be the window whose content holds the shell. */
void DrawIPodShell(void);

/* The LCD rectangle in window coordinates (always kLCDWidth x kLCDHeight). */
void GetLCDRect(Rect *r);

#endif
