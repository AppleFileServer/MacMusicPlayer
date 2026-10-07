#include <Dialogs.h>
#include <Memory.h>
#include <TextUtils.h>

#include "modal.h"
#include "libsetup.h"
#include "pstr.h"
#include "message.h"
#include "library.h"
#include "prefs.h"

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

/* DECISION (D23): during development, one alert reports skipped files. */
static void ReportSkipped(void)
{
    short skipped = Library_SkippedCount();
    Str255 text;
    Str255 number;

    if (skipped == 0)
        return;

    NumToString(skipped, number);
    text[0] = 0;
    PStrAppend(text, number);
    PStrAppend(text, skipped == 1 ? P(" file in the iPod Library was")
                                  : P(" files in the iPod Library were"));
    PStrAppend(text, P(" not recognised and "));
    PStrAppend(text, skipped == 1 ? P("was skipped.") : P("were skipped."));
    Message_Show(text, noErr);
}

void LibSetup_Rescan(void)
{
    if (!gHaveLibrary)
    {
        Library_Clear();
        return;
    }

    if (!Library_Scan(&gFolder))
        Message_Show(P("There is not enough memory to read the iPod Library."), noErr);
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
            Message_Show(P("That file isn\xD5t inside an iPod Library. Open "
                           "the \xD2iPod Library Info\xD3 file, or a song, "
                           "in the library\xD5s folder."), noErr);
            continue;
        }

        Message_Show(findExisting
                     ? P("The iPod Library could not be opened.")
                     : P("The iPod Library folder could not be created."), err);
        return false;
    }

    gFolder = folder;
    gHaveLibrary = true;

    /* The library is still used this session if saving fails. */
    err = Prefs_Save(&location);
    if (err != noErr)
        Message_Show(P("The library location could not be remembered."), err);
    return true;
}

void LibSetup_Startup(void)
{
    LibraryLocation location;
    Boolean remembered = Prefs_Load(&location);

    gHaveLibrary = remembered && Folder_Resolve(&location, &gFolder);

    if (!gHaveLibrary)
    {
        if (remembered)
            ParamText(P("The iPod Library folder could not be found. Its disk "
                        "may not be inserted, or it may have moved. Find it, "
                        "or create a new one."), P(""), P(""), P(""));
        else
            ParamText(P("MacMusic keeps its songs in a folder called "
                        "\xD2iPod Library.\xD3 Find an existing one, or "
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

Boolean LibSetup_HasLibrary(void)
{
    return gHaveLibrary;
}

const LibraryFolder *LibSetup_Folder(void)
{
    return &gFolder;
}
