#pragma once
#include "MacTypes.h"
typedef struct { OSType fdType, fdCreator; } FInfo;
typedef struct { StringPtr ioNamePtr; short ioVRefNum; short ioFDirIndex; signed char ioFlAttrib; FInfo ioFlFndrInfo; long ioDirID; } HFileInfo;
typedef struct { StringPtr ioNamePtr; short ioVRefNum; short ioFDirIndex; signed char ioFlAttrib; long ioDrDirID; } DirInfo;
typedef union { HFileInfo hFileInfo; DirInfo dirInfo; } CInfoPBRec;
OSErr PBGetCatInfoSync(CInfoPBRec *pb);
OSErr GetEOF(short ref, long *eof); OSErr SetEOF(short ref, long eof);
OSErr FSRead(short ref, long *count, void *buf); OSErr FSWrite(short ref, long *count, const void *buf);
OSErr SetFPos(short ref, short mode, long off); OSErr FSClose(short ref);
OSErr HCreate(short v, long d, ConstStringPtr n, OSType c, OSType t); OSErr FlushVol(void *n, short v);
typedef struct { void *qLink; StringPtr ioNamePtr; short ioVRefNum; short ioRefNum; SignedByte ioPermssn; } IOParam;
typedef struct { void *qLink; StringPtr ioNamePtr; short ioVRefNum; short ioRefNum; SignedByte ioPermssn; long ioDirID; } HFileParam;
typedef union { IOParam ioParam; HFileParam fileParam; } HParamBlockRec;
OSErr PBHOpenSync(HParamBlockRec *pb);

OSErr HDelete(short v,long d,ConstStringPtr n);
