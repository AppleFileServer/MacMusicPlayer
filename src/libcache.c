#include <Files.h>
#include <Memory.h>
#include <string.h>

#include "libcache.h"
#include "fileopen.h"
#include "bytes.h"
#include "pstr.h"
#include "song_format.h"
#include "library.h"

/* The cache file (all numbers big-endian):

     header   4  'MMca'
              2  the format's version, 2 (1.1 wrote 1)
              2  the number of songs
              4  a check-sum (SongChecksum) of every entry, less each
                 entry's first 4 bytes
     entries  4  the file's length (data fork), or kStale: not in the
                 check-sum, so it can be marked stale in place
              4  the file's modification date
              4  ID, 4 seconds, 4 audio offset, 4 audio length,
              4  fingerprint, 2 track, 2 flags
              4  the sample rate (version 2; 1.1's version 1 has 0 here,
                 and all its songs are 22050 Hz)
              the file's name, then title, artist, album, genre and
              composer: Pascal strings, packed

   A cache whose header, version, check-sum or strings don't add up is
   ignored.

   1.2 (N12-6): version 2 adds each song's sample rate, so 1.1 ignores a
   1.2 cache (and reads every file, skipping 22254 Hz songs, which it
   can't play); 1.2 still reads 1.1's version 1, whose songs are all
   22050 Hz, so upgrading costs no extra scan. */

#define kCacheMagic     'MMca'
#define kCacheVersion   2
#define kCacheVersion1  1           /* 1.1's: read, never written */
#define kCacheType      'iPdC'
#define kHeaderSize     12
#define kFixedSize      36          /* an entry's numbers */
#define kStale          0xFFFFFFFFUL
#define kMaxCacheFile   (512L * 1024)

static const unsigned char kFolderName[] = "\pMacMusic Cache";
static const unsigned char kFileName[] = "\pSong Details";

static Handle gCacheH;              /* the loaded file */
static long gCacheSize;
static short gCacheCount;
static long gCursor;                /* where the next match is likely */
static short gCacheVersion;         /* the loaded cache's */

/* ---- Finding the folder ------------------------------------------------ */

/* The cache folder's ID, making it (invisible) if make is set. */
static OSErr CacheFolder(const LibraryFolder *folder, Boolean make, long *dirID)
{
    CInfoPBRec pb;
    HParamBlockRec hp;
    OSErr err;

    memset(&pb, 0, sizeof pb);
    pb.dirInfo.ioNamePtr = (StringPtr)kFolderName;
    pb.dirInfo.ioVRefNum = folder->vRefNum;
    pb.dirInfo.ioDrDirID = folder->folderID;
    err = PBGetCatInfoSync(&pb);
    if (err == noErr)
    {
        if (!(pb.hFileInfo.ioFlAttrib & 0x10))
            return dupFNErr;            /* a file of that name: not ours */
        *dirID = pb.dirInfo.ioDrDirID;
        return noErr;
    }
    if (err != fnfErr || !make)
        return err;

    memset(&hp, 0, sizeof hp);
    hp.fileParam.ioNamePtr = (StringPtr)kFolderName;
    hp.fileParam.ioVRefNum = folder->vRefNum;
    hp.fileParam.ioDirID = folder->folderID;
    err = PBDirCreateSync(&hp);
    if (err != noErr)
        return err;
    *dirID = hp.fileParam.ioDirID;

    /* N11-3a: invisible in the Finder, so it isn't moved or edited by
       hand. A failure here is harmless. */
    memset(&pb, 0, sizeof pb);
    pb.dirInfo.ioNamePtr = (StringPtr)kFolderName;
    pb.dirInfo.ioVRefNum = folder->vRefNum;
    pb.dirInfo.ioDrDirID = folder->folderID;
    if (PBGetCatInfoSync(&pb) == noErr)
    {
        pb.dirInfo.ioDrUsrWds.frFlags |= fInvisible;
        pb.dirInfo.ioNamePtr = (StringPtr)kFolderName;
        pb.dirInfo.ioDrDirID = folder->folderID;
        PBSetCatInfoSync(&pb);
    }
    return noErr;
}

/* ---- Reading ----------------------------------------------------------- */

/* The size of the entry at p (end the file's end), or 0 if it runs past
   it or a string is too long. */
static long EntrySize(const unsigned char *p, const unsigned char *end)
{
    const unsigned char *q = p + kFixedSize;
    short i;

    for (i = 0; i <= kSongTagCount; i++)
    {
        if (q >= end || q[0] > (i == 0 ? 31 : 63) || q + 1 + q[0] > end)
            return 0;
        q += 1 + q[0];
    }
    return q - p;
}

