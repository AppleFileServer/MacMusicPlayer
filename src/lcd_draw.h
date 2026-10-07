#ifndef LCD_DRAW_H
#define LCD_DRAW_H

#include <Quickdraw.h>

/* LCD drawing primitives. Call only between LCD_BeginDraw/LCD_EndDraw;
   all coordinates are LCD pixels (see lcd_layout.h). */

void LCDDraw_Clear(void);

/* Header: centred Pascal-string title, battery icon, rule. */
void LCDDraw_Header(ConstStringPtr title);

/* One list row at visible position 0..kVisibleRows-1. */
/* Stage 9a: value (nil for none) is drawn at the row's right edge, in
   place of an arrow, as on the 4G's Settings screen. */
/* Stage 9b: small text (Legal): sets the small font for measuring or
   drawing, and back to the LCD's own font; draws one line of it at
   visible position 0..kSmallLines-1. */
void LCDDraw_UseSmallFont(Boolean small);
void LCDDraw_SmallLine(short position, ConstStringPtr text);

/* A scroll bar for a view showing `visible` lines of `total`. */
void LCDDraw_ScrollBarFor(short total, short top, short visible);

/* Stage 9b: the width text can take in a list row with no arrow or
   value, beside the scroll bar or not. */
short LCDDraw_RowTextWidth(Boolean hasScrollBar);

void LCDDraw_Row(short position, ConstStringPtr text, ConstStringPtr value,
                 Boolean selected,
                 Boolean hasArrow, Boolean hasScrollBar);

/* The start-up screen: the Apple logo centred on a blank LCD. */
void LCDDraw_Logo(void);

/* Small font (counter and times on Now Playing). C strings. */
short LCDDraw_SmallTextWidth(const char *text);
void LCDDraw_SmallText(short left, short top, const char *text);

/* The icon at the top left of Now Playing (Stage 10a adds fast-forward
   and rewind, shown while Next or Previous is held). */
enum { kIconPlay = 0, kIconPause, kIconFastForward, kIconRewind };
void LCDDraw_PlayStateIcon(short icon);
void LCDDraw_ProgressBar(long done, long total);

/* Stage 7: what Now Playing shows in place of the progress bar. */
typedef enum
{
    kNowNormal = 0,     /* progress bar and times */
    kNowVolume,         /* volume bar (value = level 0-7); times hidden */
    kNowScrubber,       /* progress bar with a marker; times shown */
    kNowRating          /* five bullets/stars (value = rating 0-5) */
} NowPlayingMode;

/* The whole Now Playing screen; position is 0-based. value is the mode's
   volume level or rating. */
/* Stage 10c: Now Playing's Repeat states (settings.h's kRepeatOff,
   kRepeatOne, kRepeatAll, by value). */
void LCDDraw_NowPlaying(ConstStringPtr title, ConstStringPtr artist,
                        ConstStringPtr album, short position, short count,
                        long elapsedTicks, long lengthTicks, short icon,
                        Boolean shuffled, short repeat,
                        NowPlayingMode mode, short value);

/* Scroll bar for a list of total items whose first visible item is top. */
void LCDDraw_ScrollBar(short total, short top);

#endif
