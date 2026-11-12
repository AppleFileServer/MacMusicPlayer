#include <Dialogs.h>
#include <Events.h>
#include <Fonts.h>
#include <Memory.h>
#include <OSUtils.h>
#include <Quickdraw.h>
#include <Resources.h>
#include <TextUtils.h>
#include <string.h>

#include "print.h"
#include "printlayout.h"
#include "library.h"
#include "libsetup.h"
#include "message.h"
#include "modal.h"
#include "prefs.h"
#include "pstr.h"
#include "song_format.h"

/* The Printing Manager, as Inside Macintosh, Volume II ("The Printing
   Manager") describes it for System 6, and nothing else: PrOpen; the
   print record checked with PrValidate; PrStlDialog and PrJobDialog
   unchanged; PrOpenDoc, then PrOpenPage and PrClosePage round each page,
   drawn with ordinary QuickDraw in the record's page rectangle (rPage);
   PrCloseDoc; PrPicFile if the job was spooled; PrError after each call;
   PrClose at the end. No printer-specific code, no PostScript, nothing
   of the print record's private fields. */

enum
{
    kListsDialog = 150,
    kItemPrint = 1,
    kItemCancel = 2,
    kItemSongs = 4,
    kItemAlbums = 5,
    kItemAlbumSongs = 6,
    kItemArtists = 7,
    kItemArtistAlbums = 8,
    kItemOutline = 9
};

/* N11-16: the Page Setup is kept in the prefs file's resource fork, as a
   resource of MacMusic's own (1.0 reads only the data fork). */
#define kPrintRecordType    'iPdP'
#define kPrintRecordID      128

/* PROVISIONAL (N11-13): the lists in Times 10, the heads and header in
   Helvetica bold 10; Geneva if a font isn't there. */
#define kListFont           kFontIDTimes
#define kHeadFont           kFontIDHelvetica
#define kTextSize           10

static THPrint gPrint;                  /* the session's print record */
static Boolean gChosen[kPrintListCount] = { true, false, false };
static PrintOptions gOptions = { false, false };

/* ---- The print record ------------------------------------------------- */

/* The saved Page Setup, if there is one (into gPrint). */
static Boolean LoadRecord(void)
{
    Str63 name;
    short vRefNum = Prefs_FileForResources(name), ref, saved = CurResFile();
    Handle h;
    Boolean found = false;

    if (vRefNum == 0)
        return false;
    SetResLoad(true);
    ref = OpenRFPerm(name, vRefNum, fsRdPerm);
    if (ref == -1)
        return false;
    h = Get1Resource(kPrintRecordType, kPrintRecordID);
    if (h != nil && GetHandleSize(h) == sizeof(TPrint))
    {
        BlockMove(*h, *gPrint, sizeof(TPrint));
        found = true;
    }
    CloseResFile(ref);
    UseResFile(saved);
    return found;
}

static void SaveRecord(void)
{
    Str63 name;
    short vRefNum = Prefs_FileForResources(name), ref, saved = CurResFile();
    short oldVol;
    Str63 volName;
    Handle h;

    if (vRefNum == 0)
        return;
    ref = OpenRFPerm(name, vRefNum, fsRdWrPerm);
    if (ref == -1)
    {
        /* No resource fork yet (or no prefs file): CreateResFile works
           on the default volume, so it's set to the System Folder for the
           call. */
        GetVol(volName, &oldVol);
        SetVol(nil, vRefNum);
        CreateResFile(name);
        SetVol(nil, oldVol);
        ref = OpenRFPerm(name, vRefNum, fsRdWrPerm);
        if (ref == -1)
            return;                     /* a locked disk: kept for the session */
    }
    h = Get1Resource(kPrintRecordType, kPrintRecordID);
    if (h != nil)
    {
        SetHandleSize(h, sizeof(TPrint));
        if (MemError() == noErr)
        {
            BlockMove(*gPrint, *h, sizeof(TPrint));
            ChangedResource(h);
        }
    }
    else if ((h = NewHandle(sizeof(TPrint))) != nil)
    {
        BlockMove(*gPrint, *h, sizeof(TPrint));
        AddResource(h, kPrintRecordType, kPrintRecordID, P(""));
    }
    UpdateResFile(ref);
    CloseResFile(ref);
    UseResFile(saved);
}

