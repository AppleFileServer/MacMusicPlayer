#include <Files.h>
#include <Memory.h>
#include <TextUtils.h>

#include "library.h"
#include "fileopen.h"
#include "song_format.h"
#include "bytes.h"

#define kDirectoryAttribute     0x10    /* ioFlAttrib bit 4 */

/* One scanned song. Strings are byte offsets into the string pool. */
typedef struct
{
    long id;
    long seconds;
    long audioOffset;           /* = header length */
    long audioLength;
    unsigned long checksum;     /* valid if flags has kSongFlagHasChecksum */
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
static NameSet gArtists, gAlbums, gGenres, gComposers;

static long gPlaylistNames[kMaxPlaylists];
static short gPlaylistCount;

static long gMaxSongID;
static short gSkipped;

/* Header read buffer; static to keep it off the small stack. */
static unsigned char gHeader[kSongMaxHeaderSize];

#define Songs()             ((SongRec *)*gSongsH)
#define PoolString(offset)  ((ConstStringPtr)(*gPoolH + (offset)))

void LibFilter_Clear(LibFilter *filter)
{
    filter->genre = kNone;
    filter->artist = kNone;
    filter->album = kNone;
    filter->composer = kNone;
    filter->playlist = kNone;
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

/* Insertion sorts; lists are at most kMaxSongs long. */
static void SortSongsByTitle(short *songs, short count)
{
    short i, j, s;

    for (i = 1; i < count; i++)
    {
        s = songs[i];
        for (j = i; j > 0 && CompareNames(PoolString(Songs()[songs[j - 1]].title),
                                          PoolString(Songs()[s].title)) > 0; j--)
            songs[j] = songs[j - 1];
        songs[j] = s;
    }
}

/* DECISION (D13, Inferred): an album's songs are in track order. */
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

/* Inserts a pool string into a sorted offset list, unless an identical
   name is already there. Returns false if the list is full. */
static Boolean InsertName(long *names, short *count, short max, long offset)
{
    ConstStringPtr name = PoolString(offset);
    short i, j;

    for (i = 0; i < *count; i++)
        if (EqualString(PoolString(names[i]), name, true, true))
            return true;

    if (*count >= max)
        return false;

    for (i = 0; i < *count && CompareNames(PoolString(names[i]), name) <= 0; i++)
        ;
    for (j = *count; j > i; j--)
        names[j] = names[j - 1];
    names[i] = offset;
    (*count)++;
    return true;
}

static short FindName(const NameSet *set, long offset)
{
    ConstStringPtr name = PoolString(offset);
    short i;

    for (i = 0; i < set->count; i++)
        if (EqualString(PoolString(set->names[i]), name, true, true))
            return i;
    return kNone;
}

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
    gMaxSongID = 0;
    gSkipped = 0;
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

/* Checks one file's header. On success fills song (except its string
   offsets) and the positions of its five strings within gHeader. */
static Boolean ParseHeader(long fileLength, long bytesRead,
                           SongRec *song, short stringAt[kSongStringCount])
{
    long headerLength, expectedSeconds;
    short i, at;

    if (bytesRead < kSongMinHeaderSize
        || Get32(gHeader + kSongOffsetMagic) != kSongMagic
        || Get16(gHeader + kSongOffsetVersion) != kSongFormatVersion)
        return false;

    headerLength = (long)Get32(gHeader + kSongOffsetHeaderLength);
    if (headerLength < kSongMinHeaderSize || headerLength > kSongMaxHeaderSize
        || headerLength > bytesRead)
        return false;

    if (Get32(gHeader + kSongOffsetEncoding) != kSongEncodingPCMu
        || Get32(gHeader + kSongOffsetSampleRate) != kSongSampleRate
        || Get16(gHeader + kSongOffsetBits) != kSongBitsPerSample
        || Get16(gHeader + kSongOffsetChannels) != kSongChannels)
        return false;

    song->id = (long)Get32(gHeader + kSongOffsetID);
    song->flags = Get16(gHeader + kSongOffsetFlags);
    song->track = Get16(gHeader + kSongOffsetTrack);
    song->seconds = (long)Get32(gHeader + kSongOffsetDuration);
    song->audioOffset = headerLength;
    song->audioLength = (long)Get32(gHeader + kSongOffsetAudioLength);

    if (song->id <= 0 || song->track < 0 || song->audioLength < 0
        || song->audioLength > fileLength - headerLength)
        return false;

    expectedSeconds = (song->audioLength + kSongSampleRate / 2) / kSongSampleRate;
    if (song->seconds != expectedSeconds)
        return false;

    at = kSongOffsetStrings;
    for (i = 0; i < kSongStringCount; i++)
    {
        if (at >= headerLength || gHeader[at] > kSongMaxString
            || at + 1 + gHeader[at] > headerLength)
            return false;
        stringAt[i] = at;
        at += 1 + gHeader[at];
    }

    /* Optional audio checksum straight after the strings (D31). */
    song->checksum = 0;
    if (song->flags & kSongFlagHasChecksum)
    {
        if (at + 4 > headerLength)
            return false;
        song->checksum = Get32(gHeader + at);
    }
    return true;
}

/* Reads and checks one file. Returns false if it is not a valid song
   (the caller counts it as skipped); sets *outOfMemory on a memory error. */
static Boolean AddSongFile(const LibraryFolder *folder, ConstStringPtr fileName,
                           Boolean *outOfMemory)
{
    SongRec song;
    short stringAt[kSongStringCount];
    long fileLength, bytesRead;
    long offsets[kSongStringCount + 1];
    short refNum, i;
    OSErr err;

    if (OpenDataFork(folder->vRefNum, folder->folderID, fileName, fsRdPerm, &refNum)
        != noErr)
        return false;

    err = GetEOF(refNum, &fileLength);
    bytesRead = fileLength < kSongMaxHeaderSize ? fileLength : kSongMaxHeaderSize;
    if (err == noErr)
        err = FSRead(refNum, &bytesRead, gHeader);
    FSClose(refNum);

    if ((err != noErr && err != eofErr)
        || !ParseHeader(fileLength, bytesRead, &song, stringAt))
        return false;

    for (i = 0; i < gSongCount; i++)
        if (Songs()[i].id == song.id)
            return false;               /* duplicate ID: keep the first */

    for (i = 0; i < kSongStringCount; i++)
        offsets[i] = PoolAdd(gHeader + stringAt[i]);
    offsets[kSongStringCount] = PoolAdd(fileName);
    for (i = 0; i <= kSongStringCount; i++)
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
    if (MemError() != noErr)
    {
        *outOfMemory = true;
        return false;
    }
    Songs()[gSongCount++] = song;

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

        if (gSongCount >= kMaxSongs
            || !AddSongFile(folder, name, &outOfMemory))
            gSkipped++;

        if (outOfMemory)
            return false;
    }
    return true;
}

/* Collects playlist names: the names of the files in Playlists. Their
   contents are read in a later stage (3c). */
static Boolean ScanPlaylists(const LibraryFolder *folder)
{
    CInfoPBRec pb;
    Str63 name;
    long offset;
    short index;

    for (index = 1; gPlaylistCount < kMaxPlaylists; index++)
    {
        pb.hFileInfo.ioNamePtr = name;
        pb.hFileInfo.ioVRefNum = folder->vRefNum;
        pb.hFileInfo.ioFDirIndex = index;
        pb.hFileInfo.ioDirID = folder->playlistsID;
        if (PBGetCatInfoSync(&pb) != noErr)
            break;

        if (pb.hFileInfo.ioFlAttrib & kDirectoryAttribute)
            continue;

        offset = PoolAdd(name);
        if (offset < 0)
            return false;
        gPlaylistNames[gPlaylistCount++] = offset;
    }
    return true;
}

/* Groups songs into artists, albums, genres and composers. Runs once the
   pool is locked, since it compares strings in place. */
static void IndexSongs(void)
{
    long names[kMaxPlaylists];
    short count, i;
    SongRec *song;

    /* PROVISIONAL (D18): audiobooks appear only under Audiobooks.
       PROVISIONAL (D19): albums are grouped by name alone.
       DECISION (D20): an empty field creates no group; such songs are
       still reached through "All". A full group table leaves the song
       ungrouped in the same way. */
    for (i = 0; i < gSongCount; i++)
    {
        song = &Songs()[i];
        if (song->flags & kSongFlagAudiobook)
            continue;

        if (PoolString(song->artistName)[0] != 0)
            InsertName(gArtists.names, &gArtists.count, kMaxGroups, song->artistName);
        if (PoolString(song->albumName)[0] != 0)
            InsertName(gAlbums.names, &gAlbums.count, kMaxGroups, song->albumName);
        if (PoolString(song->genreName)[0] != 0)
            InsertName(gGenres.names, &gGenres.count, kMaxGroups, song->genreName);
        if (PoolString(song->composerName)[0] != 0)
            InsertName(gComposers.names, &gComposers.count, kMaxGroups, song->composerName);
    }

    for (i = 0; i < gSongCount; i++)
    {
        song = &Songs()[i];
        song->artist = song->album = song->genre = song->composer = kNone;
        if (song->flags & kSongFlagAudiobook)
            continue;

        if (PoolString(song->artistName)[0] != 0)
            song->artist = FindName(&gArtists, song->artistName);
        if (PoolString(song->albumName)[0] != 0)
            song->album = FindName(&gAlbums, song->albumName);
        if (PoolString(song->genreName)[0] != 0)
            song->genre = FindName(&gGenres, song->genreName);
        if (PoolString(song->composerName)[0] != 0)
            song->composer = FindName(&gComposers, song->composerName);
    }

    for (i = 0; i < gSongCount; i++)
        gSongsByTitle[i] = i;
    SortSongsByTitle(gSongsByTitle, gSongCount);

    /* Playlists: sorted by name (D11), re-using the offset list sort. */
    count = 0;
    for (i = 0; i < gPlaylistCount; i++)
        InsertName(names, &count, kMaxPlaylists, gPlaylistNames[i]);
    for (i = 0; i < count; i++)
        gPlaylistNames[i] = names[i];
    gPlaylistCount = count;
}

Boolean Library_Scan(const LibraryFolder *folder)
{
    Boolean ok;

    Library_Clear();

    gSongsH = NewHandle(0);
    gPoolH = NewHandle(0);
    if (gSongsH == nil || gPoolH == nil)
    {
        Library_Clear();
        return false;
    }

    ok = ScanSongs(folder) && ScanPlaylists(folder);

    /* From here on the handles never grow, so lock them high in the heap
       and let the UI keep pointers into the pool. */
    MoveHHi(gSongsH);
    HLock(gSongsH);
    MoveHHi(gPoolH);
    HLock(gPoolH);

    if (!ok)
    {
        Library_Clear();
        return false;
    }

    IndexSongs();
    return true;
}

short Library_SkippedCount(void)
{
    return gSkipped;
}

/* Song IDs are never reused: the next one is above the highest valid ID
   found by the last scan, whatever songs are missing below it. */
long Library_NextSongID(void)
{
    return gMaxSongID + 1;
}

short Library_FindAudioMatch(long audioLength, unsigned long checksum)
{
    short i;

    for (i = 0; i < gSongCount; i++)
    {
        const SongRec *song = &Songs()[i];

        if ((song->flags & kSongFlagHasChecksum)
            && song->audioLength == audioLength
            && song->checksum == checksum)
            return i;
    }
    return kNone;
}

/* ---- Queries ---------------------------------------------------------- */

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

short Library_ListPlaylists(short *out, short max)
{
    return ListAll(gPlaylistCount, out, max);
}

short Library_ListArtists(const LibFilter *filter, short *out, short max)
{
    short a, s, n = 0;

    for (a = 0; a < gArtists.count && n < max; a++)
        for (s = 0; s < gSongCount; s++)
            if (Songs()[s].artist == a && SongMatches(s, filter))
            {
                out[n++] = a;
                break;
            }
    return n;
}

short Library_ListAlbums(const LibFilter *filter, short *out, short max)
{
    short b, s, n = 0;

    for (b = 0; b < gAlbums.count && n < max; b++)
        for (s = 0; s < gSongCount; s++)
            if (Songs()[s].album == b && SongMatches(s, filter))
            {
                out[n++] = b;
                break;
            }
    return n;
}

short Library_ListSongs(const LibFilter *filter, short *out, short max)
{
    short i, s, n = 0;

    /* Playlist contents are read in a later stage (3c). */
    if (filter->playlist != kNone)
        return 0;

    for (i = 0; i < gSongCount && n < max; i++)
    {
        s = gSongsByTitle[i];
        if (SongMatches(s, filter))
            out[n++] = s;
    }

    if (filter->album != kNone)
        SortSongsByTrack(out, n);

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
ConstStringPtr Library_PlaylistName(short playlist) { return PoolString(gPlaylistNames[playlist]); }
ConstStringPtr Library_SongTitle(short song)        { return PoolString(Songs()[song].title); }
