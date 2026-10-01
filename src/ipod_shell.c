#include "ipod_shell.h"

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
