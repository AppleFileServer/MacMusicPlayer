#include <Memory.h>
#include <TextUtils.h>
#include <string.h>

#include "printlayout.h"
#include "library.h"
#include "pstr.h"
#include "message.h"                    /* PStrAppend */

/* A line of the list before it's laid out: its type and what it shows
   (a song, album or artist number; for a sub-line, its own and its
   group's). */
typedef struct
{
    short type;
    short item;
    short group;
} LineRec;

static Handle gLinesH;
static short gLineCount;
static short gList;
static PrintPage gPage;
static short gPerPage;          /* list lines on a page, after the heads */
static short gPageStart[kMaxSongs * 2 + 4];
static short gPageCount;

/* Scratch lists, static to keep them off the small stack. */
static short gItems[kMaxSongs + kMaxGroups];
static short gSubs[kMaxSongs];

#define Lines()     ((LineRec *)*gLinesH)
#define kEllipsis   ((unsigned char)0xC9)

static const unsigned char kListNames[kPrintListCount][9] = {
    "\pSongs", "\pAlbums", "\pArtists"
};

ConstStringPtr PrintLayout_ListName(short list)
{
    return kListNames[list];
}

/* ---- Text --------------------------------------------------------------- */

static void Copy(Str255 out, ConstStringPtr s)
{
    BlockMove(s, out, s[0] + 1);
}

static void AppendNumber(Str255 out, long n, Boolean twoDigits)
{
    Str255 s;

    NumToString(n, s);
    if (twoDigits && n < 10)
        PStrAppend(out, P("0"));
    PStrAppend(out, s);
}

void PrintLayout_Time(long seconds, Str255 out)
{
    out[0] = 0;
    if (seconds >= 3600)
    {
        AppendNumber(out, seconds / 3600, false);
        PStrAppend(out, P(":"));
        AppendNumber(out, seconds / 60 % 60, true);
    }
    else
        AppendNumber(out, seconds / 60, false);
    PStrAppend(out, P(":"));
    AppendNumber(out, seconds % 60, true);
}

void PrintLayout_PageText(short page, Str255 out)
{
    out[0] = 0;
    PStrAppend(out, P("Page "));
    AppendNumber(out, page, false);
    PStrAppend(out, P(" of "));
    AppendNumber(out, gPageCount, false);
}

/* Cuts text to width, ending it with an ellipsis (as the screen does). */
static void Fit(Str255 text, short width, Boolean bold)
{
    Str255 ellipsis;

    if (gPage.textWidth(text, bold) <= width)
        return;
    ellipsis[0] = 1;
    ellipsis[1] = kEllipsis;
    while (text[0] > 0)
    {
        text[0]--;
        while (text[0] > 0 && text[text[0]] == ' ')
            text[0]--;
        text[++text[0]] = kEllipsis;
        if (gPage.textWidth(text, bold) <= width)
            return;
        text[0]--;
    }
    if (gPage.textWidth(ellipsis, bold) <= width)
        Copy(text, ellipsis);
}

/* ---- The columns (PROVISIONAL, N11-11) ---------------------------------- */

/* Each list's columns, as per mille of the page's width: left edge, right
   edge, aligned right. Sub-lines have their own, indented. */
typedef struct
{
    short left, right;
    Boolean alignRight;
} Column;

static const Column kSongColumns[kPrintColumns] = {
    { 0, 400, false }, { 410, 660, false }, { 670, 905, false }, { 915, 1000, true }
};
static const Column kAlbumColumns[kPrintColumns] = {
    { 0, 480, false }, { 490, 790, false }, { 800, 885, true }, { 895, 1000, true }
};
static const Column kAlbumSongColumns[kPrintColumns] = {
    { 40, 90, true }, { 110, 790, false }, { 0, 0, false }, { 895, 1000, true }
};
static const Column kArtistColumns[kPrintColumns] = {
    { 0, 640, false }, { 0, 0, false }, { 650, 820, true }, { 830, 1000, true }
};
static const Column kArtistAlbumColumns[kPrintColumns] = {
    { 40, 640, false }, { 0, 0, false }, { 0, 0, false }, { 830, 1000, true }
};

