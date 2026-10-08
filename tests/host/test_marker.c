/* Library folders (libfolder.c): creating and finding a library through
   Standard File, the "MacMusic Library Info" marker (and Fix 2's carry-
   over of an old "iPod Library Info"), the next song ID kept in it
   (M4: IDs never reused), and finding a remembered library again. */

#include "hosttest.h"

static const char *kMarker = "MacMusic Library Info";
static const char *kOldMarker = "iPod Library Info";

static const char *MarkerText(long dir, const char *name)
{
    static char text[2048];
    long n = TestFS_ReadFile(dir, name, text, sizeof text - 1);

    if (n < 0)
        return "(none)";
    text[n < (long)sizeof text - 1 ? n : (long)sizeof text - 1] = 0;
    return text;
}

static void WriteText(long dir, const char *name, OSType type,
                      const char *text)
{
    TestFS_WriteFile(dir, name, type, kAppCreator, text, (long)strlen(text));
}

static void CheckC(const char *got, const char *expected, int line)
{
    gChecks++;
    if (strcmp(got, expected) != 0)
    {
        gFailures++;
        printf("  FAIL %s (line %d): \"%s\", expected \"%s\"\n", gTest, line,
               got, expected);
    }
}
#define CHECK_C(got, expected) CheckC((got), (expected), __LINE__)

