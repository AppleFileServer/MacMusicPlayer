#include <string.h>
#include <Files.h>
#include <Memory.h>
#include <TextUtils.h>

#include "library.h"
#include "fileopen.h"
#include "songfile.h"
#include "fileops.h"
#include "libcache.h"

#define kDirectoryAttribute     0x10    /* ioFlAttrib bit 4 */

/* One scanned song. Strings are byte offsets into the string pool. */
typedef struct
{
    long id;
    long seconds;
    long sampleRate;            /* N12-5: 22050 or 22254 */
    long audioOffset;
    long audioLength;
    unsigned long fingerprint;  /* D31 */
    long title, artistName, albumName, genreName, composerName, fileName;
    short track;
    short flags;
    short artist, album, genre, composer;   /* group index or kNone */
} SongRec;

/* Distinct names of one kind (artists, albums...), sorted. */
typedef struct
{
    short count;
    long names[kMaxGroups];     /* pool offsets */
} NameSet;

/* Song records and the string pool live in relocatable handles that grow
   during a scan and are locked once it finishes, so the UI can hold
   string pointers into the pool between scans. */
static Handle gSongsH;
static Handle gPoolH;
static short gSongCount;
static long gPoolSize;

static short gSongsByTitle[kMaxSongs];
static short gSongsByID[kMaxSongs];     /* for Library_SongForID */
static NameSet gArtists, gAlbums, gGenres, gComposers;

/* Playlist names, in the order found or created. Kept out of the string
   pool so a new playlist can be added without moving anything the UI
   points at; Library_ListPlaylists returns them sorted. */
static Str31 gPlaylistNames[kMaxPlaylists];
static short gPlaylistCount;

/* The scanned folder: playlist files are read and written on demand. */
static LibraryFolder gFolder;
static Boolean gHaveFolder;

/* Song IDs read from one playlist file. */
static long gPlaylistIDs[kMaxPlaylistEntries];

static long gMaxSongID;
static short gSkipped;
static short gOverLimit;                /* song files beyond kMaxSongs */
static short gPlaylistsOverLimit;       /* playlist files beyond kMaxPlaylists */

/* One song file as read; static to keep it off the small stack. */
static SongInfo gInfo;

/* 1.1, the cache (libcache.c): during a scan, whether it's in use, how
   many songs were read from their files and how many came from it, and
   each song's file date and length (a handle freed when the scan ends).
   After it, where each song's entry is in the cache file, for
   Library_ForgetCached (-1: none). */
static Boolean gUseCache;
static short gReadFromFiles, gFromCache;
static Handle gStatsH;
typedef struct { unsigned long modDate; long length; } FileStat;
#define Stats()             ((FileStat *)*gStatsH)
static long gCacheAt[kMaxSongs];

/* 1.1 (N11-8): called once a scan, when its first song has to be read
   from its file (not when every song comes from the cache). */
static LibraryNotice gReadingNotice;

#define Songs()             ((SongRec *)*gSongsH)
#define PoolString(offset)  ((ConstStringPtr)(*gPoolH + (offset)))

void LibFilter_Clear(LibFilter *filter)
{
    filter->genre = kNone;
    filter->artist = kNone;
    filter->album = kNone;
    filter->composer = kNone;
    filter->playlist = kNone;
    filter->byAlbum = false;
}

/* ---- Sorting ---------------------------------------------------------- */

/* DECISION (D11, D14): names sort case-insensitively with the ROM's
   RelString, ignoring a leading "The ". A project decision; the 4G's
   actual collation is unknown. The name is displayed unchanged. */
static Boolean HasThePrefix(ConstStringPtr s)
{
    return s[0] > 4
        && (s[1] == 'T' || s[1] == 't')
        && (s[2] == 'H' || s[2] == 'h')
        && (s[3] == 'E' || s[3] == 'e')
        && s[4] == ' ';
}

/* The ROM's _RelString trap, called directly: Retro68's headers declare
   RelString but provide no glue for it. A0/A1 point at the characters,
   D0 holds length 1 (high word) and length 2 (low word); D0 returns -1,
   0 or 1. No flag bits = case-insensitive, diacritical-sensitive. */
#pragma parameter __D0 RelStringTrap(__A0, __A1, __D0)
pascal long RelStringTrap(const unsigned char *text1,
                          const unsigned char *text2,
                          long lengths) M68K_INLINE(0xA050);

static short CompareNames(ConstStringPtr a, ConstStringPtr b)
{
    const unsigned char *textA = a + 1;
    const unsigned char *textB = b + 1;
    long lengthA = a[0];
    long lengthB = b[0];

    if (HasThePrefix(a))
    {
        textA += 4;
        lengthA -= 4;
    }
    if (HasThePrefix(b))
    {
        textB += 4;
        lengthB -= 4;
    }

    return (short)RelStringTrap(textA, textB, (lengthA << 16) | lengthB);
}

/* 1.1 (N11-23): every song by title, with a merge sort. 1.0's insertion
   sort made about 62,000 comparisons for 500 songs (28 s at Plus speed);
   this makes about 4,000. It's stable, as the insertion sort was (a tie
   keeps the songs' order in the folder), so the order is 1.0's. */
static short gMergeWork[kMaxSongs];