static const Column *ColumnsFor(short type)
{
    switch (gList)
    {
        case kPrintAlbums:
            return type == kLineSub ? kAlbumSongColumns : kAlbumColumns;
        case kPrintArtists:
            return type == kLineSub ? kArtistAlbumColumns : kArtistColumns;
    }
    return kSongColumns;
}

/* ---- Building the list ------------------------------------------------- */

static Boolean AddLine(short type, short item, short group)
{
    SetHandleSize(gLinesH, (long)(gLineCount + 1) * sizeof(LineRec));
    if (MemError() != noErr)
        return false;
    Lines()[gLineCount].type = type;
    Lines()[gLineCount].item = item;
    Lines()[gLineCount].group = group;
    gLineCount++;
    return true;
}

static Boolean BuildSongs(void)
{
    LibFilter all;
    short n, i, books;

    LibFilter_Clear(&all);
    n = Library_ListSongs(&all, gItems, kMaxSongs);
    for (i = 0; i < n; i++)
        if (!AddLine(kLineItem, gItems[i], kNone))
            return false;
    /* PROVISIONAL: audiobooks after the songs, under their own heading,
       as the Songs screen leaves them out (D18). */
    books = Library_ListAudiobooks(gItems, kMaxSongs);
    if (books > 0 && !AddLine(kLineSection, kNone, kNone))
        return false;
    for (i = 0; i < books; i++)
        if (!AddLine(kLineItem, gItems[i], kNone))
            return false;
    return true;
}

static Boolean BuildGroups(const PrintOptions *options)
{
    LibFilter all, f;
    short n, i, j, subs;
    Boolean albums = gList == kPrintAlbums;

    LibFilter_Clear(&all);
    n = albums ? Library_ListAlbums(&all, gItems, kMaxGroups)
               : Library_ListArtists(&all, gItems, kMaxGroups);
    for (i = 0; i < n; i++)
    {
        if (!AddLine(kLineItem, gItems[i], kNone))
            return false;
        if (albums ? !options->albumSongs : !options->artistAlbums)
            continue;
        LibFilter_Clear(&f);
        if (albums)
        {
            f.album = gItems[i];
            subs = Library_ListSongs(&f, gSubs, kMaxSongs);
        }
        else
        {
            f.artist = gItems[i];
            subs = Library_ListAlbums(&f, gSubs, kMaxGroups);
        }
        for (j = 0; j < subs; j++)
            if (!AddLine(kLineSub, gSubs[j], gItems[i]))
                return false;
    }
    return true;
}

/* Divides the lines into pages. An item with lines under it isn't left
   alone at the foot of a page: it starts the next. */
static void Paginate(void)
{
    short i = 0, onPage;

    gPageCount = 0;
    do
    {
        gPageStart[gPageCount++] = i;
        for (onPage = 0; i < gLineCount && onPage < gPerPage; i++, onPage++)
            if (onPage == gPerPage - 1 && onPage > 0 && i + 1 < gLineCount
                && (Lines()[i].type == kLineSection
                    || (Lines()[i].type == kLineItem && Lines()[i + 1].type == kLineSub)))
                break;
    } while (i < gLineCount);
    gPageStart[gPageCount] = gLineCount;
}

Boolean PrintLayout_Begin(short list, const PrintOptions *options,
                          const PrintPage *page)
{
    Boolean ok;

    PrintLayout_End();
    gList = list;
    gPage = *page;
    gPerPage = page->lineHeight > 0
             ? page->height / page->lineHeight - kPrintHeaderLines - kPrintFooterLines - 1
             : 0;
    if (gPerPage < 1)
        return false;
    gLinesH = NewHandle(0);
    if (gLinesH == nil)
        return false;
    ok = list == kPrintSongs ? BuildSongs() : BuildGroups(options);
    if (!ok)
    {
        PrintLayout_End();
        return false;
    }
    Paginate();
    return true;
}

void PrintLayout_End(void)
{
    if (gLinesH != nil)
        DisposeHandle(gLinesH);
    gLinesH = nil;
    gLineCount = 0;
    gPageCount = 0;
}

short PrintLayout_PageCount(void)
{
    return gPageCount;
}

short PrintLayout_LineCount(short page)
{
    if (page < 1 || page > gPageCount)
        return 0;
    return 1 + gPageStart[page] - gPageStart[page - 1];
}

/* ---- One line's cells -------------------------------------------------- */

