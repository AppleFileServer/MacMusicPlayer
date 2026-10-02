#include <Files.h>
#include <StandardFile.h>
#include <TextUtils.h>

#include "libfolder.h"
#include "pstr.h"

#define kDirectoryAttribute     0x10    /* ioFlAttrib bit 4 */

static const unsigned char kPlaylistsName[] = "\pPlaylists";

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

Boolean Folder_Resolve(const LibraryLocation *location, LibraryFolder *folder)
{
    if (!FindVolume(location->volumeName, location->volumeCreated,
                    &folder->vRefNum))
        return false;
    if (!FolderExists(folder->vRefNum, location->folderID))
        return false;

    folder->folderID = location->folderID;
    return MakeFolder(folder->vRefNum, folder->folderID, kPlaylistsName,
                      &folder->playlistsID) == noErr;
}

/* An existing folder picked in the Save dialog (see ExistingFolderHook). */
static Boolean gPickedExisting;
static short gPickedVRefNum;
static long gPickedParentID;
static Str63 gPickedName;

/* Standard File's Save dialog asks "Replace existing ...?" whenever the
   typed name exists, folders included, and System 6 has no option to
   turn that off. When Save is clicked and the name is an existing folder
   in the directory being shown, this hook records it and closes the
   dialog itself (as if Cancel), so the folder is simply reused. Any other
   name, including an existing file, is left to Standard File. */
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
    HParamBlockRec vol;
    Point where;
    ConstStringPtr name;
    long parentID;
    OSErr err;

    *error = noErr;

    /* System 6 Standard File cannot select folders, so the Save dialog
       picks the location and the name (DECISION D21: the typed name is
       used; "iPod Library" is the default). An existing folder of that
       name is reused. */
    where.h = 104;
    where.v = 60;
    gPickedExisting = false;
    SFPutFile(where, P("Create iPod Library in:"), P("iPod Library"),
              ExistingFolderHook, &reply);

    if (gPickedExisting)
    {
        folder->vRefNum = gPickedVRefNum;
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

        folder->vRefNum = wd.ioWDVRefNum;
        parentID = wd.ioWDDirID;
        name = reply.fName;
    }

    err = MakeFolder(folder->vRefNum, parentID, name, &folder->folderID);
    if (err == dupFNErr)
        return kFolderNameIsFile;
    if (err == noErr)
        err = MakeFolder(folder->vRefNum, folder->folderID, kPlaylistsName,
                         &folder->playlistsID);
    if (err != noErr)
    {
        *error = err;
        return kFolderError;
    }

    /* Remember the volume by name and creation date. */
    vol.volumeParam.ioNamePtr = location->volumeName;
    vol.volumeParam.ioVRefNum = folder->vRefNum;
    vol.volumeParam.ioVolIndex = 0;
    err = PBHGetVInfoSync(&vol);
    if (err != noErr)
    {
        *error = err;
        return kFolderError;
    }

    location->volumeCreated = vol.volumeParam.ioVCrDate;
    location->folderID = folder->folderID;
    return kFolderChosen;
}