static void SortSongsByTitle(short *songs, short count)
{
    short half, i, j, k;

    if (count < 2)
        return;
    half = count / 2;
    SortSongsByTitle(songs, half);
    SortSongsByTitle(songs + half, count - half);
    for (i = 0, j = half, k = 0; i < half && j < count; )
        gMergeWork[k++] = CompareNames(PoolString(Songs()[songs[i]].title),
                                       PoolString(Songs()[songs[j]].title)) <= 0
                          ? songs[i++] : songs[j++];
    while (i < half)
        gMergeWork[k++] = songs[i++];
    while (j < count)
        gMergeWork[k++] = songs[j++];
    BlockMove(gMergeWork, songs, (long)count * sizeof(short));
}

/* DECISION (D13, Inferred): an album's songs are in track order (an
   insertion sort: an album's few songs). */
static void SortSongsByTrack(short *songs, short count)
{
    short i, j, s;

    for (i = 1; i < count; i++)
    {
        s = songs[i];
        for (j = i; j > 0 && Songs()[songs[j - 1]].track > Songs()[s].track; j--)
            songs[j] = songs[j - 1];
        songs[j] = s;
    }
}

/* 1.1 (N11-32): finds a name in a sorted offset list by binary search.
   Returns the index of the identical name, or -1 if there's none; *at is
   where the name would go (after any that compare equal to it, as 1.0's
   InsertName put it). On the Plus, 1.0's walk along the whole list, a ROM
   call for each name, was two-thirds of a start-up's 33 s. */
static short FindPlace(const long *names, short count, ConstStringPtr name, short *at)
{
    short low = 0, high = count, mid, i;

    while (low < high)
    {
        mid = (short)((low + high) >> 1);
        if (CompareNames(PoolString(names[mid]), name) <= 0)
            low = mid + 1;
        else
            high = mid;
    }
    *at = low;
    /* An identical name compares equal, so it's among those just before. */
    for (i = low - 1; i >= 0 && CompareNames(PoolString(names[i]), name) == 0; i--)
        if (EqualString(PoolString(names[i]), name, true, true))
            return i;
    return -1;
}

static short FindNameString(const NameSet *set, ConstStringPtr name)
{
    short i;

    for (i = 0; i < set->count; i++)
        if (EqualString(PoolString(set->names[i]), name, true, true))
            return i;
    return kNone;
}

short Library_FindArtist(ConstStringPtr name)   { return FindNameString(&gArtists, name); }
short Library_FindAlbum(ConstStringPtr name)    { return FindNameString(&gAlbums, name); }
short Library_FindGenre(ConstStringPtr name)    { return FindNameString(&gGenres, name); }
short Library_FindComposer(ConstStringPtr name) { return FindNameString(&gComposers, name); }

/* ---- Scanning --------------------------------------------------------- */

void Library_Clear(void)
{
    if (gSongsH != nil)
        DisposeHandle(gSongsH);
    if (gPoolH != nil)
        DisposeHandle(gPoolH);
    gSongsH = nil;
    gPoolH = nil;

    gSongCount = 0;
    gPoolSize = 0;
    gArtists.count = gAlbums.count = gGenres.count = gComposers.count = 0;
    gPlaylistCount = 0;
    gHaveFolder = false;
    gMaxSongID = 0;
    gSkipped = 0;
    gOverLimit = 0;
    gPlaylistsOverLimit = 0;
}

/* Appends a Pascal string to the pool; returns its offset or -1. */
static long PoolAdd(ConstStringPtr s)
{
    long offset = gPoolSize;

    SetHandleSize(gPoolH, gPoolSize + s[0] + 1);
    if (MemError() != noErr)
        return -1;
    BlockMove(s, *gPoolH + offset, s[0] + 1);
    gPoolSize += s[0] + 1;
    return offset;
}

/* Reads and checks one file. Returns false if it is not a valid song
   (the caller counts it as skipped); sets *outOfMemory on a memory error. */
static Boolean AddSongFile(const LibraryFolder *folder, ConstStringPtr fileName,
                           unsigned long modDate, long length, Boolean *outOfMemory)
{
    SongRec song;
    long offsets[kSongTagCount + 1];
    long at = -1;
    Boolean cached;
    short i;

    /* 1.1: from the cache if the file is as it was; else from the file. */
    cached = gUseCache && LibCache_Find(fileName, modDate, length, &gInfo, &at);
    if (!cached && !SongFile_Read(folder->vRefNum, folder->folderID, fileName, &gInfo))
        return false;

    for (i = 0; i < gSongCount; i++)
        if (Songs()[i].id == gInfo.id)
            return false;               /* duplicate ID: keep the first */

    song.id = gInfo.id;
    song.seconds = gInfo.seconds;
    song.sampleRate = gInfo.sampleRate;
    song.audioOffset = gInfo.audioOffset;
    song.audioLength = gInfo.audioLength;
    song.fingerprint = gInfo.fingerprint;
    song.track = gInfo.track;
    song.flags = gInfo.flags;

    for (i = 0; i < kSongTagCount; i++)
        offsets[i] = PoolAdd(gInfo.text[i]);
    offsets[kSongTagCount] = PoolAdd(fileName);
    for (i = 0; i <= kSongTagCount; i++)
        if (offsets[i] < 0)
        {
            *outOfMemory = true;
            return false;
        }

    song.title = offsets[0];
    song.artistName = offsets[1];
    song.albumName = offsets[2];
    song.genreName = offsets[3];
    song.composerName = offsets[4];
    song.fileName = offsets[5];

    SetHandleSize(gSongsH, (long)(gSongCount + 1) * sizeof(SongRec));
    if (MemError() == noErr && gStatsH != nil)
        SetHandleSize(gStatsH, (long)(gSongCount + 1) * sizeof(FileStat));
    if (MemError() != noErr)
    {
        *outOfMemory = true;
        return false;
    }
    if (gStatsH != nil)
    {
        Stats()[gSongCount].modDate = modDate;
        Stats()[gSongCount].length = length;
    }
    gCacheAt[gSongCount] = at;
    Songs()[gSongCount++] = song;
    if (cached)
        gFromCache++;
    else
        gReadFromFiles++;

    if (song.id > gMaxSongID)
        gMaxSongID = song.id;
    return true;
}

