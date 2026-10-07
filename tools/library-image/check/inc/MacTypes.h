#pragma once
#include <string.h>
#include <stdlib.h>
#define pascal
typedef unsigned char Boolean; typedef const unsigned char *ConstStringPtr; typedef unsigned char *StringPtr;
typedef unsigned char Str255[256], Str63[64], Str31[32];
typedef char *Ptr; typedef Ptr *Handle; typedef long Size; typedef short OSErr; typedef unsigned long OSType;
typedef signed char SignedByte;
#define true 1
#define false 0
#define nil 0
enum { noErr=0, ioErr=-36, eofErr=-39, fnfErr=-43, dupFNErr=-48, memFullErr=-108, dirFulErr=-33,
       fsRdPerm=1, fsWrPerm=2, fsRdWrPerm=3, fsFromStart=1, fsFromLEOF=2 };
#define M68K_INLINE(...)
typedef unsigned int UInt32;

enum { paramErr = -50, badFormat = -206 };
