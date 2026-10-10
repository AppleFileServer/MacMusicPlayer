#pragma once
#include "MacTypes.h"
#include "Memory.h"     /* as the Mac's Files.h brings it in */

enum { fsCurPerm = 0, fsRdPerm = 1, fsWrPerm = 2, fsRdWrPerm = 3 };
enum { fsAtMark = 0, fsFromStart = 1, fsFromLEOF = 2, fsFromMark = 3 };

typedef struct { OSType fdType, fdCreator; unsigned short fdFlags;
                 Point fdLocation; short fdFldr; } FInfo;
enum { fInvisible = 16384 };
typedef struct { Rect frRect; unsigned short frFlags; Point frLocation;
                 short frView; } DInfo;

/* The leading fields every parameter block shares. */
#define PB_HEADER \
    void *qLink; short qType, ioTrap; Ptr ioCmdAddr; void *ioCompletion; \
    OSErr ioResult; StringPtr ioNamePtr; short ioVRefNum;

typedef struct { PB_HEADER short ioFRefNum; SignedByte ioFVersNum, filler1;
    short ioFDirIndex; SignedByte ioFlAttrib, ioACUser; FInfo ioFlFndrInfo;
    long ioDirID; long ioFlLgLen; long ioFlParID;
    unsigned long ioFlMdDat; } HFileInfo;
typedef struct { PB_HEADER short ioFRefNum; SignedByte ioFVersNum, filler1;
    short ioFDirIndex; SignedByte ioFlAttrib, ioACUser; DInfo ioDrUsrWds;
    long ioDrDirID; long ioDrNmFls; long ioDrParID;
    unsigned long ioDrMdDat; } DirInfo;
typedef union { HFileInfo hFileInfo; DirInfo dirInfo; } CInfoPBRec, *CInfoPBPtr;

typedef struct { PB_HEADER short ioRefNum; SignedByte ioVersNum, ioPermssn;
    Ptr ioMisc; } IOParam;
typedef struct { PB_HEADER short ioFRefNum; SignedByte ioFVersNum, filler1;
    short ioFDirIndex; SignedByte ioFlAttrib, ioFlVersNum; FInfo ioFlFndrInfo;
    long ioDirID; } HFileParam;
typedef struct { PB_HEADER long filler2; short ioVolIndex;
    unsigned long ioVCrDate; long ioVAlBlkSiz; unsigned short ioVFrBlk;
    } HVolumeParam;
typedef union { IOParam ioParam; HFileParam fileParam;
                HVolumeParam volumeParam; } HParamBlockRec;

typedef struct { PB_HEADER short filler1; short ioWDIndex; long ioWDProcID;
    short ioWDVRefNum; long ioWDDirID; } WDPBRec;
typedef struct { PB_HEADER long ioNewDirID; long ioDirID; } CMovePBRec;

OSErr PBGetCatInfoSync(CInfoPBRec *pb);
OSErr PBSetCatInfoSync(CInfoPBRec *pb);
OSErr PBHOpenSync(HParamBlockRec *pb);
OSErr PBHGetFInfoSync(HParamBlockRec *pb);
OSErr PBHSetFInfoSync(HParamBlockRec *pb);
OSErr PBHRenameSync(HParamBlockRec *pb);
OSErr PBDirCreateSync(HParamBlockRec *pb);
OSErr PBHGetVInfoSync(HParamBlockRec *pb);
OSErr PBGetWDInfoSync(WDPBRec *pb);
OSErr PBCatMoveSync(CMovePBRec *pb);
OSErr HCreate(short vRefNum, long dirID, ConstStringPtr name, OSType creator,
              OSType type);
OSErr HDelete(short vRefNum, long dirID, ConstStringPtr name);
OSErr FSRead(short refNum, long *count, void *buffer);
OSErr FSWrite(short refNum, long *count, const void *buffer);
OSErr FSClose(short refNum);
OSErr GetEOF(short refNum, long *eof);
OSErr SetEOF(short refNum, long eof);
OSErr GetFPos(short refNum, long *position);
OSErr SetFPos(short refNum, short mode, long offset);
OSErr FlushVol(ConstStringPtr volName, short vRefNum);
