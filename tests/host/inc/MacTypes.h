/* Host stand-ins for the Mac headers MacMusic's portable modules include,
   so those modules can be built and tested on this Mac (tests/host).
   Only what those modules use is declared. Field names follow Inside
   Macintosh; layouts don't need to match the Mac's, except that the
   members of each union share their leading fields as on the Mac. */
#pragma once
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define pascal
#define M68K_INLINE(...)     /* trap words: not on the host */
typedef unsigned char Boolean;
typedef unsigned char *StringPtr;
typedef const unsigned char *ConstStringPtr;
typedef unsigned char Str255[256], Str63[64], Str32[33], Str31[32];
typedef char *Ptr;
typedef Ptr *Handle;
typedef long Size;
typedef short OSErr;
typedef uint32_t OSType;
typedef uint32_t UInt32;
typedef signed char SignedByte;
typedef struct { short v, h; } Point;
typedef struct { short top, left, bottom, right; } Rect;

#define true 1
#define false 0
#define nil 0

enum
{
    noErr = 0, dirFulErr = -33, dskFulErr = -34, nsvErr = -35, ioErr = -36,
    bdNamErr = -37, fnOpnErr = -38, eofErr = -39, posErr = -40,
    tmfoErr = -42, fnfErr = -43, wPrErr = -44, fLckdErr = -45,
    vLckdErr = -46, fBsyErr = -47, dupFNErr = -48, opWrErr = -49,
    paramErr = -50, rfNumErr = -51, permErr = -54, nsDrvErr = -56,
    wrPermErr = -61,
    dirNFErr = -120, memFullErr = -108, badFormat = -206
};
