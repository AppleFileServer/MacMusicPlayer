#include <Dialogs.h>
#include <Memory.h>
#include <TextUtils.h>

#include "libsetup.h"
#include "pstr.h"
#include "libfolder.h"
#include "library.h"
#include "prefs.h"

enum
{
    kLibraryAlert = 129,        /* Choose... / Not Now */
    kItemChoose = 1,
    kMessageAlert = 130         /* OK */
};

static LibraryFolder gFolder;

/* Appends Pascal string s to dest (Str255), truncating at 255. */
static void Append(Str255 dest, ConstStringPtr s)
{
    short n = s[0];

    if (dest[0] + n > 255)
        n = 255 - dest[0];
    BlockMove(s + 1, dest + dest[0] + 1, n);
    dest[0] += n;
}

static void ShowMessage(ConstStringPtr text, OSErr err)
{
    Str255 detail;
    Str255 number;

    detail[0] = 0;
    if (err != noErr)
    {
        NumToString(err, number);
        Append(detail, P(" (error "));
        Append(detail, number);
        Append(detail, P(")"));
    }

    ParamText(text, detail, P(""), P(""));
    Alert(kMessageAlert, nil);
}

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
    Append(text, number);
    Append(text, skipped == 1 ? P(" file in the iPod Library was")
                              : P(" files in the iPod Library were"));
    Append(text, P(" not recognised and "));
    Append(text, skipped == 1 ? P("was skipped.") : P("were skipped."));
    ShowMessage(text, noErr);
}

static void Scan(void)
{
    if (!Library_Scan(&gFolder))
        ShowMessage(P("There is not enough memory to read the iPod Library."), noErr);
    else
        ReportSkipped();
}

/* Asks for a folder until one is chosen or the user cancels; remembers it. */
static Boolean ChooseAndRemember(void)
{
    LibraryLocation location;
    short result;
    OSErr err;

    for (;;)
    {
        result = Folder_Choose(&location, &gFolder, &err);

        if (result == kFolderChosen)
            break;
        if (result == kFolderCancelled)
            return false;
        if (result == kFolderNameIsFile)
        {
            ShowMessage(P("A file with that name is already there. "
                        "Choose another name or place."), noErr);
            continue;
        }

        ShowMessage(P("The iPod Library folder could not be created."), err);
        return false;
    }

    /* The library is still used this session if saving fails. */
    err = Prefs_Save(&location);
    if (err != noErr)
        ShowMessage(P("The library location could not be remembered."), err);
    return true;
}

void LibSetup_Startup(void)
{
    LibraryLocation location;
    Boolean remembered = Prefs_Load(&location);
    Boolean haveLibrary = remembered && Folder_Resolve(&location, &gFolder);

    if (!haveLibrary)
    {
        if (remembered)
            ParamText(P("The iPod Library folder could not be found. Its disk "
                      "may not be inserted. Choose where it is, or create a "
                      "new one."), P(""), P(""), P(""));
        else
            ParamText(P("MacMusicPlayer keeps its songs in a folder called "
                      "\xD2iPod Library.\xD3 Choose where to create it, or "
                      "select an existing one."), P(""), P(""), P(""));

        /* DECISION (D22): Not Now continues with an empty library;
           File > Choose Library... is always available. */
        if (Alert(kLibraryAlert, nil) == kItemChoose)
            haveLibrary = ChooseAndRemember();
    }

    if (haveLibrary)
        Scan();
    else
        Library_Clear();
}

Boolean LibSetup_ChooseLibrary(void)
{
    if (!ChooseAndRemember())
        return false;

    Scan();
    return true;
}
