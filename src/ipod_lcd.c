#include <Memory.h>

#include "ipod_lcd.h"

/* Offscreen 1-bit bitmap and GrafPort holding the LCD contents.
   Drawing into gLCDPort uses LCD coordinates: (0,0) to (159,127). */
#define kLCDRowBytes    (kLCDWidth / 8)

static GrafPort gLCDPort;
static BitMap gLCDBits;
static Boolean gLCDReady = false;
static GrafPtr gSavedPort;

void LCD_BeginDraw(void)
{
    GetPort(&gSavedPort);
    SetPort(&gLCDPort);
}

void LCD_EndDraw(void)
{
    SetPort(gSavedPort);
}

static void LCD_Clear(void)
{
    LCD_BeginDraw();
    EraseRect(&gLCDPort.portRect);
    LCD_EndDraw();
}

Boolean LCD_Init(void)
{
    GrafPtr savedPort;

    gLCDBits.baseAddr = NewPtr((Size)kLCDRowBytes * kLCDHeight);
    if (gLCDBits.baseAddr == nil)
        return false;

    gLCDBits.rowBytes = kLCDRowBytes;
    SetRect(&gLCDBits.bounds, 0, 0, kLCDWidth, kLCDHeight);

    GetPort(&savedPort);

    OpenPort(&gLCDPort);
    SetPortBits(&gLCDBits);
    PortSize(kLCDWidth, kLCDHeight);
    RectRgn(gLCDPort.visRgn, &gLCDPort.portRect);
    ClipRect(&gLCDPort.portRect);

    TextFont(kLCDFont);
    TextSize(kLCDFontSize);

    SetPort(savedPort);

    gLCDReady = true;
    LCD_Clear();

    return true;
}

void LCD_Dispose(void)
{
    if (!gLCDReady)
        return;

    ClosePort(&gLCDPort);
    DisposePtr(gLCDBits.baseAddr);
    gLCDReady = false;
}

static Boolean gInverted;

void LCD_SetInverted(Boolean inverted)
{
    gInverted = inverted;
}

Boolean LCD_IsInverted(void)
{
    return gInverted;
}

void LCD_CopyToWindow(WindowPtr window, const Rect *destRect)
{
    /* Refuse anything but an exact 1:1 copy; CopyBits would scale. */
    if (!gLCDReady
        || destRect->right - destRect->left != kLCDWidth
        || destRect->bottom - destRect->top != kLCDHeight)
        return;

    CopyBits(&gLCDPort.portBits, &window->portBits,
             &gLCDPort.portRect, destRect,
             gInverted ? notSrcCopy : srcCopy, nil);
}

void LCD_Present(WindowPtr window, const Rect *destRect)
{
    GrafPtr savedPort;

    GetPort(&savedPort);
    SetPort(window);
    LCD_CopyToWindow(window, destRect);
    SetPort(savedPort);
}

#ifdef MACMUSIC_SMOKE_TEST
/* 1.0 Part B2: the LCD's pixels, for the smoke test (tests/smoke). */
const BitMap *LCD_TestBits(void)
{
    return &gLCDBits;
}
#endif
