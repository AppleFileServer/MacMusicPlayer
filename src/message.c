#include <Dialogs.h>
#include <Memory.h>
#include <TextUtils.h>

#include "message.h"
#include "pstr.h"

#define kMessageAlert   130

void PStrAppend(Str255 dest, ConstStringPtr s)
{
    short n = s[0];

    if (dest[0] + n > 255)
        n = 255 - dest[0];
    BlockMove(s + 1, dest + dest[0] + 1, n);
    dest[0] += n;
}

void Message_Show(ConstStringPtr text, OSErr err)
{
    Str255 detail;
    Str255 number;

    detail[0] = 0;
    if (err != noErr)
    {
        NumToString(err, number);
        PStrAppend(detail, P(" (error "));
        PStrAppend(detail, number);
        PStrAppend(detail, P(")"));
    }

    ParamText(text, detail, P(""), P(""));
    Alert(kMessageAlert, nil);
}
