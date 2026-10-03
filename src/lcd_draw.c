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

/* ---- Now Playing ------------------------------------------------------ */

/* Small font used on Now Playing for "3 of 12" and the times (F1). Glyphs
   marked exact are copied from Apple's 4G Now Playing screenshot; 4, 7
   and 9 do not appear there and are PROVISIONAL designs in the same
   style. Digits sit in 6 px cells. (In the screenshot "56" is one pixel
   wider apart than the other digit pairs; this font keeps a regular
   advance.) */
typedef struct
{
    char c;
    short width;                        /* bitmap columns */
    short advance;                      /* to the next glyph's left edge */
    unsigned short rows[kSmallFontHeight];
} SmallGlyph;

static const SmallGlyph kSmallFont[] = {
    { '0', 5, 6, { 0x7000, 0x8800, 0x9800, 0xA800, 0xC800, 0x8800, 0x7000 } },  /* exact */
    { '1', 5, 6, { 0x2000, 0x6000, 0x2000, 0x2000, 0x2000, 0x2000, 0x2000 } },  /* exact */
    { '2', 5, 6, { 0x7000, 0x8800, 0x0800, 0x1000, 0x6000, 0x8000, 0xF800 } },  /* exact */
    { '3', 5, 6, { 0xF800, 0x3000, 0x7000, 0x0800, 0x0800, 0x8800, 0x7000 } },  /* exact */
    { '4', 5, 6, { 0x1000, 0x3000, 0x5000, 0x9000, 0xF800, 0x1000, 0x1000 } },  /* PROVISIONAL design */
    { '5', 5, 6, { 0xF800, 0x8000, 0xF000, 0x0800, 0x0800, 0x8800, 0x7000 } },  /* exact */
    { '6', 5, 6, { 0x3800, 0x4000, 0xF000, 0x8800, 0x8800, 0x8800, 0x7000 } },  /* exact */
    { '7', 5, 6, { 0xF800, 0x0800, 0x1000, 0x2000, 0x4000, 0x4000, 0x4000 } },  /* PROVISIONAL design */
    { '8', 5, 6, { 0x7000, 0x8800, 0x8800, 0x7000, 0x8800, 0x8800, 0x7000 } },  /* exact */
    { '9', 5, 6, { 0x7000, 0x8800, 0x8800, 0x8800, 0x7800, 0x1000, 0xE000 } },  /* PROVISIONAL design */
    { ':', 1, 2, { 0x0000, 0x8000, 0x0000, 0x0000, 0x0000, 0x0000, 0x8000 } },  /* exact */
    { '-', 4, 5, { 0x0000, 0x0000, 0x0000, 0x0000, 0xF000, 0x0000, 0x0000 } },  /* exact */
    { 'o', 4, 6, { 0x0000, 0x6000, 0x9000, 0x9000, 0x9000, 0x9000, 0x6000 } },  /* exact */
    { 'f', 4, 5, { 0x3000, 0x4000, 0xE000, 0x4000, 0x4000, 0x4000, 0x4000 } },  /* exact */
};

#define kSmallSpaceAdvance  3

static const SmallGlyph *FindGlyph(char c)
{
    short i;

    for (i = 0; i < (short)(sizeof(kSmallFont) / sizeof(kSmallFont[0])); i++)
        if (kSmallFont[i].c == c)
            return &kSmallFont[i];
    return nil;
}

short LCDDraw_SmallTextWidth(const char *text)
{
    const SmallGlyph *g;
    short width = 0;

    for (; *text != 0; text++)
    {
        g = FindGlyph(*text);
        width += g != nil ? g->advance : kSmallSpaceAdvance;
    }
    return width > 0 ? width - 1 : 0;   /* no spacing after the last */
}

