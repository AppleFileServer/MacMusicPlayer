#include "input.h"

/* Character codes from the Mac Plus keyboard. */
#define kKeyUpArrow     0x1E
#define kKeyDownArrow   0x1F
#define kKeyReturn      0x0D
#define kKeyEnter       0x03
#define kKeyDelete      0x08

UIEvent Input_FromKey(char key, Boolean autoRepeat)
{
    switch (key)
    {
        case kKeyUpArrow:   return kEvtWheelUp;
        case kKeyDownArrow: return kEvtWheelDown;
    }

    /* Only the wheel keys auto-repeat. */
    if (autoRepeat)
        return kEvtNone;

    switch (key)
    {
        case kKeyReturn:
        case kKeyEnter:     return kEvtSelect;
        case kKeyDelete:    return kEvtMenu;
    }

    return kEvtNone;
}

UIEvent Input_FromShellZone(ShellZone zone)
{
    /* Previous, Next and Play/Pause zones are recognised but have no
       action until a later stage. */
    switch (zone)
    {
        case kZoneSelect:   return kEvtSelect;
        case kZoneMenu:     return kEvtMenu;
        default:            return kEvtNone;
    }
}
