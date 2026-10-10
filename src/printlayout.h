#ifndef PRINTLAYOUT_H
#define PRINTLAYOUT_H

#include <MacTypes.h>

/* 1.1 (docs/plans/release-1.1.md, N11-11 to N11-14): what a printed list
   of the library holds, line by line and page by page, apart from the
   Printing Manager and QuickDraw (print.c draws it), so the host tests
   can check it. Measurements are in the printing port's units; the text
   widths come from the caller. */

enum { kPrintSongs = 0, kPrintAlbums, kPrintArtists, kPrintListCount };

typedef struct
{
    Boolean albumSongs;         /* Albums: each album's songs below it */
    Boolean artistAlbums;       /* Artists: each artist's albums below it */
} PrintOptions;

/* The width of text in the list's font (bold for the column heads). */
typedef short (*PrintTextWidth)(ConstStringPtr text, Boolean bold);

typedef struct
{
    short width;                /* the printable area (rPage) */
    short height;
    short lineHeight;           /* one line of the list's text */
    PrintTextWidth textWidth;
} PrintPage;

/* Each page: a header (the library, the list's name, the date: drawn by
   the caller in the first kPrintHeaderLines), the column heads with a
   rule under them, the list's lines, and "Page n of m" in the last
   kPrintFooterLines. */
#define kPrintHeaderLines   3
#define kPrintFooterLines   2
#define kPrintColumns       4

enum
{
    kLineColumnHeads,           /* bold, a rule below */
    kLineItem,                  /* a song, an album or an artist */
    kLineSub,                   /* an album's song, an artist's album: indented */
    kLineSection                /* "Audiobooks": bold */
};

typedef struct
{
    short type;
    Str255 cells[kPrintColumns];        /* already cut to fit (an ellipsis) */
    short left[kPrintColumns];          /* where each cell's column starts */
    short right[kPrintColumns];         /* and ends */
    Boolean alignRight[kPrintColumns];
} PrintLine;

/* Lays out a list of the scanned library for this page; false if memory
   ran out, or the page is too small for a single line. */
Boolean PrintLayout_Begin(short list, const PrintOptions *options,
                          const PrintPage *page);
void PrintLayout_End(void);

short PrintLayout_PageCount(void);
/* The lines on a page (1-based), the column heads first. */
short PrintLayout_LineCount(short page);
void PrintLayout_Line(short page, short index, PrintLine *line);

/* The list's name, for the header: "Songs", "Albums", "Artists". */
ConstStringPtr PrintLayout_ListName(short list);

/* "Page 3 of 12". */
void PrintLayout_PageText(short page, Str255 out);

/* A length of time, m:ss or h:mm:ss. */
void PrintLayout_Time(long seconds, Str255 out);

#endif