/* Lists the files in folderID, calling AddSongFile on each. */
static Boolean ScanSongs(const LibraryFolder *folder)
{
    CInfoPBRec pb;
    Str63 name;
    Boolean outOfMemory = false;
    short index;

    for (index = 1; ; index++)
    {
        pb.hFileInfo.ioNamePtr = name;
        pb.hFileInfo.ioVRefNum = folder->vRefNum;
        pb.hFileInfo.ioFDirIndex = index;
        pb.hFileInfo.ioDirID = folder->folderID;    /* reset: it is output too */
        if (PBGetCatInfoSync(&pb) != noErr)
            break;

        if (pb.hFileInfo.ioFlAttrib & kDirectoryAttribute)
            continue;                   /* Playlists and other folders */
        if (pb.hFileInfo.ioFlFndrInfo.fdType == kLibraryMarkerType)
            continue;                   /* "MacMusic Library Info" (5b-3) */

        if (gSongCount >= kMaxSongs)
        {
            if (pb.hFileInfo.ioFlFndrInfo.fdType == kSongFileType)
                gOverLimit++;           /* M3: reported apart */
            else
                gSkipped++;
        }
        else if (!AddSongFile(folder, name, pb.hFileInfo.ioFlMdDat,
                              pb.hFileInfo.ioFlLgLen, &outOfMemory))
            gSkipped++;

        if (outOfMemory)
            return false;
    }
    return true;
}

/* Collects playlist names: the names of the files in Playlists
   (DECISION P10: any file there is a playlist). Contents are read when a
   playlist is opened. */
static Boolean ScanPlaylists(const LibraryFolder *folder)
{
    CInfoPBRec pb;
    Str63 name;
    short index;

    /* The catalog lists files by name, so with more than kMaxPlaylists
       the first ones by name are used; the rest are only counted (8e-2),
       and left on the disk as they are. */
    for (index = 1; ; index++)
    {
        pb.hFileInfo.ioNamePtr = name;
        pb.hFileInfo.ioVRefNum = folder->vRefNum;
        pb.hFileInfo.ioFDirIndex = index;
        pb.hFileInfo.ioDirID = folder->playlistsID;
        if (PBGetCatInfoSync(&pb) != noErr)
            break;

        if (pb.hFileInfo.ioFlAttrib & kDirectoryAttribute)
            continue;

        if (name[0] > 31)
            continue;                   /* HFS names never are */
        if (gPlaylistCount >= kMaxPlaylists)
        {
            gPlaylistsOverLimit++;
            continue;
        }
        BlockMove(name, gPlaylistNames[gPlaylistCount++], name[0] + 1);
    }
    return true;
}

enum { kArtist, kAlbum, kGenre, kComposer };

static long *GroupField(SongRec *song, short kind)
{
    return kind == kArtist ? &song->artistName : kind == kAlbum ? &song->albumName
         : kind == kGenre ? &song->genreName : &song->composerName;
}

static short *GroupOf(SongRec *song, short kind)
{
    return kind == kArtist ? &song->artist : kind == kAlbum ? &song->album
         : kind == kGenre ? &song->genre : &song->composer;
}

/* 1.1 (N11-23, N11-32): one kind of group. Each song notes the song whose
   name it's grouped under (itself, if its name is new) as the list is
   built; once the list is finished, those songs take their names' places
   and the others copy them, without comparing names again (each
   comparison is a call to the ROM). */
static short gNameSong[kMaxGroups];     /* the song each name came from */
static short gUnder[kMaxSongs];         /* the song whose name each is under */

static void GroupSongs(NameSet *set, short kind)
{
    short i, j, k, at;
    SongRec *song;
    long name;

    for (i = 0; i < gSongCount; i++)
    {
        song = &Songs()[i];
        gUnder[i] = -1;
        name = *GroupField(song, kind);
        if ((song->flags & kSongFlagAudiobook) || PoolString(name)[0] == 0)
            continue;
        k = FindPlace(set->names, set->count, PoolString(name), &at);
        if (k >= 0)
            gUnder[i] = gNameSong[k];
        else if (set->count < kMaxGroups)       /* D20: a full table leaves it out */
        {
            for (j = set->count; j > at; j--)
            {
                set->names[j] = set->names[j - 1];
                gNameSong[j] = gNameSong[j - 1];
            }
            set->names[at] = name;
            gNameSong[at] = i;
            set->count++;
            gUnder[i] = i;
        }
    }
    for (i = 0; i < gSongCount; i++)
        *GroupOf(&Songs()[i], kind) = kNone;
    for (j = 0; j < set->count; j++)
        *GroupOf(&Songs()[gNameSong[j]], kind) = j;
    for (i = 0; i < gSongCount; i++)
        if (gUnder[i] >= 0 && gUnder[i] != i)
            *GroupOf(&Songs()[i], kind) = *GroupOf(&Songs()[gUnder[i]], kind);
}

