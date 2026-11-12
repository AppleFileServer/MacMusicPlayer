/* 1.1, the library's cache (libcache.c, and the scan's use of it in
   library.c): written by the first scan, used by the next, never trusted
   over a file. Each scan's result is compared with a full read
   (Library_ScanAll), which reads every file as 1.0 does. */

#include <string.h>

#include "hosttest.h"
#include "bytes.h"

static LibraryFolder gF;
static LibFilter gAll;
static short gList[kMaxSongs + 10];

static void Song(long id, const char *title, const char *artist, short track)
{
    SongSpec s = { 0 };

    s.id = id;
    s.title = title;
    s.artist = artist;
    s.album = "Album";
    s.genre = "Pop";
    s.composer = "Someone";
    s.track = track;
    CHECK_EQ(MakeSong(gF.folderID, &s), noErr);
}

static void Library(short count)
{
    short i;
    char title[32], artist[32];

    gF = NewLibrary();
    for (i = 1; i <= count; i++)
    {
        snprintf(title, sizeof title, "Song %d", (i * 7) % count);
        snprintf(artist, sizeof artist, "Artist %d", i % 5);
        Song(i, title, artist, (short)(i % 12));
    }
}

/* Everything a scan gives, as one string: the songs by title, and each
   song's ID, file, track, flags, length and five details. */
static const char *Result(void)
{
    static char out[60000];
    char line[400];
    short n = Library_ListSongs(&gAll, gList, kMaxSongs), i, s, v;
    long dir, offset;
    ConstStringPtr file;
    SongInfo info;

    out[0] = 0;
    for (i = 0; i < n; i++)
    {
        s = gList[i];
        Library_SongFile(s, &v, &dir, &file, &offset);
        snprintf(line, sizeof line, "%ld %.*s %.*s %.*s %.*s %d %d %ld %ld|",
                 Library_SongID(s), file[0], file + 1,
                 Library_SongTitle(s)[0], Library_SongTitle(s) + 1,
                 Library_SongArtistName(s)[0], Library_SongArtistName(s) + 1,
                 Library_SongAlbumName(s)[0], Library_SongAlbumName(s) + 1,
                 Library_SongTrack(s), Library_SongRating(s),
                 Library_SongSeconds(s), offset);
        strncat(out, line, sizeof out - strlen(out) - 1);
        (void)info;
    }
    return out;
}

/* The cache folder's ID, or 0. */
static long CacheDir(void)
{
    CInfoPBRec pb;

    memset(&pb, 0, sizeof pb);
    pb.dirInfo.ioNamePtr = PS("MacMusic Cache");
    pb.dirInfo.ioVRefNum = gF.vRefNum;
    pb.dirInfo.ioDrDirID = gF.folderID;
    if (PBGetCatInfoSync(&pb) != noErr || !(pb.hFileInfo.ioFlAttrib & 0x10))
        return 0;
    return pb.dirInfo.ioDrDirID;
}

static unsigned long CacheDate(void)
{
    return TestFS_ModDate(CacheDir(), "Song Details");
}

/* The full read's result, for comparing. */
static char gFull[60000];
static void FullRead(void)
{
    CHECK(Library_ScanAll(&gF));
    strcpy(gFull, Result());
}

static void CheckSame(int line)
{
    gChecks++;
    if (strcmp(Result(), gFull) != 0)
    {
        gFailures++;
        printf("  FAIL %s (line %d): the scan differs from a full read\n", gTest, line);
    }
}
#define CHECK_SAME() CheckSame(__LINE__)

/* Changes a song file's details in place, as another program might
   (without telling MacMusic's cache). */
static void ChangeOutside(long id, const char *title)
{
    char name[16];
    SongInfo info;

    snprintf(name, sizeof name, "IPOD%04ld", id);
    CHECK(SongFile_Read(gF.vRefNum, gF.folderID, PS(name), &info));
    info.text[0][0] = (unsigned char)strlen(title);
    memcpy(info.text[0] + 1, title, info.text[0][0]);
    CHECK_EQ(SongFile_WriteInfo(gF.vRefNum, gF.folderID, PS(name), &info), noErr);
}

static int gNotices;
static void Notice(void) { gNotices++; }

static short SongWithID(long id)
{
    short n = Library_SongCount(), i;

    for (i = 0; i < n; i++)
        if (Library_SongID(i) == id)
            return i;
    return kNone;
}

/* 1.2 (N12-6): turns a version 2 cache into 1.1's version 1 (the rate's
   four bytes zero, the check-sum made again), as 1.1 would have written. */
