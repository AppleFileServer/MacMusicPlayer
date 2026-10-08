/* Checks a folder of MacMusic song files with MacMusic's own library code
   (src/library.c, src/songfile.c), built for this Mac with the host
   tests' stand-in Toolbox (tests/host): the folder's files are put on its
   in-memory volume, read from this Mac's disk as they are scanned.

   Usage: check <library folder> <song count>
   The folder holds the song files (IPOD0001...) and a Playlists folder.
   Prints the scan's counts and some songs; exits non-zero on a problem. */

#include <stdio.h>
#include <dirent.h>

#include "testfs.h"
#include "library.h"
#include "songfile.h"

/* Puts host folder dir's files into the volume's folder dirID, as files
   of type fileType (the marker as a marker). */
static void AddFolder(const char *dir, long dirID, OSType fileType)
{
    DIR *d = opendir(dir);
    struct dirent *e;
    char path[1024];
    OSType type;

    if (d == NULL)
        return;
    while ((e = readdir(d)) != NULL)
    {
        if (e->d_name[0] == '.' || e->d_type == DT_DIR)
            continue;
        snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
        type = strcmp(e->d_name, "MacMusic Library Info") == 0
               || strcmp(e->d_name, "iPod Library Info") == 0
               ? kLibraryMarkerType : fileType;
        TestFS_AddHostFile(dirID, e->d_name, path, type, kAppCreator);
    }
    closedir(d);
}

/* ---- The check -------------------------------------------------------- */

static void Show(ConstStringPtr s)
{
    printf("%.*s", s[0], s + 1);
}

int main(int argc, char **argv)
{
    static SongInfo info;
    static short list[600];
    LibraryFolder folder;
    LibFilter all;
    Str63 name;
    char playlists[512];
    int expected, problems = 0;
    short i, n;

    if (argc != 3)
    {
        fprintf(stderr, "usage: check <library folder> <song count>\n");
        return 2;
    }
    expected = atoi(argv[2]);
    TestFS_Reset();
    folder.vRefNum = TestFS_VRefNum();
    folder.folderID = TestFS_MakeDir(TestFS_Root(), "MacMusic Library");
    folder.playlistsID = TestFS_MakeDir(folder.folderID, "Playlists");
    AddFolder(argv[1], folder.folderID, kSongFileType);
    snprintf(playlists, sizeof playlists, "%s/Playlists", argv[1]);
    AddFolder(playlists, folder.playlistsID, kPlaylistFileType);
    if (!Library_Scan(&folder))
    {
        printf("FAIL: the scan ran out of memory\n");
        return 1;
    }

    LibFilter_Clear(&all);
    printf("songs %d, skipped %d, over the limit %d, next ID %ld\n",
           Library_SongCount(), Library_SkippedCount(),
           Library_OverLimitCount(), Library_NextSongID());
    printf("artists %d, albums %d\n",
           Library_ListArtists(&all, list, 600),
           Library_ListAlbums(&all, list, 600));
    if (Library_SongCount() != expected || Library_SkippedCount() != 0
        || Library_OverLimitCount() != 0)
    {
        printf("FAIL: expected %d songs, none skipped\n", expected);
        problems++;
    }

    for (i = 1; i <= expected; i++)
    {
        sprintf((char *)name + 1, "IPOD%04d", i);
        name[0] = 8;
        if (!SongFile_Read(folder.vRefNum, folder.folderID, name, &info) || info.id != i)
        {
            printf("FAIL: IPOD%04d does not read back as song %d\n", i, i);
            problems++;
        }
    }

    n = Library_ListSongs(&all, list, 600);
    printf("some songs (title / artist / album, seconds):\n");
    for (i = 0; i < n; i += n / 10 > 0 ? n / 10 : 1)
    {
        printf("  ");
        Show(Library_SongTitle(list[i]));
        printf(" / ");
        Show(Library_SongArtistName(list[i]));
        printf(" / ");
        Show(Library_SongAlbumName(list[i]));
        printf("  (%ld)\n", Library_SongSeconds(list[i]));
    }
    printf(problems ? "%d problem(s)\n" : "OK\n", problems);
    return problems ? 1 : 0;
}