/* Groups songs into artists, albums, genres and composers. Runs once the
   pool is locked, since it compares strings in place. */
static void IndexSongs(void)
{
    short i, j, s;

    /* PROVISIONAL (D18): audiobooks appear only under Audiobooks.
       PROVISIONAL (D19): albums are grouped by name alone.
       DECISION (D20): an empty field creates no group; such songs are
       still reached through "All". A full group table leaves the song
       ungrouped in the same way. */
    GroupSongs(&gArtists, kArtist);
    GroupSongs(&gAlbums, kAlbum);
    GroupSongs(&gGenres, kGenre);
    GroupSongs(&gComposers, kComposer);

    for (i = 0; i < gSongCount; i++)
        gSongsByTitle[i] = i;
    SortSongsByTitle(gSongsByTitle, gSongCount);

    /* Songs sorted by ID, so playlists can find them by binary search. */
    for (i = 0; i < gSongCount; i++)
    {
        s = i;
        for (j = i; j > 0 && Songs()[gSongsByID[j - 1]].id > Songs()[s].id; j--)
            gSongsByID[j] = gSongsByID[j - 1];
        gSongsByID[j] = s;
    }
}

/* 1.1: writes a new cache from the scanned songs, in their folder order. */
static void SaveCache(const LibraryFolder *folder)
{
    SongRec *song;
    short i;

    if (gStatsH == nil || LibCache_BeginSave(folder) != noErr)
    {
        LibCache_EndSave();
        for (i = 0; i < gSongCount; i++)
            gCacheAt[i] = -1;
        return;
    }
    for (i = 0; i < gSongCount; i++)
    {
        song = &Songs()[i];
        memset(&gInfo, 0, sizeof gInfo);
        gInfo.id = song->id;
        gInfo.seconds = song->seconds;
        gInfo.sampleRate = song->sampleRate;
        gInfo.audioOffset = song->audioOffset;
        gInfo.audioLength = song->audioLength;
        gInfo.fingerprint = song->fingerprint;
        gInfo.track = song->track;
        gInfo.flags = song->flags;
        BlockMove(PoolString(song->title), gInfo.text[0], PoolString(song->title)[0] + 1);
        BlockMove(PoolString(song->artistName), gInfo.text[1], PoolString(song->artistName)[0] + 1);
        BlockMove(PoolString(song->albumName), gInfo.text[2], PoolString(song->albumName)[0] + 1);
        BlockMove(PoolString(song->genreName), gInfo.text[3], PoolString(song->genreName)[0] + 1);
        BlockMove(PoolString(song->composerName), gInfo.text[4], PoolString(song->composerName)[0] + 1);
        gCacheAt[i] = LibCache_SaveEntry(PoolString(song->fileName), Stats()[i].modDate,
                                         Stats()[i].length, &gInfo);
    }
    if (LibCache_EndSave() != noErr)
        for (i = 0; i < gSongCount; i++)
            gCacheAt[i] = -1;
}

/* One pass over the folder, with or without the cache. */
static Boolean ScanOnce(const LibraryFolder *folder, Boolean useCache)
{
    Library_Clear();
    gUseCache = useCache;
    gReadFromFiles = gFromCache = 0;
    if (gStatsH != nil)
        SetHandleSize(gStatsH, 0);

    gSongsH = NewHandle(0);
    gPoolH = NewHandle(0);
    if (gSongsH == nil || gPoolH == nil)
        return false;
    return ScanSongs(folder) && ScanPlaylists(folder);
}

static Boolean ScanWith(const LibraryFolder *folder, Boolean useCache)
{
    Boolean ok;

    /* N11-9: the cache is held in memory only during the scan. If it
       can't be loaded, or the scan then runs short of memory, the scan is
       done again without it, as 1.0 does: any library 1.0 can read, 1.1
       can. */
    if (gReadingNotice != nil)
        gReadingNotice();               /* N11-29: from the start */
    useCache = useCache && LibCache_Load(folder);
    gStatsH = NewHandle(0);
    ok = ScanOnce(folder, useCache);
    if (!ok && useCache)
    {
        LibCache_Unload();
        ok = ScanOnce(folder, false);
        useCache = false;
    }

    /* From here on the handles never grow, so lock them high in the heap
       and let the UI keep pointers into the pool. */
    if (ok)
    {
        MoveHHi(gSongsH);
        HLock(gSongsH);
        MoveHHi(gPoolH);
        HLock(gPoolH);
    }
    else
    {
        Library_Clear();
        LibCache_Unload();
        if (gStatsH != nil)
            DisposeHandle(gStatsH);
        gStatsH = nil;
        return false;
    }

    IndexSongs();
    gFolder = *folder;
    gHaveFolder = true;

    /* A new cache if anything is different: no cache, a song read from
       its file, or one in the cache that's gone. (A file that isn't a
       song is read again each time, and changes nothing.) */
    if (!useCache || gReadFromFiles > 0 || gFromCache != LibCache_Count())
    {
        LibCache_Unload();
        SaveCache(folder);
    }
    LibCache_Unload();
    if (gStatsH != nil)
        DisposeHandle(gStatsH);
    gStatsH = nil;
    gUseCache = false;
    return true;
}

Boolean Library_Scan(const LibraryFolder *folder)
{
    return ScanWith(folder, true);
}

