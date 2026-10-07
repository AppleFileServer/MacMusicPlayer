#include <Files.h>
#include <Memory.h>
#include <TextUtils.h>

#include "library.h"
#include "fileopen.h"
#include "songfile.h"

#define kDirectoryAttribute     0x10    /* ioFlAttrib bit 4 */

/* One scanned song. Strings are byte offsets into the string pool. */
typedef struct
{
    long id;
    long seconds;
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

/* One song file as read; static to keep it off the small stack. */
static SongInfo gInfo;

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
    gHaveFolder = false;
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

/* Reads and checks one file. Returns false if it is not a valid song
   (the caller counts it as skipped); sets *outOfMemory on a memory error. */
static Boolean AddSongFile(const LibraryFolder *folder, ConstStringPtr fileName,
                           Boolean *outOfMemory)
{
    SongRec song;
    long offsets[kSongTagCount + 1];
    short i;

    if (!SongFile_Read(folder->vRefNum, folder->folderID, fileName, &gInfo))
        return false;

    for (i = 0; i < gSongCount; i++)
        if (Songs()[i].id == gInfo.id)
            return false;               /* duplicate ID: keep the first */

    song.id = gInfo.id;
    song.seconds = gInfo.seconds;
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
        if (pb.hFileInfo.ioFlFndrInfo.fdType == kLibraryMarkerType)
            continue;                   /* "iPod Library Info" (5b-3) */

        if (gSongCount >= kMaxSongs
            || !AddSongFile(folder, name, &outOfMemory))
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

        if (name[0] > 31)
            continue;                   /* HFS names never are */
        BlockMove(name, gPlaylistNames[gPlaylistCount++], name[0] + 1);
    }
    return true;
}

/* Groups songs into artists, albums, genres and composers. Runs once the
   pool is locked, since it compares strings in place. */
static void IndexSongs(void)
{
    short i, j, s;
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

    /* Songs sorted by ID, so playlists can find them by binary search. */
    for (i = 0; i < gSongCount; i++)
    {
        s = i;
        for (j = i; j > 0 && Songs()[gSongsByID[j - 1]].id > Songs()[s].id; j--)
            gSongsByID[j] = gSongsByID[j - 1];
        gSongsByID[j] = s;
    }
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
    gFolder = *folder;
    gHaveFolder = true;
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

/* Rewrites one playlist file without the lines for id. */
static OSErr RemoveFromPlaylist(short playlist, long id)
{
    Ptr text;
    long length, count, at, end, kept = 0;
    short refNum;
    Boolean changed = false;
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

    /* Compact in place: each line (with its CR, LF or CRLF ending) is
       kept unless it starts with this ID. */
    for (at = 0; err == noErr && at < length; at = end)
    {
        end = at;
        while (end < length && text[end] != '\r' && text[end] != '\n')
            end++;
        if (end < length && text[end] == '\r')
            end++;
        if (end < length && text[end] == '\n')
            end++;

        if (LineID((unsigned char *)text, at, length) == id)
            changed = true;
        else
        {
            BlockMove(text + at, text + kept, end - at);
            kept += end - at;
        }
    }

    if (err == noErr && changed)
    {
        count = kept;
        err = SetFPos(refNum, fsFromStart, 0);
        if (err == noErr)
            err = FSWrite(refNum, &count, text);
        if (err == noErr)
            err = SetEOF(refNum, kept);
    }
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
        err = RemoveFromPlaylist(i, id);
        if (first == noErr)
            first = err;
    }
    FlushVol(nil, gFolder.vRefNum);
    return first;
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
    err = SongFile_WriteFlags(gFolder.vRefNum, gFolder.folderID,
                              PoolString(Songs()[song].fileName),
                              Songs()[song].id, flags);
    if (err == noErr)
        Songs()[song].flags = flags;
    return err;
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
