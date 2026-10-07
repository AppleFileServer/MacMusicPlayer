#include <Files.h>
#include <StandardFile.h>
#include <TextUtils.h>
#include <string.h>

#include "modal.h"
#include "libfolder.h"
#include "fileopen.h"
#include "pstr.h"
#include "song_format.h"

#define kDirectoryAttribute     0x10    /* ioFlAttrib bit 4 */

static const unsigned char kPlaylistsName[] = "\pPlaylists";
static const unsigned char kMarkerName[] = "\piPod Library Info";

/* "Use it?" when Create New is used inside a library (ALRT 140). */
enum
{
    kUseLibraryHereAlert = 140,
    kItemUseIt = 1
};

/* Looks up name inside parentID. Returns noErr and the item's directory
   ID if it is a folder; fnfErr if absent; dupFNErr if it is a file. */
static OSErr LookUpFolder(short vRefNum, long parentID,
                        ConstStringPtr name, long *folderID)
{
    CInfoPBRec pb;
    Str63 nameCopy;
    OSErr err;

    BlockMove(name, nameCopy, name[0] + 1);
    pb.dirInfo.ioNamePtr = nameCopy;
    pb.dirInfo.ioVRefNum = vRefNum;
    pb.dirInfo.ioFDirIndex = 0;
    pb.dirInfo.ioDrDirID = parentID;

    err = PBGetCatInfoSync(&pb);
    if (err != noErr)
        return err;
    if (!(pb.dirInfo.ioFlAttrib & kDirectoryAttribute))
        return dupFNErr;

    *folderID = pb.dirInfo.ioDrDirID;
    return noErr;
}

/* Creates folder name inside parentID, or finds it if it already exists. */
static OSErr MakeFolder(short vRefNum, long parentID,
                        ConstStringPtr name, long *folderID)
{
    HParamBlockRec pb;
    Str63 nameCopy;
    OSErr err;

    BlockMove(name, nameCopy, name[0] + 1);
    pb.fileParam.ioNamePtr = nameCopy;
    pb.fileParam.ioVRefNum = vRefNum;
    pb.fileParam.ioDirID = parentID;

    err = PBDirCreateSync(&pb);
    if (err == noErr)
    {
        *folderID = pb.fileParam.ioDirID;
        return noErr;
    }
    if (err == dupFNErr)
        return LookUpFolder(vRefNum, parentID, name, folderID);
    return err;
}

/* Checks that folderID is still a folder on the volume. */
static Boolean FolderExists(short vRefNum, long folderID)
{
    CInfoPBRec pb;
    Str63 name;

    pb.dirInfo.ioNamePtr = name;
    pb.dirInfo.ioVRefNum = vRefNum;
    pb.dirInfo.ioFDirIndex = -1;            /* look up by directory ID */
    pb.dirInfo.ioDrDirID = folderID;

    return PBGetCatInfoSync(&pb) == noErr
        && (pb.dirInfo.ioFlAttrib & kDirectoryAttribute);
}

/* Finds the mounted volume with this name and creation date. */
static Boolean FindVolume(ConstStringPtr volumeName, unsigned long created,
                          short *vRefNum)
{
    HParamBlockRec pb;
    Str31 name;
    short index;

    for (index = 1; ; index++)
    {
        pb.volumeParam.ioNamePtr = name;
        pb.volumeParam.ioVRefNum = 0;
        pb.volumeParam.ioVolIndex = index;
        if (PBHGetVInfoSync(&pb) != noErr)
            return false;

        if (pb.volumeParam.ioVCrDate == created
            && EqualString(name, volumeName, false, true))
        {
            *vRefNum = pb.volumeParam.ioVRefNum;
            return true;
        }
    }
}

/* ---- The library marker (Stage 5b-3) ---------------------------------- */

/* Looks up a file (not a folder) inside dirID; its Finder type if found. */
static Boolean FindFile(short vRefNum, long dirID, ConstStringPtr name,
                        OSType *type)
{
    CInfoPBRec pb;
    Str63 nameCopy;

    BlockMove(name, nameCopy, name[0] + 1);
    pb.hFileInfo.ioNamePtr = nameCopy;
    pb.hFileInfo.ioVRefNum = vRefNum;
    pb.hFileInfo.ioFDirIndex = 0;
    pb.hFileInfo.ioDirID = dirID;
    if (PBGetCatInfoSync(&pb) != noErr
        || (pb.hFileInfo.ioFlAttrib & kDirectoryAttribute))
        return false;
    *type = pb.hFileInfo.ioFlFndrInfo.fdType;
    return true;
}

