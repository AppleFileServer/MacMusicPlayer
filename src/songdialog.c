#include <Dialogs.h>
#include <TextUtils.h>
#include <ToolUtils.h>

#include "songdialog.h"
#include "fieldclick.h"
#include "message.h"
#include "modal.h"
#include "pstr.h"

/* The song details window (DLOG/DITL 131). */
enum
{
    kSongDialog = 131,
    kItemOK = 1,                /* "Import" or "Save" */
    kItemCancel = 2,
    kItemTitle = 3,             /* edit fields 3-7: title ... composer */
    kItemTrack = 8,
    kItemAudiobook = 9,
    kItemOutline = 17           /* user item: default button outline */
};

static ConstStringPtr const kFieldNames[kSongTagCount] = {
    P("The title"), P("The artist"), P("The album"), P("The genre"),
    P("The composer")
};

/* 1.2 (N12-1, N12-2): the song window's filter. Modal_DialogFilter's
   work first; then a click on the margin between a field's frame and its
   text is moved inside the field, so ModalDialog selects that field (and
   places the insertion point) as for a click on the text. Only this
   window, whose fields stand in a column (N12-2). */
static pascal Boolean SongFilter(DialogPtr dialog, EventRecord *event,
                                 short *item)
{
    Rect fields[kItemTrack - kItemTitle + 1];
    short i, type;
    Handle handle;
    Point pt;
    GrafPtr savePort;

    if (Modal_DialogFilter(dialog, event, item))
        return true;
    if (event->what != mouseDown)
        return false;

    for (i = 0; i <= kItemTrack - kItemTitle; i++)
        GetDialogItem(dialog, kItemTitle + i, &type, &handle, &fields[i]);

    GetPort(&savePort);
    SetPort(dialog);
    pt = event->where;
    GlobalToLocal(&pt);
    if (FieldClick_Adjust(fields, kItemTrack - kItemTitle + 1, &pt))
    {
        LocalToGlobal(&pt);
        event->where = pt;
    }
    SetPort(savePort);
    return false;
}

/* "m:ss" for a number of seconds. */
static void FormatDuration(long seconds, Str255 out)
{
    Str255 number;

    NumToString(seconds / 60, out);
    PStrAppend(out, P(":"));
    if (seconds % 60 < 10)
        PStrAppend(out, P("0"));
    NumToString(seconds % 60, number);
    PStrAppend(out, number);
}

/* Checks the fields; on a problem explains it, selects the field and
   returns false (DECISION D25: the title may not be blank). */
static Boolean ReadDetails(DialogPtr dialog, SongDetails *details)
{
    Str255 text, message;
    short i;
    long track;

    for (i = 0; i < kSongTagCount; i++)
    {
        GetDialogItemText(Modal_ItemHandle(dialog, kItemTitle + i), text);
        if (text[0] > kSongMaxString)
        {
            message[0] = 0;
            PStrAppend(message, kFieldNames[i]);
            PStrAppend(message, P(" is longer than 63 characters."));
            Message_Show(message, noErr);
            SelectDialogItemText(dialog, kItemTitle + i, 0, 32767);
            return false;
        }
        BlockMove(text, details->text[i], text[0] + 1);
    }

    if (details->text[0][0] == 0)
    {
        Message_Show(P("Please enter a title."), noErr);
        SelectDialogItemText(dialog, kItemTitle, 0, 32767);
        return false;
    }

    GetDialogItemText(Modal_ItemHandle(dialog, kItemTrack), text);
    track = 0;
    if (text[0] > 0)
    {
        for (i = 1; i <= text[0]; i++)
            if (text[i] < '0' || text[i] > '9')
                break;
        if (i <= text[0] || text[0] > 4)
        {
            Message_Show(P("Track must be a whole number from 0 to 9999."), noErr);
            SelectDialogItemText(dialog, kItemTrack, 0, 32767);
            return false;
        }
        StringToNum(text, &track);
    }
    details->track = (short)track;

    details->audiobook =
        GetControlValue((ControlHandle)Modal_ItemHandle(dialog,
                                                        kItemAudiobook)) != 0;
    return true;
}

Boolean SongDialog_Run(ConstStringPtr okTitle, ConstStringPtr fileName,
                       long seconds, SongDetails *details)
{
    DialogPtr dialog;
    Str255 duration, number;
    short item, type, i;
    Handle handle;
    ControlHandle checkbox;
    Rect r;
    Boolean ok = false;

    FormatDuration(seconds, duration);
    ParamText(fileName, duration, P(""), P(""));

    dialog = GetNewDialog(kSongDialog, nil, (WindowPtr)-1);
    if (dialog == nil)
    {
        Message_Show(P("The song details window could not be opened."), noErr);
        return false;
    }

    SetControlTitle((ControlHandle)Modal_ItemHandle(dialog, kItemOK), okTitle);
    for (i = 0; i < kSongTagCount; i++)
        SetDialogItemText(Modal_ItemHandle(dialog, kItemTitle + i),
                          details->text[i]);
    if (details->track > 0)
    {
        NumToString(details->track, number);
        SetDialogItemText(Modal_ItemHandle(dialog, kItemTrack), number);
    }
    SetControlValue((ControlHandle)Modal_ItemHandle(dialog, kItemAudiobook),
                    details->audiobook ? 1 : 0);

    GetDialogItem(dialog, kItemOutline, &type, &handle, &r);
    SetDialogItem(dialog, kItemOutline, type,
                  (Handle)Modal_DrawDefaultOutline, &r);

    SelectDialogItemText(dialog, kItemTitle, 0, 32767);
    ShowWindow(dialog);

    for (;;)
    {
        ModalDialog(SongFilter, &item);

        if (item == kItemOK)
        {
            if (ReadDetails(dialog, details))
            {
                ok = true;
                break;
            }
        }
        else if (item == kItemCancel)
            break;
        else if (item == kItemAudiobook)
        {
            checkbox = (ControlHandle)Modal_ItemHandle(dialog, kItemAudiobook);
            SetControlValue(checkbox, !GetControlValue(checkbox));
        }
    }

    DisposeDialog(dialog);
    return ok;
}
