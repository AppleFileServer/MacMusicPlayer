#include <Dialogs.h>
#include <Memory.h>
#include <TextUtils.h>
#include <ToolUtils.h>

#include "modal.h"
#include "playlists.h"
#include "library.h"
#include "message.h"
#include "pstr.h"
#include "ui_nav.h"

enum
{
    kItemNewPlaylist = 1,
    kItemFirstPlaylist = 3,     /* after the divider */

    kNewPlaylistDialog = 133,
    kItemCreate = 1,
    kItemCancel = 2,
    kItemName = 3,
    kItemOutline = 5,           /* user item: default button outline */

    kAlreadyAlert = 134,        /* "already in the playlist" (P5) */
    kItemAddAgain = 2
};

/* Menu item n (from kItemFirstPlaylist) adds to playlist gMenuPlaylists[n]. */
static short gMenuPlaylists[kMaxPlaylists];
static short gMenuPlaylistCount;

void Playlists_AdjustMenu(MenuHandle menu, Boolean hasLibrary)
{
    Boolean canAdd = hasLibrary && UI_SelectedSong() != kNone;
    Str255 text;
    short i, item;

    while (CountMItems(menu) >= kItemFirstPlaylist)
        DeleteMenuItem(menu, kItemFirstPlaylist);

    gMenuPlaylistCount = hasLibrary
        ? Library_ListPlaylists(gMenuPlaylists, kMaxPlaylists) : 0;

    if (gMenuPlaylistCount == 0)
    {
        AppendMenu(menu, P("No Playlists"));
        DisableItem(menu, kItemFirstPlaylist);
    }

    for (i = 0; i < gMenuPlaylistCount; i++)
    {
        /* Append a placeholder, then set the real text: AppendMenu would
           treat characters such as ; ( ! / in a name as commands. */
        text[0] = 0;
        PStrAppend(text, P("Add to \xD2"));
        PStrAppend(text, Library_PlaylistName(gMenuPlaylists[i]));
        PStrAppend(text, P("\xD3"));
        AppendMenu(menu, P("x"));
        item = kItemFirstPlaylist + i;
        SetMenuItemText(menu, item, text);

        /* Only the first 31 items can be disabled; the handler re-checks. */
        if (canAdd)
            EnableItem(menu, item);
        else
            DisableItem(menu, item);
    }

    if (hasLibrary)
        EnableItem(menu, kItemNewPlaylist);
    else
        DisableItem(menu, kItemNewPlaylist);
}

/* ---- New Playlist... -------------------------------------------------- */

static pascal void DrawDefaultOutline(DialogPtr dialog, short item)
{
    short type;
    Handle handle;
    Rect r;

    (void)item;
    GetDialogItem(dialog, kItemCreate, &type, &handle, &r);
    InsetRect(&r, -4, -4);
    PenSize(3, 3);
    FrameRoundRect(&r, 16, 16);
    PenNormal();
}

static Handle ItemHandle(DialogPtr dialog, short item)
{
    short type;
    Handle handle;
    Rect r;

    GetDialogItem(dialog, item, &type, &handle, &r);
    return handle;
}

/* DECISION P9: "Untitled Playlist", or "Untitled Playlist 2", 3... if
   that name is taken. */
static void DefaultName(Str255 name)
{
    Str255 number;
    short n;

    for (n = 1; n < 1000; n++)
    {
        name[0] = 0;
        PStrAppend(name, P("Untitled Playlist"));
        if (n > 1)
        {
            NumToString(n, number);
            PStrAppend(name, P(" "));
            PStrAppend(name, number);
        }
        if (Library_FindPlaylist(name) == kNone)
            return;
    }
}

/* DECISION P9: 1-31 characters, no colon, not already used. Explains the
   problem and returns false if the name can't be used. */
static Boolean NameIsUsable(ConstStringPtr name)
{
    short i;

    if (name[0] == 0)
    {
        Message_Show(P("Please enter a name for the playlist."), noErr);
        return false;
    }
    if (name[0] > 31)
    {
        Message_Show(P("Playlist names can be at most 31 characters long."), noErr);
        return false;
    }
    for (i = 1; i <= name[0]; i++)
        if (name[i] == ':')
        {
            Message_Show(P("Playlist names can\xD5t contain a colon (:)."), noErr);
            return false;
        }
    if (Library_FindPlaylist(name) != kNone)
    {
        Message_Show(P("There is already a playlist with that name."), noErr);
        return false;
    }
    return true;
}

static Boolean NewPlaylist(void)
{
    DialogPtr dialog;
    Str255 name;
    short item, type, playlist;
    Handle handle;
    Rect r;
    OSErr err;

    dialog = GetNewDialog(kNewPlaylistDialog, nil, (WindowPtr)-1);
    if (dialog == nil)
        return false;

    DefaultName(name);
    SetDialogItemText(ItemHandle(dialog, kItemName), name);
    GetDialogItem(dialog, kItemOutline, &type, &handle, &r);
    SetDialogItem(dialog, kItemOutline, type, (Handle)DrawDefaultOutline, &r);
    SelectDialogItemText(dialog, kItemName, 0, 32767);
    ShowWindow(dialog);

    for (;;)
    {
        ModalDialog(Modal_DialogFilter, &item);
        if (item == kItemCancel)
        {
            DisposeDialog(dialog);
            return false;
        }
        if (item != kItemCreate)
            continue;

        GetDialogItemText(ItemHandle(dialog, kItemName), name);
        if (!NameIsUsable(name))
        {
            SelectDialogItemText(dialog, kItemName, 0, 32767);
            continue;
        }

        err = Library_AddPlaylist(name, &playlist);
        if (err == noErr)
            break;

        if (err == dirFulErr)
            Message_Show(P("There are already as many playlists as "
                           "MacMusicPlayer can handle."), noErr);
        else if (err == dupFNErr)
            Message_Show(P("There is already a file with that name in the "
                           "Playlists folder."), noErr);
        else
            Message_Show(P("The playlist could not be created."), err);
        SelectDialogItemText(dialog, kItemName, 0, 32767);
    }

    DisposeDialog(dialog);
    return true;
}

/* ---- Add to "<playlist>" ---------------------------------------------- */

static Boolean AddToPlaylist(short playlist)
{
    short song = UI_SelectedSong();
    OSErr err;

    /* Items past the 31st can't be disabled by the Menu Manager. */
    if (song == kNone)
    {
        SysBeep(10);
        return false;
    }

    /* DECISION P5: ask before adding a song that is already there. */
    if (Library_PlaylistContains(playlist, song))
    {
        ParamText(Library_SongTitle(song), Library_PlaylistName(playlist),
                  P(""), P(""));
        if (Alert(kAlreadyAlert, Modal_DialogFilter) != kItemAddAgain)
            return false;
    }

    err = Library_AppendToPlaylist(playlist, song);
    if (err != noErr)
    {
        Message_Show(P("The song could not be added to the playlist."), err);
        return false;
    }
    return true;
}

Boolean Playlists_HandleMenu(short item)
{
    short index = item - kItemFirstPlaylist;

    if (item == kItemNewPlaylist)
        return NewPlaylist();
    if (index >= 0 && index < gMenuPlaylistCount)
        return AddToPlaylist(gMenuPlaylists[index]);
    return false;
}
