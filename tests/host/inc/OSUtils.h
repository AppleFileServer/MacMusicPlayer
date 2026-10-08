#pragma once
#include "MacTypes.h"
typedef struct { short environsVersion, machineType, systemVersion,
                 processor; Boolean hasFPU, hasColorQD; short keyBoardType,
                 atDrvrVersNum, sysVRefNum; } SysEnvRec;
OSErr SysEnvirons(short versionRequested, SysEnvRec *env);
