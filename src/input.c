#include <Events.h>

#include "modal.h"
#include "input.h"

/* Stage 10a (F1): how long Next or Previous must be held, without the
   wheel turning, to be a hold. PROVISIONAL. */
#define kHoldTicks          30

/* Samples closer than this to the wheel centre are ignored: the angle
   there is unstable. Doubled units, as from IPodShell_WheelOffset. */
#define kWheelDeadRadius    (2 * 8)

/* atan(i / 64) in whole degrees, i = 0..64 (0 to 45 degrees). */
static const unsigned char kAtanDegrees[65] = {
     0,  1,  2,  3,  4,  4,  5,  6,  7,  8,  9, 10, 11, 11, 12, 13,
    14, 15, 16, 17, 17, 18, 19, 20, 21, 21, 22, 23, 24, 24, 25, 26,
    27, 27, 28, 29, 29, 30, 31, 31, 32, 33, 33, 34, 35, 35, 36, 36,
    37, 37, 38, 39, 39, 40, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45,
    45
};

/* Angle of (dx, dy) in degrees, 0..359: 0 points right and the angle
   grows clockwise on screen (QuickDraw's y axis points down). */
static short AngleDegrees(long dx, long dy)
{
    long ax = dx < 0 ? -dx : dx;
    long ay = dy < 0 ? -dy : dy;
    short a;

    if (ax >= ay)
        a = kAtanDegrees[(64 * ay) / ax];
    else
        a = 90 - kAtanDegrees[(64 * ax) / ay];

    if (dx >= 0)
        return dy >= 0 ? a : (a == 0 ? 0 : 360 - a);

    return dy >= 0 ? 180 - a : 180 + a;
}

/* Returns false if the point is too close to the centre to measure. */
static Boolean WheelAngle(Point where, short *angle)
{
    long dx, dy;

    IPodShell_WheelOffset(where, &dx, &dy);
    if (dx * dx + dy * dy < (long)kWheelDeadRadius * kWheelDeadRadius)
        return false;

    *angle = AngleDegrees(dx, dy);
    return true;
}

void Input_TrackWheel(ShellZone startZone, UIEventSink emit)
{
    Point where;
    short angle;
    short lastAngle = 0;
    short delta, step;
    short accumulated = 0;
    Boolean haveAngle = false;
    Boolean dragged = false;
    Boolean holding = false;
    Boolean holdable = (startZone == kZoneNext || startZone == kZonePrevious)
                       && UI_HoldAllowed();
    unsigned long pressed = TickCount();

    while (StillDown())
    {
        Modal_Service();                /* 6b-2: keep the audio fed */
        emit(kEvtNone);                 /* redraw the last sample's steps */

        /* 10a: held still long enough: a hold, not a press. */
        if (holdable && !dragged && !holding
            && TickCount() - pressed >= kHoldTicks)
        {
            holding = true;
            emit(startZone == kZoneNext ? kEvtHoldNext : kEvtHoldPrevious);
        }
        if (holding)
            continue;                   /* turning is ignored now */

        GetMouse(&where);
        if (!WheelAngle(where, &angle))
            continue;

        if (!haveAngle)
        {
            lastAngle = angle;
            haveAngle = true;
            continue;
        }

        /* Shortest way round, so crossing 0/360 degrees is smooth. */
        delta = angle - lastAngle;
        if (delta > 180)
            delta -= 360;
        else if (delta < -180)
            delta += 360;
        lastAngle = angle;

        /* Partial movement carries over to the next sample. The step
           size can change between samples (7c: the scrubber's). */
        accumulated += delta;
        step = UI_WheelStepDegrees();
        while (accumulated >= step)
        {
            accumulated -= step;
            dragged = true;
            emit(kEvtWheelDown);
        }
        while (accumulated <= -step)
        {
            accumulated += step;
            dragged = true;
            emit(kEvtWheelUp);
        }
    }

    /* Under one step of movement: the gesture was a button press, unless
       it became a hold (10a: released, and no skip). */
    if (holding)
        emit(kEvtHoldEnd);
    else if (!dragged)
        emit(Input_FromShellZone(startZone));
    emit(kEvtNone);
}

/* Character codes from the Mac Plus keyboard. */
#define kKeyUpArrow     0x1E
#define kKeyDownArrow   0x1F
#define kKeyReturn      0x0D
#define kKeyEnter       0x03
#define kKeyDelete      0x08
#define kKeyLeftArrow   0x1C
#define kKeyRightArrow  0x1D
#define kKeySpace       ' '

UIEvent Input_FromKey(char key, Boolean autoRepeat, Boolean option)
{
    /* 10a (F2): Option-arrows are held Next and Previous, seen only as
       auto-repeats (System 6 reports no key releases); a tap does
       nothing. */
    if (option && (key == kKeyRightArrow || key == kKeyLeftArrow))
    {
        if (!autoRepeat)
            return kEvtNone;
        return key == kKeyRightArrow ? kEvtKeyHoldNext : kEvtKeyHoldPrevious;
    }

    switch (key)
    {
        case kKeyUpArrow:   return kEvtArrowUp;
        case kKeyDownArrow: return kEvtArrowDown;
    }

    /* Only the wheel keys auto-repeat. */
    if (autoRepeat)
        return kEvtNone;

    switch (key)
    {
        case kKeyReturn:
        case kKeyEnter:     return kEvtSelect;
        case kKeyDelete:    return kEvtMenu;
        /* Stage 4 additions to the D8 emulator mapping. */
        case kKeyLeftArrow: return kEvtPrevious;
        case kKeyRightArrow:return kEvtNext;
        case kKeySpace:     return kEvtPlayPause;
    }

    return kEvtNone;
}

UIEvent Input_FromShellZone(ShellZone zone)
{
    /* A press (not a drag) on a wheel zone. Held presses (fast-forward,
       rewind, power off) are a later stage. */
    switch (zone)
    {
        case kZoneSelect:   return kEvtSelect;
        case kZoneMenu:     return kEvtMenu;
        case kZonePrevious: return kEvtPrevious;
        case kZoneNext:     return kEvtNext;
        case kZonePlayPause:return kEvtPlayPause;
        default:            return kEvtNone;
    }
}