Boolean Library_ScanAll(const LibraryFolder *folder)
{
    return ScanWith(folder, false);
}

short Library_ReadFromFilesCount(void)
{
    return gReadFromFiles;
}

void Library_SetReadingNotice(LibraryNotice notice)
{
    gReadingNotice = notice;
}

void Library_ForgetCached(short song)
{
    if (gHaveFolder && song >= 0 && song < gSongCount && gCacheAt[song] >= 0)
    {
        LibCache_Forget(&gFolder, gCacheAt[song]);
        gCacheAt[song] = -1;
    }
}

short Library_SkippedCount(void)
{
    return gSkipped;
}

short Library_SongCount(void)
{
    return gSongCount;
}

short Library_OverLimitCount(void)
{
    return gOverLimit;
}

short Library_PlaylistsOverLimitCount(void)
{
    return gPlaylistsOverLimit;
}

long Library_TotalAudioBytes(void)
{
    long total = 0;
    short i;

    for (i = 0; i < gSongCount; i++)
        total += Songs()[i].audioLength;
    return total;
}

short Library_PlaylistCount(void)
{
    return gPlaylistCount;
}

/* One more than the highest valid ID found by the last scan. Import also
   takes the library's next-ID counter into account (M4), so an ID is
   never given out twice, even after the highest-numbered song has been
   exported. */
long Library_NextSongID(void)
{
    return gMaxSongID + 1;
}

short Library_FindAudioMatch(long audioLength, unsigned long fingerprint)
{
    short i;

    for (i = 0; i < gSongCount; i++)
    {
        const SongRec *song = &Songs()[i];

        if (song->audioLength == audioLength && song->fingerprint == fingerprint)
            return i;
    }
    return kNone;
}

/* ---- Playlist contents ------------------------------------------------ */

/* A playlist file is text, one song per line: the decimal song ID, a tab,
   then the title when added (DECISION P1; only the ID is used). Lines
   end in CR, LF or CRLF; lines not starting with a digit, and IDs <= 0,
   are ignored. Membership is by ID only, never position or file name. */

