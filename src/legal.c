#include <Resources.h>
#include <Quickdraw.h>

#include "legal.h"
#include "ipod_lcd.h"
#include "lcd_draw.h"

#define kLegalText      128
#define kMaxLines       600

static Handle gText;
static long gLength;
static short gLineCount;
static short gLineStart[kMaxLines];
static unsigned char gLineLength[kMaxLines];
static Boolean gWrapped;

static void AddLine(long start, long length)
{
    if (gLineCount >= kMaxLines)
        return;
    gLineStart[gLineCount] = (short)start;
    gLineLength[gLineCount] = (unsigned char)(length < 255 ? length : 255);
    gLineCount++;
}

/* Breaks each paragraph (ended by a return) into lines no wider than a
   list row beside the scroll bar, at spaces where possible. Measured in
   the small font it is drawn in (Geneva 9), with the LCD as the port. */
static void Wrap(void)
{
    const unsigned char *t;             /* unsigned: Mac Roman above 127 */
    long at = 0, start, lastSpace, end;
    short maxWidth = LCDDraw_RowTextWidth(true);

    gText = GetResource('TEXT', kLegalText);
    if (gText == nil)
        return;
    HNoPurge(gText);
    gLength = GetHandleSize(gText);
    HLock(gText);
    t = (const unsigned char *)*gText;

    LCD_BeginDraw();
    LCDDraw_UseSmallFont(true);         /* measured as it is drawn */
    while (at < gLength)
    {
        end = at;
        while (end < gLength && t[end] != '\r')
            end++;
        if (end == at)
            AddLine(at, 0);                     /* an empty line */
        start = at;
        while (start < end)
        {
            long n = start;
            short width = 0;

            /* The LCD font isn't kerned, so a line's width is the sum of
               its characters' widths: one CharWidth call per character. */
            lastSpace = -1;
            while (n < end && width + CharWidth(t[n]) <= maxWidth)
            {
                width += CharWidth(t[n]);
                if (t[n] == ' ')
                    lastSpace = n;
                n++;
            }
            if (n < end && lastSpace > start)
                n = lastSpace;                  /* break at the last space */
            if (n == start)
                n = start + 1;                  /* a word too long: cut */
            AddLine(start, n - start);
            start = n;
            while (start < end && t[start] == ' ')
                start++;
        }
        at = end + 1;
    }
    LCDDraw_UseSmallFont(false);
    LCD_EndDraw();
    HUnlock(gText);
}

short Legal_LineCount(void)
{
    if (!gWrapped)
    {
        gWrapped = true;
        Wrap();
    }
    return gLineCount;
}

void Legal_Line(short n, Str255 out)
{
    out[0] = 0;
    if (gText == nil || n < 0 || n >= gLineCount)
        return;
    out[0] = gLineLength[n];
    BlockMove(*gText + gLineStart[n], out + 1, out[0]);
}
