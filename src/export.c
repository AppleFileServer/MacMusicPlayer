#include <Files.h>
#include <Memory.h>
#include <StandardFile.h>
#include <TextUtils.h>
#include <string.h>

#include "modal.h"
#include "export.h"
#include "bytes.h"
#include "fileopen.h"
#include "fileops.h"
#include "library.h"
#include "message.h"
#include "pstr.h"
#include "songfile.h"

#define kCopyBufferSize     32768L
#define kMinBufferSize      4096L

/* A name not used in folder dirID, for the exported file while it is
   being put in place (E1): the song file's own name, else with "-n". */
static OSErr ChooseTempName(short vRefNum, long dirID, ConstStringPtr base,
                            Str255 name)
{
    Str255 number;
    short n;

    BlockMove(base, name, base[0] + 1);
    for (n = 1; FileExists(vRefNum, dirID, name); n++)
    {
        if (n > 99)
            return dupFNErr;
        BlockMove(base, name, base[0] + 1);
        NumToString(n, number);
        PStrAppend(name, P("-"));
        PStrAppend(name, number);
    }
    return noErr;
}

/* Gives the exported file (named temp in the destination folder) its
   final name; with replace, the existing file of that name is deleted
   first. The new file is complete by now, so nothing is lost if this
   fails (E1). */
static OSErr GiveFinalName(short vRefNum, long dirID, ConstStringPtr temp,
                           ConstStringPtr name, Boolean replace)
{
    OSErr err = noErr;

    if (EqualString(temp, name, false, true))
        return noErr;
    if (replace)
        err = HDelete(vRefNum, dirID, name);
    if (err == noErr)
        err = RenameFile(vRefNum, dirID, temp, name);
    return err;
}

/* Same disk: the song file is turned back into the original WAV where it
   is, then moved out of the library; the audio is never copied. On a
   failure before the file has left the library, it is restored. Once it
   has left, the export has happened: if it can't then be given its
   final name, *renamed is false and it keeps the song file's name. */
static OSErr MoveOut(short vRefNum, long dirID, ConstStringPtr songName,
                     const SongInfo *info, long destDirID,
                     ConstStringPtr destName, Boolean replace,
                     Boolean *renamed)
{
    OSErr err;

    *renamed = true;

    err = ChangeChunk(vRefNum, dirID, songName, info, false);
    if (err == noErr)
    {
        SetFileType(vRefNum, dirID, songName, info->originalType,
                    info->originalCreator);
        err = MoveFile(vRefNum, dirID, songName, destDirID);
        if (err != noErr)
            SetFileType(vRefNum, dirID, songName, kSongFileType, kAppCreator);
    }
    if (err != noErr)
    {
        ChangeChunk(vRefNum, dirID, songName, info, true);
        return err;
    }

    *renamed = GiveFinalName(vRefNum, destDirID, songName, destName,
                             replace) == noErr;
    FlushVol(nil, vRefNum);
    return noErr;
}

/* Another disk: copies the original WAV's bytes (the song file without
   its chunk, with the RIFF size put back) to the destination, checks the
   copy, then deletes the song file. *removed is false if the song file
   could not be deleted afterwards. */
