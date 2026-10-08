/* Shuffle (shuffle.c): Shuffle Songs and Shuffle Albums orders over a
   list of library songs (Stage 9c, T3, S-2). */

#include "hosttest.h"
#include "shuffle.h"
#include "settings.h"

static LibraryFolder gF;
static short gList[kMaxSongs], gCopy[kMaxSongs];

static void Song(long id, const char *album, short track, short flags)
{
    SongSpec s = { 0 };
    char title[16];

    snprintf(title, sizeof title, "S%02ld", id);
    s.id = id;
    s.title = title;
    s.album = album;
    s.track = track;
    s.flags = flags;
    MakeSong(gF.folderID, &s);
}

/* Albums A (4 tracks), B (3), C (2), and 2 songs without an album:
   IDs 1-11, listed by title S01..S11. */
static short Library11(void)
{
    LibFilter all;

    gF = NewLibrary();
    Song(1, "A", 3, 0); Song(2, "A", 1, 0); Song(3, "A", 4, 0); Song(4, "A", 2, 0);
    Song(5, "B", 2, 0); Song(6, "B", 1, 0); Song(7, "B", 3, 0);
    Song(8, "C", 1, 0); Song(9, "C", 2, 0);
    Song(10, nil, 0, 0); Song(11, nil, 0, 0);
    Library_Scan(&gF);
    LibFilter_Clear(&all);
    return Library_ListSongs(&all, gList, kMaxSongs);
}

/* Every song of the original list exactly once. */
static Boolean IsPermutation(const short *a, const short *b, short n)
{
    short i, j, count;

    for (i = 0; i < n; i++)
    {
        count = 0;
        for (j = 0; j < n; j++)
            if (a[j] == b[i])
                count++;
        if (count != 1)
            return false;
    }
    return true;
}

static Boolean SameOrder(const short *a, const short *b, short n)
{
    return memcmp(a, b, n * sizeof(short)) == 0;
}

/* Each album's songs together, in track order. */
static Boolean AlbumsTogether(const short *songs, short n)
{
    short i, j;

    for (i = 1; i < n; i++)
    {
        if (Library_SongAlbum(songs[i]) == Library_SongAlbum(songs[i - 1]))
        {
            if (Library_SongTrack(songs[i]) < Library_SongTrack(songs[i - 1]))
                return false;
            continue;
        }
        for (j = 0; j < i - 1; j++)             /* that album ended before */
            if (Library_SongAlbum(songs[j]) == Library_SongAlbum(songs[i]))
                return false;
    }
    return true;
}

int main(void)
{
    short n, start, i, differs, seed, s5;
    short firsts;

    Test("Shuffle Off: the order kept");
    n = Library11();
    memcpy(gCopy, gList, sizeof gList);
    CHECK_EQ(Shuffle_Order(gList, n, 4, true, kShuffleOff), 4);
    CHECK(SameOrder(gList, gCopy, n));

    Test("Shuffle Songs from a song row: that song first, the rest mixed");
    n = Library11();
    memcpy(gCopy, gList, sizeof gList);
    s5 = gList[4];
    CHECK_EQ(Shuffle_Order(gList, n, 4, true, kShuffleSongs), 0);
    CHECK_EQ(gList[0], s5);
    CHECK(IsPermutation(gList, gCopy, n));
    CHECK(!SameOrder(gList, gCopy, n));

    Test("Shuffle Songs from the menu: all mixed, start at the top");
    n = Library11();
    memcpy(gCopy, gList, sizeof gList);
    CHECK_EQ(Shuffle_Order(gList, n, 0, false, kShuffleSongs), 0);
    CHECK(IsPermutation(gList, gCopy, n));
    /* Over many seeds every song comes first sometimes. */
    {
        Boolean seen[12] = { 0 };

        for (seed = 1; seed <= 200; seed++)
        {
            TestRandom_Seed((unsigned long)seed * 7919);
            memcpy(gList, gCopy, sizeof gList);
            Shuffle_Order(gList, n, 0, false, kShuffleSongs);
            seen[Library_SongID(gList[0])] = true;
        }
        for (i = 1, firsts = 0; i <= 11; i++)
            firsts += seen[i];
        CHECK_EQ(firsts, 11);
    }

    Test("the same seed, the same order; another seed, another");
    n = Library11();
    memcpy(gCopy, gList, sizeof gList);
    TestRandom_Seed(5);
    Shuffle_Order(gList, n, 0, false, kShuffleSongs);
    memcpy(gCopy + 100, gList, n * sizeof(short));
    memcpy(gList, gCopy, sizeof(short) * n);
    TestRandom_Seed(5);
    Shuffle_Order(gList, n, 0, false, kShuffleSongs);
    CHECK(SameOrder(gList, gCopy + 100, n));
    differs = 0;
    for (seed = 6; seed < 16; seed++)
    {
        memcpy(gList, gCopy, sizeof(short) * n);
        TestRandom_Seed(seed);
        Shuffle_Order(gList, n, 0, false, kShuffleSongs);
        if (!SameOrder(gList, gCopy + 100, n))
            differs++;
    }
    CHECK(differs >= 9);

    Test("Shuffle Albums from a song row: its album first, in track order");
    for (seed = 1; seed <= 20; seed++)
    {
        n = Library11();
        memcpy(gCopy, gList, sizeof gList);
        TestRandom_Seed(seed);
        start = Shuffle_Order(gList, n, 6, true, kShuffleAlbums); /* S07: B 3 */
        CHECK(IsPermutation(gList, gCopy, n));
        CHECK(AlbumsTogether(gList, n));
        CHECK_STR(Library_SongAlbumName(gList[0]), "B");
        CHECK_EQ(Library_SongTrack(gList[0]), 1);
        CHECK_EQ(Library_SongID(gList[start]), 7);              /* plays first */
    }

    Test("Shuffle Albums from the menu: every album whole");
    n = Library11();
    memcpy(gCopy, gList, sizeof gList);
    start = Shuffle_Order(gList, n, 0, false, kShuffleAlbums);
    CHECK_EQ(start, 0);
    CHECK(IsPermutation(gList, gCopy, n));
    CHECK(AlbumsTogether(gList, n));

    Test("an audiobook to start with: no shuffle (S-2)");
    gF = NewLibrary();
    Song(1, "A", 1, kSongFlagAudiobook);
    Song(2, "A", 2, kSongFlagAudiobook);
    Song(3, "A", 3, kSongFlagAudiobook);
    Library_Scan(&gF);
    n = Library_ListAudiobooks(gList, kMaxSongs);
    memcpy(gCopy, gList, sizeof gList);
    CHECK_EQ(Shuffle_Order(gList, n, 1, true, kShuffleSongs), 1);
    CHECK(SameOrder(gList, gCopy, n));

    Test("one song, or none");
    n = Library11();
    CHECK_EQ(Shuffle_Order(gList, 1, 0, true, kShuffleSongs), 0);
    CHECK_EQ(Shuffle_Order(gList, 0, 0, false, kShuffleAlbums), 0);

    Test("Shuffle_Range: only the part after first moves");
    n = Library11();
    memcpy(gCopy, gList, sizeof gList);
    Shuffle_Range(gList, 3, n);
    CHECK(SameOrder(gList, gCopy, 3));
    CHECK(IsPermutation(gList, gCopy, n));

    return TestsDone("shuffle");
}
