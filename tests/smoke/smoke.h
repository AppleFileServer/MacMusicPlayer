#ifndef SMOKE_H
#define SMOKE_H

/* MacMusic's emulator smoke test (1.0 Part B2): built in only with
   -DMACMUSIC_SMOKE_TEST=ON (tests/run-smoke.sh). Smoke_Run takes the
   place of the library start-up: it makes a small library, drives the
   iPod through a scripted tour and logs each check to the file "out",
   which LaunchAPPL prints. The released app has none of this. */

#include "ui_lists.h"
#include "libfolder.h"
#include <Quickdraw.h>

void Smoke_Run(void);

/* Test-only hooks, compiled into their modules only in the smoke build. */
const Screen *UI_TestScreen(void);                          /* ui_nav.c */
void UI_TestClockText(StringPtr time, StringPtr date);      /* ui_nav.c */
void LibSetup_TestUseFolder(const LibraryFolder *folder);   /* libsetup.c */
const BitMap *LCD_TestBits(void);                           /* ipod_lcd.c */

#endif