/* True if the folder holds a song file. */
static Boolean HasSongFile(short vRefNum, long dirID)
{
    CInfoPBRec pb;
    Str63 name;
    short index;

    for (index = 1; ; index++)
    {
        pb.hFileInfo.ioNamePtr = name;
        pb.hFileInfo.ioVRefNum = vRefNum;
        pb.hFileInfo.ioFDirIndex = index;
        pb.hFileInfo.ioDirID = dirID;
        if (PBGetCatInfoSync(&pb) != noErr)
            return false;
        if (!(pb.hFileInfo.ioFlAttrib & kDirectoryAttribute)
            && pb.hFileInfo.ioFlFndrInfo.fdType == kSongFileType)
            return true;
    }
}

/* A folder is an iPod Library if it holds the marker, or song files and
   a Playlists folder (a library made before 5b-3). */
static Boolean IsLibraryFolder(short vRefNum, long dirID)
{
    OSType type;
    long playlistsID;

    if (FindFile(vRefNum, dirID, kMarkerName, &type)
        && type == kLibraryMarkerType)
        return true;
    return LookUpFolder(vRefNum, dirID, kPlaylistsName, &playlistsID) == noErr
        && HasSongFile(vRefNum, dirID);
}

/* Adds the marker if it is missing. Failure (a locked disk) is harmless:
   the library still works, it just can't be picked by its marker. */
static void EnsureMarker(short vRefNum, long dirID)
{
    static const char text[] =
        "This folder is an iPod Library for MacMusic. To use it, "
        "choose File > Choose Library..., then Use Existing Library, and "
        "open this file.\r";
    HParamBlockRec pb;
    Str63 name;
    long count = sizeof(text) - 1;
    short refNum;

    if (HCreate(vRefNum, dirID, kMarkerName, kAppCreator, kLibraryMarkerType)
        != noErr)
        return;

    BlockMove(kMarkerName, name, kMarkerName[0] + 1);
    memset(&pb, 0, sizeof(pb));
    pb.ioParam.ioNamePtr = name;
    pb.ioParam.ioVRefNum = vRefNum;
    pb.ioParam.ioPermssn = fsRdWrPerm;
    pb.fileParam.ioDirID = dirID;
    if (PBHOpenSync(&pb) == noErr)
    {
        refNum = pb.ioParam.ioRefNum;
        FSWrite(refNum, &count, text);
        FSClose(refNum);
    }
    FlushVol(nil, vRefNum);
}

/* ---- The next song ID (Stage 8d, M4) --------------------------------- */

#define kMarkerMaxText      1024
static const char kNextIDLabel[] = "Next song ID: ";

/* Reads the marker's text into buf (at most kMarkerMaxText bytes). */
static OSErr ReadMarker(const LibraryFolder *folder, char *buf, long *count)
{
    short refNum;
    OSErr err;

    *count = 0;
    err = OpenDataFork(folder->vRefNum, folder->folderID, kMarkerName,
                       fsRdPerm, &refNum);
    if (err != noErr)
        return err;
    *count = kMarkerMaxText;
    err = FSRead(refNum, count, buf);
    FSClose(refNum);
    return err == eofErr ? noErr : err;
}

/* Where the "Next song ID" line starts in the text, or count if none. */
static long FindNextIDLine(const char *buf, long count)
{
    long label = sizeof(kNextIDLabel) - 1, at;

    for (at = 0; at + label <= count; at++)
        if ((at == 0 || buf[at - 1] == '\r' || buf[at - 1] == '\n')
            && memcmp(buf + at, kNextIDLabel, label) == 0)
            return at;
    return count;
}

long Folder_ReadNextID(const LibraryFolder *folder)
{
    static char buf[kMarkerMaxText];
    long count, at, next = 0;

    if (ReadMarker(folder, buf, &count) != noErr)
        return 0;
    at = FindNextIDLine(buf, count);
    if (at >= count)
        return 0;
    for (at += sizeof(kNextIDLabel) - 1;
         at < count && buf[at] >= '0' && buf[at] <= '9' && next < 100000000L;
         at++)
        next = next * 10 + (buf[at] - '0');
    return next;
}

