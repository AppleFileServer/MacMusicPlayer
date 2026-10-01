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
void LCD_CopyToWindow(WindowPtr window, const Rect *destRect);

#endif
