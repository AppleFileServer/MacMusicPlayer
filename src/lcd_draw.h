#ifndef LCD_DRAW_H
#define LCD_DRAW_H

#include <Quickdraw.h>

/* LCD drawing primitives. Call only between LCD_BeginDraw/LCD_EndDraw;
   all coordinates are LCD pixels (see lcd_layout.h). */

void LCDDraw_Clear(void);

/* Header: centred Pascal-string title, battery icon, rule. */
void LCDDraw_Header(ConstStringPtr title);

/* One list row at visible position 0..kVisibleRows-1. */
void LCDDraw_Row(short position, ConstStringPtr text, Boolean selected,
                 Boolean hasArrow, Boolean hasScrollBar);

/* The start-up screen: the Apple logo centred on a blank LCD. */
void LCDDraw_Logo(void);

/* Small font (counter and times on Now Playing). C strings. */
short LCDDraw_SmallTextWidth(const char *text);
void LCDDraw_SmallText(short left, short top, const char *text);

void LCDDraw_PlayStateIcon(Boolean paused);
void LCDDraw_ProgressBar(long done, long total);

/* The whole Now Playing screen; position is 0-based. */
void LCDDraw_NowPlaying(ConstStringPtr title, ConstStringPtr artist,
                        ConstStringPtr album, short position, short count,
                        long elapsedTicks, long lengthTicks, Boolean paused);

/* Scroll bar for a list of total items whose first visible item is top. */
void LCDDraw_ScrollBar(short total, short top);

#endif
