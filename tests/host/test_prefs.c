/* The prefs file (prefs.c) and the Settings kept in it (settings.c):
   "MacMusic Prefs" in the System Folder, format version 4, versions 1-3
   still read, and the old "MacMusicPlayer Prefs" carried over (R6).

   prefs.c looks for the old file once per run, so that is tested first,
   and each way it can go in a run of its own:
       test_prefs              the old file renamed, then everything else
       test_prefs locked-old   the old file locked: read where it is
       test_prefs both         both files: the new one used */

#include "hosttest.h"
#include "prefs.h"
#include "settings.h"

static const char *kPrefs = "MacMusic Prefs";
static const char *kOldPrefs = "MacMusicPlayer Prefs";

static unsigned char gBuf[600];
static long gLength;

/* Prefs bytes built by hand, as the spec in prefs.h has them. */
static void Begin(short version)
{
    memcpy(gBuf, "MMPf", 4);
    gBuf[4] = 0;
    gBuf[5] = (unsigned char)version;
    gLength = 6;
}
static void Byte(int b) { gBuf[gLength++] = (unsigned char)b; }
static void Long(unsigned long v)
{
    Byte(v >> 24); Byte(v >> 16); Byte(v >> 8); Byte(v);
}
static void PStr(const char *s)
{
    Byte((int)strlen(s));
    memcpy(gBuf + gLength, s, strlen(s));
    gLength += strlen(s);
}
static void Location(const char *volume, unsigned long created, long dir)
{
    PStr(volume);
    Long(created);
    Long((unsigned long)dir);
}
static void Write(const char *name)
{
    TestFS_WriteFile(TestFS_SystemFolder(), name, kPrefsFileType, kAppCreator,
                     gBuf, gLength);
}

static void MakeLocation(LibraryLocation *l, const char *volume,
                         unsigned long created, long dir)
{
    memset(l, 0, sizeof(*l));
    memcpy(l->volumeName, PS(volume), strlen(volume) + 1);
    l->volumeCreated = created;
    l->folderID = dir;
}

static Boolean SameLocation(const LibraryLocation *l, const char *volume,
                            unsigned long created, long dir)
{
    return l->volumeName[0] == strlen(volume)
        && memcmp(l->volumeName + 1, volume, l->volumeName[0]) == 0
        && l->volumeCreated == created && l->folderID == dir;
}

/* A version 2 file with a library and volume 3, under name. */
static void WriteV2(const char *name, long dir)
{
    Begin(2);
    Byte(1);
    Byte(3);
    Location("Disk A", 0x11223344, dir);
    Write(name);
}

static int OldFileRuns(const char *mode)
{
    LibraryLocation l;

    if (strcmp(mode, "locked-old") == 0)
    {
        Test("the old prefs file locked: read where it is");
        WriteV2(kOldPrefs, 77);
        TestFS_Lock(TestFS_SystemFolder(), kOldPrefs, true);
        CHECK(Prefs_Load(&l));
        CHECK(SameLocation(&l, "Disk A", 0x11223344, 77));
        CHECK(TestFS_Exists(TestFS_SystemFolder(), kOldPrefs));
        CHECK(!TestFS_Exists(TestFS_SystemFolder(), kPrefs));
        CHECK_EQ(Prefs_LoadVolume(), 3);
        /* The next save writes the new file, which is read from then. */
        CHECK_EQ(Prefs_SaveVolume(6), noErr);
        CHECK(TestFS_Exists(TestFS_SystemFolder(), kPrefs));
        CHECK_EQ(Prefs_LoadVolume(), 6);
        CHECK(Prefs_Load(&l));
        CHECK(SameLocation(&l, "Disk A", 0x11223344, 77));
    }
    else
    {
        Test("both prefs files: the new one used, the old left alone");
        WriteV2(kOldPrefs, 77);
        WriteV2(kPrefs, 88);
        CHECK(Prefs_Load(&l));
        CHECK_EQ(l.folderID, 88);
        CHECK(TestFS_Exists(TestFS_SystemFolder(), kOldPrefs));
    }
    return TestsDone(strcmp(mode, "locked-old") == 0 ? "prefs (locked)"
                                                     : "prefs (both)");
}

