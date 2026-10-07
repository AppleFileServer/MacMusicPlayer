#include <Files.h>
#include <OSUtils.h>
#include <Memory.h>

#include "prefs.h"
#include "fileopen.h"
#include "bytes.h"
#include "song_format.h"

#define kPrefsMagic         'MMPf'
#define kPrefsVersion1      1
#define kPrefsVersion       2
#define kMaxVolumeName      27
#define kNoVolume           0xFF
#define kPrefsMaxSize       (8 + 1 + kMaxVolumeName + 4 + 4)

static const unsigned char kPrefsName[] = "\pMacMusicPlayer Prefs";

typedef struct
{
    Boolean hasLibrary;
    LibraryLocation location;
    short volume;                       /* 0-7, or -1 */
} Prefs;

/* Working-directory reference for the System Folder. */
static short SystemFolder(void)
{
    SysEnvRec env;

    if (SysEnvirons(1, &env) != noErr)
        return 0;
    return env.sysVRefNum;
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

/* Reads the prefs file (either version); false if there is none or it is
   not recognised. */
static Boolean ReadPrefs(Prefs *prefs)
{
    unsigned char buf[kPrefsMaxSize];
    long count = sizeof(buf);
    short refNum;
    OSErr err;

    prefs->hasLibrary = false;
    prefs->volume = -1;
    if (OpenDataFork(SystemFolder(), 0, kPrefsName, fsRdPerm, &refNum) != noErr)
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
    if (Get16(buf + 4) != kPrefsVersion || count < 8)
        return false;
    if (buf[7] <= 7)
        prefs->volume = buf[7];
    if (buf[6])
        prefs->hasLibrary = ReadLocation(buf, count, 8, &prefs->location);
    return true;
}

static OSErr WritePrefs(const Prefs *prefs)
{
    unsigned char buf[kPrefsMaxSize];
    short nameLength = 0;
    short sysFolder = SystemFolder();
    long count = 8;
    short refNum;
    OSErr err;

    Put32(buf, kPrefsMagic);
    Put16(buf + 4, kPrefsVersion);
    buf[6] = prefs->hasLibrary ? 1 : 0;
    buf[7] = prefs->volume >= 0 && prefs->volume <= 7 ? prefs->volume : kNoVolume;
    if (prefs->hasLibrary)
    {
        nameLength = prefs->location.volumeName[0];
        if (nameLength < 1 || nameLength > kMaxVolumeName)
            return paramErr;
        BlockMove(prefs->location.volumeName, buf + 8, nameLength + 1);
        Put32(buf + 9 + nameLength, prefs->location.volumeCreated);
        Put32(buf + 13 + nameLength, (unsigned long)prefs->location.folderID);
        count = 17 + nameLength;
    }

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
        err = FlushVol(nil, sysFolder);
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
