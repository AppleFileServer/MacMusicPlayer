#include "ipod_shell.h"

/* Stage 5c-1: the Click Wheel's markings, from the labelled controls
   diagram in Apple's iPod (4th generation) user guide, p. 14: MENU at the
   top, Previous/Rewind left, Next/Fast-forward right, Play/Pause at the
   bottom, each on its axis about 80% of the way out. Apple draws them
   white on the pale wheel; on a 1-bit screen that is invisible, so they
   are black on a white patch one pixel larger (DECISION W1), 7 pixels
   tall for legibility (W2; Apple's proportions give about 5), with MENU
   in hand-drawn capitals (W3). Leftmost pixel = most significant bit;
   rows are padded to whole words. They are drawn only; the click areas
   are still the ring's four quarters. */
#define kMarkHeight         7
#define kMarkOffset         56      /* centre distance from the wheel centre */

static const unsigned short kMenuMark[kMarkHeight * 2] = {     /* 22 wide */
    0x8BD1, 0x4400,
    0xDA19, 0x4400,
    0xAA15, 0x4400,
    0xAB95, 0x4400,
    0x8A13, 0x4400,
    0x8A11, 0x4400,
    0x8BD1, 0x3800
};

static const unsigned short kPreviousMark[kMarkHeight] = {     /* 10 wide */
    0x8440, 0x8CC0, 0x9DC0, 0xBFC0, 0x9DC0, 0x8CC0, 0x8440
};

static const unsigned short kNextMark[kMarkHeight] = {         /* 10 wide */
    0x8840, 0xCC40, 0xEE40, 0xFF40, 0xEE40, 0xCC40, 0x8840
};

static const unsigned short kPlayPauseMark[kMarkHeight] = {    /* 11 wide */
    0x8360, 0xC360, 0xE360, 0xF360, 0xE360, 0xC360, 0x8360
};

/* Converts a rectangle from body coordinates to window coordinates. */
static void BodyRect(Rect *r, short left, short top, short width, short height)
{
    SetRect(r, left, top, left + width, top + height);
    OffsetRect(r, kShellMargin, kShellMargin);
}

void GetLCDRect(Rect *r)
{
    BodyRect(r, kLCDLeft, kLCDTop, kLCDWidth, kLCDHeight);
}

static void DrawBody(void)
{
    Rect r;

    BodyRect(&r, 0, 0, kBodyWidth, kBodyHeight);
    EraseRoundRect(&r, kBodyCornerRadius, kBodyCornerRadius);
}

static void DrawBezel(void)
{
    Rect r;

    GetLCDRect(&r);
    InsetRect(&r, -kBezelInset, -kBezelInset);

    FillRoundRect(&r, kBezelCornerRadius, kBezelCornerRadius, &qd.gray);
    FrameRoundRect(&r, kBezelCornerRadius, kBezelCornerRadius);
}

/* Draws a marking centred (dx, dy) from the wheel centre: a white patch,
   then the marking in black. */
static void DrawMark(const unsigned short *bits, short rowBytes, short width,
                     short dx, short dy)
{
    BitMap mark;
    Rect dest, patch;

    BodyRect(&dest, kWheelLeft + kWheelDiameter / 2 + dx - width / 2,
             kWheelTop + kWheelDiameter / 2 + dy - kMarkHeight / 2,
             width, kMarkHeight);
    patch = dest;
    InsetRect(&patch, -1, -1);
    EraseRect(&patch);

    mark.baseAddr = (Ptr)bits;
    mark.rowBytes = rowBytes;
    SetRect(&mark.bounds, 0, 0, width, kMarkHeight);
    CopyBits(&mark, &qd.thePort->portBits, &mark.bounds, &dest, srcOr, nil);
}

static void DrawWheel(void)
{
    Rect r;
    short selectOffset = (kWheelDiameter - kSelectDiameter) / 2;

    BodyRect(&r, kWheelLeft, kWheelTop, kWheelDiameter, kWheelDiameter);
    FillOval(&r, &qd.ltGray);
    FrameOval(&r);

    BodyRect(&r, kWheelLeft + selectOffset, kWheelTop + selectOffset,
             kSelectDiameter, kSelectDiameter);
    EraseOval(&r);
    FrameOval(&r);

    DrawMark(kMenuMark, 4, 22, 0, -kMarkOffset);
    DrawMark(kPreviousMark, 2, 10, -kMarkOffset, 0);
    DrawMark(kNextMark, 2, 10, kMarkOffset, 0);
    DrawMark(kPlayPauseMark, 2, 11, 0, kMarkOffset);
}

void DrawIPodShell(void)
{
    DrawBody();
    DrawBezel();
    DrawWheel();
}

void IPodShell_WheelOffset(Point where, long *dx, long *dy)
{
    *dx = 2L * (where.h - kShellMargin - kWheelLeft) - kWheelDiameter;
    *dy = 2L * (where.v - kShellMargin - kWheelTop) - kWheelDiameter;
}

ShellZone IPodShell_HitTest(Point where)
{
    /* Doubled offsets from the wheel centre. Integer maths only. */
    long dx, dy;
    long distSq;
    long absX, absY;

    IPodShell_WheelOffset(where, &dx, &dy);
    distSq = dx * dx + dy * dy;
    absX = dx < 0 ? -dx : dx;
    absY = dy < 0 ? -dy : dy;

    if (distSq <= (long)kSelectDiameter * kSelectDiameter)
        return kZoneSelect;

    if (distSq > (long)kWheelDiameter * kWheelDiameter)
        return kZoneNone;

    /* Four 90-degree quarters split along the diagonals. */
    if (absY >= absX)
        return dy < 0 ? kZoneMenu : kZonePlayPause;

    return dx < 0 ? kZonePrevious : kZoneNext;
}
