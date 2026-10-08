#pragma once
#include "MacTypes.h"
typedef struct { unsigned short what; unsigned long message, when;
                 Point where; unsigned short modifiers; } EventRecord;
