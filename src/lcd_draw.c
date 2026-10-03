#include <Memory.h>

#include "lcd_draw.h"
#include "lcd_layout.h"
#include "ipod_lcd.h"

/* 1-bit icon bitmaps transcribed from the measured screenshot pixel maps.
   Leftmost pixel = most significant bit; rows are padded to whole words. */

static const unsigned short kArrowBits[kArrowHeight] = {
    0x4000, 0xE000, 0x7000, 0x3800, 0x1C00, 0x3800, 0x7000, 0xE000, 0x4000
};

static const unsigned short kBatteryBits[kBatteryHeight * 2] = {
    0xFFFF, 0xE000,
    0x8000, 0x2000,
    0xBFFF, 0xA000,
    0xBFFF, 0xB000,
    0xBFFF, 0x9000,
    0xBFFF, 0x9000,
    0xBFFF, 0xB000,
    0xBFFF, 0xA000,
    0x8000, 0x2000,
    0xFFFF, 0xE000
};

/* Apple logo shown while the iPod starts (manual pp. 15, 49: "until the
   Apple logo appears"). PROVISIONAL (D32): its size and exact shape on the
   4G LCD are not in any screenshot; this 30x34 silhouette is centred. */
#define kLogoWidth      30
#define kLogoHeight     34

static const unsigned short kLogoBits[kLogoHeight * 2] = {
    0x0000, 0x3000,  0x0000, 0x7000,  0x0000, 0xF000,  0x0001, 0xF000,
    0x0003, 0xE000,  0x0003, 0xC000,  0x0003, 0x0000,  0x0000, 0x0000,
    0x0040, 0x0800,  0x03F8, 0x7F00,  0x0FFC, 0xFFC0,  0x1FFF, 0xFFE0,
    0x1FFF, 0xFFC0,  0x3FFF, 0xFF80,  0x7FFF, 0xFF80,  0x7FFF, 0xFF00,
    0x7FFF, 0xFF00,  0xFFFF, 0xFF00,  0xFFFF, 0xFF00,  0xFFFF, 0xFF80,
    0xFFFF, 0xFF80,  0xFFFF, 0xFFC0,  0xFFFF, 0xFFE0,  0xFFFF, 0xFFFC,
    0xFFFF, 0xFFFC,  0x7FFF, 0xFFF8,  0x7FFF, 0xFFF8,  0x7FFF, 0xFFF8,
    0x3FFF, 0xFFF0,  0x1FFF, 0xFFE0,  0x1FFF, 0xFFE0,  0x0FFF, 0xFFC0,
    0x03F8, 0x7F00,  0x0040, 0x0800
};

/* Copies an icon into the current (LCD) port: srcOr draws it black,
   srcBic draws it white. */
static void DrawIcon(const unsigned short *bits, short rowBytes,
                     short width, short height,
                     short left, short top, short mode)
{
    BitMap icon;
    Rect dest;

    icon.baseAddr = (Ptr)bits;
    icon.rowBytes = rowBytes;
    SetRect(&icon.bounds, 0, 0, width, height);
    SetRect(&dest, left, top, left + width, top + height);

    CopyBits(&icon, &qd.thePort->portBits, &icon.bounds, &dest, mode, nil);
}

/* Mac Roman ellipsis; Chicago 12 has the glyph. */
#define kEllipsis       ((unsigned char)0xC9)

/* Copies text into out, shortened with an ellipsis if it is wider than
   maxWidth in the current font (D10, D16). TruncString is not used: it
   is not available on every System 6 configuration. */
static void FitText(ConstStringPtr text, short maxWidth, Str255 out)
{
    short ellipsisWidth;

    BlockMove(text, out, text[0] + 1);
    if (StringWidth(out) <= maxWidth)
        return;

    ellipsisWidth = CharWidth(kEllipsis);
    while (out[0] > 0 && StringWidth(out) + ellipsisWidth > maxWidth)
        out[0]--;
    while (out[0] > 0 && out[out[0]] == ' ')
        out[0]--;

    out[++out[0]] = kEllipsis;
}

void LCDDraw_Clear(void)
{
    PenNormal();
    TextMode(srcOr);
    EraseRect(&qd.thePort->portRect);
}

void LCDDraw_Header(ConstStringPtr title)
{
    Str255 fitted;

    FitText(title, kHeaderTitleMaxWidth, fitted);
    MoveTo((kLCDWidth - StringWidth(fitted)) / 2, kHeaderBaseline);
    DrawString(fitted);

    DrawIcon(kBatteryBits, 4, kBatteryWidth, kBatteryHeight,
             kBatteryLeft, kBatteryTop, srcOr);

    MoveTo(0, kHeaderRuleY);
    LineTo(kLCDWidth - 1, kHeaderRuleY);
}

void LCDDraw_Row(short position, ConstStringPtr text, Boolean selected,
                 Boolean hasArrow, Boolean hasScrollBar)
{
    short top = kRowTop0 + position * kRowHeight;
    short iconMode = selected ? srcBic : srcOr;
    short arrowLeft = hasScrollBar ? kArrowLeftWithBar : kArrowLeft;
    short barRight = hasScrollBar ? kSelRightWithBar : kSelRight;
    short textRight = (hasArrow ? arrowLeft : barRight) - kTextRightGap;
    Str255 fitted;
    Rect bar;

    if (selected)
    {
        SetRect(&bar, 0, top, barRight, top + kRowHeight);
        PaintRect(&bar);
        TextMode(srcBic);
    }

    FitText(text, textRight - kTextLeft, fitted);
    MoveTo(kTextLeft, top + kRowBaseline);
    DrawString(fitted);
    TextMode(srcOr);

    if (hasArrow)
        DrawIcon(kArrowBits, 2, kArrowWidth, kArrowHeight,
                 arrowLeft, top + kArrowTopOffset, iconMode);
}

void LCDDraw_ScrollBar(short total, short top)
{
    Rect frame;
    Rect thumb;
    short track = kTrackBottom - kTrackTop;
    short thumbHeight;
    short thumbTop;

    SetRect(&frame, kScrollLeft, kScrollTop, kScrollRight, kScrollBottom);
    EraseRect(&frame);
    FrameRect(&frame);

    /* PROVISIONAL (spec unknown #11, decision D5): thumb height is
       proportional to visible/total, at least kThumbMinHeight. Apple's rule
       is unknown; this gives 92 px for the 7-item Music menu where Apple's
       screenshot measures 101 px. The mismatch is known and deliberate. */
    thumbHeight = (short)(((long)track * kVisibleRows + total / 2) / total);
    if (thumbHeight < kThumbMinHeight)
        thumbHeight = kThumbMinHeight;

    thumbTop = kTrackTop;
    if (total > kVisibleRows)
        thumbTop += (short)(((long)(track - thumbHeight) * top)
                            / (total - kVisibleRows));

    SetRect(&thumb, kThumbLeft, thumbTop, kThumbRight, thumbTop + thumbHeight);
    PaintRect(&thumb);
}

void LCDDraw_Logo(void)
{
    LCDDraw_Clear();
    DrawIcon(kLogoBits, 4, kLogoWidth, kLogoHeight,
             (kLCDWidth - kLogoWidth) / 2, (kLCDHeight - kLogoHeight) / 2,
             srcOr);
}