void LibCache_Unload(void)
{
    if (gCacheH != nil)
        DisposeHandle(gCacheH);
    gCacheH = nil;
    gCacheSize = 0;
    gCacheCount = 0;
    gCursor = 0;
}

Boolean LibCache_Load(const LibraryFolder *folder)
{
    long dirID, eof, count, at, size;
    short refNum, i;
    unsigned char *p, *end;
    SongChecksum sum;
    OSErr err;

    LibCache_Unload();
    if (CacheFolder(folder, false, &dirID) != noErr)
        return false;
    if (OpenDataFork(folder->vRefNum, dirID, kFileName, fsRdPerm, &refNum) != noErr)
        return false;
    err = GetEOF(refNum, &eof);
    if (err == noErr && (eof < kHeaderSize || eof > kMaxCacheFile))
        err = eofErr;
    if (err == noErr)
    {
        gCacheH = NewHandle(eof);
        if (gCacheH == nil)
            err = memFullErr;
    }
    if (err == noErr)
    {
        count = eof;
        MoveHHi(gCacheH);               /* out of the way of the growing pool */
        HLock(gCacheH);
        err = FSRead(refNum, &count, *gCacheH);
        if (err == noErr && count != eof)
            err = eofErr;
    }
    FSClose(refNum);
    if (err != noErr)
    {
        LibCache_Unload();
        return false;
    }

    /* Check it: the header, every entry's strings, the check-sum. */
    p = (unsigned char *)*gCacheH;
    end = p + eof;
    if (Get32(p) != kCacheMagic
        || (Get16(p + 4) != kCacheVersion && Get16(p + 4) != kCacheVersion1)
        || Get16(p + 6) > kMaxSongs)
    {
        LibCache_Unload();
        return false;
    }
    SongChecksum_Init(&sum);
    for (i = 0, at = kHeaderSize; i < (short)Get16(p + 6); i++)
    {
        size = EntrySize(p + at, end);
        if (size == 0)
        {
            LibCache_Unload();
            return false;
        }
        SongChecksum_Add(&sum, p + at + 4, size - 4);
        at += size;
    }
    if (at != eof || SongChecksum_Value(&sum) != Get32(p + 8))
    {
        LibCache_Unload();
        return false;
    }
    gCacheSize = eof;
    gCacheCount = (short)Get16(p + 6);
    gCacheVersion = (short)Get16(p + 4);
    gCursor = kHeaderSize;
    return true;
}

short LibCache_Count(void)
{
    return gCacheH != nil ? gCacheCount : 0;
}

static Boolean Matches(const unsigned char *e, ConstStringPtr name,
                       unsigned long modDate, long length)
{
    const unsigned char *n = e + kFixedSize;

    return Get32(e) == (unsigned long)length && Get32(e) != kStale
        && Get32(e + 4) == modDate
        && (gCacheVersion == kCacheVersion1
            || SongRate_Valid((long)Get32(e + 32)))
        && n[0] == name[0] && memcmp(n + 1, name + 1, name[0]) == 0;
}

static void Fill(const unsigned char *e, SongInfo *info)
{
    const unsigned char *q = e + kFixedSize;
    short i;

    memset(info, 0, sizeof *info);
    info->id = (long)Get32(e + 8);
    info->seconds = (long)Get32(e + 12);
    info->audioOffset = (long)Get32(e + 16);
    info->audioLength = (long)Get32(e + 20);
    info->fingerprint = Get32(e + 24);
    info->track = (short)Get16(e + 28);
    info->flags = (short)Get16(e + 30);
    info->sampleRate = gCacheVersion == kCacheVersion1 ? kSongSampleRate
                                                       : (long)Get32(e + 32);
    q += 1 + q[0];                      /* past the file's name */
    for (i = 0; i < kSongTagCount; i++)
    {
        BlockMove(q, info->text[i], q[0] + 1);
        q += 1 + q[0];
    }
}

Boolean LibCache_Find(ConstStringPtr name, unsigned long modDate, long length,
                      SongInfo *info, long *at)
{
    unsigned char *p, *end;
    long pos;
    short i;

    if (gCacheH == nil)
        return false;
    p = (unsigned char *)*gCacheH;
    end = p + gCacheSize;

    /* Usually the next entry: the folder lists in the same order as when
       the cache was written. Otherwise, from the start. */
    if (gCursor < gCacheSize && Matches(p + gCursor, name, modDate, length))
        pos = gCursor;
    else
    {
        for (i = 0, pos = kHeaderSize; i < gCacheCount; i++)
        {
            if (Matches(p + pos, name, modDate, length))
                break;
            pos += EntrySize(p + pos, end);
        }
        if (i == gCacheCount)
            return false;
    }
    Fill(p + pos, info);
    *at = pos;
    gCursor = pos + EntrySize(p + pos, end);
    return true;
}

