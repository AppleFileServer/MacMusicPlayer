/* 1.1, printing (printlayout.c, N11-11 to N11-14): what each printed
   list holds, how it's divided into pages, and how its text is cut to
   fit its columns. Text here is 6 units a character (7 in bold). */

#include <string.h>

#include "hosttest.h"
#include "printlayout.h"

static LibraryFolder gF;

static short Width(ConstStringPtr text, Boolean bold)
{
    return (short)(text[0] * (bold ? 7 : 6));
}

static PrintPage Page(short width, short lines)
{
    PrintPage p;

    p.width = width;
    p.lineHeight = 12;
    p.height = (short)(lines * 12);
    p.textWidth = Width;
    return p;
}

static void Song(long id, const char *title, const char *artist,
                 const char *album, short track, long seconds, short flags)
{
    SongSpec s = { 0 };

    s.id = id;
    s.title = title;
    s.artist = artist;
    s.album = album;
    s.track = track;
    s.flags = flags;
    s.samples = seconds * 22050;
    CHECK_EQ(MakeSong(gF.folderID, &s), noErr);
}

static void Small(void)
{
    gF = NewLibrary();
    Song(1, "Taxman", "The Beatles", "Revolver", 1, 9, 0);
    Song(2, "Eleanor Rigby", "The Beatles", "Revolver", 2, 6, 0);
    Song(3, "Something", "The Beatles", "Abbey Road", 2, 8, 0);
    Song(4, "Dancing Queen", "ABBA", "Arrival", 2, 7, 0);
    Song(5, "Duet", "Other Singer", "Arrival", 9, 3, 0);
    Song(6, "A Book", "Narrator", "", 0, 5, kSongFlagAudiobook);
    CHECK(Library_Scan(&gF));
}

/* A line's cells, joined with '|'. */
static const char *Cells(short page, short index)
{
    static char out[600];
    PrintLine line;
    short i;

    PrintLayout_Line(page, index, &line);
    out[0] = 0;
    for (i = 0; i < kPrintColumns; i++)
    {
        if (i)
            strcat(out, "|");
        strncat(out, (const char *)line.cells[i] + 1, line.cells[i][0]);
    }
    return out;
}

static short TypeOf(short page, short index)
{
    PrintLine line;

    PrintLayout_Line(page, index, &line);
    return line.type;
}

static void CheckText(const char *got, const char *expected, int at)
{
    gChecks++;
    if (strcmp(got, expected) != 0)
    {
        gFailures++;
        printf("  FAIL %s (line %d): \"%s\", expected \"%s\"\n", gTest, at, got, expected);
    }
}
#define CHECK_TEXT(got, expected) CheckText((got), (expected), __LINE__)

