#ifndef LCD_LAYOUT_H
#define LCD_LAYOUT_H

/* iPod LCD geometry in LCD pixels, measured from the 160x128 screenshots
   in Apple's 2004 4G iPod User's Guide (see "iPod 4G UI Specification").
   Absolute positions are +/-1 px; sizes and spacings are exact.
   Rect-style right/bottom values are exclusive, as in QuickDraw. */

/* Header */
#define kHeaderBaseline         12      /* title baseline; cap top y 4 */
#define kHeaderRuleY            17      /* 1-px rule, full width */

/* List rows */
#define kRowTop0                19      /* top of the first row */
#define kRowHeight              18
#define kVisibleRows            6
#define kTextLeft               5
#define kRowBaseline            12      /* baseline below row top */

/* PROVISIONAL (D16): text that does not fit ends with an ellipsis this
   far before the arrow, or before the bar's right edge, mirroring the
   left margin. The 4G's long-name handling is unknown (spec unknown #7). */
#define kTextRightGap           5

/* Header title must fit between the left edge and the battery, centred. */
#define kHeaderTitleMaxWidth    (2 * (kBatteryLeft - kTextRightGap - kLCDWidth / 2))

/* Selection bar right edge (exclusive) */
#define kSelRight               160     /* x 0-159 */
#define kSelRightWithBar        151     /* x 0-150, beside the scroll bar */

/* Submenu arrow, 6x9 */
#define kArrowWidth             6
#define kArrowHeight            9
#define kArrowLeft              149     /* x 149-154 */
#define kArrowLeftWithBar       140     /* x 140-145 */
#define kArrowTopOffset         5       /* below row top */

/* Stage 9b: the Legal screen's small text: Geneva 9 (Chicago has no
   9-point bitmap on System 6, and a scaled one is hard to read), 12-px
   lines, 9 of them below the header. PROVISIONAL: the 4G's Legal screen
   used a smaller font than its lists; its exact layout is unknown. */
#define kSmallLineTop0          20      /* top of the first line */
#define kSmallLineHeight        12
#define kSmallLines             9

/* Scroll bar: 1-px frame, 1-px gap, 5-px thumb */
#define kScrollLeft             151     /* frame x 151-159 */
#define kScrollRight            160
#define kScrollTop              17      /* frame top shares the header rule */
#define kScrollBottom           128     /* frame y 17-127 */
#define kThumbLeft              153     /* thumb x 153-157 */
#define kThumbRight             158
#define kTrackTop               19      /* thumb travel y 19-125 */
#define kTrackBottom            126
#define kThumbMinHeight         9

/* Battery icon, 20x10 */
#define kBatteryLeft            134     /* x 134-153 */
#define kBatteryTop             3       /* y 3-12 */
#define kBatteryWidth           20
#define kBatteryHeight          10

/* Now Playing (measured from Apple's Now Playing screenshot) */
#define kPlayIconLeft           9       /* 9x9 play/pause icon, x 9-17 */
#define kPlayIconTop            3       /* y 3-11 */
#define kPlayIconSize           9
#define kCounterLeft            10      /* "3 of 12", small font */
#define kCounterTop             24      /* y 24-30 */
#define kNowLineBaseline0       48      /* title; ink from y 39 */
#define kNowLineSpacing         18      /* artist 65, album 83 */
#define kNowLineMaxWidth        (kLCDWidth - 2 * kTextLeft)
#define kProgressLeft           8       /* bar x 8-150, y 99-107 */
#define kProgressRight          151     /* exclusive */
#define kProgressTop            99
#define kProgressBottom         108     /* exclusive */
#define kTimeTop                114     /* y 114-120 */
#define kElapsedLeft            10      /* "1:28" (its 1 sits at x 11) */
#define kRemainingRight         152     /* exclusive: "-0:56" ends at x 151 */

/* Small font (counter, times): 7 px tall glyphs. */
#define kSmallFontHeight        7

#endif
