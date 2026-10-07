#include "settings.h"
#include "prefs.h"
#include "pstr.h"

/* T1: the defaults, as the 4G shipped. */
static const unsigned char kDefaults[kSettingCount] = {
    kShuffleOff, kRepeatOff, 1
};
static const short kValueCounts[kSettingCount] = { 3, 3, 2 };

static unsigned char gValues[kSettingCount];

static void Save(void)
{
    Prefs_SaveSettings(gValues, kSettingCount);
}

void Settings_Load(void)
{
    short i;

    for (i = 0; i < kSettingCount; i++)
        gValues[i] = kDefaults[i];
    Prefs_LoadSettings(gValues, kSettingCount);
    for (i = 0; i < kSettingCount; i++)
        if (gValues[i] >= kValueCounts[i])
            gValues[i] = kDefaults[i];
}

short Settings_Get(short setting)
{
    return gValues[setting];
}

void Settings_Cycle(short setting)
{
    gValues[setting] = (gValues[setting] + 1) % kValueCounts[setting];
    Save();
}

ConstStringPtr Settings_ValueText(short setting)
{
    switch (setting)
    {
        case kSettingShuffle:
            return gValues[setting] == kShuffleSongs ? P("Songs")
                 : gValues[setting] == kShuffleAlbums ? P("Albums") : P("Off");
        case kSettingRepeat:
            return gValues[setting] == kRepeatOne ? P("One")
                 : gValues[setting] == kRepeatAll ? P("All") : P("Off");
        default:
            return gValues[setting] ? P("On") : P("Off");
    }
}

void Settings_ResetAll(void)
{
    short i;

    for (i = 0; i < kSettingCount; i++)
        gValues[i] = kDefaults[i];
    Save();
}
