#include <Files.h>
#include <OSUtils.h>
#include <Memory.h>

#include "prefs.h"
#include "fileopen.h"
#include "bytes.h"
#include "song_format.h"
#include "fileops.h"

#define kPrefsMagic         'MMPf'
#define kPrefsVersion1      1
#define kPrefsVersion2      2
#define kPrefsVersion3      3
#define kPrefsVersion       4
#define kMaxVolumeName      27
#define kNoVolume           0xFF
#define kLocationMax        (1 + kMaxVolumeName + 4 + 4)
#define kPrefsMaxSize       (8 + kLocationMax + 1 \
                             + kMaxRecent * (kLocationMax + 32) \
                             + 1 + kMaxSettings)

static const unsigned char kPrefsName[] = "\pMacMusic Prefs";
static const unsigned char kOldPrefsName[] = "\pMacMusicPlayer Prefs";

static Boolean gMigrated;               /* checked once per run */
static Boolean gReadOld;                /* the rename failed: read the old */

typedef struct
{
    Boolean hasLibrary;
    LibraryLocation location;
    short volume;                       /* 0-7, or -1 */
    short recentCount;
    RecentLibrary recent[kMaxRecent];
    short settingCount;                 /* Stage 9a */
    unsigned char settings[kMaxSettings];
} Prefs;

/* Working-directory reference for the System Folder. */
static short SystemFolder(void)
{
    SysEnvRec env;

    if (SysEnvirons(1, &env) != noErr)
        return 0;
    return env.sysVRefNum;
}

/* The app was renamed from MacMusicPlayer (R6): its prefs file is renamed
   to the new name, so nothing is copied or lost. If both exist the new
   one is used and the old one left alone; it is never deleted. If the
   rename fails (a locked disk), the old file is read until the next save
   writes the new one. */
static ConstStringPtr NameToRead(short sysFolder)
{
    if (!gMigrated)
    {
        gMigrated = true;
        if (!FileExists(sysFolder, 0, kPrefsName)
            && FileExists(sysFolder, 0, kOldPrefsName)
            && RenameFile(sysFolder, 0, kOldPrefsName, kPrefsName) != noErr)
            gReadOld = true;
    }
    return gReadOld ? kOldPrefsName : kPrefsName;
}

/* Reads the library location stored at buf + at (count bytes in all). */
static Boolean ReadLocation(const unsigned char *buf, long count, short at,
                            LibraryLocation *location)
{
    short nameLength = buf[at];

    if (nameLength < 1 || nameLength > kMaxVolumeName
        || count < at + 1 + nameLength + 8)
        return false;
    BlockMove(buf + at, location->volumeName, nameLength + 1);
    location->volumeCreated = Get32(buf + at + 1 + nameLength);
    location->folderID = (long)Get32(buf + at + 5 + nameLength);
    return true;
}

/* The bytes a location takes (after ReadLocation succeeded on it). */
static short LocationSize(const LibraryLocation *location)
{
    return 1 + location->volumeName[0] + 8;
}

/* Writes a location at buf + at; returns the bytes written, or 0 if the
   volume name doesn't fit. */
static short WriteLocation(unsigned char *buf, short at,
                           const LibraryLocation *location)
{
    short nameLength = location->volumeName[0];

    if (nameLength < 1 || nameLength > kMaxVolumeName)
        return 0;
    BlockMove(location->volumeName, buf + at, nameLength + 1);
    Put32(buf + at + 1 + nameLength, location->volumeCreated);
    Put32(buf + at + 5 + nameLength, (unsigned long)location->folderID);
    return 1 + nameLength + 8;
}

/* Version 3's recent list, from buf + at; stops at anything malformed.
   Returns where the list ends, or -1 if it was malformed. */
static short ReadRecent(const unsigned char *buf, long count, short at,
                        Prefs *prefs)
{
    short n, i, nameLength;
    RecentLibrary *r, extra;            /* entries past kMaxRecent: skipped */

    if (at >= count)
        return -1;
    n = buf[at++];
    for (i = 0; i < n; i++)
    {
        r = i < kMaxRecent ? &prefs->recent[i] : &extra;
        if (!ReadLocation(buf, count, at, &r->location))
            return -1;
        at += LocationSize(&r->location);
        if (at >= count || buf[at] > 31 || at + 1 + buf[at] > count)
            return -1;
        nameLength = buf[at];
        BlockMove(buf + at, r->folderName, nameLength + 1);
        at += 1 + nameLength;
        if (i < kMaxRecent)
            prefs->recentCount = i + 1;
    }
    return at;
}

/* Version 4's settings, from buf + at. */
static void ReadSettings(const unsigned char *buf, long count, short at,
                         Prefs *prefs)
{
    short n, i;

    if (at < 0 || at >= count)
        return;
    n = buf[at++];
    for (i = 0; i < n && i < kMaxSettings && at < count; i++)
        prefs->settings[i] = buf[at++];
    prefs->settingCount = i;
}

/* Reads the prefs file (either version); false if there is none or it is
   not recognised. */
