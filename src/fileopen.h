#ifndef FILEOPEN_H
#define FILEOPEN_H

#include <Files.h>
#include <string.h>

/* Opens a file's data fork. Use this instead of HOpen: Retro68's HOpen
   glue (libInterface) builds its parameter block on the stack without
   setting ioVersNum or ioMisc, so the File Manager receives stack garbage
   and can hang or crash depending on what was there. Here the whole
   block is cleared first. */
static inline OSErr OpenDataFork(short vRefNum, long dirID,
                                 ConstStringPtr name, SignedByte permission,
                                 short *refNum)
{
    HParamBlockRec pb;
    OSErr err;

    memset(&pb, 0, sizeof(pb));
    pb.ioParam.ioNamePtr = (StringPtr)name;
    pb.ioParam.ioVRefNum = vRefNum;
    pb.ioParam.ioPermssn = permission;
    pb.fileParam.ioDirID = dirID;

    err = PBHOpenSync(&pb);
    *refNum = pb.ioParam.ioRefNum;
    return err;
}

#endif
