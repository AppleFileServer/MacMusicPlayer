#include <Dialogs.h>
#include <Memory.h>
#include <TextUtils.h>

#include "modal.h"
#include "libsetup.h"
#include "pstr.h"
#include "message.h"
#include "library.h"
#include "prefs.h"
#include "fileops.h"

/* DECISION L1 (5b-3): finding an existing library and creating a new
   one are separate choices, at startup (ALRT 129, with Not Now) and in
   File > Choose Library... (ALRT 139, with Cancel). */
enum
{
    kLibraryAlert = 129,        /* Find Existing... / Create New... / Not Now */
    kChooseAlert = 139,         /* Use Existing... / Create New... / Cancel */
    kItemFindExisting = 1,
    kItemCreateNew = 2
};

static LibraryFolder gFolder;
static Boolean gHaveLibrary;
static LibraryLocation gLocation;       /* where gFolder is, if any */

/* Stage 8d-3: the libraries used before, most recent first. */
static RecentLibrary gRecent[kMaxRecent];
static short gRecentCount;

/* The current library goes to the top of the recent list (once only),
   with its folder's name as it is now, and the list is saved. */
static void RememberRecent(const LibraryLocation *location)
{
    RecentLibrary entry;
    short i;

    entry.location = *location;
    if (!Folder_Find(location, entry.folderName))
        entry.folderName[0] = 0;

    for (i = 0; i < gRecentCount; i++)
        if (Folder_SameLocation(&gRecent[i].location, location))
            break;
    if (i == 0 && gRecentCount > 0
        && EqualString(gRecent[0].folderName, entry.folderName, true, true))
        return;                         /* already first: nothing to save */
    if (i == gRecentCount && gRecentCount < kMaxRecent)
        gRecentCount++;
    if (i >= gRecentCount)
        i = gRecentCount - 1;           /* full: the oldest goes */
    for (; i > 0; i--)
        gRecent[i] = gRecent[i - 1];
    gRecent[0] = entry;
    Prefs_SaveRecent(gRecent, gRecentCount);
}

/* DECISION (D23): during development, one alert reports skipped files. */
static void ReportSkipped(void)
{
    short skipped = Library_SkippedCount();
    short overLimit = Library_OverLimitCount();
    short playlistsOver = Library_PlaylistsOverLimitCount();
    Str255 text;
    Str255 number;

    /* Stage 8e-2: playlists left out because there are already 10. */
    if (playlistsOver > 0)
    {
        NumToString(playlistsOver, number);
        text[0] = 0;
        PStrAppend(text, number);
        PStrAppend(text, playlistsOver == 1 ? P(" playlist wasn\xD5t")
                                            : P(" playlists weren\xD5t"));
        PStrAppend(text, P(" loaded: MacMusic can have up to 10 playlists."));
        Message_Show(text, noErr);
    }

    /* Stage 8d (M3): songs left out because the library is full. */
    if (overLimit > 0)
    {
        NumToString(overLimit, number);
        text[0] = 0;
        PStrAppend(text, number);
        PStrAppend(text, overLimit == 1 ? P(" song wasn\xD5t") : P(" songs weren\xD5t"));
        PStrAppend(text, P(" loaded: the MacMusic Library holds at most 500 "
                           "songs. Export songs to make room."));
        Message_Show(text, noErr);
    }

    if (skipped == 0)
        return;

    NumToString(skipped, number);
    text[0] = 0;
    PStrAppend(text, number);
    PStrAppend(text, skipped == 1 ? P(" file in the MacMusic Library was")
                                  : P(" files in the MacMusic Library were"));
    PStrAppend(text, P(" not recognised and "));
    PStrAppend(text, skipped == 1 ? P("was skipped.") : P("were skipped."));
    Message_Show(text, noErr);
}

/* ---- Library Info (Stage 8d-2) ---------------------------------------- */

#define kInfoAlert          141

void LibSetup_AppendMB(Str255 text, long bytes)
{
    Str255 number;
    long tenths = (bytes / 1024 * 10 + 512) / 1024;

    NumToString(tenths / 10, number);
    PStrAppend(text, number);
    PStrAppend(text, P("."));
    NumToString(tenths % 10, number);
    PStrAppend(text, number);
    PStrAppend(text, P(" MB"));
}

static void AppendNumber(Str255 text, long n)
{
    Str255 number;

    NumToString(n, number);
    PStrAppend(text, number);
}

void LibSetup_ShowInfo(void)
{
    Str255 text;
    short overLimit = Library_OverLimitCount();

    if (!gHaveLibrary)
        return;

    /* Songs counts audiobooks too: every song file in the library. */
    text[0] = 0;
    PStrAppend(text, P("Songs: "));
    AppendNumber(text, Library_SongCount());
    PStrAppend(text, P(" of 500\rPlaylists: "));
    AppendNumber(text, Library_PlaylistCount());
    PStrAppend(text, P("\rSize: "));
    LibSetup_AppendMB(text, Library_TotalAudioBytes());
    PStrAppend(text, P("\rFree: "));
    LibSetup_AppendMB(text, FreeBytes(gFolder.vRefNum));
    PStrAppend(text, P("\rUnreadable Files: "));
    AppendNumber(text, Library_SkippedCount());
    if (overLimit > 0)
    {
        PStrAppend(text, P("\rSongs Over the Limit: "));
        AppendNumber(text, overLimit);
    }
    if (Library_PlaylistsOverLimitCount() > 0)
    {
        PStrAppend(text, P("\rPlaylists Over the Limit: "));
        AppendNumber(text, Library_PlaylistsOverLimitCount());
    }

    ParamText(text, P(""), P(""), P(""));
    Alert(kInfoAlert, Modal_DialogFilter);
}