static Boolean ReadPrefs(Prefs *prefs)
{
    unsigned char buf[kPrefsMaxSize];
    long count = sizeof(buf);
    short sysFolder = SystemFolder();
    short refNum, version, end;
    OSErr err;

    prefs->hasLibrary = false;
    prefs->volume = -1;
    prefs->recentCount = 0;
    prefs->settingCount = 0;
    if (OpenDataFork(sysFolder, 0, NameToRead(sysFolder), fsRdPerm, &refNum)
        != noErr)
        return false;
    err = FSRead(refNum, &count, buf);
    FSClose(refNum);
    if ((err != noErr && err != eofErr) || count < 7 || Get32(buf) != kPrefsMagic)
        return false;

    if (Get16(buf + 4) == kPrefsVersion1)
    {
        prefs->hasLibrary = ReadLocation(buf, count, 6, &prefs->location);
        return true;
    }
    version = Get16(buf + 4);
    if (version < kPrefsVersion2 || version > kPrefsVersion || count < 8)
        return false;
    if (buf[7] <= 7)
        prefs->volume = buf[7];
    if (buf[6])
        prefs->hasLibrary = ReadLocation(buf, count, 8, &prefs->location);
    if (version >= kPrefsVersion3 && (prefs->hasLibrary || !buf[6]))
    {
        end = ReadRecent(buf, count,
                         8 + (prefs->hasLibrary
                              ? LocationSize(&prefs->location) : 0),
                         prefs);
        if (version >= kPrefsVersion)
            ReadSettings(buf, count, end, prefs);
    }
    return true;
}

static OSErr WritePrefs(const Prefs *prefs)
{
    unsigned char buf[kPrefsMaxSize];
    short sysFolder = SystemFolder();
    long count = 8;
    short refNum, i, size, n = 0, countAt;
    const RecentLibrary *r;
    OSErr err;

    Put32(buf, kPrefsMagic);
    Put16(buf + 4, kPrefsVersion);
    buf[6] = prefs->hasLibrary ? 1 : 0;
    buf[7] = prefs->volume >= 0 && prefs->volume <= 7 ? prefs->volume : kNoVolume;
    if (prefs->hasLibrary)
    {
        size = WriteLocation(buf, 8, &prefs->location);
        if (size == 0)
            return paramErr;
        count += size;
    }

    /* The recent list; an entry that doesn't fit is left out. */
    countAt = count++;
    for (i = 0; i < prefs->recentCount && i < kMaxRecent; i++)
    {
        r = &prefs->recent[i];
        size = WriteLocation(buf, count, &r->location);
        if (size == 0 || r->folderName[0] > 31)
            continue;
        count += size;
        BlockMove(r->folderName, buf + count, r->folderName[0] + 1);
        count += 1 + r->folderName[0];
        n++;
    }
    buf[countAt] = (unsigned char)n;

    /* Stage 9a: the settings. */
    buf[count++] = (unsigned char)prefs->settingCount;
    for (i = 0; i < prefs->settingCount; i++)
        buf[count++] = prefs->settings[i];

    err = HCreate(sysFolder, 0, kPrefsName, kAppCreator, kPrefsFileType);
    if (err != noErr && err != dupFNErr)
        return err;

    err = OpenDataFork(sysFolder, 0, kPrefsName, fsRdWrPerm, &refNum);
    if (err != noErr)
        return err;

    err = FSWrite(refNum, &count, buf);
    if (err == noErr)
        err = SetEOF(refNum, count);
    FSClose(refNum);
    if (err == noErr)
    {
        gReadOld = false;               /* the new file is written now */
        err = FlushVol(nil, sysFolder);
    }
    return err;
}

Boolean Prefs_Load(LibraryLocation *location)
{
    Prefs prefs;

    if (!ReadPrefs(&prefs) || !prefs.hasLibrary)
        return false;
    *location = prefs.location;
    return true;
}

OSErr Prefs_Save(const LibraryLocation *location)
{
    Prefs prefs;

    ReadPrefs(&prefs);                  /* keep the volume */
    prefs.hasLibrary = true;
    prefs.location = *location;
    return WritePrefs(&prefs);
}

short Prefs_LoadVolume(void)
{
    Prefs prefs;

    ReadPrefs(&prefs);
    return prefs.volume;
}

OSErr Prefs_SaveVolume(short level)
{
    Prefs prefs;

    ReadPrefs(&prefs);                  /* keep the library location */
    prefs.volume = level;
    return WritePrefs(&prefs);
}

short Prefs_LoadRecent(RecentLibrary *recent)
{
    Prefs prefs;
    short i;

    ReadPrefs(&prefs);
    for (i = 0; i < prefs.recentCount; i++)
        recent[i] = prefs.recent[i];
    return prefs.recentCount;
}

OSErr Prefs_SaveRecent(const RecentLibrary *recent, short count)
{
    Prefs prefs;
    short i;

    ReadPrefs(&prefs);                  /* keep the library and volume */
    if (count > kMaxRecent)
        count = kMaxRecent;
    for (i = 0; i < count; i++)
        prefs.recent[i] = recent[i];
    prefs.recentCount = count;
    return WritePrefs(&prefs);
}

void Prefs_LoadSettings(unsigned char *values, short count)
{
    Prefs prefs;
    short i;

    ReadPrefs(&prefs);
    for (i = 0; i < count && i < prefs.settingCount; i++)
        values[i] = prefs.settings[i];
}

OSErr Prefs_SaveSettings(const unsigned char *values, short count)
{
    Prefs prefs;
    short i;

    ReadPrefs(&prefs);                  /* keep everything else */
    if (count > kMaxSettings)
        count = kMaxSettings;
    for (i = 0; i < count; i++)
        prefs.settings[i] = values[i];
    prefs.settingCount = count;
    return WritePrefs(&prefs);
}