static void MakeVersion1(unsigned char *b, long length)
{
    SongChecksum sum;
    long at = 12, size;
    short i, k;

    SongChecksum_Init(&sum);
    for (i = 0; i < (short)Get16(b + 6); i++)
    {
        size = 36;
        for (k = 0; k < 6; k++)
            size += 1 + b[at + size];
        memset(b + at + 32, 0, 4);
        SongChecksum_Add(&sum, b + at + 4, size - 4);
        at += size;
    }
    CHECK_EQ(at, length);
    b[4] = 0;
    b[5] = 1;
    Put32(b + 8, SongChecksum_Value(&sum));
}

int main(void)
{
    unsigned long date;
    unsigned char bytes[70000];
    long length;

    LibFilter_Clear(&gAll);

    Test("the first scan writes the cache, invisible");
    Library(20);
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_SongCount(), 20);
    CHECK_EQ(Library_ReadFromFilesCount(), 20);
    CHECK(CacheDir() != 0);
    CHECK(TestFS_Exists(CacheDir(), "Song Details"));
    CHECK(TestFS_FinderFlags(gF.folderID, "MacMusic Cache") & fInvisible);
    CHECK_EQ(Library_SkippedCount(), 0);        /* the folder isn't a song */
    CHECK_EQ(TestFS_OpenFiles(), 0);

    Test("the next scan reads no song file, and gives the same");
    Library(20);
    FullRead();
    date = CacheDate();
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_ReadFromFilesCount(), 0);
    CHECK_SAME();
    CHECK_EQ(CacheDate(), date);                /* nothing changed: not rewritten */
    CHECK_EQ(TestFS_OpenFiles(), 0);

    Test("a full read (Rescan Library) reads every file");
    Library(20);
    CHECK(Library_Scan(&gF));
    CHECK(Library_ScanAll(&gF));
    CHECK_EQ(Library_ReadFromFilesCount(), 20);

    Test("a song changed outside MacMusic is read again");
    Library(20);
    CHECK(Library_Scan(&gF));
    ChangeOutside(5, "Changed Elsewhere");
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_ReadFromFilesCount(), 1);
    CHECK_STR(Library_SongTitle(SongWithID(5)), "Changed Elsewhere");
    strcpy(gFull, Result());
    CHECK(Library_Scan(&gF));                   /* and the cache has it now */
    CHECK_EQ(Library_ReadFromFilesCount(), 0);
    CHECK_SAME();

    Test("a new song is read, a removed one dropped");
    Library(20);
    CHECK(Library_Scan(&gF));
    Song(21, "A New One", "Artist 9", 1);
    TestFS_Delete(gF.folderID, "IPOD0003");
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_SongCount(), 20);
    CHECK_EQ(Library_ReadFromFilesCount(), 1);
    CHECK_EQ(SongWithID(3), kNone);
    CHECK(SongWithID(21) != kNone);
    date = CacheDate();
    FullRead();
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_ReadFromFilesCount(), 0);
    CHECK_SAME();

    Test("a flat battery: ratings in two sessions at the same time");
    TestFS_SetClock(100, true);
    Library(10);                 /* 1 Jan 1904, stopped */
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_SetSongRating(SongWithID(4), 3), noErr);
    CHECK(Library_Scan(&gF));                   /* the next session */
    CHECK_EQ(Library_ReadFromFilesCount(), 1);  /* the forgotten entry */
    CHECK_EQ(Library_SongRating(SongWithID(4)), 3);
    CHECK_EQ(Library_SetSongRating(SongWithID(4), 5), noErr);  /* same date again */
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_SongRating(SongWithID(4)), 5);
    FullRead();
    CHECK(Library_Scan(&gF));
    CHECK_SAME();

    Test("a flat battery: a change from outside on the same date and length");
    TestFS_SetClock(100, true);
    Library(10);
    CHECK(Library_Scan(&gF));
    ChangeOutside(2, "Song X");                 /* same length as "Song 4" */
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_ReadFromFilesCount(), 0);  /* can't be seen: the known limit */
    CHECK(Library_ScanAll(&gF));                /* Rescan Library puts it right */
    CHECK_STR(Library_SongTitle(SongWithID(2)), "Song X");
    CHECK(Library_Scan(&gF));
    CHECK_STR(Library_SongTitle(SongWithID(2)), "Song X");

    Test("Edit Song's way: forget, then write");
    TestFS_SetClock(100, true);
    Library(10);
    CHECK(Library_Scan(&gF));
    Library_ForgetCached(SongWithID(6));
    ChangeOutside(6, "Edited");
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_ReadFromFilesCount(), 1);
    CHECK_STR(Library_SongTitle(SongWithID(6)), "Edited");

    Test("a damaged cache is ignored and rewritten");
    Library(20);
    CHECK(Library_Scan(&gF));
    length = TestFS_ReadFile(CacheDir(), "Song Details", bytes, sizeof bytes);
    CHECK(length > 100);
    bytes[length - 3] ^= 0x20;                  /* a letter of the last detail */
    TestFS_WriteFile(CacheDir(), "Song Details", 'iPdC', kAppCreator, bytes, length);
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_ReadFromFilesCount(), 20);
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_ReadFromFilesCount(), 0);

    Test("a cache cut short, or of another version, is ignored");
    Library(20);
    CHECK(Library_Scan(&gF));
    length = TestFS_ReadFile(CacheDir(), "Song Details", bytes, sizeof bytes);
    TestFS_WriteFile(CacheDir(), "Song Details", 'iPdC', kAppCreator, bytes, length - 10);
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_ReadFromFilesCount(), 20);
    length = TestFS_ReadFile(CacheDir(), "Song Details", bytes, sizeof bytes);
    bytes[5] = 9;                               /* version 9 */
    TestFS_WriteFile(CacheDir(), "Song Details", 'iPdC', kAppCreator, bytes, length);
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_ReadFromFilesCount(), 20);
    TestFS_WriteFile(CacheDir(), "Song Details", 'iPdC', kAppCreator, "", 0);
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_ReadFromFilesCount(), 20);

    Test("1.2 (N12-6): each song's rate kept in the cache");
    {
        SongSpec p = { 0 };

        gF = NewLibrary();
        p.id = 1; p.title = "Plus Rate"; p.rate = 22254;
        CHECK_EQ(MakeSong(gF.folderID, &p), noErr);
        p.id = 2; p.title = "Old Rate"; p.rate = 0;
        CHECK_EQ(MakeSong(gF.folderID, &p), noErr);
        CHECK(Library_Scan(&gF));               /* writes version 2 */
        CHECK(Library_Scan(&gF));               /* reads it */
        CHECK_EQ(Library_ReadFromFilesCount(), 0);
        CHECK_EQ(Library_SongSampleRate(Library_SongForID(1)), 22254);
        CHECK_EQ(Library_SongSampleRate(Library_SongForID(2)), 22050);
        length = TestFS_ReadFile(CacheDir(), "Song Details", bytes, sizeof bytes);
        CHECK_EQ(Get16(bytes + 4), 2);

        Test("1.1's cache (version 1) is still used, its songs 22050 Hz");
        gF = NewLibrary();
        p.id = 1; p.title = "One"; p.rate = 0;
        CHECK_EQ(MakeSong(gF.folderID, &p), noErr);
        p.id = 2; p.title = "Two";
        CHECK_EQ(MakeSong(gF.folderID, &p), noErr);
        CHECK(Library_Scan(&gF));
        length = TestFS_ReadFile(CacheDir(), "Song Details", bytes, sizeof bytes);
        MakeVersion1(bytes, length);
        TestFS_WriteFile(CacheDir(), "Song Details", 'iPdC', kAppCreator, bytes, length);
        CHECK(Library_Scan(&gF));
        CHECK_EQ(Library_ReadFromFilesCount(), 0);  /* no extra scan */
        CHECK_EQ(Library_SongSampleRate(Library_SongForID(1)), 22050);
        CHECK_EQ(Library_SongSampleRate(Library_SongForID(2)), 22050);
        length = TestFS_ReadFile(CacheDir(), "Song Details", bytes, sizeof bytes);
        CHECK_EQ(Get16(bytes + 4), 1);          /* nothing changed: kept */
        CHECK(Library_ScanAll(&gF));            /* Rescan Library writes */
        length = TestFS_ReadFile(CacheDir(), "Song Details", bytes, sizeof bytes);
        CHECK_EQ(Get16(bytes + 4), 2);          /* version 2 */

        Test("a version 2 entry with a bad rate: its file is read");
        gF = NewLibrary();
        p.id = 1; p.title = "One"; p.rate = 22254;
        CHECK_EQ(MakeSong(gF.folderID, &p), noErr);
        p.id = 2; p.title = "Two"; p.rate = 0;
        CHECK_EQ(MakeSong(gF.folderID, &p), noErr);
        CHECK(Library_Scan(&gF));               /* writes version 2 */
        length = TestFS_ReadFile(CacheDir(), "Song Details", bytes, sizeof bytes);
        {
            SongChecksum sum;
            long at = 12, size;
            short i, k;

            bytes[12 + 32 + 3] ^= 1;            /* the first entry's rate */
            SongChecksum_Init(&sum);
            for (i = 0; i < (short)Get16(bytes + 6); i++)
            {
                size = 36;
                for (k = 0; k < 6; k++)
                    size += 1 + bytes[at + size];
                SongChecksum_Add(&sum, bytes + at + 4, size - 4);
                at += size;
            }
            Put32(bytes + 8, SongChecksum_Value(&sum));
        }
        TestFS_WriteFile(CacheDir(), "Song Details", 'iPdC', kAppCreator, bytes, length);
        CHECK(Library_Scan(&gF));
        CHECK_EQ(Library_ReadFromFilesCount(), 1);
        CHECK_EQ(Library_SongCount(), 2);
        CHECK_EQ(Library_SongSampleRate(Library_SongForID(1)), 22254);
    }

    Test("a file called MacMusic Cache isn't used or replaced");
    Library(5);
    TestFS_WriteFile(gF.folderID, "MacMusic Cache", 'TEXT', 'ttxt', "x", 1);
    CHECK(Library_Scan(&gF));
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_ReadFromFilesCount(), 5);
    CHECK_EQ(Library_SkippedCount(), 1);        /* as in 1.0: not a song */
    CHECK_EQ(TestFS_Type(gF.folderID, "MacMusic Cache"), 'TEXT');

    Test("a locked disk: no cache, and every scan reads the files");
    Library(10);
    TestFS_LockVolume(true);
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_SongCount(), 10);
    CHECK_EQ(CacheDir(), 0);
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_ReadFromFilesCount(), 10);
    TestFS_LockVolume(false);

    Test("files that aren't songs, and a duplicate ID, as in 1.0");
    Library(10);
    TestFS_WriteFile(gF.folderID, "Notes", 'TEXT', 'ttxt', "hello", 5);
    {
        SongSpec dup = { 0 };
        dup.id = 3;
        dup.title = "Duplicate";
        dup.fileName = "Z Duplicate";
        CHECK_EQ(MakeSong(gF.folderID, &dup), noErr);
    }
    FullRead();
    CHECK_EQ(Library_SkippedCount(), 2);
    CHECK(Library_Scan(&gF));
    date = CacheDate();
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_SkippedCount(), 2);
    CHECK_EQ(Library_SongCount(), 10);
    CHECK_SAME();
    CHECK_EQ(CacheDate(), date);                /* not rewritten each time */

    Test("500 songs and more: the limit as in 1.0");
    Library(kMaxSongs + 3);
    FullRead();
    CHECK_EQ(Library_OverLimitCount(), 3);
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_ReadFromFilesCount(), 0);
    CHECK_EQ(Library_SongCount(), kMaxSongs);
    CHECK_EQ(Library_OverLimitCount(), 3);
    CHECK_SAME();

    Test("short of memory: whatever a full read fits, a cached scan fits");
    {
        long low = 1000, high = 2000000, limit, mid;
        Library(200);
        CHECK(Library_Scan(&gF));               /* a cache, for the next */
        while (high - low > 64)                 /* the least a full read needs */
        {
            mid = (low + high) / 2;
            TestMem_SetLimit(mid);
            if (Library_ScanAll(&gF))
                high = mid;
            else
                low = mid;
            TestMem_SetLimit(0);
            CHECK(Library_Scan(&gF));           /* the cache, back as it was */
        }
        limit = high;
        TestMem_SetLimit(limit);
        CHECK(Library_Scan(&gF));
        CHECK_EQ(Library_SongCount(), 200);
        TestMem_SetLimit(0);
        FullRead();
        TestMem_SetLimit(limit);
        CHECK(Library_Scan(&gF));
        CHECK_SAME();
        TestMem_SetLimit(0);
    }
    CHECK_EQ(TestFS_OpenFiles(), 0);

    Test("the loading notice: once at the start of every scan (N11-29)");
    Library(20);
    Library_SetReadingNotice(Notice);
    gNotices = 0;
    CHECK(Library_Scan(&gF));                   /* the first start: no cache */
    CHECK_EQ(gNotices, 1);
    gNotices = 0;
    CHECK(Library_Scan(&gF));                   /* all from the cache */
    CHECK_EQ(gNotices, 1);
    gNotices = 0;
    ChangeOutside(7, "Changed");
    CHECK(Library_Scan(&gF));                   /* one file read */
    CHECK_EQ(gNotices, 1);
    gNotices = 0;
    TestFS_WriteFile(gF.folderID, "Notes", 'TEXT', 'ttxt', "hello", 5);
    CHECK(Library_Scan(&gF));                   /* a file that isn't a song */
    CHECK_EQ(gNotices, 1);
    gNotices = 0;
    CHECK(Library_ScanAll(&gF));                /* Rescan Library */
    CHECK_EQ(gNotices, 1);
    gNotices = 0;
    TestMem_SetLimit(1000);                     /* a scan that fails and retries */
    CHECK(!Library_Scan(&gF));
    TestMem_SetLimit(0);
    CHECK_EQ(gNotices, 1);
    Library_SetReadingNotice(nil);
    gNotices = 0;
    CHECK(Library_ScanAll(&gF));
    CHECK_EQ(gNotices, 0);

    return TestsDone("test_cache");
}
