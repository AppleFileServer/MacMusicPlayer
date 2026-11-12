/* The library (library.c): scanning a library folder, IDs, the 500-song
   limit, the lists and their order, the groups, and the per-song calls.
   Playlists are in test_playlists.c. */

#include "hosttest.h"

static LibraryFolder gF;
static LibFilter gAll;
static short gList[kMaxSongs + 10];

static void Song(long id, const char *title, const char *artist,
                 const char *album, const char *genre, short track)
{
    SongSpec s = { 0 };

    s.id = id;
    s.title = title;
    s.artist = artist;
    s.album = album;
    s.genre = genre;
    s.track = track;
    CHECK_EQ(MakeSong(gF.folderID, &s), noErr);
}

/* The titles of a list of songs, joined with '/'. */
static const char *Titles(const short *songs, short n)
{
    static char out[2000];
    short i;
    ConstStringPtr t;

    out[0] = 0;
    for (i = 0; i < n; i++)
    {
        t = Library_SongTitle(songs[i]);
        if (i)
            strcat(out, "/");
        strncat(out, (const char *)t + 1, t[0]);
    }
    return out;
}

static void CheckList(const char *got, const char *expected, int line)
{
    gChecks++;
    if (strcmp(got, expected) != 0)
    {
        gFailures++;
        printf("  FAIL %s (line %d): \"%s\", expected \"%s\"\n", gTest, line,
               got, expected);
    }
}
#define CHECK_LIST(got, expected) CheckList((got), (expected), __LINE__)

static const char *Names(ConstStringPtr (*name)(short), const short *items,
                         short n)
{
    static char out[2000];
    short i;
    ConstStringPtr t;

    out[0] = 0;
    for (i = 0; i < n; i++)
    {
        t = name(items[i]);
        if (i)
            strcat(out, "/");
        strncat(out, (const char *)t + 1, t[0]);
    }
    return out;
}

/* A small library of seven songs and an audiobook. */
static void SmallLibrary(void)
{
    SongSpec book = { 0 };

    gF = NewLibrary();
    Song(3, "Yellow Submarine", "The Beatles", "Revolver", "Rock", 6);
    Song(1, "Taxman", "The Beatles", "Revolver", "Rock", 1);
    Song(2, "Eleanor Rigby", "The Beatles", "Revolver", "Rock", 2);
    Song(4, "Something", "the beatles", "Abbey Road", "Rock", 2);
    Song(5, "the Long Run", "Eagles", "The Long Run", "Rock", 1);
    Song(6, "Abba Song", "ABBA", "", "Pop", 0);
    Song(9, "No Artist", "", "", "", 0);
    book.id = 7;
    book.title = "A Book";
    book.artist = "Narrator";
    book.flags = kSongFlagAudiobook;
    CHECK_EQ(MakeSong(gF.folderID, &book), noErr);
    TestFS_WriteFile(gF.folderID, "MacMusic Library Info", kLibraryMarkerType,
                     kAppCreator, "x", 1);
}