void LibSetup_Rescan(void)
{
    if (!gHaveLibrary)
    {
        Library_Clear();
        return;
    }

    if (!Library_Scan(&gFolder))
        Message_Show(P("There is not enough memory to read the MacMusic Library."), noErr);
    else
        ReportSkipped();
}

/* Asks for a library (an existing one if findExisting, else where to
   create one) until one is chosen or the user cancels; remembers it. */
static Boolean ChooseAndRemember(Boolean findExisting)
{
    LibraryLocation location;
    LibraryFolder folder;
    short result;
    OSErr err;

    for (;;)
    {
        result = findExisting ? Folder_FindExisting(&location, &folder, &err)
                              : Folder_Choose(&location, &folder, &err);

        if (result == kFolderChosen)
            break;
        if (result == kFolderCancelled)
            return false;
        if (result == kFolderNameIsFile)
        {
            Message_Show(P("A file with that name is already there. "
                           "Choose another name or place."), noErr);
            continue;
        }
        if (result == kFolderNotLibrary)
        {
            Message_Show(P("That file isn\xD5t inside a MacMusic Library. Open "
                           "the \xD2MacMusic Library Info\xD3 file, or a song, "
                           "in the library\xD5s folder."), noErr);
            continue;
        }

        Message_Show(findExisting
                     ? P("The MacMusic Library could not be opened.")
                     : P("The MacMusic Library folder could not be created."), err);
        return false;
    }

    gFolder = folder;
    gHaveLibrary = true;
    gLocation = location;

    /* The library is still used this session if saving fails. */
    err = Prefs_Save(&location);
    if (err != noErr)
        Message_Show(P("The library location could not be remembered."), err);
    RememberRecent(&location);
    return true;
}

void LibSetup_Startup(void)
{
    LibraryLocation location;
    Boolean remembered = Prefs_Load(&location);

    gRecentCount = Prefs_LoadRecent(gRecent);
    gHaveLibrary = remembered && Folder_Resolve(&location, &gFolder);
    if (gHaveLibrary)
    {
        gLocation = location;
        RememberRecent(&location);  /* a library from before 8d-3 joins */
    }

    if (!gHaveLibrary)
    {
        if (remembered)
            ParamText(P("The MacMusic Library folder could not be found. Its disk "
                        "may not be inserted, or it may have moved. Find it, "
                        "or create a new one."), P(""), P(""), P(""));
        else
            ParamText(P("MacMusic keeps its songs in a folder called "
                        "\xD2MacMusic Library.\xD3 Find an existing one, or "
                        "create a new one."), P(""), P(""), P(""));

        /* DECISION (D22): Not Now continues with an empty library;
           File > Choose Library... is always available. */
        switch (Alert(kLibraryAlert, Modal_DialogFilter))
        {
            case kItemFindExisting:
                ChooseAndRemember(true);
                break;
            case kItemCreateNew:
                ChooseAndRemember(false);
                break;
        }
    }

    LibSetup_Rescan();
}

Boolean LibSetup_ChooseLibrary(void)
{
    switch (Alert(kChooseAlert, Modal_DialogFilter))
    {
        case kItemFindExisting:
            return ChooseAndRemember(true);
        case kItemCreateNew:
            return ChooseAndRemember(false);
    }
    return false;
}

short LibSetup_RecentCount(void)
{
    return gRecentCount;
}

void LibSetup_RecentItem(short i, Str255 text, Boolean *available,
                         Boolean *current)
{
    RecentLibrary *r = &gRecent[i];
    Str31 name;

    /* The folder may have been renamed in the Finder: shown as it is. */
    *available = Folder_Find(&r->location, name);
    if (*available)
        BlockMove(name, r->folderName, name[0] + 1);
    *current = gHaveLibrary && Folder_SameLocation(&r->location, &gLocation);

    text[0] = 0;
    PStrAppend(text, r->folderName[0] > 0 ? r->folderName : P("MacMusic Library"));
    PStrAppend(text, P(" ("));
    PStrAppend(text, r->location.volumeName);
    PStrAppend(text, P(")"));
}

Boolean LibSetup_UseRecent(short i)
{
    LibraryLocation location;
    LibraryFolder folder;
    OSErr err;

    if (i < 0 || i >= gRecentCount)
        return false;
    location = gRecent[i].location;
    if (gHaveLibrary && Folder_SameLocation(&location, &gLocation))
        return false;
    if (!Folder_Resolve(&location, &folder))
    {
        Message_Show(P("That MacMusic Library can\xD5t be found. Its disk may "
                       "not be inserted, or it may have moved."), noErr);
        return false;
    }

    gFolder = folder;
    gHaveLibrary = true;
    gLocation = location;
    err = Prefs_Save(&location);
    if (err != noErr)
        Message_Show(P("The library location could not be remembered."), err);
    RememberRecent(&location);
    return true;
}

Boolean LibSetup_HasLibrary(void)
{
    return gHaveLibrary;
}

const LibraryFolder *LibSetup_Folder(void)
{
    return &gFolder;
}
