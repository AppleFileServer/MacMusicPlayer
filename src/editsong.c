#include <Memory.h>
#include <string.h>

#include "editsong.h"
#include "library.h"
#include "message.h"
#include "pstr.h"
#include "songdialog.h"
#include "songfile.h"

static Boolean SameText(ConstStringPtr a, ConstStringPtr b)
{
    return a[0] == b[0] && memcmp(a + 1, b + 1, a[0]) == 0;
}

Boolean EditSong_Run(short song, Boolean *rescan)
{
    static SongInfo info, edited;       /* off the small stack */
    SongDetails details;
    ConstStringPtr fileName;
    short vRefNum, i;
    long dirID, audioOffset;
    Boolean changed;
    OSErr err;

    *rescan = false;
    if (song == kNone
        || !Library_SongFile(song, &vRefNum, &dirID, &fileName, &audioOffset)
        || !SongFile_Read(vRefNum, dirID, fileName, &info)
        || info.id != Library_SongID(song))
    {
        Message_Show(P("The song\xD5s file could not be read."), noErr);
        return false;
    }

    for (i = 0; i < kSongTagCount; i++)
        BlockMove(info.text[i], details.text[i], info.text[i][0] + 1);
    details.track = info.track;
    details.audiobook = (info.flags & kSongFlagAudiobook) != 0;

    if (!SongDialog_Run(P("Save"), info.text[kSongStringFileName],
                        info.seconds, &details))
        return false;

    /* Only the details the window shows change; the rating and the
       other flag bits are kept. */
    edited = info;
    for (i = 0; i < kSongTagCount; i++)
        BlockMove(details.text[i], edited.text[i], details.text[i][0] + 1);
    edited.track = details.track;
    edited.flags = details.audiobook ? (info.flags | kSongFlagAudiobook)
                                     : (info.flags & ~kSongFlagAudiobook);

    changed = edited.track != info.track || edited.flags != info.flags;
    for (i = 0; i < kSongTagCount; i++)
        if (!SameText(edited.text[i], info.text[i]))
            changed = true;
    if (!changed)
        return false;

    Library_ForgetCached(song);         /* 1.1: before the file changes */
    err = SongFile_WriteInfo(vRefNum, dirID, fileName, &edited);
    if (err != noErr)
    {
        Message_Show(P("The song\xD5s details couldn\xD5t be saved."), err);
        return false;
    }

    /* L11: playlist files show the title too. */
    if (!SameText(edited.text[0], info.text[0])
        && Library_RetitleInPlaylists(edited.id, edited.text[0]) != noErr)
        Message_Show(P("The song\xD5s details were saved, but its title "
                       "couldn\xD5t be updated in every playlist."), noErr);

    if (!Library_UpdateSong(song, &edited))
        *rescan = true;
    return true;
}