static long TotalSeconds(const short *songs, short n)
{
    long total = 0;
    short i;

    for (i = 0; i < n; i++)
        total += Library_SongSeconds(songs[i]);
    return total;
}

static void Number(Str255 out, long n)
{
    NumToString(n, out);
}

static void Cells(const LineRec *rec, Str255 cells[kPrintColumns])
{
    LibFilter f;
    short n, i;

    for (i = 0; i < kPrintColumns; i++)
        cells[i][0] = 0;

    if (rec->type == kLineSection)
    {
        Copy(cells[0], P("Audiobooks"));
        return;
    }
    switch (gList)
    {
        case kPrintSongs:
            Copy(cells[0], Library_SongTitle(rec->item));
            Copy(cells[1], Library_SongArtistName(rec->item));
            Copy(cells[2], Library_SongAlbumName(rec->item));
            PrintLayout_Time(Library_SongSeconds(rec->item), cells[3]);
            break;

        case kPrintAlbums:
            if (rec->type == kLineSub)
            {
                if (Library_SongTrack(rec->item) > 0)
                    Number(cells[0], Library_SongTrack(rec->item));
                Copy(cells[1], Library_SongTitle(rec->item));
                PrintLayout_Time(Library_SongSeconds(rec->item), cells[3]);
                break;
            }
            Copy(cells[0], Library_AlbumName(rec->item));
            LibFilter_Clear(&f);
            f.album = rec->item;
            /* PROVISIONAL: an album of several artists shows "Various
               Artists". */
            n = Library_ListArtists(&f, gSubs, kMaxGroups);
            if (n == 1)
                Copy(cells[1], Library_ArtistName(gSubs[0]));
            else if (n > 1)
                Copy(cells[1], P("Various Artists"));
            n = Library_ListSongs(&f, gSubs, kMaxSongs);
            Number(cells[2], n);
            PrintLayout_Time(TotalSeconds(gSubs, n), cells[3]);
            break;

        case kPrintArtists:
            if (rec->type == kLineSub)
            {
                Copy(cells[0], Library_AlbumName(rec->item));
                LibFilter_Clear(&f);
                f.artist = rec->group;
                f.album = rec->item;
                Number(cells[3], Library_ListSongs(&f, gSubs, kMaxSongs));
                break;
            }
            Copy(cells[0], Library_ArtistName(rec->item));
            LibFilter_Clear(&f);
            f.artist = rec->item;
            Number(cells[2], Library_ListAlbums(&f, gSubs, kMaxGroups));
            Number(cells[3], Library_ListSongs(&f, gSubs, kMaxSongs));
            break;
    }
}

static void Heads(Str255 cells[kPrintColumns])
{
    static const unsigned char kHeads[kPrintListCount][kPrintColumns][9] = {
        { "\pTitle", "\pArtist", "\pAlbum", "\pTime" },
        { "\pAlbum", "\pArtist", "\pSongs", "\pTime" },
        { "\pArtist", "\p", "\pAlbums", "\pSongs" },
    };
    short i;

    for (i = 0; i < kPrintColumns; i++)
        Copy(cells[i], kHeads[gList][i]);
}

void PrintLayout_Line(short page, short index, PrintLine *line)
{
    const Column *columns;
    LineRec rec;
    short i;
    Boolean bold;

    memset(line, 0, sizeof *line);
    if (page < 1 || page > gPageCount || index < 0 || index >= PrintLayout_LineCount(page))
        return;
    if (index == 0)
    {
        rec.type = kLineColumnHeads;
        Heads(line->cells);
    }
    else
    {
        rec = Lines()[gPageStart[page - 1] + index - 1];
        Cells(&rec, line->cells);
    }
    line->type = rec.type;
    columns = ColumnsFor(rec.type);
    bold = rec.type == kLineColumnHeads || rec.type == kLineSection;
    for (i = 0; i < kPrintColumns; i++)
    {
        line->left[i] = (short)((long)gPage.width * columns[i].left / 1000);
        line->right[i] = (short)((long)gPage.width * columns[i].right / 1000);
        line->alignRight[i] = columns[i].alignRight;
        if (rec.type == kLineSection && i == 0)
            line->right[0] = gPage.width;
        if (line->right[i] <= line->left[i])
            line->cells[i][0] = 0;
        else
            Fit(line->cells[i], line->right[i] - line->left[i], bold);
    }
}
