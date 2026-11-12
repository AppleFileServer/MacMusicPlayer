/* 1.1 (N11-23): the library's lists come out exactly as 1.0's did. 1.0
   sorted the songs by title with an insertion sort, and found each song's
   artist, album, genre and composer by a search from the start of each
   list; 1.1 uses a merge sort and a binary search. This test rebuilds
   1.0's way here, from the scanned songs in their folder order, and
   checks the library's songs list and every song's groups against it,
   for libraries full of ties: names equal but for capitals, a leading
   "The", accents, and repeated titles. */

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "hosttest.h"

static LibraryFolder gF;
static LibFilter gAll;
static short gList[kMaxSongs + 10];

/* ---- 1.0's way, as it was in library.c ---------------------------------- */

static Boolean HasThePrefix(ConstStringPtr s)
{
    return s[0] > 4
        && (s[1] == 'T' || s[1] == 't')
        && (s[2] == 'H' || s[2] == 'h')
        && (s[3] == 'E' || s[3] == 'e')
        && s[4] == ' ';
}

long RelStringTrap(const unsigned char *a, const unsigned char *b, long lens);

static short CompareNames(ConstStringPtr a, ConstStringPtr b)
{
    const unsigned char *textA = a + 1, *textB = b + 1;
    long lengthA = a[0], lengthB = b[0];

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

static void OldSortByTitle(short *songs, short count)
{
    short i, j, s;

    for (i = 1; i < count; i++)
    {
        s = songs[i];
        for (j = i; j > 0 && CompareNames(Library_SongTitle(songs[j - 1]),
                                          Library_SongTitle(s)) > 0; j--)
            songs[j] = songs[j - 1];
        songs[j] = s;
    }
}

/* 1.0's group lists: InsertName's sorted, deduplicated names. */
typedef struct
{
    short count;
    Str255 names[kMaxGroups];
} OldSet;

static void OldInsert(OldSet *set, ConstStringPtr name)
{
    short i, j;

    for (i = 0; i < set->count; i++)
        if (EqualString(set->names[i], name, true, true))
            return;
    if (set->count >= kMaxGroups)
        return;
    for (i = 0; i < set->count && CompareNames(set->names[i], name) <= 0; i++)
        ;
    for (j = set->count; j > i; j--)
        memcpy(set->names[j], set->names[j - 1], sizeof(Str255));
    memcpy(set->names[i], name, name[0] + 1);
    set->count++;
}

static short OldFind(const OldSet *set, ConstStringPtr name)
{
    short i;

    for (i = 0; i < set->count; i++)
        if (EqualString(set->names[i], name, true, true))
            return i;
    return kNone;
}

/* ---- The test libraries -------------------------------------------------- */

/* Simple repeatable numbers, so each library is the same every run. */
static unsigned long gSeed;
static int Pick(int n)
{
    gSeed = gSeed * 1103515245UL + 12345UL;
    return (int)((gSeed >> 16) % (unsigned long)n);
}

static const char *kTitles[] = {
    "Yesterday", "yesterday", "YESTERDAY", "The Yesterday", "the yesterday",
    "Help!", "help!", "Hey Jude", "The End", "End", "the end", "Theme",
    "The", "The ", "Th\x8E End", "Th\x8F End", "A", "a", "Ab", "Abba", "ABBA",
    "Zebra", "zebra", "Caf\x8E", "Cafe", "CAF\x83", "Eleanor Rigby",
    "Something", "Something ", " Something", "1999", "2001", "",
};
static const char *kArtists[] = {
    "The Beatles", "the beatles", "Beatles", "THE BEATLES", "beatles",
    "ABBA", "Abba", "abba", "The The", "The", "Eagles", "The Eagles",
    "Caf\x8E Society", "Cafe Society", "", "Queen", "queen",
};
static const char *kAlbums[] = {
    "Revolver", "revolver", "The Revolver", "Abbey Road", "abbey road",
    "Help!", "Help", "", "The Long Run", "Long Run", "long run",
};
static const char *kGenres[] = { "Rock", "rock", "ROCK", "Pop", "", "The Pop" };
static const char *kComposers[] = {
    "Lennon", "lennon", "Lennon-McCartney", "", "The Composer", "Composer",
};

#define N(a) ((int)(sizeof(a) / sizeof((a)[0])))

/* 1.1 (N11-32): with gManyNames, each song's names are made from a number
   of up to 400, in random case, some with "The " or an accent: long lists
   for the binary search, with names that compare equal. 1.2 (N12-9): with
   gManyNames 2, from a number of up to 100,000, so nearly every song's
   names differ and a table fills to kMaxGroups (now kMaxSongs, so it can
   no longer overflow). */
static short gManyNames;
static char gMade[5][kMaxSongs + 1][32];

static const char *ManyName(int kind, short song)
{
    char *out = gMade[kind][song];
    const char *stem = kind == 0 ? "Artist" : kind == 1 ? "album" : kind == 2 ? "Genre" : "Composer";
    int n = Pick(gManyNames == 2 ? 100000 : 400), c;

    snprintf(out, sizeof gMade[0][0], "%s%s %d%s", Pick(4) == 0 ? "The " : "", stem, n,
             Pick(6) == 0 ? " Caf\x8E" : Pick(6) == 0 ? " Cafe" : "");
    if (Pick(3) == 0)
        for (c = 0; out[c]; c++)
            out[c] = (char)toupper((unsigned char)out[c]);
    return Pick(30) == 0 ? "" : out;
}

static void MakeLibrary(unsigned long seed, short count)
{
    short i;
    SongSpec s;

    gSeed = seed;
    gF = NewLibrary();
    for (i = 1; i <= count; i++)
    {
        memset(&s, 0, sizeof s);
        s.id = i;
        s.title = kTitles[Pick(N(kTitles))];
        s.artist = kArtists[Pick(N(kArtists))];
        s.album = kAlbums[Pick(N(kAlbums))];
        s.genre = kGenres[Pick(N(kGenres))];
        s.composer = kComposers[Pick(N(kComposers))];
        if (gManyNames)
        {
            s.artist = ManyName(0, i);
            s.album = ManyName(1, i);
            s.genre = ManyName(2, i);
            s.composer = ManyName(3, i);
        }
        s.track = (short)Pick(12);
        s.flags = Pick(25) == 0 ? kSongFlagAudiobook : 0;
        s.samples = 16;
        CHECK_EQ(MakeSong(gF.folderID, &s), noErr);
    }
}

/* ---- The checks ---------------------------------------------------------- */

static void CheckTitles(void)
{
    static short expected[kMaxSongs];
    short n = Library_SongCount(), i, got, mismatch = -1;

    short kept = 0;

    for (i = 0; i < n; i++)
        expected[i] = i;
    OldSortByTitle(expected, n);
    for (i = 0; i < n; i++)             /* Songs leaves the audiobooks out (D18) */
        if (!Library_SongIsAudiobook(expected[i]))
            expected[kept++] = expected[i];

    got = Library_ListSongs(&gAll, gList, kMaxSongs);
    CHECK_EQ(got, kept);
    for (i = 0; i < kept && mismatch < 0; i++)
        if (gList[i] != expected[i])
            mismatch = i;
    CHECK_EQ(mismatch, -1);
}

typedef ConstStringPtr (*NameOf)(short song);
typedef ConstStringPtr (*GroupName)(short group);

/* One kind of group: the library's list of names is 1.0's, and each song
   is in the group 1.0 put it in. */
static void CheckGroups(const char *kind, NameOf nameOf, GroupName groupName,
                        short (*list)(short *out, short max),
                        void (*set)(LibFilter *f, short group))
{
    static OldSet old;
    static short groupOf[kMaxSongs];
    short n = Library_SongCount(), i, j, groups, members, bad = 0;
    LibFilter f;
    ConstStringPtr name;

    old.count = 0;
    for (i = 0; i < n; i++)
        if (!Library_SongIsAudiobook(i) && nameOf(i)[0] != 0)
            OldInsert(&old, nameOf(i));

    groups = list(gList, kMaxSongs);
    CHECK_EQ(groups, old.count);
    for (i = 0; i < groups && i < old.count; i++)
    {
        CHECK_EQ(gList[i], i);
        name = groupName(gList[i]);
        if (!EqualString(name, old.names[i], true, true))
            bad++;
    }

    for (i = 0; i < n; i++)
        groupOf[i] = kNone;
    for (j = 0; j < groups; j++)
    {
        static short songs[kMaxSongs];
        LibFilter_Clear(&f);
        set(&f, gList[j]);
        members = Library_ListSongs(&f, songs, kMaxSongs);
        for (i = 0; i < members; i++)
            groupOf[songs[i]] = gList[j];
    }
    for (i = 0; i < n; i++)
    {
        short expect = Library_SongIsAudiobook(i) || nameOf(i)[0] == 0
                       ? kNone : OldFind(&old, nameOf(i));
        if (groupOf[i] != expect)
            bad++;
    }
    if (bad)
        printf("  %s: %d differences\n", kind, bad);
    CHECK_EQ(bad, 0);
}

/* Library's per-song names for each kind, for the comparisons. */
static ConstStringPtr Artist(short s)   { return Library_SongArtistName(s); }
static ConstStringPtr Album(short s)    { return Library_SongAlbumName(s); }

static Str255 gGenreNames[kMaxSongs], gComposerNames[kMaxSongs];
static ConstStringPtr Genre(short s)    { return gGenreNames[s]; }
static ConstStringPtr Composer(short s) { return gComposerNames[s]; }

static short ListArtists(short *out, short max) { return Library_ListArtists(&gAll, out, max); }
static short ListAlbums(short *out, short max)  { return Library_ListAlbums(&gAll, out, max); }
static void SetArtist(LibFilter *f, short g)   { f->artist = g; }
static void SetAlbum(LibFilter *f, short g)    { f->album = g; }
static void SetGenre(LibFilter *f, short g)    { f->genre = g; }
static void SetComposer(LibFilter *f, short g) { f->composer = g; }

/* The genre and composer of each song, as made (the library has no
   per-song call for them): read back from the song files. */
static void ReadGenresAndComposers(void)
{
    short n = Library_SongCount(), i, v;
    long dir;
    ConstStringPtr file;
    long offset;
    SongInfo info;

    for (i = 0; i < n; i++)
    {
        gGenreNames[i][0] = gComposerNames[i][0] = 0;
        if (Library_SongFile(i, &v, &dir, &file, &offset)
            && SongFile_Read(v, dir, file, &info))
        {
            memcpy(gGenreNames[i], info.text[3], info.text[3][0] + 1);
            memcpy(gComposerNames[i], info.text[4], info.text[4][0] + 1);
        }
    }
}

/* 1.2 (N12-11): a filter's albums (or artists), from the one-pass lists,
   are exactly those of the songs the filter takes, in the groups' order. */
static short gExpect[kMaxSongs], gGot[kMaxSongs], gSongs[kMaxSongs + 1];

static void CheckOneFilter(const LibFilter *f, Boolean albums)
{
    short n, i, j, k, g, e = 0, got;
    Boolean ok;

    n = Library_ListSongs(f, gSongs, kMaxSongs);
    for (i = 0; i < n; i++)
    {
        g = albums ? Library_SongAlbum(gSongs[i]) : Library_SongArtist(gSongs[i]);
        if (g == kNone)
            continue;
        for (j = 0; j < e && gExpect[j] != g; j++)
            ;
        if (j < e)
            continue;
        for (k = e; k > 0 && gExpect[k - 1] > g; k--)   /* in the groups' order */
            gExpect[k] = gExpect[k - 1];
        gExpect[k] = g;
        e++;
    }
    got = albums ? Library_ListAlbums(f, gGot, kMaxSongs) : Library_ListArtists(f, gGot, kMaxSongs);
    ok = got == e;
    for (i = 0; ok && i < e; i++)
        ok = gGot[i] == gExpect[i];
    CHECK(ok);
}

static void CheckFilteredGroups(void)
{
    LibFilter f;
    short n, i;
    short artists[kMaxSongs], albums[kMaxSongs], genres[kMaxSongs];
    short na, nb, ng;

    LibFilter_Clear(&f);
    CheckOneFilter(&f, true);
    CheckOneFilter(&f, false);
    na = Library_ListArtists(&f, artists, kMaxSongs);
    nb = Library_ListAlbums(&f, albums, kMaxSongs);
    ng = Library_ListGenres(genres, kMaxSongs);
    for (i = 0; i < na; i++)                    /* an artist's albums */
    {
        LibFilter_Clear(&f);
        f.artist = artists[i];
        CheckOneFilter(&f, true);
    }
    for (i = 0; i < nb; i++)                    /* an album's artists */
    {
        LibFilter_Clear(&f);
        f.album = albums[i];
        CheckOneFilter(&f, false);
    }
    for (i = 0; i < ng; i++)                    /* a genre's artists, and albums */
    {
        LibFilter_Clear(&f);
        f.genre = genres[i];
        CheckOneFilter(&f, false);
        CheckOneFilter(&f, true);
        if (na > 0)
        {
            f.artist = artists[i % na];         /* a genre and an artist */
            CheckOneFilter(&f, true);
        }
    }
    (void)n;
}

static void CheckLibrary(const char *what, unsigned long seed, short count)
{
    Test(what);
    MakeLibrary(seed, count);
    CHECK(Library_Scan(&gF));
    CHECK_EQ(Library_SongCount(), count);
    ReadGenresAndComposers();
    CheckTitles();
    CheckGroups("artists", Artist, Library_ArtistName, ListArtists, SetArtist);
    CheckGroups("albums", Album, Library_AlbumName, ListAlbums, SetAlbum);
    CheckGroups("genres", Genre, Library_GenreName, Library_ListGenres, SetGenre);
    CheckGroups("composers", Composer, Library_ComposerName, Library_ListComposers,
                SetComposer);
    CheckFilteredGroups();
}

int main(void)
{
    LibFilter_Clear(&gAll);

    CheckLibrary("1.0's order: one song", 7, 1);
    CheckLibrary("1.0's order: two songs", 8, 2);
    CheckLibrary("1.0's order: 37 songs", 9, 37);
    CheckLibrary("1.0's order: 500 songs, many ties", 10, kMaxSongs);
    CheckLibrary("1.0's order: 500 songs, another mix", 11, kMaxSongs);
    CheckLibrary("1.0's order: 499 songs", 12, kMaxSongs - 1);
    gManyNames = 1;
    CheckLibrary("1.0's groups: 500 songs, many names alike", 13, kMaxSongs);
    CheckLibrary("1.0's groups: 200 songs, many names", 14, 200);
    gManyNames = 2;
    CheckLibrary("1.2's groups: 500 songs, nearly every name different (N12-9)", 15, kMaxSongs);
    gManyNames = 0;
    return TestsDone("test_sorting");
}
