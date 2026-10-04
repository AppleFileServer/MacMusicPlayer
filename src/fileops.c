#include <Files.h>
#include <string.h>

#include "fileops.h"
#include "fileopen.h"

/* Parameter blocks are cleared first, as in fileopen.h: the File Manager
   reads fields the glue leaves unset. */

/* The volume and folder of a working directory (as Standard File gives). */
OSErr ResolveWD(short wdRefNum, short *vRefNum, long *dirID)
{
    WDPBRec pb;
    OSErr err;

    memset(&pb, 0, sizeof(pb));
    pb.ioVRefNum = wdRefNum;
    err = PBGetWDInfoSync(&pb);
    *vRefNum = pb.ioWDVRefNum;
    *dirID = pb.ioWDDirID;
    return err;
}

OSErr GetFileInfo(short vRefNum, long dirID, ConstStringPtr name,
                         HParamBlockRec *pb)
{
    memset(pb, 0, sizeof(*pb));
    pb->fileParam.ioNamePtr = (StringPtr)name;
    pb->fileParam.ioVRefNum = vRefNum;
    pb->fileParam.ioDirID = dirID;
    return PBHGetFInfoSync(pb);
}

Boolean FileExists(short vRefNum, long dirID, ConstStringPtr name)
{
    HParamBlockRec pb;

    return GetFileInfo(vRefNum, dirID, name, &pb) == noErr;
}

OSErr SetFileType(short vRefNum, long dirID, ConstStringPtr name,
                         OSType type, OSType creator)
{
    HParamBlockRec pb;
    OSErr err = GetFileInfo(vRefNum, dirID, name, &pb);

    if (err == noErr)
    {
        pb.fileParam.ioFlFndrInfo.fdType = type;
        pb.fileParam.ioFlFndrInfo.fdCreator = creator;
        pb.fileParam.ioDirID = dirID;      /* output too: reset */
        err = PBHSetFInfoSync(&pb);
    }
    return err;
}

OSErr RenameFile(short vRefNum, long dirID, ConstStringPtr name,
                        ConstStringPtr newName)
{
    HParamBlockRec pb;

    memset(&pb, 0, sizeof(pb));
    pb.ioParam.ioNamePtr = (StringPtr)name;
    pb.ioParam.ioVRefNum = vRefNum;
    pb.ioParam.ioMisc = (Ptr)newName;
    pb.fileParam.ioDirID = dirID;
    return PBHRenameSync(&pb);
}

/* Moves a file to another folder on the same volume, keeping its name. */
OSErr MoveFile(short vRefNum, long dirID, ConstStringPtr name,
                      long newDirID)
{
    CMovePBRec pb;

    memset(&pb, 0, sizeof(pb));
    pb.ioNamePtr = (StringPtr)name;
    pb.ioVRefNum = vRefNum;
    pb.ioNewDirID = newDirID;
    pb.ioDirID = dirID;
    return PBCatMoveSync(&pb);
}

/* Free bytes on a volume. */
long FreeBytes(short vRefNum)
{
    HParamBlockRec pb;

    memset(&pb, 0, sizeof(pb));
    pb.volumeParam.ioVRefNum = vRefNum;
    if (PBHGetVInfoSync(&pb) != noErr)
        return 0;
    return (long)pb.volumeParam.ioVFrBlk * pb.volumeParam.ioVAlBlkSiz;
}

/* Errors that mean the file can be read but not changed or moved (E4):
   locked file or disk, or open in another program. */
Boolean IsCantChangeError(OSErr err)
{
    return err == fLckdErr || err == vLckdErr || err == wPrErr
        || err == permErr || err == opWrErr || err == fBsyErr;
}

/* Adds or removes the chunk of a file in place. */
OSErr ChangeChunk(short vRefNum, long dirID, ConstStringPtr name,
                         const SongInfo *info, Boolean add)
{
    short refNum;
    OSErr err;

    err = OpenDataFork(vRefNum, dirID, name, fsRdWrPerm, &refNum);
    if (err != noErr)
        return err;
    err = add ? SongFile_AddChunk(refNum, info)
              : SongFile_RemoveChunk(refNum, info);
    FSClose(refNum);
    if (err == noErr)
        err = FlushVol(nil, vRefNum);
    return err;
}