int main(int argc, char **argv)
{
    LibraryLocation l, m;
    RecentLibrary recent[kMaxRecent + 2], back[kMaxRecent];
    unsigned char values[kMaxSettings];
    short i;
    long n;

    if (argc > 1)
        return OldFileRuns(argv[1]);

    Test("the old prefs file is renamed, nothing lost (R6)");
    WriteV2(kOldPrefs, 77);
    CHECK(Prefs_Load(&l));
    CHECK(SameLocation(&l, "Disk A", 0x11223344, 77));
    CHECK(!TestFS_Exists(TestFS_SystemFolder(), kOldPrefs));
    CHECK(TestFS_Exists(TestFS_SystemFolder(), kPrefs));
    CHECK_EQ(Prefs_LoadVolume(), 3);

    Test("no prefs: nothing remembered");
    CHECK(!Prefs_Load(&l));
    CHECK_EQ(Prefs_LoadVolume(), -1);
    CHECK_EQ(Prefs_LoadRecent(back), 0);

    Test("saving: version 4, byte for byte");
    MakeLocation(&l, "Scratch", 0xA1B2C3D4, 1234);
    CHECK_EQ(Prefs_Save(&l), noErr);
    CHECK_EQ(Prefs_SaveVolume(5), noErr);
    MakeLocation(&recent[0].location, "Scratch", 0xA1B2C3D4, 1234);
    memcpy(recent[0].folderName, PS("MacMusic Library"), 17);
    MakeLocation(&recent[1].location, "Floppy", 7, 99);
    memcpy(recent[1].folderName, PS("Old Library"), 12);
    CHECK_EQ(Prefs_SaveRecent(recent, 2), noErr);
    values[0] = 1; values[1] = 2; values[2] = 0;
    CHECK_EQ(Prefs_SaveSettings(values, 3), noErr);
    Begin(4);
    Byte(1);
    Byte(5);
    Location("Scratch", 0xA1B2C3D4, 1234);
    Byte(2);
    Location("Scratch", 0xA1B2C3D4, 1234);
    PStr("MacMusic Library");
    Location("Floppy", 7, 99);
    PStr("Old Library");
    Byte(3); Byte(1); Byte(2); Byte(0);
    {
        unsigned char got[600];

        n = TestFS_ReadFile(TestFS_SystemFolder(), kPrefs, got, sizeof got);
        CHECK_EQ(n, gLength);
        CHECK(memcmp(got, gBuf, gLength) == 0);
        CHECK_EQ(TestFS_Type(TestFS_SystemFolder(), kPrefs), kPrefsFileType);
    }

    Test("each save keeps the rest");
    MakeLocation(&l, "Scratch", 0xA1B2C3D4, 1234);
    Prefs_Save(&l);
    Prefs_SaveVolume(2);
    MakeLocation(&recent[0].location, "Floppy", 7, 99);
    memcpy(recent[0].folderName, PS("Old Library"), 12);
    Prefs_SaveRecent(recent, 1);
    values[0] = 2; values[1] = 1; values[2] = 1;
    Prefs_SaveSettings(values, 3);
    MakeLocation(&m, "Other", 1, 2);
    Prefs_Save(&m);
    CHECK_EQ(Prefs_LoadVolume(), 2);
    CHECK_EQ(Prefs_LoadRecent(back), 1);
    CHECK(SameLocation(&back[0].location, "Floppy", 7, 99));
    CHECK_STR(back[0].folderName, "Old Library");
    memset(values, 9, sizeof values);
    Prefs_LoadSettings(values, 3);
    CHECK(values[0] == 2 && values[1] == 1 && values[2] == 1);
    CHECK(Prefs_Load(&l));
    CHECK(SameLocation(&l, "Other", 1, 2));

    Test("the recent list: at most kMaxRecent kept");
    for (i = 0; i < kMaxRecent + 2; i++)
    {
        char name[8];

        snprintf(name, sizeof name, "D%d", i);
        MakeLocation(&recent[i].location, name, (unsigned long)i, 100 + i);
        memcpy(recent[i].folderName, PS("Lib"), 4);
    }
    Prefs_SaveRecent(recent, kMaxRecent + 2);
    CHECK_EQ(Prefs_LoadRecent(back), kMaxRecent);
    CHECK(SameLocation(&back[kMaxRecent - 1].location, "D4", 4, 104));

    Test("reading version 1: the location only");
    Begin(1);
    Location("Macintosh HD", 42, 600);
    Write(kPrefs);
    CHECK(Prefs_Load(&l));
    CHECK(SameLocation(&l, "Macintosh HD", 42, 600));
    CHECK_EQ(Prefs_LoadVolume(), -1);
    CHECK_EQ(Prefs_LoadRecent(back), 0);

    Test("reading version 2: no recent list, no settings");
    WriteV2(kPrefs, 31);
    CHECK(Prefs_Load(&l));
    CHECK_EQ(l.folderID, 31);
    CHECK_EQ(Prefs_LoadVolume(), 3);
    CHECK_EQ(Prefs_LoadRecent(back), 0);
    memset(values, 9, sizeof values);
    Prefs_LoadSettings(values, 3);
    CHECK_EQ(values[0], 9);                     /* left as they were */

    Test("reading version 3: no settings; no library but a recent list");
    Begin(3);
    Byte(0);
    Byte(0xFF);
    Byte(1);
    Location("Disk B", 5, 6);
    PStr("Lib B");
    Write(kPrefs);
    CHECK(!Prefs_Load(&l));
    CHECK_EQ(Prefs_LoadVolume(), -1);
    CHECK_EQ(Prefs_LoadRecent(back), 1);
    CHECK_STR(back[0].folderName, "Lib B");

    Test("damaged or unknown prefs are ignored");
    Begin(4);
    gBuf[0] = 'X';
    Byte(1); Byte(1);
    Location("Disk", 1, 2);
    Write(kPrefs);
    CHECK(!Prefs_Load(&l));
    Begin(9);
    Byte(1); Byte(1);
    Location("Disk", 1, 2);
    Write(kPrefs);
    CHECK(!Prefs_Load(&l));
    Begin(4);
    Byte(1); Byte(1);
    PStr("Disk");                               /* cut short */
    Write(kPrefs);
    CHECK(!Prefs_Load(&l));
    CHECK_EQ(Prefs_LoadVolume(), 1);
    MakeLocation(&l, "Scratch", 3, 4);
    CHECK_EQ(Prefs_Save(&l), noErr);            /* rewritten whole */
    CHECK(Prefs_Load(&l));
    CHECK(SameLocation(&l, "Scratch", 3, 4));

    Test("Settings: the defaults, cycling, saved at once");
    Settings_Load();
    CHECK_EQ(Settings_Get(kSettingShuffle), kShuffleOff);
    CHECK_EQ(Settings_Get(kSettingRepeat), kRepeatOff);
    CHECK_EQ(Settings_Get(kSettingClicker), 1);
    CHECK_STR(Settings_ValueText(kSettingShuffle), "Off");
    Settings_Cycle(kSettingShuffle);
    CHECK_STR(Settings_ValueText(kSettingShuffle), "Songs");
    Settings_Cycle(kSettingShuffle);
    CHECK_STR(Settings_ValueText(kSettingShuffle), "Albums");
    Settings_Cycle(kSettingShuffle);
    CHECK_STR(Settings_ValueText(kSettingShuffle), "Off");
    Settings_Cycle(kSettingShuffle);
    Settings_Cycle(kSettingRepeat);
    CHECK_STR(Settings_ValueText(kSettingRepeat), "One");
    Settings_Cycle(kSettingRepeat);
    CHECK_STR(Settings_ValueText(kSettingRepeat), "All");
    Settings_Load();                            /* as at the next launch */
    CHECK_EQ(Settings_Get(kSettingShuffle), kShuffleSongs);
    CHECK_EQ(Settings_Get(kSettingRepeat), kRepeatAll);

    Test("Settings: Reset All restores the defaults, keeps the rest");
    MakeLocation(&l, "Scratch", 3, 4);
    Prefs_Save(&l);
    Prefs_SaveVolume(7);
    Settings_Load();
    Settings_Cycle(kSettingShuffle);
    Settings_Cycle(kSettingRepeat);
    Settings_ResetAll();
    CHECK_EQ(Settings_Get(kSettingShuffle), kShuffleOff);
    CHECK_EQ(Settings_Get(kSettingRepeat), kRepeatOff);
    Settings_Load();
    CHECK_EQ(Settings_Get(kSettingShuffle), kShuffleOff);
    CHECK_EQ(Prefs_LoadVolume(), 7);
    CHECK(Prefs_Load(&l));
    CHECK(SameLocation(&l, "Scratch", 3, 4));

    Test("Settings: values out of range read as the defaults");
    values[0] = 3; values[1] = 9; values[2] = 2;
    Prefs_SaveSettings(values, 3);
    Settings_Load();
    CHECK_EQ(Settings_Get(kSettingShuffle), kShuffleOff);
    CHECK_EQ(Settings_Get(kSettingRepeat), kRepeatOff);
    CHECK_EQ(Settings_Get(kSettingClicker), 1);

    Test("a locked disk: saving fails, nothing changes");
    MakeLocation(&l, "Scratch", 3, 4);
    Prefs_Save(&l);
    TestFS_LockVolume(true);
    MakeLocation(&m, "Other", 1, 2);
    CHECK(Prefs_Save(&m) != noErr);
    TestFS_LockVolume(false);
    CHECK(Prefs_Load(&l));
    CHECK(SameLocation(&l, "Scratch", 3, 4));

    return TestsDone("prefs");
}