int main(void)
{
    PrintOptions none = { false, false }, all = { true, true };
    PrintPage page;
    PrintLine line;
    Str255 s;
    short p, i, total;

    Test("times and page numbers");
    PrintLayout_Time(0, s);       CHECK_STR(s, "0:00");
    PrintLayout_Time(59, s);      CHECK_STR(s, "0:59");
    PrintLayout_Time(222, s);     CHECK_STR(s, "3:42");
    PrintLayout_Time(3600, s);    CHECK_STR(s, "1:00:00");
    PrintLayout_Time(3725, s);    CHECK_STR(s, "1:02:05");
    CHECK_STR(PrintLayout_ListName(kPrintArtists), "Artists");

    Test("Songs: by title, then the audiobooks under a heading");
    Small();
    page = Page(600, 40);
    CHECK(PrintLayout_Begin(kPrintSongs, &none, &page));
    CHECK_EQ(PrintLayout_PageCount(), 1);
    CHECK_EQ(PrintLayout_LineCount(1), 1 + 5 + 2);
    CHECK_TEXT(Cells(1, 0), "Title|Artist|Album|Time");
    CHECK_EQ(TypeOf(1, 0), kLineColumnHeads);
    CHECK_TEXT(Cells(1, 1), "Dancing Queen|ABBA|Arrival|0:07");
    CHECK_TEXT(Cells(1, 2), "Duet|Other Singer|Arrival|0:03");
    CHECK_TEXT(Cells(1, 3), "Eleanor Rigby|The Beatles|Revolver|0:06");
    CHECK_TEXT(Cells(1, 5), "Taxman|The Beatles|Revolver|0:09");
    CHECK_EQ(TypeOf(1, 6), kLineSection);
    CHECK_TEXT(Cells(1, 6), "Audiobooks|||");
    CHECK_TEXT(Cells(1, 7), "A Book|Narrator||0:05");
    PrintLayout_PageText(1, s);
    CHECK_STR(s, "Page 1 of 1");

    Test("Albums, with and without their songs; Various Artists");
    Small();
    page = Page(600, 40);
    CHECK(PrintLayout_Begin(kPrintAlbums, &none, &page));
    CHECK_EQ(PrintLayout_LineCount(1), 1 + 3);
    CHECK_TEXT(Cells(1, 0), "Album|Artist|Songs|Time");
    CHECK_TEXT(Cells(1, 1), "Abbey Road|The Beatles|1|0:08");
    CHECK_TEXT(Cells(1, 2), "Arrival|Various Artists|2|0:10");
    CHECK_TEXT(Cells(1, 3), "Revolver|The Beatles|2|0:15");
    CHECK(PrintLayout_Begin(kPrintAlbums, &all, &page));
    CHECK_EQ(PrintLayout_LineCount(1), 1 + 3 + 5);
    CHECK_TEXT(Cells(1, 3), "Arrival|Various Artists|2|0:10");
    CHECK_EQ(TypeOf(1, 4), kLineSub);
    CHECK_TEXT(Cells(1, 4), "2|Dancing Queen||0:07");     /* in track order */
    CHECK_TEXT(Cells(1, 5), "9|Duet||0:03");
    CHECK_TEXT(Cells(1, 7), "1|Taxman||0:09");
    PrintLayout_Line(1, 4, &line);
    CHECK(line.left[1] > 0);                                /* indented */
    CHECK(line.alignRight[3]);

    Test("Artists, with and without their albums");
    Small();
    page = Page(600, 40);
    CHECK(PrintLayout_Begin(kPrintArtists, &none, &page));
    CHECK_EQ(PrintLayout_LineCount(1), 1 + 3);              /* not the narrator */
    CHECK_TEXT(Cells(1, 0), "Artist||Albums|Songs");
    CHECK_TEXT(Cells(1, 1), "ABBA||1|1");
    CHECK_TEXT(Cells(1, 2), "The Beatles||2|3");      /* "The" ignored */
    CHECK_TEXT(Cells(1, 3), "Other Singer||1|1");
    CHECK(PrintLayout_Begin(kPrintArtists, &all, &page));
    CHECK_TEXT(Cells(1, 2), "Arrival|||1");
    CHECK_TEXT(Cells(1, 3), "The Beatles||2|3");
    CHECK_TEXT(Cells(1, 4), "Abbey Road|||1");
    CHECK_TEXT(Cells(1, 5), "Revolver|||2");
    CHECK_TEXT(Cells(1, 6), "Other Singer||1|1");

    Test("text too wide for its column is cut, with an ellipsis");
    Small();
    page = Page(300, 40);                       /* Title: 120 units, 20 characters */
    CHECK(PrintLayout_Begin(kPrintSongs, &none, &page));
    PrintLayout_Line(1, 1, &line);
    CHECK(line.cells[0][0] > 0);
    CHECK(Width(line.cells[0], false) <= line.right[0] - line.left[0]);
    page = Page(100, 40);                       /* Artist: 25 units, 4 characters */
    CHECK(PrintLayout_Begin(kPrintSongs, &none, &page));
    PrintLayout_Line(1, 3, &line);              /* "The Beatles" */
    CHECK_STR(line.cells[1], "The\xC9");
    PrintLayout_Line(1, 0, &line);              /* bold heads: 7 a character */
    CHECK(Width(line.cells[1], true) <= line.right[1] - line.left[1]);

    Test("pages: lines per page, and every line once");
    gF = NewLibrary();
    for (i = 1; i <= 100; i++)
    {
        char t[16];
        snprintf(t, sizeof t, "Song %03d", i);
        Song(i, t, "Artist", "Album", (short)(i % 20), 1, 0);
    }
    CHECK(Library_Scan(&gF));
    page = Page(600, 26);                       /* 26 - 3 - 2 - 1 = 20 a page */
    CHECK(PrintLayout_Begin(kPrintSongs, &none, &page));
    CHECK_EQ(PrintLayout_PageCount(), 5);
    for (p = 1, total = 0; p <= PrintLayout_PageCount(); p++)
    {
        CHECK_EQ(TypeOf(p, 0), kLineColumnHeads);   /* heads on every page */
        total += PrintLayout_LineCount(p) - 1;
    }
    CHECK_EQ(total, 100);
    CHECK_TEXT(Cells(2, 1), "Song 021|Artist|Album|0:01");
    PrintLayout_PageText(3, s);
    CHECK_STR(s, "Page 3 of 5");

    Test("an album isn't left alone at the foot of a page");
    gF = NewLibrary();
    for (i = 1; i <= 19; i++)
    {
        char t[16];
        snprintf(t, sizeof t, "Song %03d", i);
        Song(i, t, "Artist", "A First", (short)i, 1, 0);
    }
    Song(20, "Last", "Artist", "B Second", 1, 1, 0);
    Song(21, "Very Last", "Artist", "B Second", 2, 1, 0);
    CHECK(Library_Scan(&gF));
    page = Page(600, 26);                       /* 20 lines a page */
    CHECK(PrintLayout_Begin(kPrintAlbums, &all, &page));
    /* Page 1: A First and its 19 songs (20 lines); then B Second, whose
       heading would have been line 20 of a page, starts page 2. */
    CHECK_EQ(PrintLayout_LineCount(1) - 1, 20);
    CHECK_TEXT(Cells(2, 1), "B Second|Artist|2|0:02");

    Test("an album that would start on a page's last line starts the next");
    page = Page(600, 26);
    gF = NewLibrary();
    for (i = 1; i <= 18; i++)
    {
        char t[16];
        snprintf(t, sizeof t, "Song %03d", i);
        Song(i, t, "Artist", "A First", (short)i, 1, 0);
    }
    Song(20, "Last", "Artist", "B Second", 1, 1, 0);
    CHECK(Library_Scan(&gF));
    CHECK(PrintLayout_Begin(kPrintAlbums, &all, &page));
    CHECK_EQ(PrintLayout_LineCount(1) - 1, 19);             /* moved on */
    CHECK_TEXT(Cells(2, 1), "B Second|Artist|1|0:01");
    CHECK_TEXT(Cells(2, 2), "1|Last||0:01");

    Test("a page too small, an empty library, no memory");
    Small();
    page = Page(600, 6);                        /* no room for a line */
    CHECK(!PrintLayout_Begin(kPrintSongs, &none, &page));
    page = Page(600, 7);
    CHECK(PrintLayout_Begin(kPrintSongs, &none, &page));
    CHECK_EQ(PrintLayout_PageCount(), 7);

    Test("an empty library: the heads alone; and no memory");
    gF = NewLibrary();
    CHECK(Library_Scan(&gF));
    page = Page(600, 40);
    CHECK(PrintLayout_Begin(kPrintSongs, &none, &page));
    CHECK_EQ(PrintLayout_PageCount(), 1);
    CHECK_EQ(PrintLayout_LineCount(1), 1);                  /* the heads alone */
    Small();
    TestMem_SetLimit(1);
    CHECK(!PrintLayout_Begin(kPrintSongs, &none, &page));
    TestMem_SetLimit(0);
    PrintLayout_End();

    return TestsDone("test_printlayout");
}