short Library_SongForID(long id)
{
    short low = 0, high = gSongCount - 1, mid;
    long midID;

    while (low <= high)
    {
        mid = (low + high) / 2;
        midID = Songs()[gSongsByID[mid]].id;
        if (midID == id)
            return gSongsByID[mid];
        if (midID < id)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return kNone;
}

/* Reads up to max song IDs from a playlist file into gPlaylistIDs. */
static short ReadPlaylistIDs(short playlist, short max)
{
    unsigned char buf[256];
    long count, id = 0;
    short refNum, i, n = 0;
    Boolean lineStart = true, inNumber = false, skipLine = false;
    unsigned char c;
    OSErr err;

    if (!gHaveFolder || playlist < 0 || playlist >= gPlaylistCount)
        return 0;
    if (OpenDataFork(gFolder.vRefNum, gFolder.playlistsID,
                     gPlaylistNames[playlist], fsRdPerm, &refNum) != noErr)
        return 0;

    do
    {
        count = sizeof(buf);
        err = FSRead(refNum, &count, buf);
        for (i = 0; i < count && n < max; i++)
        {
            c = buf[i];
            if (c == '\r' || c == '\n')
            {
                if (inNumber && id > 0)
                    gPlaylistIDs[n++] = id;
                lineStart = true;
                inNumber = skipLine = false;
                id = 0;
            }
            else if (skipLine)
                ;
            else if (c >= '0' && c <= '9' && (lineStart || inNumber))
            {
                if (id < 100000000L)
                    id = id * 10 + (c - '0');
                inNumber = true;
                lineStart = false;
            }
            else
            {
                /* Tab (or anything) after the number ends it; a line
                   that does not start with a digit is ignored. */
                if (inNumber && id > 0)
                    gPlaylistIDs[n++] = id;
                inNumber = false;
                skipLine = true;
            }
        }
    } while (err == noErr && count > 0 && n < max);

    if (inNumber && id > 0 && n < max)     /* last line without an end */
        gPlaylistIDs[n++] = id;

    FSClose(refNum);
    return n;
}

/* DECISION P8: IDs not in the library are skipped (they stay in the file). */
static short ListPlaylistSongs(short playlist, short *out, short max)
{
    short count = ReadPlaylistIDs(playlist, kMaxPlaylistEntries);
    short i, song, n = 0;

    for (i = 0; i < count && n < max; i++)
    {
        song = Library_SongForID(gPlaylistIDs[i]);
        if (song != kNone)
            out[n++] = song;
    }
    return n;
}

Boolean Library_PlaylistContains(short playlist, short song)
{
    short count = ReadPlaylistIDs(playlist, kMaxPlaylistEntries);
    short i;

    for (i = 0; i < count; i++)
        if (gPlaylistIDs[i] == Songs()[song].id)
            return true;
    return false;
}

short Library_FindPlaylist(ConstStringPtr name)
{
    short i;

    /* HFS names ignore case, so playlists do too. */
    for (i = 0; i < gPlaylistCount; i++)
        if (EqualString(gPlaylistNames[i], name, false, true))
            return i;
    return kNone;
}

OSErr Library_AddPlaylist(ConstStringPtr name, short *playlist)
{
    OSErr err;

    if (!gHaveFolder)
        return fnfErr;
    if (gPlaylistCount >= kMaxPlaylists)
        return dirFulErr;
    if (Library_FindPlaylist(name) != kNone)
        return dupFNErr;

    err = HCreate(gFolder.vRefNum, gFolder.playlistsID, name,
                  kAppCreator, kPlaylistFileType);
    if (err != noErr)
        return err;
    FlushVol(nil, gFolder.vRefNum);

    BlockMove(name, gPlaylistNames[gPlaylistCount], name[0] + 1);
    *playlist = gPlaylistCount++;
    return noErr;
}

OSErr Library_AppendToPlaylist(short playlist, short song)
{
    unsigned char line[80];
    unsigned char last;
    Str255 number;
    ConstStringPtr title = Library_SongTitle(song);
    long length = 0, count;
    short refNum;
    OSErr err;

    if (!gHaveFolder || playlist < 0 || playlist >= gPlaylistCount)
        return fnfErr;

    err = OpenDataFork(gFolder.vRefNum, gFolder.playlistsID,
                       gPlaylistNames[playlist], fsRdWrPerm, &refNum);
    if (err != noErr)
        return err;

    /* Start on a new line if the file does not end with one. */
    err = GetEOF(refNum, &length);
    if (err == noErr && length > 0)
    {
        count = 1;
        err = SetFPos(refNum, fsFromStart, length - 1);
        if (err == noErr)
            err = FSRead(refNum, &count, &last);
        count = 0;
        if (err == noErr && last != '\r' && last != '\n')
            line[count++] = '\r';
    }
    else
        count = 0;

    /* "<id><tab><title><CR>" */
    NumToString(Songs()[song].id, number);
    BlockMove(number + 1, line + count, number[0]);
    count += number[0];
    line[count++] = '\t';
    BlockMove(title + 1, line + count, title[0]);
    count += title[0];
    line[count++] = '\r';

    if (err == noErr)
        err = SetFPos(refNum, fsFromLEOF, 0);
    if (err == noErr)
        err = FSWrite(refNum, &count, line);
    FSClose(refNum);
    if (err == noErr)
        err = FlushVol(nil, gFolder.vRefNum);
    return err;
}

/* The leading song ID of the line starting at text[at], or 0 if the line
   does not start with a digit (as ReadPlaylistIDs reads it). */
static long LineID(const unsigned char *text, long at, long length)
{
    long id = 0;

    while (at < length && text[at] >= '0' && text[at] <= '9')
    {
        if (id < 100000000L)
            id = id * 10 + (text[at] - '0');
        at++;
    }
    return id;
}

/* Rewrites one playlist file without the lines for id, or (title not
   nil, Stage 8c) with those lines rewritten as "<id><tab><title>", each
   keeping its own line ending. With nth >= 0 (Stage 8d-4) only the nth
   line for id (0-based) is affected. Files without the ID are not
   rewritten. */
static OSErr RewritePlaylist(short playlist, long id, ConstStringPtr title,
                             short nth)
{
    short seen = 0;
    Ptr text, out = nil;
    long length, count, at, end, body, kept = 0;
    short refNum;
    Boolean changed = false;
    Str255 number;
    OSErr err;

    err = OpenDataFork(gFolder.vRefNum, gFolder.playlistsID,
                       gPlaylistNames[playlist], fsRdWrPerm, &refNum);
    if (err != noErr)
        return err;

    err = GetEOF(refNum, &length);
    text = err == noErr ? NewPtr(length > 0 ? length : 1) : nil;
    if (err == noErr && text == nil)
        err = memFullErr;
    count = length;
    if (err == noErr && length > 0)
        err = FSRead(refNum, &count, text);
    if (err == noErr && count != length)
        err = ioErr;

    /* The output: no longer than the file when removing; a rewritten line
       ("<id><tab><title>") is at most 11 + 1 + 63 characters plus its
       ending, so allow for every line being one. */
    if (err == noErr)
    {
        out = NewPtr(title == nil ? (length > 0 ? length : 1)
                                  : length + (length / 2 + 1) * 80);
        if (out == nil)
            err = memFullErr;
    }
    NumToString(id, number);

    /* Each line (with its CR, LF or CRLF ending) is copied unless it
       starts with this ID; such a line is dropped, or rewritten. */
    for (at = 0; err == noErr && at < length; at = end)
    {
        end = at;
        while (end < length && text[end] != '\r' && text[end] != '\n')
            end++;
        body = end;
        if (end < length && text[end] == '\r')
            end++;
        if (end < length && text[end] == '\n')
            end++;

        if (LineID((unsigned char *)text, at, length) != id
            || (nth >= 0 && seen++ != nth))
        {
            BlockMove(text + at, out + kept, end - at);
            kept += end - at;
            continue;
        }
        changed = true;
        if (title == nil)
            continue;
        BlockMove(number + 1, out + kept, number[0]);
        kept += number[0];
        out[kept++] = '\t';
        BlockMove(title + 1, out + kept, title[0]);
        kept += title[0];
        BlockMove(text + body, out + kept, end - body);
        kept += end - body;
    }

    if (err == noErr && changed)
    {
        count = kept;
        err = SetFPos(refNum, fsFromStart, 0);
        if (err == noErr)
            err = FSWrite(refNum, &count, out);
        if (err == noErr)
            err = SetEOF(refNum, kept);
    }
    if (out != nil)
        DisposePtr(out);
    if (text != nil)
        DisposePtr(text);
    FSClose(refNum);
    return err;
}

OSErr Library_RemoveFromPlaylists(long id)
{
    short i;
    OSErr err, first = noErr;

    if (!gHaveFolder || id <= 0)
        return noErr;
    for (i = 0; i < gPlaylistCount; i++)
    {
        err = RewritePlaylist(i, id, nil, -1);
        if (first == noErr)
            first = err;
    }
    FlushVol(nil, gFolder.vRefNum);
    return first;
}

OSErr Library_RenamePlaylist(short playlist, ConstStringPtr name)
{
    short other = Library_FindPlaylist(name);
    OSErr err;

    if (!gHaveFolder || playlist < 0 || playlist >= gPlaylistCount
        || name[0] < 1 || name[0] > 31)
        return paramErr;
    if (other != kNone && other != playlist)
        return dupFNErr;            /* a change of case only is allowed */
    err = RenameFile(gFolder.vRefNum, gFolder.playlistsID,
                     gPlaylistNames[playlist], name);
    if (err != noErr)
        return err;
    FlushVol(nil, gFolder.vRefNum);
    BlockMove(name, gPlaylistNames[playlist], name[0] + 1);
    return noErr;
}

OSErr Library_DeletePlaylist(short playlist)
{
    OSErr err;
    short i;

    if (!gHaveFolder || playlist < 0 || playlist >= gPlaylistCount)
        return paramErr;
    err = HDelete(gFolder.vRefNum, gFolder.playlistsID,
                  gPlaylistNames[playlist]);
    if (err != noErr)
        return err;
    FlushVol(nil, gFolder.vRefNum);
    for (i = playlist; i < gPlaylistCount - 1; i++)
        BlockMove(gPlaylistNames[i + 1], gPlaylistNames[i],
                  gPlaylistNames[i + 1][0] + 1);
    gPlaylistCount--;
    return noErr;
}

OSErr Library_RemoveFromPlaylist(short playlist, long id, short nth)
{
    OSErr err;

    if (!gHaveFolder || playlist < 0 || playlist >= gPlaylistCount || nth < 0)
        return paramErr;
    err = RewritePlaylist(playlist, id, nil, nth);
    FlushVol(nil, gFolder.vRefNum);
    return err;
}

OSErr Library_RetitleInPlaylists(long id, ConstStringPtr title)
{
    short i;
    OSErr err, first = noErr;

    if (!gHaveFolder || id <= 0)
        return noErr;
    for (i = 0; i < gPlaylistCount; i++)
    {
        err = RewritePlaylist(i, id, title, -1);
        if (first == noErr)
            first = err;
    }
    FlushVol(nil, gFolder.vRefNum);
    return first;
}

/* ---- Queries ---------------------------------------------------------- */

static long gSongChecks;    /* songs looked at by the lists, for tests */

static Boolean SongMatches(short s, const LibFilter *f)
{
    const SongRec *song = &Songs()[s];

    return !(song->flags & kSongFlagAudiobook)
        && (f->genre == kNone || song->genre == f->genre)
        && (f->artist == kNone || song->artist == f->artist)
        && (f->album == kNone || song->album == f->album)
        && (f->composer == kNone || song->composer == f->composer);
}

static short ListAll(short count, short *out, short max)
{
    short i;

    for (i = 0; i < count && i < max; i++)
        out[i] = i;
    return i;
}

short Library_ListGenres(short *out, short max)
{
    return ListAll(gGenres.count, out, max);
}

short Library_ListComposers(short *out, short max)
{
    return ListAll(gComposers.count, out, max);
}

/* Playlists sorted by name (D11, P2). */
short Library_ListPlaylists(short *out, short max)
{
    short i, j, n = 0;

    for (i = 0; i < gPlaylistCount && n < max; i++)
    {
        for (j = n; j > 0 && CompareNames(gPlaylistNames[out[j - 1]],
                                          gPlaylistNames[i]) > 0; j--)
            out[j] = out[j - 1];
        out[j] = i;
        n++;
    }
    return n;
}

/* 1.2 (N12-11): the groups with a song the filter takes, in one pass over
   the songs (each group marked), then in the groups' order: the same list
   as 1.1's, which checked every group against every song (for a library
   of 204 artists, 272 albums and 376 songs, printing the Albums and
   Artists lists took tens of millions of checks: the owner's Plus seemed
   to hang). */
static Boolean gMarked[kMaxGroups];

static short ListGroups(Boolean albums, short groups, const LibFilter *filter,
                        short *out, short max)
{
    short g, s, n = 0;

    for (g = 0; g < groups; g++)
        gMarked[g] = false;
    gSongChecks += gSongCount;
    for (s = 0; s < gSongCount; s++)
    {
        g = albums ? Songs()[s].album : Songs()[s].artist;
        if (g != kNone && !gMarked[g] && SongMatches(s, filter))
            gMarked[g] = true;
    }
    for (g = 0; g < groups && n < max; g++)
        if (gMarked[g])
            out[n++] = g;
    return n;
}

short Library_ListArtists(const LibFilter *filter, short *out, short max)
{
    return ListGroups(false, gArtists.count, filter, out, max);
}

short Library_ListAlbums(const LibFilter *filter, short *out, short max)
{
    return ListGroups(true, gAlbums.count, filter, out, max);
}

long Library_SongChecks(void)
{
    return gSongChecks;
}

/* 1.2 (N12-3, PROVISIONAL): an "All" row's songs by album (in the albums'
   order, D14; songs with no album last), then track (as D13), with a
   stable merge sort, so songs that tie keep their title order (D14). */
static long AlbumTrackKey(short song)
{
    const SongRec *r = &Songs()[song];

    return ((long)(r->album == kNone ? kMaxGroups : r->album) << 16)
         | (unsigned short)r->track;
}

static void SortSongsByAlbum(short *songs, short count)
{
    short half, i, j, k;

    if (count < 2)
        return;
    half = count / 2;
    SortSongsByAlbum(songs, half);
    SortSongsByAlbum(songs + half, count - half);
    for (i = 0, j = half, k = 0; i < half && j < count; )
        gMergeWork[k++] = AlbumTrackKey(songs[i]) <= AlbumTrackKey(songs[j])
                          ? songs[i++] : songs[j++];
    while (i < half)
        gMergeWork[k++] = songs[i++];
    while (j < count)
        gMergeWork[k++] = songs[j++];
    BlockMove(gMergeWork, songs, (long)count * sizeof(short));
}

short Library_ListSongs(const LibFilter *filter, short *out, short max)
{
    short i, s, n = 0;

    if (filter->playlist != kNone)
        return ListPlaylistSongs(filter->playlist, out, max);

    for (i = 0; i < gSongCount && n < max; i++)
    {
        s = gSongsByTitle[i];
        if (SongMatches(s, filter))
            out[n++] = s;
    }

    if (filter->album != kNone)
        SortSongsByTrack(out, n);
    else if (filter->byAlbum)
        SortSongsByAlbum(out, n);

    return n;
}

short Library_ListAudiobooks(short *out, short max)
{
    short i, s, n = 0;

    for (i = 0; i < gSongCount && n < max; i++)
    {
        s = gSongsByTitle[i];
        if (Songs()[s].flags & kSongFlagAudiobook)
            out[n++] = s;
    }
    return n;
}

ConstStringPtr Library_GenreName(short genre)       { return PoolString(gGenres.names[genre]); }
ConstStringPtr Library_ArtistName(short artist)     { return PoolString(gArtists.names[artist]); }
ConstStringPtr Library_AlbumName(short album)       { return PoolString(gAlbums.names[album]); }
ConstStringPtr Library_ComposerName(short composer) { return PoolString(gComposers.names[composer]); }
ConstStringPtr Library_PlaylistName(short playlist) { return gPlaylistNames[playlist]; }
ConstStringPtr Library_SongTitle(short song)        { return PoolString(Songs()[song].title); }
long Library_SongID(short song)                     { return Songs()[song].id; }
ConstStringPtr Library_SongArtistName(short song)   { return PoolString(Songs()[song].artistName); }
ConstStringPtr Library_SongAlbumName(short song)    { return PoolString(Songs()[song].albumName); }
long Library_SongSeconds(short song)                { return Songs()[song].seconds; }
long Library_SongAudioLength(short song)            { return Songs()[song].audioLength; }
long Library_SongSampleRate(short song)             { return Songs()[song].sampleRate; }

short Library_SongAlbum(short song)        { return Songs()[song].album; }
short Library_SongArtist(short song)       { return Songs()[song].artist; }
short Library_SongTrack(short song)        { return Songs()[song].track; }
Boolean Library_SongIsAudiobook(short song)
{
    return (Songs()[song].flags & kSongFlagAudiobook) != 0;
}

short Library_SongRating(short song)
{
    return SongFlags_Rating(Songs()[song].flags);
}

OSErr Library_SetSongRating(short song, short rating)
{
    short flags = SongFlags_WithRating(Songs()[song].flags, rating);
    OSErr err;

    if (!gHaveFolder)
        return fnfErr;
    if (flags == Songs()[song].flags)
        return noErr;
    Library_ForgetCached(song);         /* 1.1: before the file changes */
    err = SongFile_WriteFlags(gFolder.vRefNum, gFolder.folderID,
                              PoolString(Songs()[song].fileName),
                              Songs()[song].id, flags);
    if (err == noErr)
        Songs()[song].flags = flags;
    return err;
}

Boolean Library_UpdateSong(short song, const SongInfo *info)
{
    long offsets[kSongTagCount];
    SongRec *rec;
    short i;
    Boolean ok = true;

    /* The new names go on the end of the pool; the old ones stay unused
       until the next scan. The pool must be unlocked to grow, so it may
       move: everything that points into it is rebuilt by the caller. */
    HUnlock(gPoolH);
    for (i = 0; i < kSongTagCount && ok; i++)
    {
        offsets[i] = PoolAdd(info->text[i]);
        ok = offsets[i] >= 0;
    }
    MoveHHi(gPoolH);
    HLock(gPoolH);
    if (!ok)
        return false;

    rec = &Songs()[song];
    rec->title = offsets[0];
    rec->artistName = offsets[1];
    rec->albumName = offsets[2];
    rec->genreName = offsets[3];
    rec->composerName = offsets[4];
    rec->track = info->track;
    rec->flags = info->flags;

    /* The groups and sort orders are rebuilt from the records, as after a
       scan, without reading any file. */
    gArtists.count = gAlbums.count = gGenres.count = gComposers.count = 0;
    IndexSongs();
    return true;
}

Boolean Library_SongFile(short song, short *vRefNum, long *dirID,
                         ConstStringPtr *fileName, long *audioOffset)
{
    if (!gHaveFolder)
        return false;
    *vRefNum = gFolder.vRefNum;
    *dirID = gFolder.folderID;
    *fileName = PoolString(Songs()[song].fileName);
    *audioOffset = Songs()[song].audioOffset;
    return true;
}
