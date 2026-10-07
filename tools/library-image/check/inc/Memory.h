#pragma once
#include "MacTypes.h"
Handle NewHandle(Size s); void SetHandleSize(Handle h, Size s); OSErr MemError(void); void MoveHHi(Handle h); void HLock(Handle h); void DisposeHandle(Handle h);
void BlockMove(const void *s, void *d, Size n);
Ptr NewPtr(Size s); void DisposePtr(Ptr p);

void HUnlock(Handle h);