/* ---- Writing ----------------------------------------------------------- */

static short gSaveRef;
static long gSaveAt;
static short gSaveCount;
static SongChecksum gSaveSum;
static OSErr gSaveErr;
static short gSaveVRefNum;
static unsigned char gEntry[kFixedSize + 32 + kSongTagCount * 64];

OSErr LibCache_BeginSave(const LibraryFolder *folder)
{
    long dirID, count = kHeaderSize;
    unsigned char header[kHeaderSize];
    OSErr err;

    gSaveRef = 0;
    gSaveErr = CacheFolder(folder, true, &dirID);
    if (gSaveErr != noErr)
        return gSaveErr;
    err = HCreate(folder->vRefNum, dirID, kFileName, kAppCreator, kCacheType);
    if (err != noErr && err != dupFNErr)
        return gSaveErr = err;
    gSaveErr = OpenDataFork(folder->vRefNum, dirID, kFileName, fsRdWrPerm, &gSaveRef);
    if (gSaveErr != noErr)
    {
        gSaveRef = 0;
        return gSaveErr;
    }
    gSaveVRefNum = folder->vRefNum;

    /* A header that can't be mistaken for a good one, until End. */
    memset(header, 0, sizeof header);
    gSaveErr = SetEOF(gSaveRef, 0);
    if (gSaveErr == noErr)
        gSaveErr = FSWrite(gSaveRef, &count, header);
    gSaveAt = kHeaderSize;
    gSaveCount = 0;
    SongChecksum_Init(&gSaveSum);
    return gSaveErr;
}

long LibCache_SaveEntry(ConstStringPtr name, unsigned long modDate, long length,
                        const SongInfo *info)
{
    unsigned char *q = gEntry + kFixedSize;
    long size, at = gSaveAt;
    short i;

    if (gSaveRef == 0 || gSaveErr != noErr || gSaveCount >= kMaxSongs)
        return -1;
    Put32(gEntry, (unsigned long)length);
    Put32(gEntry + 4, modDate);
    Put32(gEntry + 8, (unsigned long)info->id);
    Put32(gEntry + 12, (unsigned long)info->seconds);
    Put32(gEntry + 16, (unsigned long)info->audioOffset);
    Put32(gEntry + 20, (unsigned long)info->audioLength);
    Put32(gEntry + 24, info->fingerprint);
    Put16(gEntry + 28, (unsigned short)info->track);
    Put16(gEntry + 30, (unsigned short)info->flags);
    Put32(gEntry + 32, (unsigned long)info->sampleRate);
    BlockMove(name, q, name[0] + 1);
    q += 1 + name[0];
    for (i = 0; i < kSongTagCount; i++)
    {
        BlockMove(info->text[i], q, info->text[i][0] + 1);
        q += 1 + info->text[i][0];
    }
    size = q - gEntry;
    SongChecksum_Add(&gSaveSum, gEntry + 4, size - 4);
    gSaveErr = FSWrite(gSaveRef, &size, gEntry);
    if (gSaveErr != noErr)
        return -1;
    gSaveAt += size;
    gSaveCount++;
    return at;
}

OSErr LibCache_EndSave(void)
{
    unsigned char header[kHeaderSize];
    long count = kHeaderSize;
    OSErr err = gSaveErr;

    if (gSaveRef == 0)
        return err;
    if (err == noErr)
    {
        Put32(header, kCacheMagic);
        Put16(header + 4, kCacheVersion);
        Put16(header + 6, (unsigned short)gSaveCount);
        Put32(header + 8, SongChecksum_Value(&gSaveSum));
        err = SetFPos(gSaveRef, fsFromStart, 0);
        if (err == noErr)
            err = FSWrite(gSaveRef, &count, header);
    }
    if (err != noErr)
        SetEOF(gSaveRef, 0);            /* no half-made cache is kept */
    FSClose(gSaveRef);
    FlushVol(nil, gSaveVRefNum);
    gSaveRef = 0;
    return err;
}

void LibCache_Forget(const LibraryFolder *folder, long at)
{
    long dirID, count = 4;
    short refNum;
    unsigned char stale[4];

    if (at < kHeaderSize || CacheFolder(folder, false, &dirID) != noErr)
        return;
    if (OpenDataFork(folder->vRefNum, dirID, kFileName, fsRdWrPerm, &refNum) != noErr)
        return;
    Put32(stale, kStale);
    if (SetFPos(refNum, fsFromStart, at) == noErr)
        FSWrite(refNum, &count, stale);
    FSClose(refNum);
    FlushVol(nil, folder->vRefNum);
}
