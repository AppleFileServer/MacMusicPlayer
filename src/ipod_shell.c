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
