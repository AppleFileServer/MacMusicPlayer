#include <Memory.h>

#include "ipod_lcd.h"

/* Offscreen 1-bit bitmap and GrafPort holding the LCD contents.
   Drawing into gLCDPort uses LCD coordinates: (0,0) to (159,127). */
#define kLCDRowBytes    (kLCDWidth / 8)

static GrafPort gLCDPort;
static BitMap gLCDBits;
static Boolean gLCDReady = false;

static void LCD_Clear(void)
{
    GrafPtr savedPort;

    GetPort(&savedPort);
    SetPort(&gLCDPort);

    EraseRect(&gLCDPort.portRect);

    /* Temporary test frame along the exact LCD edges. */
    FrameRect(&gLCDPort.portRect);

    SetPort(savedPort);
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

void LCD_CopyToWindow(WindowPtr window, const Rect *destRect)
{
    /* Refuse anything but an exact 1:1 copy; CopyBits would scale. */
    if (!gLCDReady
        || destRect->right - destRect->left != kLCDWidth
        || destRect->bottom - destRect->top != kLCDHeight)
        return;

    CopyBits(&gLCDPort.portBits, &window->portBits,
             &gLCDPort.portRect, destRect, srcCopy, nil);
}
