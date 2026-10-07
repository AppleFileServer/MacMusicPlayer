#ifndef SETTINGS_H
#define SETTINGS_H

#include <MacTypes.h>

/* Stage 9a: the iPod's Settings (specified in Stage 8b): Shuffle and
   Repeat, kept in the prefs. Clicker keeps its place in the prefs, but
   isn't in the Settings menu: the Clicker (9e) was dropped (2026-10-07). */

enum
{
    kSettingShuffle = 0,    /* kShuffleOff, kShuffleSongs, kShuffleAlbums */
    kSettingRepeat,         /* kRepeatOff, kRepeatOne, kRepeatAll */
    kSettingClicker,        /* 0 off, 1 on */
    kSettingCount
};

enum { kShuffleOff = 0, kShuffleSongs, kShuffleAlbums };
enum { kRepeatOff = 0, kRepeatOne, kRepeatAll };

/* Reads the settings from the prefs (defaults if none saved). */
void Settings_Load(void);

short Settings_Get(short setting);

/* Select on a setting's row (T2): the next value, saved at once. */
void Settings_Cycle(short setting);

/* The value as shown on its row: "Off", "Songs", "On"... */
ConstStringPtr Settings_ValueText(short setting);

/* Reset All Settings: the defaults (T1: Shuffle Off, Repeat Off, Clicker
   On), saved. The volume is not a setting here and is left alone (S-5). */
void Settings_ResetAll(void);

#endif