/* The record, made and filled the first time; true if it came from a
   saved Page Setup (or one chosen this session). */
static Boolean gHaveSetup;

static Boolean PrepareRecord(void)
{
    if (gPrint == nil)
    {
        gPrint = (THPrint)NewHandleClear(sizeof(TPrint));
        if (gPrint == nil)
            return false;
        gHaveSetup = LoadRecord();
        if (!gHaveSetup)
            PrintDefault(gPrint);
    }
    PrValidate(gPrint);                 /* for the printer chosen now */
    return PrError() == noErr;
}

/* ---- Messages (N11-19) -------------------------------------------------- */

/* 1.2 (N12-12): while MacMusic prepares the pages (with the ImageWriter,
   it spools every page before the printer's own status box appears), a
   box says how to stop, and Command-period stops it, as Cancel does. */
static WindowPtr ShowPreparing(void)
{
    Rect r;
    WindowPtr w;
    GrafPtr saved;
    short left = (qd.screenBits.bounds.right - 340) / 2;
    short top = (qd.screenBits.bounds.bottom - 70) / 2;

    SetRect(&r, left, top, left + 340, top + 70);
    w = NewWindow(nil, &r, P(""), true, dBoxProc, (WindowPtr)-1, false, 0);
    if (w == nil)
        return nil;
    GetPort(&saved);
    SetPort(w);
    TextFont(systemFont);
    TextSize(12);
    MoveTo(20, 28);
    DrawString(P("Preparing the pages to print\xC9"));
    MoveTo(20, 50);
    DrawString(P("To cancel, press Command-period."));
    SetPort(saved);
    return w;
}

static Boolean CancelPressed(void)
{
    EventRecord event;
    Boolean cancel = false;

    while (GetOSEvent(keyDownMask | autoKeyMask, &event))
        if ((event.modifiers & cmdKey) && (event.message & charCodeMask) == '.')
            cancel = true;
    return cancel;
}

static void Problem(OSErr err)
{
    if (err == iPrAbort || err == noErr)
        return;                         /* cancelled: nothing more to say */
    if (err == memFullErr)
        Message_Show(P("There isn\xD5t enough memory to print. Quit other "
                       "programs, or give MacMusic more memory in Get Info."), noErr);
    else if (err == fnfErr || err == resNotFound)
        Message_Show(P("No printer is chosen. Choose one in the Chooser, "
                       "from the Apple menu."), noErr);
    else
        Message_Show(P("The library couldn\xD5t be printed. Check that the "
                       "printer is switched on, and chosen in the Chooser."), err);
}

/* ---- Page Setup -------------------------------------------------------- */

void Print_PageSetup(void)
{
    OSErr err;

    PrOpen();
    err = PrError();
    if (err == noErr && !PrepareRecord())
        err = PrError() != noErr ? PrError() : memFullErr;
    if (err == noErr && PrStlDialog(gPrint))
    {
        gHaveSetup = true;
        SaveRecord();
    }
    if (err == noErr)
        err = PrError();
    PrClose();
    Problem(err);
}

/* ---- Which lists (N11-11) ---------------------------------------------- */

static void SetBox(DialogPtr d, short item, Boolean on)
{
    SetControlValue((ControlHandle)Modal_ItemHandle(d, item), on ? 1 : 0);
}

static Boolean Box(DialogPtr d, short item)
{
    return GetControlValue((ControlHandle)Modal_ItemHandle(d, item)) != 0;
}

static void Enable(DialogPtr d, short item, Boolean on)
{
    HiliteControl((ControlHandle)Modal_ItemHandle(d, item), on ? 0 : 255);
}

static void UpdateBoxes(DialogPtr d)
{
    Enable(d, kItemAlbumSongs, Box(d, kItemAlbums));
    Enable(d, kItemArtistAlbums, Box(d, kItemArtists));
    Enable(d, kItemPrint, Box(d, kItemSongs) || Box(d, kItemAlbums) || Box(d, kItemArtists));
}