int main(void)
{
    LibraryLocation loc, again;
    LibraryFolder f;
    short err;
    long lib, playlists;
    Str31 folderName;

    Test("Create New: the folder, its Playlists and the marker");
    TestSF_SetReply(true, TestFS_NewWD(TestFS_Root()), 0, "My Music");
    CHECK_EQ(Folder_Choose(&loc, &f, &err), kFolderChosen);
    CHECK(TestFS_Exists(TestFS_Root(), "My Music"));
    lib = f.folderID;
    CHECK(TestFS_Exists(lib, "Playlists"));
    CHECK_EQ(TestFS_Type(lib, kMarker), kLibraryMarkerType);
    CHECK(strstr(MarkerText(lib, kMarker), "MacMusic Library") != nil);
    CHECK_STR(loc.volumeName, "Test Disk");
    CHECK_EQ(loc.volumeCreated, TestFS_VolumeCreated());
    CHECK_EQ(loc.folderID, lib);
    CHECK_EQ(Folder_ReadNextID(&f), 0);

    Test("Create New: cancelled, or the name is a file's");
    TestSF_SetReply(false, 0, 0, "");
    CHECK_EQ(Folder_Choose(&loc, &f, &err), kFolderCancelled);
    WriteText(TestFS_Root(), "Taken", 'TEXT', "x");
    TestSF_SetReply(true, TestFS_NewWD(TestFS_Root()), 0, "Taken");
    CHECK_EQ(Folder_Choose(&loc, &f, &err), kFolderNameIsFile);

    Test("Create New on an existing folder reuses it");
    lib = TestFS_MakeDir(TestFS_Root(), "MacMusic Library");
    WriteText(lib, "Notes", 'TEXT', "keep me");
    TestSF_SetReply(true, TestFS_NewWD(TestFS_Root()), 0, "MacMusic Library");
    CHECK_EQ(Folder_Choose(&loc, &f, &err), kFolderChosen);
    CHECK_EQ(f.folderID, lib);
    CHECK_C(MarkerText(lib, "Notes"), "keep me");

    Test("Use Existing: by the marker, a song or a playlist");
    f = NewLibrary();
    WriteText(f.folderID, kMarker, kLibraryMarkerType, "x\r");
    WriteText(f.playlistsID, "Mix", kPlaylistFileType, "");
    TestSF_SetReply(true, TestFS_NewWD(f.folderID), kLibraryMarkerType, kMarker);
    lib = f.folderID;
    CHECK_EQ(Folder_FindExisting(&loc, &f, &err), kFolderChosen);
    CHECK_EQ(f.folderID, lib);
    TestSF_SetReply(true, TestFS_NewWD(f.playlistsID), kPlaylistFileType, "Mix");
    CHECK_EQ(Folder_FindExisting(&loc, &f, &err), kFolderChosen);
    CHECK_EQ(f.folderID, lib);

    Test("Use Existing: a folder that isn't a library");
    lib = TestFS_MakeDir(TestFS_Root(), "Elsewhere");
    WriteText(lib, "Letter", kSongFileType, "not a library");
    TestSF_SetReply(true, TestFS_NewWD(lib), kSongFileType, "Letter");
    CHECK_EQ(Folder_FindExisting(&loc, &f, &err), kFolderNotLibrary);
    CHECK(!TestFS_Exists(lib, kMarker));

    Test("Use Existing: a library from before markers gets one");
    f = NewLibrary();
    {
        SongSpec s = { 0 };

        s.id = 1;
        s.title = "Old Song";
        MakeSong(f.folderID, &s);
    }
    lib = f.folderID;
    TestSF_SetReply(true, TestFS_NewWD(lib), kSongFileType, "IPOD0001");
    CHECK_EQ(Folder_FindExisting(&loc, &f, &err), kFolderChosen);
    CHECK(TestFS_Exists(lib, kMarker));

    Test("Fix 2: an old marker is renamed, its text and counter kept");
    f = NewLibrary();
    WriteText(f.folderID, kOldMarker, kLibraryMarkerType,
              "Old text.\rNext song ID: 42\r");
    loc.volumeName[0] = 0;
    memcpy(loc.volumeName, TestFS_VolumeName(), TestFS_VolumeName()[0] + 1);
    loc.volumeCreated = TestFS_VolumeCreated();
    loc.folderID = f.folderID;
    CHECK(Folder_Resolve(&loc, &f));
    CHECK(!TestFS_Exists(f.folderID, kOldMarker));
    CHECK_C(MarkerText(f.folderID, kMarker), "Old text.\rNext song ID: 42\r");
    CHECK_EQ(Folder_ReadNextID(&f), 42);

    Test("Fix 2: both markers: the new one used, the old left alone");
    f = NewLibrary();
    WriteText(f.folderID, kOldMarker, kLibraryMarkerType, "Next song ID: 3\r");
    WriteText(f.folderID, kMarker, kLibraryMarkerType, "Next song ID: 7\r");
    loc.folderID = f.folderID;
    CHECK(Folder_Resolve(&loc, &f));
    CHECK_EQ(Folder_ReadNextID(&f), 7);
    CHECK_C(MarkerText(f.folderID, kOldMarker), "Next song ID: 3\r");

    Test("Fix 2: a locked old marker is kept and still read");
    f = NewLibrary();
    WriteText(f.folderID, kOldMarker, kLibraryMarkerType, "Next song ID: 13\r");
    TestFS_Lock(f.folderID, kOldMarker, true);
    loc.folderID = f.folderID;
    CHECK(Folder_Resolve(&loc, &f));
    CHECK(TestFS_Exists(f.folderID, kOldMarker));
    CHECK(!TestFS_Exists(f.folderID, kMarker));
    CHECK_EQ(Folder_ReadNextID(&f), 13);

    Test("the next song ID: written as the last line, the text kept");
    f = NewLibrary();
    WriteText(f.folderID, kMarker, kLibraryMarkerType, "Some words");
    CHECK_EQ(Folder_ReadNextID(&f), 0);
    Folder_WriteNextID(&f, 5);
    CHECK_C(MarkerText(f.folderID, kMarker), "Some words\rNext song ID: 5\r");
    CHECK_EQ(Folder_ReadNextID(&f), 5);
    Folder_WriteNextID(&f, 123);
    CHECK_C(MarkerText(f.folderID, kMarker), "Some words\rNext song ID: 123\r");
    CHECK_EQ(Folder_ReadNextID(&f), 123);

    Test("the next song ID with no marker: one is made");
    f = NewLibrary();
    Folder_WriteNextID(&f, 9);
    CHECK_EQ(Folder_ReadNextID(&f), 9);
    CHECK(strstr(MarkerText(f.folderID, kMarker), "MacMusic Library") != nil);

    Test("the next song ID on a locked disk: left as it was");
    f = NewLibrary();
    WriteText(f.folderID, kMarker, kLibraryMarkerType, "Next song ID: 4\r");
    TestFS_LockVolume(true);
    Folder_WriteNextID(&f, 8);
    TestFS_LockVolume(false);
    CHECK_EQ(Folder_ReadNextID(&f), 4);

    Test("a remembered library: found, Playlists remade, or gone");
    f = NewLibrary();
    lib = f.folderID;
    memcpy(loc.volumeName, TestFS_VolumeName(), TestFS_VolumeName()[0] + 1);
    loc.volumeCreated = TestFS_VolumeCreated();
    loc.folderID = lib;
    TestFS_Delete(lib, "Playlists");
    CHECK(Folder_Resolve(&loc, &f));
    CHECK(TestFS_Exists(lib, "Playlists"));
    playlists = f.playlistsID;
    CHECK(playlists != 0);
    CHECK(Folder_Find(&loc, folderName));
    CHECK_STR(folderName, "MacMusic Library");
    again = loc;
    CHECK(Folder_SameLocation(&loc, &again));
    again.volumeName[1] = 't';                  /* names ignore case */
    CHECK(Folder_SameLocation(&loc, &again));
    again.folderID++;
    CHECK(!Folder_SameLocation(&loc, &again));
    again = loc;
    again.volumeCreated++;                      /* another disk, same name */
    CHECK(!Folder_Resolve(&again, &f));
    CHECK(!Folder_Find(&again, folderName));
    TestFS_SetMounted(false);
    CHECK(!Folder_Resolve(&loc, &f));
    CHECK(!Folder_Find(&loc, folderName));
    TestFS_SetMounted(true);
    TestFS_Delete(lib, "Playlists");
    TestFS_Delete(lib, kMarker);
    TestFS_Delete(TestFS_Root(), "MacMusic Library");
    CHECK(!Folder_Resolve(&loc, &f));

    return TestsDone("marker");
}
