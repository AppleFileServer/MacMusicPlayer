#ifndef INPUT_H
#define INPUT_H

#include "ui_nav.h"
#include "ipod_shell.h"

/* Mac keyboard to iPod event. PROVISIONAL (D8): an emulator mapping,
   not iPod behaviour. */
UIEvent Input_FromKey(char key, Boolean autoRepeat);

/* Click on the drawn Click Wheel to iPod event. */
UIEvent Input_FromShellZone(ShellZone zone);

/* Tracks a mouse gesture that started on the wheel ring (any zone except
   Select) until the button is released. Every 15 degrees of rotation emits
   kEvtWheelDown (clockwise) or kEvtWheelUp (counter-clockwise). If no step
   was emitted, the gesture is a press and emits the zone's event on
   release. The window must be the current port. */
typedef void (*UIEventSink)(UIEvent event);

void Input_TrackWheel(ShellZone startZone, UIEventSink emit);

#endif
