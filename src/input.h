#ifndef INPUT_H
#define INPUT_H

#include "ui_nav.h"
#include "ipod_shell.h"

/* Mac keyboard to iPod event. PROVISIONAL (D8): an emulator mapping,
   not iPod behaviour. */
UIEvent Input_FromKey(char key, Boolean autoRepeat);

/* Click on the drawn Click Wheel to iPod event. */
UIEvent Input_FromShellZone(ShellZone zone);

#endif