static Boolean ChooseLists(void)
{
    DialogPtr d;
    short item, type;
    Handle handle;
    Rect r;
    Boolean ok = false;

    d = GetNewDialog(kListsDialog, nil, (WindowPtr)-1);
    if (d == nil)
    {
        Message_Show(P("The Print Library window could not be opened."), noErr);
        return false;
    }
    SetBox(d, kItemSongs, gChosen[kPrintSongs]);
    SetBox(d, kItemAlbums, gChosen[kPrintAlbums]);
    SetBox(d, kItemAlbumSongs, gOptions.albumSongs);
    SetBox(d, kItemArtists, gChosen[kPrintArtists]);
    SetBox(d, kItemArtistAlbums, gOptions.artistAlbums);
    GetDialogItem(d, kItemOutline, &type, &handle, &r);
    SetDialogItem(d, kItemOutline, type, (Handle)Modal_DrawDefaultOutline, &r);
    UpdateBoxes(d);
    ShowWindow(d);

    for (;;)
    {
        ModalDialog(Modal_DialogFilter, &item);
        if (item == kItemPrint)
        {
            /* Return chooses Print even when it's dimmed: ignored then. */
            if (Box(d, kItemSongs) || Box(d, kItemAlbums) || Box(d, kItemArtists))
            {
                ok = true;
                break;
            }
        }
        else if (item == kItemCancel)
            break;
        else if (item >= kItemSongs && item <= kItemArtistAlbums)
        {
            SetBox(d, item, !Box(d, item));
            UpdateBoxes(d);
        }
    }
    if (ok)
    {
        gChosen[kPrintSongs] = Box(d, kItemSongs);
        gChosen[kPrintAlbums] = Box(d, kItemAlbums);
        gChosen[kPrintArtists] = Box(d, kItemArtists);
        gOptions.albumSongs = Box(d, kItemAlbumSongs);
        gOptions.artistAlbums = Box(d, kItemArtistAlbums);
    }
    DisposeDialog(d);
    return ok;
}

/* ---- Drawing the pages -------------------------------------------------- */

static short gListFont, gHeadFont;

static void UseFont(Boolean heading)
{
    TextFont(heading ? gHeadFont : gListFont);
    TextSize(kTextSize);
    TextFace(heading ? bold : normal);
}

static short TextWidthOf(ConstStringPtr text, Boolean heading)
{
    short w;

    UseFont(heading);
    w = StringWidth(text);
    UseFont(false);
    return w;
}

/* A font if the System has it, else Geneva. */
static short FontOr(short font)
{
    Str255 name;

    GetFontName(font, name);
    return name[0] > 0 ? font : kFontIDGeneva;
}

static void DrawCell(const Rect *page, short baseline, const PrintLine *line, short i)
{
    short x;

    if (line->cells[i][0] == 0)
        return;
    x = line->alignRight[i] ? page->left + line->right[i] - StringWidth(line->cells[i])
                            : page->left + line->left[i];
    MoveTo(x, baseline);
    DrawString(line->cells[i]);
}

/* One page: the header, the column heads and their rule, the lines, the
   page number. */
static void DrawPage(const Rect *page, short lineHeight, short ascent,
                     short list, short number, ConstStringPtr library,
                     ConstStringPtr date)
{
    PrintLine line;
    Str255 text;
    short i, n = PrintLayout_LineCount(number), y, c;

    /* Header: the library, the list's name, the date, a rule. */
    UseFont(true);
    y = page->top + ascent;
    MoveTo(page->left, y);
    DrawString(library);
    MoveTo(page->left + (page->right - page->left - StringWidth(PrintLayout_ListName(list))) / 2, y);
    DrawString(PrintLayout_ListName(list));
    MoveTo(page->right - StringWidth(date), y);
    DrawString(date);
    MoveTo(page->left, page->top + lineHeight + lineHeight / 2);
    LineTo(page->right - 1, page->top + lineHeight + lineHeight / 2);

    for (i = 0; i < n; i++)
    {
        PrintLayout_Line(number, i, &line);
        UseFont(line.type == kLineColumnHeads || line.type == kLineSection);
        y = page->top + (kPrintHeaderLines + i) * lineHeight + ascent;
        for (c = 0; c < kPrintColumns; c++)
            DrawCell(page, y, &line, c);
        if (line.type == kLineColumnHeads)
        {
            MoveTo(page->left, y + lineHeight - ascent - 1);
            LineTo(page->right - 1, y + lineHeight - ascent - 1);
        }
    }

    /* Footer: "Page 3 of 12", centred on the last line. */
    UseFont(false);
    PrintLayout_PageText(number, text);
    MoveTo(page->left + (page->right - page->left - StringWidth(text)) / 2,
           page->bottom - lineHeight + ascent);
    DrawString(text);
}