int main(void)
{
    short n, i, s;
    LibFilter f;

    LibFilter_Clear(&gAll);

    Test("an empty library");
    gF = NewLibrary();
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_SongCount(), 0);
    CHECK_EQ(Library_SkippedCount(), 0);
    CHECK_EQ(Library_NextSongID(), 1);
    CHECK_EQ(Library_ListSongs(&gAll, gList, kMaxSongs), 0);
    CHECK_EQ(Library_ListArtists(&gAll, gList, kMaxSongs), 0);

    Test("scanning: counts, the marker ignored, the next ID");
    SmallLibrary();
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_SongCount(), 8);
    CHECK_EQ(Library_SkippedCount(), 0);
    CHECK_EQ(Library_OverLimitCount(), 0);
    CHECK_EQ(Library_NextSongID(), 10);
    CHECK_EQ(Library_TotalAudioBytes(), 8 * 2206);
    CHECK_EQ(Library_SongID(Library_SongForID(4)), 4);
    CHECK_EQ(Library_SongForID(8), kNone);

    Test("songs: by title, case and a leading \"The \" ignored; no audiobooks");
    SmallLibrary();
    Library_Scan(&gF);
    n = Library_ListSongs(&gAll, gList, kMaxSongs);
    CHECK_LIST(Titles(gList, n), "Abba Song/Eleanor Rigby/the Long Run/"
                                 "No Artist/Something/Taxman/Yellow Submarine");
    n = Library_ListAudiobooks(gList, kMaxSongs);
    CHECK_LIST(Titles(gList, n), "A Book");
    CHECK(Library_SongIsAudiobook(gList[0]));

    Test("only \"The \" with its space is ignored in sorting");
    gF = NewLibrary();
    Song(1, "Theatre", "", "", "", 0);
    Song(2, "Bravo", "", "", "", 0);
    Song(3, "The Zoo", "", "", "", 0);
    Song(4, "Them", "", "", "", 0);
    Song(5, "Then", "", "", "", 0);
    Library_Scan(&gF);
    n = Library_ListSongs(&gAll, gList, kMaxSongs);
    CHECK_LIST(Titles(gList, n), "Bravo/Theatre/Them/Then/The Zoo");

    Test("artists, albums and genres: grouped and sorted");
    SmallLibrary();
    Library_Scan(&gF);
    n = Library_ListArtists(&gAll, gList, kMaxSongs);
    /* Names group exactly ("the beatles" apart); the audiobook's narrator
       and the song without an artist are in no group. */
    CHECK_LIST(Names(Library_ArtistName, gList, n),
               "ABBA/The Beatles/the beatles/Eagles");
    n = Library_ListAlbums(&gAll, gList, kMaxSongs);
    CHECK_LIST(Names(Library_AlbumName, gList, n),
               "Abbey Road/The Long Run/Revolver");
    n = Library_ListGenres(gList, kMaxSongs);
    CHECK_LIST(Names(Library_GenreName, gList, n), "Pop/Rock");
    CHECK_EQ(Library_ListComposers(gList, kMaxSongs), 0);
    CHECK(Library_FindArtist(PS("Eagles")) != kNone);
    CHECK_EQ(Library_FindArtist(PS("Nobody")), kNone);
    CHECK_EQ(Library_SongAlbum(Library_SongForID(6)), kNone);

    Test("filters: an artist's albums, an album's songs in track order");
    SmallLibrary();
    Library_Scan(&gF);
    LibFilter_Clear(&f);
    f.artist = Library_FindArtist(PS("The Beatles"));
    n = Library_ListAlbums(&f, gList, kMaxSongs);
    CHECK_LIST(Names(Library_AlbumName, gList, n), "Revolver");
    f.album = gList[0];
    n = Library_ListSongs(&f, gList, kMaxSongs);
    CHECK_LIST(Titles(gList, n), "Taxman/Eleanor Rigby/Yellow Submarine");
    LibFilter_Clear(&f);
    f.genre = Library_FindGenre(PS("Rock"));
    n = Library_ListArtists(&f, gList, kMaxSongs);
    CHECK_LIST(Names(Library_ArtistName, gList, n),
               "The Beatles/the beatles/Eagles");

    Test("Albums > All (N12-3): by album, then track; no album last");
    SmallLibrary();
    Library_Scan(&gF);
    LibFilter_Clear(&f);
    f.byAlbum = true;
    n = Library_ListSongs(&f, gList, kMaxSongs);
    /* Abbey Road, The Long Run ("The " ignored: L), Revolver in track
       order, then the songs with no album by title; no audiobook. */
    CHECK_LIST(Titles(gList, n), "Something/the Long Run/Taxman/"
               "Eleanor Rigby/Yellow Submarine/Abba Song/No Artist");
    n = Library_ListSongs(&gAll, gList, kMaxSongs);     /* Music > Songs */
    CHECK_LIST(Titles(gList, n), "Abba Song/Eleanor Rigby/the Long Run/"
               "No Artist/Something/Taxman/Yellow Submarine");
    f.artist = Library_FindArtist(PS("The Beatles"));   /* an artist's All */
    n = Library_ListSongs(&f, gList, kMaxSongs);
    CHECK_LIST(Titles(gList, n), "Taxman/Eleanor Rigby/Yellow Submarine");
    LibFilter_Clear(&f);
    f.byAlbum = true;
    f.genre = Library_FindGenre(PS("Rock"));            /* a genre's All */
    n = Library_ListSongs(&f, gList, kMaxSongs);
    CHECK_LIST(Titles(gList, n), "Something/the Long Run/Taxman/"
               "Eleanor Rigby/Yellow Submarine");

    Test("Albums > All: equal tracks keep title order; track 0 first");
    gF = NewLibrary();
    Song(1, "Zeta", "A", "One", "", 2);
    Song(2, "Alpha", "A", "One", "", 2);
    Song(3, "Middle", "A", "One", "", 0);
    Song(4, "First", "A", "One", "", 1);
    Song(5, "Other", "A", "Two", "", 1);
    Library_Scan(&gF);
    LibFilter_Clear(&f);
    f.byAlbum = true;
    n = Library_ListSongs(&f, gList, kMaxSongs);
    CHECK_LIST(Titles(gList, n), "Middle/First/Alpha/Zeta/Other");
    f.album = Library_SongAlbum(gList[0]);              /* the album's own */
    n = Library_ListSongs(&f, gList, kMaxSongs);        /* screen: the same */
    CHECK_LIST(Titles(gList, n), "Middle/First/Alpha/Zeta");

    Test("300 albums, one song each: all listed (N12-9; 1.1 stopped at 256)");
    gF = NewLibrary();
    for (i = 1; i <= 300; i++)
    {
        char title[16], album[16], artist[16];

        snprintf(title, sizeof title, "Song %d", i);
        snprintf(album, sizeof album, "Album %d", i);
        snprintf(artist, sizeof artist, "Artist %d", i);
        Song(i, title, artist, album, "Pop", 1);
    }
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_SongCount(), 300);
    CHECK_EQ(Library_ListAlbums(&gAll, gList, kMaxSongs), 300);
    CHECK_EQ(Library_ListArtists(&gAll, gList, kMaxSongs), 300);
    for (i = 1; i <= 300; i++)
        CHECK(Library_SongAlbum(Library_SongForID(i)) != kNone);

    Test("unreadable files and duplicate IDs are skipped");
    SmallLibrary();
    TestFS_WriteFile(gF.folderID, "Read Me", 'TEXT', 'ttxt', "hello", 5);
    {
        SongSpec dup = { 0 };

        dup.id = 3;                             /* Yellow Submarine's ID */
        dup.title = "Duplicate";
        dup.fileName = "IPOD9999";              /* listed after IPOD0003 */
        MakeSong(gF.folderID, &dup);
    }
    TestFS_MakeDir(gF.folderID, "Some Folder");
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_SongCount(), 8);
    CHECK_EQ(Library_SkippedCount(), 2);
    CHECK_STR(Library_SongTitle(Library_SongForID(3)), "Yellow Submarine");

    Test("the 500-song limit");
    gF = NewLibrary();
    for (i = 1; i <= kMaxSongs + 2; i++)
    {
        SongSpec t = { 0 };
        char title[16];

        snprintf(title, sizeof title, "Song %d", i);
        t.id = i;
        t.title = title;
        t.samples = 100;
        MakeSong(gF.folderID, &t);
    }
    TestFS_WriteFile(gF.folderID, "Zzz Read Me", 'TEXT', 'ttxt', "x", 1);
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_SongCount(), kMaxSongs);
    CHECK_EQ(Library_OverLimitCount(), 2);      /* IPOD0501, IPOD0502 */
    CHECK_EQ(Library_SkippedCount(), 1);        /* the text file */
    CHECK_EQ(Library_SongForID(501), kNone);
    CHECK_EQ(Library_ListSongs(&gAll, gList, kMaxSongs + 10), kMaxSongs);

    Test("out of memory: the scan fails and the library is empty");
    SmallLibrary();
    TestMem_SetLimit(400);
    CHECK(!Library_Scan(&gF));
    TestMem_SetLimit(0);
    CHECK_EQ(Library_SongCount(), 0);
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_SongCount(), 8);

    Test("details: names, length, file, audio match");
    SmallLibrary();
    Library_Scan(&gF);
    s = Library_SongForID(2);
    CHECK_STR(Library_SongArtistName(s), "The Beatles");
    CHECK_STR(Library_SongAlbumName(s), "Revolver");
    CHECK_STR(Library_SongArtistName(Library_SongForID(9)), "");
    CHECK_EQ(Library_SongTrack(s), 2);
    CHECK_EQ(Library_SongSeconds(s), 0);
    CHECK_EQ(Library_SongAudioLength(s), 2206);
    {
        short v;
        long d, offset;
        ConstStringPtr name;
        SongInfo info;

        CHECK(Library_SongFile(s, &v, &d, &name, &offset));
        CHECK_EQ(d, gF.folderID);
        CHECK_STR(name, "IPOD0002");
        CHECK_EQ(offset, 44);
        CHECK(SongFile_Read(v, d, name, &info));
        CHECK_EQ(Library_FindAudioMatch(2206, info.fingerprint), s);
        CHECK_EQ(Library_FindAudioMatch(2205, info.fingerprint), kNone);
    }

    Test("ratings: saved in the file and the library");
    SmallLibrary();
    Library_Scan(&gF);
    s = Library_SongForID(5);
    CHECK_EQ(Library_SongRating(s), 0);
    CHECK_EQ(Library_SetSongRating(s, 3), noErr);
    CHECK_EQ(Library_SongRating(s), 3);
    Library_Scan(&gF);
    CHECK_EQ(Library_SongRating(Library_SongForID(5)), 3);
    TestFS_Lock(gF.folderID, "IPOD0005", true);
    CHECK(Library_SetSongRating(Library_SongForID(5), 5) != noErr);
    CHECK_EQ(Library_SongRating(Library_SongForID(5)), 3);

    Test("editing a song's details regroups it without a rescan");
    SmallLibrary();
    Library_Scan(&gF);
    s = Library_SongForID(5);
    {
        SongInfo info;
        short v;
        long d, offset;
        ConstStringPtr name;

        Library_SongFile(s, &v, &d, &name, &offset);
        SongFile_Read(v, d, name, &info);
        memcpy(info.text[0], PS("Hotel California"), 17);
        memcpy(info.text[1], PS("ABBA"), 5);
        memcpy(info.text[2], PS("Hotel California"), 17);
        CHECK_EQ(SongFile_WriteInfo(v, d, name, &info), noErr);
        CHECK(Library_UpdateSong(s, &info));
    }
    CHECK_EQ(Library_SongForID(5), s);
    CHECK_STR(Library_SongTitle(s), "Hotel California");
    n = Library_ListArtists(&gAll, gList, kMaxSongs);
    CHECK_LIST(Names(Library_ArtistName, gList, n),
               "ABBA/The Beatles/the beatles");     /* Eagles has gone */
    LibFilter_Clear(&f);
    f.artist = Library_FindArtist(PS("ABBA"));
    n = Library_ListSongs(&f, gList, kMaxSongs);
    CHECK_LIST(Titles(gList, n), "Abba Song/Hotel California");
    Library_Scan(&gF);                          /* the file agrees */
    CHECK_STR(Library_SongTitle(Library_SongForID(5)), "Hotel California");

    Test("cleared: nothing listed");
    SmallLibrary();
    Library_Scan(&gF);
    Library_Clear();
    CHECK_EQ(Library_SongCount(), 0);
    CHECK_EQ(Library_ListSongs(&gAll, gList, kMaxSongs), 0);

    return TestsDone("library");
}