void LCDDraw_SmallText(short left, short top, const char *text)
{
    const SmallGlyph *g;

    for (; *text != 0; text++)
    {
        g = FindGlyph(*text);
        if (g == nil)
        {
            left += kSmallSpaceAdvance;
            continue;
        }
        DrawIcon(g->rows, 2, g->width, kSmallFontHeight, left, top, srcOr);
        left += g->advance;
    }
}

/* Play: the screenshot's 9x9 triangle without its anti-aliased grey
   pixels. Pause: PROVISIONAL (N9) two 3 px bars with a 3 px gap. */
static const unsigned short kPlayIconBits[kPlayIconSize] = {
    0x8000, 0xE000, 0xF800, 0xFE00, 0xFF80, 0xFE00, 0xF800, 0xE000, 0x8000
};
static const unsigned short kPauseIconBits[kPlayIconSize] = {
    0xE380, 0xE380, 0xE380, 0xE380, 0xE380, 0xE380, 0xE380, 0xE380, 0xE380
};

void LCDDraw_PlayStateIcon(Boolean paused)
{
    DrawIcon(paused ? kPauseIconBits : kPlayIconBits, 2,
             kPlayIconSize, kPlayIconSize, kPlayIconLeft, kPlayIconTop, srcOr);
}

/* The rounded progress bar: 1 px outline whose ends round off over two
   pixels (as measured), filled black from the left for fraction
   done/total. */
void LCDDraw_ProgressBar(long done, long total)
{
    short left = kProgressLeft, right = kProgressRight - 1;
    short top = kProgressTop, bottom = kProgressBottom - 1;
    short fillRight, y, rowLeft, rowRight;
    Rect r;

    MoveTo(left + 2, top);      LineTo(right - 2, top);
    MoveTo(left + 2, bottom);   LineTo(right - 2, bottom);
    MoveTo(left + 1, top + 1);  LineTo(left + 1, top + 1);
    MoveTo(left + 1, bottom - 1); LineTo(left + 1, bottom - 1);
    MoveTo(right - 1, top + 1); LineTo(right - 1, top + 1);
    MoveTo(right - 1, bottom - 1); LineTo(right - 1, bottom - 1);
    MoveTo(left, top + 2);      LineTo(left, bottom - 2);
    MoveTo(right, top + 2);     LineTo(right, bottom - 2);

    if (total <= 0 || done <= 0)
        return;
    if (done > total)
        done = total;

    /* Last filled column, across the bar's full width. */
    fillRight = left + (short)(((long)(right - left + 1) * done) / total) - 1;

    for (y = top + 1; y <= bottom - 1; y++)
    {
        rowLeft = (y == top + 1 || y == bottom - 1) ? left + 1 : left;
        rowRight = (y == top + 1 || y == bottom - 1) ? right - 1 : right;
        if (fillRight < rowLeft)
            continue;
        SetRect(&r, rowLeft, y, (fillRight < rowRight ? fillRight : rowRight) + 1, y + 1);
        PaintRect(&r);
    }
}

/* Appends the decimal digits of n (>= 0) to a C string. */
static char *AppendNumber(char *out, long n)
{
    char digits[12];
    short count = 0;

    do
    {
        digits[count++] = (char)('0' + n % 10);
        n /= 10;
    } while (n > 0);
    while (count > 0)
        *out++ = digits[--count];
    *out = 0;
    return out;
}

/* "m:ss" */
static char *AppendTime(char *out, long seconds)
{
    out = AppendNumber(out, seconds / 60);
    *out++ = ':';
    if (seconds % 60 < 10)
        *out++ = '0';
    return AppendNumber(out, seconds % 60);
}

/* Ink measurement: StringWidth includes letter spacing that leaves no
   ink, varying with the first and last letters, so Now Playing lines are
   placed by their visible ink instead. A line is drawn once into a small
   scratch bitmap and its columns scanned. */
#define kScratchWidth       256             /* > kNowLineMaxWidth + margin */
#define kScratchHeight      20
#define kScratchPenX        16
#define kScratchBaseline    15

