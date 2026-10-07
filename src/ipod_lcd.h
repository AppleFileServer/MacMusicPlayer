#ifndef IPOD_LCD_H
#define IPOD_LCD_H

#include <Quickdraw.h>
#include <Windows.h>
#include <Fonts.h>

/* The iPod LCD: 160x128, one iPod pixel = one Mac screen pixel. */
#define kLCDWidth       160
#define kLCDHeight      128

#define kLCDFont        systemFont      /* Chicago on System 6 */
#define kLCDFontSize    12

Boolean LCD_Init(void);
void LCD_Dispose(void);

/* Make the LCD the current port (LCD coordinates), then restore the
   previous port. Calls must be paired and not nested. */
void LCD_BeginDraw(void);
void LCD_EndDraw(void);

/* Copy the LCD into the window at destRect, 1:1. CopyToWindow expects the
   window to be the current port (update events); Present sets it itself. */
void LCD_CopyToWindow(WindowPtr window, const Rect *destRect);

/* Stage 9a (S-9): the main menu's Backlight: the LCD is shown inverted
   (black pixels white, white black), at no extra cost: the copy to the
   window uses notSrcCopy. */
void LCD_SetInverted(Boolean inverted);
Boolean LCD_IsInverted(void);
void LCD_Present(WindowPtr window, const Rect *destRect);

#endif
