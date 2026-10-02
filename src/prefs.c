#include <Files.h>
#include <OSUtils.h>
#include <Memory.h>

#include "prefs.h"
#include "fileopen.h"
#include "bytes.h"
#include "song_format.h"

#define kPrefsMagic         'MMPf'
#define kPrefsVersion       1
#define kMaxVolumeName      27
#define kPrefsMaxSize       (6 + 1 + kMaxVolumeName + 4 + 4)

static const unsigned char kPrefsName[] = "\pMacMusicPlayer Prefs";

/* Working-directory reference for the System Folder. */
static short SystemFolder(void)
{
    SysEnvRec env;

    if (SysEnvirons(1, &env) != noErr)
        return 0;
    return env.sysVRefNum;
}

Boolean Prefs_Load(LibraryLocation *location)
{
    unsigned char buf[kPrefsMaxSize];
    long count = sizeof(buf);
    short refNum;
    short nameLength;
    OSErr err;

    if (OpenDataFork(SystemFolder(), 0, kPrefsName, fsRdPerm, &refNum) != noErr)
        return false;
    err = FSRead(refNum, &count, buf);
    FSClose(refNum);
    if (err != noErr && err != eofErr)
        return false;

    if (count < 7 || Get32(buf) != kPrefsMagic || Get16(buf + 4) != kPrefsVersion)
        return false;

    nameLength = buf[6];
    if (nameLength < 1 || nameLength > kMaxVolumeName
        || count < 7 + nameLength + 8)
        return false;

    BlockMove(buf + 6, location->volumeName, nameLength + 1);
    location->volumeCreated = Get32(buf + 7 + nameLength);
    location->folderID = (long)Get32(buf + 11 + nameLength);
    return true;
}

OSErr Prefs_Save(const LibraryLocation *location)
{
    unsigned char buf[kPrefsMaxSize];
    short nameLength = location->volumeName[0];
    short sysFolder = SystemFolder();
    long count;
    short refNum;
    OSErr err;

    if (nameLength < 1 || nameLength > kMaxVolumeName)
        return paramErr;

    Put32(buf, kPrefsMagic);
    Put16(buf + 4, kPrefsVersion);
    BlockMove(location->volumeName, buf + 6, nameLength + 1);
    Put32(buf + 7 + nameLength, location->volumeCreated);
    Put32(buf + 11 + nameLength, (unsigned long)location->folderID);
    count = 15 + nameLength;

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