static unsigned short gScratchBits[kScratchHeight * kScratchWidth / 16];
static GrafPort gScratchPort;
static Boolean gScratchReady;

static void MeasureInk(ConstStringPtr text, short *inkLeft, short *inkWidth)
{
    GrafPtr savedPort;
    BitMap bits;
    short x, y, first = -1, last = -1;
    const unsigned short *row;

    GetPort(&savedPort);
    if (!gScratchReady)
    {
        bits.baseAddr = (Ptr)gScratchBits;
        bits.rowBytes = kScratchWidth / 8;
        SetRect(&bits.bounds, 0, 0, kScratchWidth, kScratchHeight);
        OpenPort(&gScratchPort);
        SetPortBits(&bits);
        PortSize(kScratchWidth, kScratchHeight);
        RectRgn(gScratchPort.visRgn, &gScratchPort.portRect);
        ClipRect(&gScratchPort.portRect);
        TextFont(kLCDFont);
        TextSize(kLCDFontSize);
        gScratchReady = true;
    }
    SetPort(&gScratchPort);
    EraseRect(&gScratchPort.portRect);
    MoveTo(kScratchPenX, kScratchBaseline);
    DrawString(text);
    SetPort(savedPort);

    for (x = 0; x < kScratchWidth; x++)
        for (y = 0; y < kScratchHeight; y++)
        {
            row = gScratchBits + y * (kScratchWidth / 16);
            if (row[x >> 4] & (0x8000 >> (x & 15)))
            {
                if (first < 0)
                    first = x;
                last = x;
                break;
            }
        }

    *inkLeft = first < 0 ? 0 : first - kScratchPenX;
    *inkWidth = first < 0 ? 0 : last - first + 1;
}

/* Measured from Apple's Now Playing screenshot: each line's ink starts at
   (162 - ink width) / 2, rounded down. (The header uses the plain
   StringWidth centring of the list screens, which matched there.) */
static void DrawCentredLine(short baseline, ConstStringPtr text)
{
    Str255 fitted;
    short inkLeft, inkWidth;

    FitText(text, kNowLineMaxWidth, fitted);
    if (fitted[0] == 0)
        return;
    MeasureInk(fitted, &inkLeft, &inkWidth);
    MoveTo((162 - inkWidth) / 2 - inkLeft, baseline);
    DrawString(fitted);
}

void LCDDraw_NowPlaying(ConstStringPtr title, ConstStringPtr artist,
                        ConstStringPtr album, short position, short count,
                        long elapsedTicks, long lengthTicks, Boolean paused)
{
    char text[32];
    long elapsed = elapsedTicks / 60;
    long length = (lengthTicks + 30) / 60;
    long remaining = length - elapsed;

    LCDDraw_Clear();
    LCDDraw_Header((ConstStringPtr)"\pNow Playing");
    LCDDraw_PlayStateIcon(paused);

    /* "3 of 12": the song's place in the list it plays from. */
    {
        char *p = AppendNumber(text, position + 1);
        *p++ = ' '; *p++ = 'o'; *p++ = 'f'; *p++ = ' ';
        AppendNumber(p, count);
    }
    LCDDraw_SmallText(kCounterLeft, kCounterTop, text);

    /* DECISION N8: an empty artist or album leaves its line blank. */
    DrawCentredLine(kNowLineBaseline0, title);
    DrawCentredLine(kNowLineBaseline0 + kNowLineSpacing, artist);
    DrawCentredLine(kNowLineBaseline0 + 2 * kNowLineSpacing, album);

    LCDDraw_ProgressBar(elapsedTicks, lengthTicks);

    if (remaining < 0)
        remaining = 0;
    AppendTime(text, elapsed);
    LCDDraw_SmallText(kElapsedLeft, kTimeTop, text);
    text[0] = '-';
    AppendTime(text + 1, remaining);
    LCDDraw_SmallText(kRemainingRight - LCDDraw_SmallTextWidth(text),
                      kTimeTop, text);
}
