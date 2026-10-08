#pragma once
#include "MacTypes.h"
#include "Events.h"
typedef void *DialogPtr;
typedef void *WindowPtr;
typedef pascal Boolean (*ModalFilterProcPtr)(DialogPtr, EventRecord *, short *);
short Alert(short alertID, ModalFilterProcPtr filter);
void GetDialogItem(DialogPtr d, short item, short *type, Handle *h, Rect *r);
void GetDialogItemText(Handle item, Str255 text);
