#pragma once
#include "MacTypes.h"
Handle NewHandle(Size size);
Handle NewHandleClear(Size size);
void SetHandleSize(Handle h, Size size);
Size GetHandleSize(Handle h);
void DisposeHandle(Handle h);
void HLock(Handle h);
void HUnlock(Handle h);
void HNoPurge(Handle h);
void MoveHHi(Handle h);
OSErr MemError(void);
Ptr NewPtr(Size size);
Ptr NewPtrClear(Size size);
void DisposePtr(Ptr p);
void BlockMove(const void *src, void *dest, Size count);
