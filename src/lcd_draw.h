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

/* Scroll bar for a list of total items whose first visible item is top. */
void LCDDraw_ScrollBar(short total, short top);

#endif
