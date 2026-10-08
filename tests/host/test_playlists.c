/* Playlists (library.c): files in the library's Playlists folder, one
   "<id><tab><title><CR>" line per entry. Creating, adding, the 10-playlist
   limit, removing one entry or a song from all, renaming, deleting and
   retitling, each leaving other lines exactly as they were. */

#include "hosttest.h"

static LibraryFolder gF;
static LibFilter gAll;
static short gList[kMaxSongs];

static void Library4(void)
{
    static const char *titles[] = { "Alpha", "Bravo", "Charlie", "Delta" };
    SongSpec s = { 0 };
    long id;

    gF = NewLibrary();
    for (id = 1; id <= 4; id++)
    {
        s.id = id * 10;                         /* IDs 10, 20, 30, 40 */
        s.title = titles[id - 1];
        MakeSong(gF.folderID, &s);
    }
}

static void CheckText(const char *name, const char *expected, int line)
{
    const char *got = PlaylistText(&gF, name);

    gChecks++;
    if (strcmp(got, expected) != 0)
    {
        gFailures++;
        printf("  FAIL %s (line %d): %s is \"%s\", expected \"%s\"\n", gTest,
               line, name, got, expected);
    }
}
#define CHECK_TEXT(name, expected) CheckText((name), (expected), __LINE__)

static short Playlist(const char *name)
{
    return Library_FindPlaylist(PS(name));
}

static long IDs(short playlist)
{
    LibFilter f;
    short n, i;
    long packed = 0;

    LibFilter_Clear(&f);
    f.playlist = playlist;
    n = Library_ListSongs(&f, gList, kMaxSongs);
    for (i = 0; i < n; i++)
        packed = packed * 100 + Library_SongID(gList[i]);
    return packed;                              /* 10, 20 -> 1020 */
}