/* ---- Print Library ------------------------------------------------------- */

void Print_Library(void)
{
    TPPrPort port;
    TPrStatus status;
    GrafPtr savedPort;
    PrintPage layout;
    FontInfo info;
    Rect page;
    Str255 date;
    Str31 library;
    unsigned long now;
    short list, p, lineHeight;
    OSErr err;
    Boolean go;
    WindowPtr preparing;

    if (!ChooseLists())
        return;

    PrOpen();
    err = PrError();
    if (err == noErr && !PrepareRecord())
        err = PrError() != noErr ? PrError() : memFullErr;

    /* N11-16: the first print, with no Page Setup saved, asks for one
       first (the paper: A4 by default). */
    go = err == noErr;
    if (go && !gHaveSetup)
    {
        go = PrStlDialog(gPrint);
        if (go)
        {
            gHaveSetup = true;
            SaveRecord();
        }
    }
    if (go)
        go = PrJobDialog(gPrint);
    if (!go)
    {
        err = PrError();
        PrClose();
        Problem(err);
        return;
    }

    SetCursor(*GetCursor(watchCursor));
    GetPort(&savedPort);
    LibSetup_LibraryName(library);
    GetDateTime(&now);
    IUDateString((long)now, abbrevDate, date);

    port = PrOpenDoc(gPrint, nil, nil);
    err = PrError();
    preparing = err == noErr ? ShowPreparing() : nil;
    if (err == noErr)
    {
        SetPort(&port->gPort);
        gListFont = FontOr(kListFont);
        gHeadFont = FontOr(kHeadFont);
        UseFont(false);
        GetFontInfo(&info);
        lineHeight = info.ascent + info.descent + info.leading + 1;
        page = (**gPrint).prInfo.rPage;

        layout.width = page.right - page.left;
        layout.height = page.bottom - page.top;
        layout.lineHeight = lineHeight;
        layout.textWidth = TextWidthOf;

        /* Each list starts a page of its own. */
        for (list = 0; list < kPrintListCount && err == noErr; list++)
        {
            if (!gChosen[list])
                continue;
            if (CancelPressed())
            {
                err = iPrAbort;
                break;
            }
            if (!PrintLayout_Begin(list, &gOptions, &layout))
            {
                err = memFullErr;
                break;
            }
            for (p = 1; p <= PrintLayout_PageCount() && err == noErr; p++)
            {
                if (CancelPressed())
                {
                    err = iPrAbort;
                    break;
                }
                PrOpenPage(port, nil);
                err = PrError();
                if (err == noErr)
                    DrawPage(&page, lineHeight, info.ascent, list, p, library, date);
                PrClosePage(port);
                if (err == noErr)
                    err = PrError();
            }
            PrintLayout_End();
        }
    }
    PrCloseDoc(port);
    if (err == noErr)
        err = PrError();
    if (preparing != nil)
        DisposeWindow(preparing);       /* the printer's own box follows */

    /* A spooled job (the ImageWriter's, without background printing) is
       printed now, from its spool file. */
    if (err == noErr && (**gPrint).prJob.bJDocLoop == bSpoolLoop)
    {
        PrPicFile(gPrint, nil, nil, nil, &status);
        err = PrError();
    }
    PrClose();
    SetPort(savedPort);
    InitCursor();
    Problem(err);
}