void Folder_WriteNextID(const LibraryFolder *folder, long next)
{
    static char buf[kMarkerMaxText + 32];
    Str255 number;
    long count, at;
    short refNum;

    EnsureMarker(folder->vRefNum, folder->folderID);
    if (ReadMarker(folder, buf, &count) != noErr)
        return;

    /* The text before any old "Next song ID" line is kept as it is. */
    at = FindNextIDLine(buf, count);
    if (at > 0 && buf[at - 1] != '\r' && buf[at - 1] != '\n')
        buf[at++] = '\r';
    NumToString(next, number);
    memcpy(buf + at, kNextIDLabel, sizeof(kNextIDLabel) - 1);
    at += sizeof(kNextIDLabel) - 1;
    memcpy(buf + at, number + 1, number[0]);
    at += number[0];
    buf[at++] = '\r';

    if (OpenDataFork(folder->vRefNum, folder->folderID, kMarkerName,
                     fsRdWrPerm, &refNum) != noErr)
        return;
    count = at;
    if (SetFPos(refNum, fsFromStart, 0) == noErr
        && FSWrite(refNum, &count, buf) == noErr)
        SetEOF(refNum, at);
    FSClose(refNum);
    FlushVol(nil, folder->vRefNum);
}

Boolean Folder_Resolve(const LibraryLocation *location, LibraryFolder *folder)
{
    if (!FindVolume(location->volumeName, location->volumeCreated,
                    &folder->vRefNum))
        return false;
    if (!FolderExists(folder->vRefNum, location->folderID))
        return false;

    folder->folderID = location->folderID;
    if (MakeFolder(folder->vRefNum, folder->folderID, kPlaylistsName,
                   &folder->playlistsID) != noErr)
        return false;
    EnsureMarker(folder->vRefNum, folder->folderID);
    return true;
}

/* Uses folderID on vRefNum as the library: makes sure it has Playlists
   and the marker, and fills in location (the volume is remembered by
   name and creation date). */
static short UseFolder(short vRefNum, long folderID,
                       LibraryLocation *location, LibraryFolder *folder,
                       short *error)
{
    HParamBlockRec vol;
    OSErr err;

    folder->vRefNum = vRefNum;
    folder->folderID = folderID;
    err = MakeFolder(vRefNum, folderID, kPlaylistsName, &folder->playlistsID);
    if (err == dupFNErr)
        return kFolderNameIsFile;
    if (err == noErr)
    {
        vol.volumeParam.ioNamePtr = location->volumeName;
        vol.volumeParam.ioVRefNum = vRefNum;
        vol.volumeParam.ioVolIndex = 0;
        err = PBHGetVInfoSync(&vol);
    }
    if (err != noErr)
    {
        *error = err;
        return kFolderError;
    }
    EnsureMarker(vRefNum, folderID);

    location->volumeCreated = vol.volumeParam.ioVCrDate;
    location->folderID = folderID;
    return kFolderChosen;
}

/* An existing folder picked in the Save dialog (see ExistingFolderHook),
   or the folder shown being a library itself (gInLibrary). */
static Boolean gPickedExisting;
static Boolean gInLibrary;
static short gPickedVRefNum;
static long gPickedParentID;
static Str63 gPickedName;

/* Standard File's Save dialog asks "Replace existing ...?" whenever the
   typed name exists, folders included, and System 6 has no option to
   turn that off. When Save is clicked and the name is an existing folder
   in the directory being shown, this hook records it and closes the
   dialog itself (as if Cancel), so the folder is simply reused. Any other
   name, including an existing file, is left to Standard File.
   DECISION L3 (5b-3): Save while showing the inside of a library would
   create a library within it, so the hook closes the dialog and the
   caller asks whether to use the library shown instead. */
static pascal short ExistingFolderHook(short item, DialogPtr dialog)
{
    short itemType;
    Handle itemHandle;
    Rect itemRect;
    Str255 name;
    short vRefNum;
    long parentID, folderID;

    if (item != putSave)
        return item;

    GetDialogItem(dialog, putName, &itemType, &itemHandle, &itemRect);
    GetDialogItemText(itemHandle, name);
    if (name[0] == 0 || name[0] > 31)
        return item;

    /* The directory the dialog is showing (Standard File globals). */
    vRefNum = -LMGetSFSaveDisk();
    parentID = LMGetCurDirStore();

    if (IsLibraryFolder(vRefNum, parentID))
    {
        gInLibrary = true;
        gPickedVRefNum = vRefNum;
        gPickedParentID = parentID;
        return putCancel;
    }

    if (LookUpFolder(vRefNum, parentID, name, &folderID) != noErr)
        return item;

    gPickedExisting = true;
    gPickedVRefNum = vRefNum;
    gPickedParentID = parentID;
    BlockMove(name, gPickedName, name[0] + 1);
    return putCancel;
}