int main(void)
{
    short p, q, i, n;
    char name[32];
    OSErr err;

    LibFilter_Clear(&gAll);

    Test("a new playlist: an empty TEXT file, names unique ignoring case");
    Library4();
    Library_Scan(&gF);
    CHECK_EQ(Library_AddPlaylist(PS("Road Trip"), &p), noErr);
    CHECK_EQ(p, 0);
    CHECK(TestFS_Exists(gF.playlistsID, "Road Trip"));
    CHECK_EQ(TestFS_Type(gF.playlistsID, "Road Trip"), kPlaylistFileType);
    CHECK_TEXT("Road Trip", "");
    CHECK_EQ(Library_AddPlaylist(PS("road trip"), &q), dupFNErr);
    CHECK_EQ(Library_PlaylistCount(), 1);
    Library_Scan(&gF);
    CHECK_EQ(Playlist("ROAD TRIP"), 0);

    Test("adding songs: one line each; the file read back in order");
    Library4();
    Library_Scan(&gF);
    Library_AddPlaylist(PS("Mix"), &p);
    CHECK_EQ(Library_AppendToPlaylist(p, Library_SongForID(30)), noErr);
    CHECK_EQ(Library_AppendToPlaylist(p, Library_SongForID(10)), noErr);
    CHECK_EQ(Library_AppendToPlaylist(p, Library_SongForID(30)), noErr);
    CHECK_TEXT("Mix", "30\tCharlie|10\tAlpha|30\tCharlie|");
    CHECK_EQ(IDs(p), 301030);
    CHECK(Library_PlaylistContains(p, Library_SongForID(10)));
    CHECK(!Library_PlaylistContains(p, Library_SongForID(20)));

    Test("hand-edited files: other lines kept and skipped, no final CR");
    Library4();
    WritePlaylist(&gF, "Edited", "# my list\n20\tBravo\n99\tGone\n\n40\tDelta");
    Library_Scan(&gF);
    p = Playlist("Edited");
    CHECK_EQ(IDs(p), 2040);                     /* 99 isn't in the library */
    CHECK_EQ(Library_AppendToPlaylist(p, Library_SongForID(10)), noErr);
    CHECK_TEXT("Edited", "# my list\n20\tBravo\n99\tGone\n\n40\tDelta|10\tAlpha|");
    CHECK_EQ(IDs(p), 204010);

    Test("IDs of every length, including 1");
    gF = NewLibrary();
    {
        SongSpec t = { 0 };
        static const long ids[] = { 1, 2, 9, 100, 4321 };

        for (i = 0; i < 5; i++)
        {
            t.id = ids[i];
            t.title = "x";
            MakeSong(gF.folderID, &t);
        }
    }
    WritePlaylist(&gF, "Mix", "4321\tx\r1\tx\r100\tx\r9\tx\r2\tx\r");
    WritePlaylist(&gF, "Bare", "1\r9\n100");     /* numbers only, no end */
    Library_Scan(&gF);
    CHECK_EQ(IDs(Playlist("Bare")), 11000);         /* 1, 9, 100 packed */
    {
        LibFilter f;

        LibFilter_Clear(&f);
        f.playlist = Playlist("Mix");
        n = Library_ListSongs(&f, gList, kMaxSongs);
        CHECK_EQ(n, 5);
        CHECK_EQ(Library_SongID(gList[0]), 4321);
        CHECK_EQ(Library_SongID(gList[1]), 1);
        CHECK_EQ(Library_SongID(gList[2]), 100);
        CHECK_EQ(Library_SongID(gList[3]), 9);
        CHECK_EQ(Library_SongID(gList[4]), 2);
    }

    Test("the 10-playlist limit, in creating and in scanning");
    Library4();
    Library_Scan(&gF);
    for (i = 1; i <= kMaxPlaylists; i++)
    {
        snprintf(name, sizeof name, "List %02d", i);
        CHECK_EQ(Library_AddPlaylist(PS(name), &p), noErr);
    }
    CHECK_EQ(Library_AddPlaylist(PS("List 11"), &p), dirFulErr);
    CHECK(!TestFS_Exists(gF.playlistsID, "List 11"));
    WritePlaylist(&gF, "Aaa Extra", "10\tAlpha\r");
    WritePlaylist(&gF, "Zzz Extra", "10\tAlpha\r");
    Library_Scan(&gF);
    CHECK_EQ(Library_PlaylistCount(), kMaxPlaylists);
    CHECK_EQ(Library_PlaylistsOverLimitCount(), 2);
    CHECK(Playlist("Aaa Extra") != kNone);      /* the first ten by name */
    CHECK_EQ(Playlist("List 10"), kNone);
    CHECK_EQ(Playlist("Zzz Extra"), kNone);

    Test("playlists listed by name, ignoring a leading \"The \"");
    Library4();
    WritePlaylist(&gF, "Zebra", "");
    WritePlaylist(&gF, "The Best", "");
    WritePlaylist(&gF, "apple", "");
    Library_Scan(&gF);
    n = Library_ListPlaylists(gList, kMaxSongs);
    CHECK_EQ(n, 3);
    CHECK_STR(Library_PlaylistName(gList[0]), "apple");
    CHECK_STR(Library_PlaylistName(gList[1]), "The Best");
    CHECK_STR(Library_PlaylistName(gList[2]), "Zebra");

    Test("removing one entry: the nth line for that song only");
    Library4();
    WritePlaylist(&gF, "Mix", "30\tCharlie\r# note\r10\tAlpha\r30\tCharlie\r");
    Library_Scan(&gF);
    p = Playlist("Mix");
    CHECK_EQ(Library_RemoveFromPlaylist(p, 30, 1), noErr);
    CHECK_TEXT("Mix", "30\tCharlie|# note|10\tAlpha|");
    CHECK_EQ(Library_RemoveFromPlaylist(p, 30, 0), noErr);
    CHECK_TEXT("Mix", "# note|10\tAlpha|");
    CHECK_EQ(Library_RemoveFromPlaylist(p, 10, -1), paramErr);

    Test("a song leaving the library: removed from every playlist");
    Library4();
    WritePlaylist(&gF, "One", "20\tBravo\r30\tCharlie\r20\tBravo\r");
    WritePlaylist(&gF, "Two", "40\tDelta\n");
    Library_Scan(&gF);
    CHECK_EQ(Library_RemoveFromPlaylists(20), noErr);
    CHECK_TEXT("One", "30\tCharlie|");
    CHECK_TEXT("Two", "40\tDelta\n");           /* untouched, LF kept */

    Test("renaming: same number, names unique");
    Library4();
    WritePlaylist(&gF, "Old", "10\tAlpha\r");
    WritePlaylist(&gF, "Other", "");
    Library_Scan(&gF);
    p = Playlist("Old");
    CHECK_EQ(Library_RenamePlaylist(p, PS("New")), noErr);
    CHECK_EQ(Playlist("New"), p);
    CHECK(!TestFS_Exists(gF.playlistsID, "Old"));
    CHECK_TEXT("New", "10\tAlpha|");
    CHECK_EQ(Library_RenamePlaylist(p, PS("other")), dupFNErr);
    CHECK_STR(Library_PlaylistName(p), "New");
    CHECK_EQ(Library_RenamePlaylist(p, PS("NEW")), noErr);  /* case only */
    CHECK_STR(Library_PlaylistName(p), "NEW");

    Test("deleting: the file goes, the songs stay, later numbers move down");
    Library4();
    WritePlaylist(&gF, "A", "10\tAlpha\r");
    WritePlaylist(&gF, "B", "20\tBravo\r");
    WritePlaylist(&gF, "C", "30\tCharlie\r");
    Library_Scan(&gF);
    CHECK_EQ(Library_DeletePlaylist(Playlist("B")), noErr);
    CHECK(!TestFS_Exists(gF.playlistsID, "B"));
    CHECK_EQ(Library_PlaylistCount(), 2);
    CHECK_EQ(IDs(Playlist("C")), 30);
    CHECK_EQ(Library_SongCount(), 4);
    CHECK(Library_SongForID(20) != kNone);

    Test("retitling: every line for the song, other lines kept");
    Library4();
    WritePlaylist(&gF, "One", "10\tAlpha\r20\tBravo\r10\told title\r");
    WritePlaylist(&gF, "Two", "20\tBravo\r");
    Library_Scan(&gF);
    CHECK_EQ(Library_RetitleInPlaylists(10, PS("Alpha (Remastered)")), noErr);
    CHECK_TEXT("One", "10\tAlpha (Remastered)|20\tBravo|10\tAlpha (Remastered)|");
    CHECK_TEXT("Two", "20\tBravo|");

    Test("a locked playlist: errors, nothing changed");
    Library4();
    WritePlaylist(&gF, "Locked", "10\tAlpha\r");
    Library_Scan(&gF);
    TestFS_Lock(gF.playlistsID, "Locked", true);
    p = Playlist("Locked");
    err = Library_AppendToPlaylist(p, Library_SongForID(20));
    CHECK(err != noErr);
    CHECK(Library_RemoveFromPlaylist(p, 10, 0) != noErr);
    CHECK(Library_DeletePlaylist(p) != noErr);
    CHECK_TEXT("Locked", "10\tAlpha|");
    CHECK_EQ(Library_PlaylistCount(), 1);

    return TestsDone("playlists");
}