static OSErr CopyOut(short vRefNum, long dirID, ConstStringPtr songName,
                     const SongInfo *info, short destVRefNum, long destDirID,
                     ConstStringPtr destName, Boolean replace,
                     Boolean *removed)
{
    unsigned char size[4];
    Str255 temp;
    Ptr buffer;
    long bufferSize = kCopyBufferSize, remaining = info->originalLength;
    long count, written, length;
    short srcRef = 0, destRef = 0;
    OSErr err;

    *removed = false;
    if (info->originalLength > FreeBytes(destVRefNum))
        return dskFulErr;

    buffer = NewPtr(bufferSize);
    if (buffer == nil)
    {
        bufferSize = kMinBufferSize;
        buffer = NewPtr(bufferSize);
    }
    if (buffer == nil)
        return memFullErr;

    err = ChooseTempName(destVRefNum, destDirID, songName, temp);
    if (err == noErr)
        err = HCreate(destVRefNum, destDirID, temp, info->originalCreator,
                      info->originalType);
    if (err != noErr)
    {
        DisposePtr(buffer);
        return err;
    }

    err = OpenDataFork(vRefNum, dirID, songName, fsRdPerm, &srcRef);
    if (err == noErr)
        err = OpenDataFork(destVRefNum, destDirID, temp, fsRdWrPerm, &destRef);
    while (err == noErr && remaining > 0)
    {
        count = remaining < bufferSize ? remaining : bufferSize;
        err = FSRead(srcRef, &count, buffer);
        if (err == eofErr && count > 0)
            err = noErr;
        if (err == noErr && count == 0)
            err = eofErr;
        if (err != noErr)
            break;
        written = count;
        err = FSWrite(destRef, &written, buffer);
        remaining -= count;
    }
    if (err == noErr)
    {
        PutLE32(size, info->originalRIFFSize);
        count = 4;
        err = SetFPos(destRef, fsFromStart, 4);
        if (err == noErr)
            err = FSWrite(destRef, &count, size);
    }
    if (err == noErr)
        err = GetEOF(destRef, &length);
    if (err == noErr && length != info->originalLength)
        err = ioErr;
    if (srcRef != 0)
        FSClose(srcRef);
    if (destRef != 0)
        FSClose(destRef);
    DisposePtr(buffer);
    if (err == noErr)
        err = FlushVol(nil, destVRefNum);

    if (err != noErr)
    {
        HDelete(destVRefNum, destDirID, temp);
        return err;
    }

    err = GiveFinalName(destVRefNum, destDirID, temp, destName, replace);
    if (err != noErr)
        HDelete(destVRefNum, destDirID, temp);
    FlushVol(nil, destVRefNum);
    if (err != noErr)
        return err;

    *removed = HDelete(vRefNum, dirID, songName) == noErr;
    FlushVol(nil, vRefNum);
    return noErr;
}

Boolean Export_Run(const LibraryFolder *folder, short song)
{
    static SongInfo info;           /* off the small stack */
    SFReply reply;
    Str63 songName;
    ConstStringPtr fileName;
    Point where;
    short vRefNum, destVRefNum;
    long dirID, destDirID, audioOffset;
    Boolean replace, removed = true, renamed = true;
    DialogPtr working;
    OSErr err;

    if (song == kNone
        || !Library_SongFile(song, &vRefNum, &dirID, &fileName, &audioOffset))
        return false;
    BlockMove(fileName, songName, fileName[0] + 1);

    if (!SongFile_Read(vRefNum, dirID, songName, &info))
    {
        Message_Show(P("The song\xD5s file could not be read."), noErr);
        return false;
    }

    /* Plan item 4: a Save dialog, starting with the original name. */
    where.h = 104;
    where.v = 60;
    SFPPutFile(where, P("Export song as:"), info.text[kSongStringFileName],
               nil, &reply, putDlgID, Modal_FileFilter);
    if (!reply.good)
        return false;

    if (ResolveWD(reply.vRefNum, &destVRefNum, &destDirID) != noErr)
    {
        Message_Show(P("That folder could not be found."), noErr);
        return false;
    }
    if (destVRefNum == folder->vRefNum
        && (destDirID == folder->folderID || destDirID == folder->playlistsID))
    {
        Message_Show(P("Choose a folder outside the iPod Library."), noErr);
        return false;
    }

    /* Standard File has already asked whether to replace it (E1). */
    replace = FileExists(destVRefNum, destDirID, reply.fName);

    working = Message_BeginWorking(P("Exporting\xC9"));
    if (destVRefNum == vRefNum)
        err = MoveOut(vRefNum, dirID, songName, &info, destDirID,
                      reply.fName, replace, &renamed);
    else
        err = CopyOut(vRefNum, dirID, songName, &info, destVRefNum, destDirID,
                      reply.fName, replace, &removed);

    /* Plan item 5: the song leaves the library, so its playlist lines
       go too; done last, so a failed export loses nothing. */
    if (err == noErr && removed)
        Library_RemoveFromPlaylists(info.id);
    Message_EndWorking(working);

    if (err == dskFulErr)
    {
        Message_Show(P("There is not enough space on that disk."), noErr);
        return false;
    }
    if (err != noErr)
    {
        Message_Show(P("The song could not be exported."), err);
        return false;
    }
    if (!removed)
        Message_Show(P("The song was exported, but its file couldn\xD5t be "
                       "removed from the iPod Library."), noErr);
    if (!renamed)
        Message_Show(P("The song was exported, but it couldn\xD5t be given "
                       "the name you chose, so it kept its iPod Library "
                       "name."), noErr);
    return removed;
}