short Folder_Choose(LibraryLocation *location, LibraryFolder *folder,
                    short *error)
{
    SFReply reply;
    WDPBRec wd;
    Point where;
    ConstStringPtr name;
    short vRefNum;
    long parentID, folderID;
    OSErr err;

    *error = noErr;

    /* System 6 Standard File cannot select folders, so the Save dialog
       picks the location and the name (DECISION D21: the typed name is
       used; "iPod Library" is the default). An existing folder of that
       name is reused. */
    where.h = 104;
    where.v = 60;
    gPickedExisting = false;
    gInLibrary = false;
    SFPPutFile(where, P("Create new iPod Library in:"), P("iPod Library"),
               ExistingFolderHook, &reply, putDlgID, Modal_FileFilter);

    if (gInLibrary)
    {
        if (Alert(kUseLibraryHereAlert, Modal_DialogFilter) != kItemUseIt)
            return kFolderCancelled;
        return UseFolder(gPickedVRefNum, gPickedParentID, location, folder,
                         error);
    }

    if (gPickedExisting)
    {
        vRefNum = gPickedVRefNum;
        parentID = gPickedParentID;
        name = gPickedName;
    }
    else
    {
        if (!reply.good)
            return kFolderCancelled;

        /* The reply names a working directory; convert it to the real
           volume and directory ID. */
        wd.ioNamePtr = nil;
        wd.ioVRefNum = reply.vRefNum;
        wd.ioWDIndex = 0;
        wd.ioWDProcID = 0;
        err = PBGetWDInfoSync(&wd);
        if (err != noErr)
        {
            *error = err;
            return kFolderError;
        }

        vRefNum = wd.ioWDVRefNum;
        parentID = wd.ioWDDirID;
        name = reply.fName;
    }

    err = MakeFolder(vRefNum, parentID, name, &folderID);
    if (err == dupFNErr)
        return kFolderNameIsFile;
    if (err != noErr)
    {
        *error = err;
        return kFolderError;
    }
    return UseFolder(vRefNum, folderID, location, folder, error);
}

/* Lists only files that identify a library: the marker, song files and
   playlists (all with this app's creator). Returns true to hide. */
static pascal Boolean LibraryFileFilter(CInfoPBPtr pb)
{
    OSType type = pb->hFileInfo.ioFlFndrInfo.fdType;

    if (pb->hFileInfo.ioFlAttrib & kDirectoryAttribute)
        return false;
    if (pb->hFileInfo.ioFlFndrInfo.fdCreator != kAppCreator)
        return true;
    return type != kLibraryMarkerType && type != kSongFileType
        && type != kPlaylistFileType;
}

/* The parent folder of dirID. */
static OSErr ParentFolder(short vRefNum, long dirID, long *parentID)
{
    CInfoPBRec pb;
    Str63 name;
    OSErr err;

    pb.dirInfo.ioNamePtr = name;
    pb.dirInfo.ioVRefNum = vRefNum;
    pb.dirInfo.ioFDirIndex = -1;            /* look up by directory ID */
    pb.dirInfo.ioDrDirID = dirID;
    err = PBGetCatInfoSync(&pb);
    *parentID = pb.dirInfo.ioDrParID;
    return err;
}

short Folder_FindExisting(LibraryLocation *location, LibraryFolder *folder,
                          short *error)
{
    SFReply reply;
    WDPBRec wd;
    Point where;
    long dirID;
    OSErr err;

    *error = noErr;

    /* DECISION (5b-3): System 6 can't pick a folder in the Open dialog,
       so a file in the library is picked instead: its marker, any song,
       or a playlist (whose folder, Playlists, is inside the library). */
    where.h = 82;
    where.v = 50;
    SFPGetFile(where, P(""), LibraryFileFilter, -1, nil, nil, &reply,
               getDlgID, Modal_FileFilter);
    if (!reply.good)
        return kFolderCancelled;

    wd.ioNamePtr = nil;
    wd.ioVRefNum = reply.vRefNum;
    wd.ioWDIndex = 0;
    wd.ioWDProcID = 0;
    err = PBGetWDInfoSync(&wd);
    if (err != noErr)
    {
        *error = err;
        return kFolderError;
    }

    dirID = wd.ioWDDirID;
    if (reply.fType == kPlaylistFileType)
    {
        err = ParentFolder(wd.ioWDVRefNum, dirID, &dirID);
        if (err != noErr)
        {
            *error = err;
            return kFolderError;
        }
    }

    if (!IsLibraryFolder(wd.ioWDVRefNum, dirID))
        return kFolderNotLibrary;
    return UseFolder(wd.ioWDVRefNum, dirID, location, folder, error);
}
