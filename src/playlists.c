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
    kItemRename = 2,            /* Stage 8e-4 */
    kItemDelete = 3,            /* Stage 8e-3 */
    kItemRemove = 4,            /* Stage 8d-4 (L7) */
    kItemFirstPlaylist = 6,     /* after the divider */

    kDeleteAlert = 142,         /* Stage 8e-3: Cancel is item 1 */
    kItemDeleteIt = 2,

    kNewPlaylistDialog = 133,
    kItemCreate = 1,
    kItemCancel = 2,
    kItemName = 3,
    kItemPrompt = 4,            /* "Name of the new playlist:" */
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

    {
        short playlist, nth;
        long id;

        if (hasLibrary && UI_SelectedPlaylistEntry(&playlist, &id, &nth))
            EnableItem(menu, kItemRemove);
        else
            DisableItem(menu, kItemRemove);

        /* "Rename "Mix"..." and "Delete "Mix"...", naming the playlist
           they act on. */
        playlist = hasLibrary ? UI_HighlightedPlaylist() : kNone;
        if (playlist != kNone)
        {
            text[0] = 0;
            PStrAppend(text, P("Rename \xD2"));
            PStrAppend(text, Library_PlaylistName(playlist));
            PStrAppend(text, P("\xD3\xC9"));
            SetMenuItemText(menu, kItemRename, text);
            EnableItem(menu, kItemRename);

            text[0] = 0;
            PStrAppend(text, P("Delete \xD2"));
            PStrAppend(text, Library_PlaylistName(playlist));
            PStrAppend(text, P("\xD3\xC9"));
            SetMenuItemText(menu, kItemDelete, text);
            EnableItem(menu, kItemDelete);
        }
        else
        {
            SetMenuItemText(menu, kItemRename, P("Rename Playlist\xC9"));
            DisableItem(menu, kItemRename);
            SetMenuItemText(menu, kItemDelete, P("Delete Playlist\xC9"));
            DisableItem(menu, kItemDelete);
        }
    }
}

/* ---- Delete Playlist... (Stage 8e-3) ------------------------------------ */

/* Deletes the highlighted or shown playlist after asking: its file goes,
   its songs stay in the library. The iPod's screens are found again by
   name, so one showing that playlist closes to the Playlists list. */
static Boolean DeletePlaylist(void)
{
    short playlist = UI_HighlightedPlaylist();
    OSErr err;

    if (playlist == kNone)
        return false;
    ParamText(Library_PlaylistName(playlist), P(""), P(""), P(""));
    if (Alert(kDeleteAlert, Modal_DialogFilter) != kItemDeleteIt)
        return false;

    UI_BeginLibraryEdit();
    err = Library_DeletePlaylist(playlist);
    UI_EndLibraryEdit();
    if (err != noErr)
    {
        Message_Show(P("The playlist could not be deleted."), err);
        return false;
    }
    return true;
}

/* ---- Remove from Playlist (Stage 8d-4, L7) ----------------------------- */

/* Removes the highlighted entry from the playlist being shown. Only that
   playlist line goes: the song stays in the library (L1). */
static Boolean RemoveFromPlaylist(void)
{
    short playlist, nth;
    long id;
    OSErr err;

    if (!UI_SelectedPlaylistEntry(&playlist, &id, &nth))
        return false;
    err = Library_RemoveFromPlaylist(playlist, id, nth);
    if (err != noErr)
    {
        Message_Show(P("The song could not be removed from the playlist."), err);
        return false;
    }
    UI_KeepRowOnRefresh();
    return true;
}

/* ---- New Playlist... -------------------------------------------------- */

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
/* except: the playlist being renamed (8e-4), which may keep its name with
   other capitals; kNone for a new playlist. */
static Boolean NameIsUsable(ConstStringPtr name, short except)
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
    if (Library_FindPlaylist(name) != kNone
        && Library_FindPlaylist(name) != except)
    {
        Message_Show(P("There is already a playlist with that name."), noErr);
        return false;
    }
    return true;
}

/* The playlist name window (DLOG 133): for a new playlist (playlist is
   kNone), or (Stage 8e-4) to rename one, filled with its name and with a
   Rename button. Returns true if a playlist was created or renamed. */
static Boolean AskPlaylistName(short playlist)
{
    DialogPtr dialog;
    Str255 name;
    short item, type, created;
    Handle handle;
    Rect r;
    OSErr err;

    dialog = GetNewDialog(kNewPlaylistDialog, nil, (WindowPtr)-1);
    if (dialog == nil)
        return false;

    if (playlist == kNone)
        DefaultName(name);
    else
    {
        BlockMove(Library_PlaylistName(playlist), name,
                  Library_PlaylistName(playlist)[0] + 1);
        SetDialogItemText(Modal_ItemHandle(dialog, kItemPrompt),
                          P("New name for the playlist:"));
        SetControlTitle((ControlHandle)Modal_ItemHandle(dialog, kItemCreate),
                        P("Rename"));
    }
    SetDialogItemText(Modal_ItemHandle(dialog, kItemName), name);
    GetDialogItem(dialog, kItemOutline, &type, &handle, &r);
    SetDialogItem(dialog, kItemOutline, type,
                  (Handle)Modal_DrawDefaultOutline, &r);
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

        GetDialogItemText(Modal_ItemHandle(dialog, kItemName), name);
        if (!NameIsUsable(name, playlist))
        {
            SelectDialogItemText(dialog, kItemName, 0, 32767);
            continue;
        }

        if (playlist == kNone)
            err = Library_AddPlaylist(name, &created);
        else if (EqualString(name, Library_PlaylistName(playlist), true, true))
        {
            DisposeDialog(dialog);      /* unchanged */
            return false;
        }
        else
            err = Library_RenamePlaylist(playlist, name);
        if (err == noErr)
            break;

        if (err == dirFulErr)
            Message_Show(P("MacMusic can have up to 10 playlists."), noErr);
        else if (err == dupFNErr)
            Message_Show(P("There is already a file with that name in the "
                           "Playlists folder."), noErr);
        else if (playlist == kNone)
            Message_Show(P("The playlist could not be created."), err);
        else
            Message_Show(P("The playlist could not be renamed."), err);
        SelectDialogItemText(dialog, kItemName, 0, 32767);
    }

    DisposeDialog(dialog);
    return true;
}

static Boolean NewPlaylist(void)
{
    /* 8e-2: at the limit, say so before asking for a name. */
    if (Library_PlaylistCount() >= kMaxPlaylists)
    {
        Message_Show(P("MacMusic can have up to 10 playlists."), noErr);
        return false;
    }
    return AskPlaylistName(kNone);
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
    if (item == kItemRemove)
        return RemoveFromPlaylist();
    if (item == kItemDelete)
        return DeletePlaylist();
    if (item == kItemRename && UI_HighlightedPlaylist() != kNone)
        return AskPlaylistName(UI_HighlightedPlaylist());
    if (index >= 0 && index < gMenuPlaylistCount)
        return AddToPlaylist(gMenuPlaylists[index]);
    return false;
}
