/* 1.1 (N11-25): a run of Import Song's own list (importrun.c): the
   duplicate warning finds songs imported earlier in the run as well as
   the library's, and the 500-song limit counts the run's songs. */

#include <string.h>

#include "hosttest.h"
#include "testfs.h"
#include "importrun.h"
#include "library.h"
#include "songfile.h"

static LibraryFolder gF;

static void Library(short count)
{
    short i;
    char title[16];
    SongSpec s;

    gF = NewLibrary();
    for (i = 1; i <= count; i++)
    {
        memset(&s, 0, sizeof s);
        snprintf(title, sizeof title, "Song %d", i);
        s.id = i;
        s.title = title;
        CHECK_EQ(MakeSong(gF.folderID, &s), noErr);
    }
    CHECK(Library_Scan(&gF));
}

/* The fingerprint of the library's song with this ID. */
static unsigned long Fingerprint(long id)
{
    short v;
    long dir, offset;
    ConstStringPtr file;
    SongInfo info;

    if (!Library_SongFile(Library_SongForID(id), &v, &dir, &file, &offset)
        || !SongFile_Read(v, dir, file, &info))
        return 0;
    return info.fingerprint;
}

int main(void)
{
    Str255 title;
    unsigned long two;
    short i;

    Test("a new run: empty; the library's songs found as duplicates");
    Library(3);
    two = Fingerprint(2);
    ImportRun_Begin();
    CHECK_EQ(ImportRun_Count(), 0);
    CHECK(!ImportRun_LibraryFull());
    CHECK(ImportRun_FindDuplicate(2206, two, title));
    CHECK_STR(title, "Song 2");
    CHECK(!ImportRun_FindDuplicate(2205, two, title));
    CHECK(!ImportRun_FindDuplicate(1234, 99, title));

    ImportRun_End();

    Test("songs imported in the run: found, counted");
    Library(3);                                 /* each test starts afresh */
    two = Fingerprint(2);
    ImportRun_Begin();
    ImportRun_Add(1234, 99, PS("New One"));
    ImportRun_Add(5678, 98, PS("New Two"));
    CHECK_EQ(ImportRun_Count(), 2);
    CHECK(ImportRun_FindDuplicate(1234, 99, title));
    CHECK_STR(title, "New One");
    CHECK(ImportRun_FindDuplicate(5678, 98, title));
    CHECK_STR(title, "New Two");
    CHECK(!ImportRun_FindDuplicate(1234, 98, title));
    CHECK(ImportRun_FindDuplicate(2206, two, title));   /* the library first */
    CHECK_STR(title, "Song 2");

    ImportRun_End();

    Test("a title over 63 characters is kept to 63");
    ImportRun_Begin();
    ImportRun_Add(4321, 97, PS("A very long title indeed, longer than any song file can hold, by far"));
    CHECK(ImportRun_FindDuplicate(4321, 97, title));
    CHECK_EQ(title[0], 63);

    ImportRun_End();

    Test("the run's end: the list freed");
    ImportRun_Begin();
    ImportRun_Add(1234, 99, PS("New One"));
    ImportRun_Add(5678, 98, PS("New Two"));
    ImportRun_End();
    CHECK_EQ(ImportRun_Count(), 0);
    CHECK(!ImportRun_FindDuplicate(1234, 99, title));
    ImportRun_Begin();
    CHECK(!ImportRun_FindDuplicate(5678, 98, title));
    ImportRun_End();

    Test("short of memory: the song still counted, its warning lost");
    ImportRun_Begin();
    TestMem_SetLimit(1);
    ImportRun_Add(1234, 99, PS("New One"));
    TestMem_SetLimit(0);
    CHECK_EQ(ImportRun_Count(), 1);
    CHECK(!ImportRun_FindDuplicate(1234, 99, title));
    ImportRun_Add(5678, 98, PS("New Two"));
    CHECK_EQ(ImportRun_Count(), 2);
    CHECK(ImportRun_FindDuplicate(5678, 98, title));
    ImportRun_End();

    Test("the 500-song limit counts the run's songs");
    Library(kMaxSongs - 3);
    ImportRun_Begin();
    for (i = 0; i < 2; i++)
    {
        CHECK(!ImportRun_LibraryFull());
        ImportRun_Add(100 + i, i, PS("New"));
    }
    CHECK(!ImportRun_LibraryFull());                    /* 499 */
    ImportRun_Add(200, 0, PS("New"));
    CHECK(ImportRun_LibraryFull());                     /* 500 */
    ImportRun_End();
    CHECK(!ImportRun_LibraryFull());                    /* a new run */

    Test("a library already over the limit: full from the start");
    Library(kMaxSongs + 1);
    CHECK_EQ(Library_OverLimitCount(), 1);
    ImportRun_Begin();
    CHECK(ImportRun_LibraryFull());
    ImportRun_End();

    return TestsDone("test_importrun");
}
