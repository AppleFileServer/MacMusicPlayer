#include <Dialogs.h>
#include <Files.h>
#include <Memory.h>
#include <StandardFile.h>
#include <TextUtils.h>
#include <ToolUtils.h>
#include <string.h>

#include "import.h"
#include "bytes.h"
#include "fileopen.h"
#include "library.h"
#include "message.h"
#include "pstr.h"
#include "fileops.h"
#include "songfile.h"
#include "wav.h"

/* Import Song window (DLOG/DITL 131). */
enum
{
    kImportDialog = 131,
    kItemImport = 1,
    kItemCancel = 2,
    kItemTitle = 3,             /* edit fields 3-7: title ... composer */
    kItemTrack = 8,
    kItemAudiobook = 9,
    kItemOutline = 17           /* user item: default button outline */
};

/* Already-imported warning (ALRT 132). */
enum
{
    kDuplicateAlert = 132,
    kItemImportAnyway = 2
};

/* Stage 5b-2 alerts: from another disk (135), move or copy (136), and a
   file that can't be moved (138). Item 1 is the default in each. */
enum
{
    kOtherDiskAlert = 135,
    kItemOtherDiskImport = 1,
    kMoveAlert = 136,
    kItemMove = 1,
    kItemJustCopy = 2,
    kCantMoveAlert = 138,
    kItemCopyInstead = 1
};

/* Copy buffer: larger transfers are much faster than 8 KB ones; at least
   kSongFingerprintSample, which the fingerprint reads with it. */
#define kCopyBufferSize     32768L


typedef struct
{
    Str63 text[kSongTagCount];      /* title, artist, album, genre, composer */
    short track;
    Boolean audiobook;
} SongDetails;

static ConstStringPtr const kFieldNames[kSongTagCount] = {
    P("The title"), P("The artist"), P("The album"), P("The genre"),
    P("The composer")
};

/* ---- Small helpers ---------------------------------------------------- */

/* "IPOD" + the ID, at least four digits. */
static void MakeSongFileName(long id, Str63 name)
{
    Str255 digits;
    short pad;

    NumToString(id, digits);
    name[0] = 0;
    PStrAppend(name, P("IPOD"));
    for (pad = digits[0]; pad < 4; pad++)
        PStrAppend(name, P("0"));
    PStrAppend(name, digits);
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

/* The source file name without a ".wav" ending (any case), at most 63. */
static void TitleFromFileName(ConstStringPtr fileName, Str63 title)
{
    short length = fileName[0];

    if (length > 4 && fileName[length - 3] == '.'
        && (fileName[length - 2] | 0x20) == 'w'
        && (fileName[length - 1] | 0x20) == 'a'
        && (fileName[length] | 0x20) == 'v')
        length -= 4;
    if (length > 63)
        length = 63;

    title[0] = length;
    BlockMove(fileName + 1, title + 1, length);
}

/* ---- Import Song window ----------------------------------------------- */

/* Draws the thick outline round the default (Import) button. */
static pascal void DrawDefaultOutline(DialogPtr dialog, short item)
{
    short type;
    Handle handle;
    Rect r;

    (void)item;
    GetDialogItem(dialog, kItemImport, &type, &handle, &r);
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

/* Checks the fields; on a problem explains it, selects the field and
   returns false (DECISION D25: the title may not be blank). */
static Boolean ReadDetails(DialogPtr dialog, SongDetails *details)
{
    Str255 text, message;
    short i;
    long track;

    for (i = 0; i < kSongTagCount; i++)
    {
        GetDialogItemText(ItemHandle(dialog, kItemTitle + i), text);
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

    GetDialogItemText(ItemHandle(dialog, kItemTrack), text);
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
        GetControlValue((ControlHandle)ItemHandle(dialog, kItemAudiobook)) != 0;
    return true;
}

/* Shows the Import Song window. Returns true with the details if the
   user clicks Import, false on Cancel. */
static Boolean AskForDetails(ConstStringPtr fileName, const WavInfo *wav,
                             SongDetails *details)
{
    DialogPtr dialog;
    Str255 duration, number;
    Str63 title;
    short item, type;
    Handle handle;
    ControlHandle checkbox;
    Rect r;
    Boolean ok = false;

    /* DECISION D28: defaults from the WAV's INFO tags; without a title
       tag, the file name minus ".wav". */
    if (wav->title[0] > 0)
        BlockMove(wav->title, title, wav->title[0] + 1);
    else
        TitleFromFileName(fileName, title);

    FormatDuration((wav->dataLength + kSongSampleRate / 2) / kSongSampleRate,
                   duration);
    ParamText(fileName, duration, P(""), P(""));

    dialog = GetNewDialog(kImportDialog, nil, (WindowPtr)-1);
    if (dialog == nil)
    {
        Message_Show(P("The Import Song window could not be opened."), noErr);
        return false;
    }

    SetDialogItemText(ItemHandle(dialog, kItemTitle), title);
    SetDialogItemText(ItemHandle(dialog, kItemTitle + 1), wav->artist);
    SetDialogItemText(ItemHandle(dialog, kItemTitle + 2), wav->album);
    SetDialogItemText(ItemHandle(dialog, kItemTitle + 3), wav->genre);
    if (wav->track > 0)
    {
        NumToString(wav->track, number);
        SetDialogItemText(ItemHandle(dialog, kItemTrack), number);
    }

    GetDialogItem(dialog, kItemOutline, &type, &handle, &r);
    SetDialogItem(dialog, kItemOutline, type, (Handle)DrawDefaultOutline, &r);

    SelectDialogItemText(dialog, kItemTitle, 0, 32767);
    ShowWindow(dialog);

    for (;;)
    {
        ModalDialog(nil, &item);

        if (item == kItemImport)
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
            checkbox = (ControlHandle)ItemHandle(dialog, kItemAudiobook);
            SetControlValue(checkbox, !GetControlValue(checkbox));
        }
    }

    DisposeDialog(dialog);
    return ok;
}

/* ---- The source file ---------------------------------------------------- */

typedef struct
{
    short vRefNum;
    long dirID;
    Str63 name;
    Boolean locked;
    OSType type, creator;
    long length;                /* the file without any old iPod chunk */
    long riffSize;              /* its RIFF size field */
} Source;

/* Fills in the source's Finder information and lengths. */
static OSErr ReadSource(Source *src, const WavInfo *wav)
{
    HParamBlockRec pb;
    unsigned char head[8];
    long count = 8;
    short refNum;
    OSErr err;

    err = GetFileInfo(src->vRefNum, src->dirID, src->name, &pb);
    if (err != noErr)
        return err;
    src->locked = (pb.fileParam.ioFlAttrib & kLockedAttribute) != 0;
    src->type = pb.fileParam.ioFlFndrInfo.fdType;
    src->creator = pb.fileParam.ioFlFndrInfo.fdCreator;

    /* E5: a chunk left by an earlier import is replaced, so the file's
       "original" form is everything before it. */
    if (wav->ipodChunkAt != 0)
    {
        src->length = wav->ipodChunkAt;
        src->riffSize = wav->ipodChunkAt - 8;
        return noErr;
    }

    err = OpenDataFork(src->vRefNum, src->dirID, src->name, fsRdPerm, &refNum);
    if (err != noErr)
        return err;
    err = GetEOF(refNum, &src->length);
    if (err == noErr)
        err = FSRead(refNum, &count, head);
    FSClose(refNum);
    src->riffSize = (long)GetLE32(head + 4);
    return err;
}

static OSErr FingerprintSource(const Source *src, const WavInfo *wav,
                               Ptr buffer, unsigned long *fingerprint)
{
    short refNum;
    OSErr err;

    err = OpenDataFork(src->vRefNum, src->dirID, src->name, fsRdPerm, &refNum);
    if (err != noErr)
        return err;
    err = SongFile_Fingerprint(refNum, wav->dataOffset, wav->dataLength,
                               buffer, fingerprint);
    FSClose(refNum);
    return err;
}

/* ---- Writing the song ------------------------------------------------- */

/* A free song ID whose file name is unused in the library (and, when
   moving, in the source's folder, where the file is renamed first):
   Library_NextSongID() (above every existing valid ID), then upwards. */
static OSErr ChooseSongName(const LibraryFolder *folder, const Source *src,
                            Boolean alsoInSource, long *id, Str63 name)
{
    short attempts;

    *id = Library_NextSongID();
    for (attempts = 0; attempts < 1000; attempts++, (*id)++)
    {
        MakeSongFileName(*id, name);
        if (!FileExists(folder->vRefNum, folder->folderID, name)
            && !(alsoInSource && FileExists(src->vRefNum, src->dirID, name)))
            return noErr;
    }
    return dupFNErr;
}

static void FillSongInfo(SongInfo *info, long id, const SongDetails *d,
                         const WavInfo *wav, unsigned long fingerprint,
                         const Source *src)
{
    short i;

    memset(info, 0, sizeof(*info));
    info->id = id;
    info->flags = d->audiobook ? kSongFlagAudiobook : 0;
    info->track = d->track;
    info->audioOffset = wav->dataOffset;
    info->audioLength = wav->dataLength;
    info->fingerprint = fingerprint;
    info->originalLength = src->length;
    info->originalRIFFSize = src->riffSize;
    info->originalType = src->type;
    info->originalCreator = src->creator;
    for (i = 0; i < kSongTagCount; i++)
        BlockMove(d->text[i], info->text[i], d->text[i][0] + 1);
    BlockMove(src->name, info->text[kSongStringFileName], src->name[0] + 1);
}

/* Same disk: adds the chunk to the file where it is, renames it and
   moves it into the library; the audio is never copied. On any failure
   every step done so far is undone, leaving the file as it was. */
static OSErr MoveIntoLibrary(const LibraryFolder *folder, const Source *src,
                             const SongInfo *info, ConstStringPtr songName)
{
    OSErr err;

    err = ChangeChunk(src->vRefNum, src->dirID, src->name, info, true);
    if (err != noErr)
    {
        ChangeChunk(src->vRefNum, src->dirID, src->name, info, false);
        return err;
    }

    err = RenameFile(src->vRefNum, src->dirID, src->name, songName);
    if (err == noErr)
    {
        err = MoveFile(src->vRefNum, src->dirID, songName, folder->folderID);
        if (err != noErr)
            RenameFile(src->vRefNum, src->dirID, songName, src->name);
    }
    if (err != noErr)
    {
        ChangeChunk(src->vRefNum, src->dirID, src->name, info, false);
        return err;
    }

    /* The Finder codes only label the file; a song plays without them. */
    SetFileType(folder->vRefNum, folder->folderID, songName,
                kSongFileType, kAppCreator);
    FlushVol(nil, folder->vRefNum);
    return noErr;
}

/* Copies the source (without any old chunk) into a new library file and
   adds the chunk; the source is not changed. The new file is checked by
   reading it back as the library will; on any failure it is deleted. */
static OSErr CopyIntoLibrary(const LibraryFolder *folder, const Source *src,
                             const SongInfo *info, ConstStringPtr songName,
                             Ptr buffer, long bufferSize)
{
    SongInfo check;
    long remaining = src->length, count, written;
    short srcRef = 0, destRef = 0;
    OSErr err;

    if (src->length + kSongMaxChunk > FreeBytes(folder->vRefNum))
        return dskFulErr;

    err = HCreate(folder->vRefNum, folder->folderID, songName,
                  kAppCreator, kSongFileType);
    if (err != noErr)
        return err;

    err = OpenDataFork(src->vRefNum, src->dirID, src->name, fsRdPerm, &srcRef);
    if (err == noErr)
        err = OpenDataFork(folder->vRefNum, folder->folderID, songName,
                           fsRdWrPerm, &destRef);
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
        err = SongFile_AddChunk(destRef, info);
    if (srcRef != 0)
        FSClose(srcRef);
    if (destRef != 0)
        FSClose(destRef);
    if (err == noErr)
        err = FlushVol(nil, folder->vRefNum);

    if (err == noErr
        && (!SongFile_Read(folder->vRefNum, folder->folderID, songName, &check)
            || check.id != info->id || check.audioLength != info->audioLength
            || check.fingerprint != info->fingerprint))
        err = ioErr;

    if (err != noErr)
        HDelete(folder->vRefNum, folder->folderID, songName);
    return err;
}

/* ---- Import Song... --------------------------------------------------- */

/* E4: the file can't be moved; offers to copy it instead. */
static Boolean AskToCopyInstead(ConstStringPtr reason)
{
    ParamText(reason, P(""), P(""), P(""));
    return Alert(kCantMoveAlert, nil) == kItemCopyInstead;
}

Boolean Import_Run(const LibraryFolder *folder)
{
    SFReply reply;
    Source src;
    WavInfo wav;
    SongDetails details;
    SongInfo info;
    Str255 problem;
    Str63 songName;
    Point where;
    Ptr buffer;
    long bufferSize = kCopyBufferSize, id;
    unsigned long fingerprint;
    short match;
    Boolean sameDisk, move, imported = false;
    DialogPtr working;
    OSErr err;

    /* Any file can be chosen: WAVs are recognised by their contents. */
    where.h = 82;
    where.v = 50;
    SFGetFile(where, P(""), nil, -1, nil, nil, &reply);
    if (!reply.good)
        return false;

    if (ResolveWD(reply.vRefNum, &src.vRefNum, &src.dirID) != noErr)
    {
        Message_Show(P("The file could not be found."), noErr);
        return false;
    }
    BlockMove(reply.fName, src.name, reply.fName[0] + 1);

    if (src.vRefNum == folder->vRefNum
        && (src.dirID == folder->folderID || src.dirID == folder->playlistsID))
    {
        Message_Show(P("This file is already inside the iPod Library folder."),
                     noErr);
        return false;
    }

    if (!Wav_Read(src.vRefNum, src.dirID, src.name, &wav, problem))
    {
        Message_Show(problem, noErr);
        return false;
    }

    buffer = NewPtr(bufferSize);
    if (buffer == nil)
    {
        bufferSize = kSongFingerprintSample;
        buffer = NewPtr(bufferSize);
    }
    if (buffer == nil)
    {
        Message_Show(P("There is not enough memory to import a song."), noErr);
        return false;
    }

    /* DECISION D31: warn if the same audio is already in the library. */
    err = ReadSource(&src, &wav);
    if (err == noErr)
        err = FingerprintSource(&src, &wav, buffer, &fingerprint);
    if (err != noErr)
    {
        DisposePtr(buffer);
        Message_Show(P("The WAV file could not be read."), err);
        return false;
    }

    match = Library_FindAudioMatch(wav.dataLength, fingerprint);
    if (match != kNone)
    {
        ParamText(Library_SongTitle(match), P(""), P(""), P(""));
        if (Alert(kDuplicateAlert, nil) != kItemImportAnyway)
        {
            DisposePtr(buffer);
            return false;
        }
    }

    /* Stage 5b-2: from another disk the file has to be copied, which is
       slow, so ask first. Either way the file is moved unless the user
       chooses Just Copy. */
    sameDisk = src.vRefNum == folder->vRefNum;
    if (!sameDisk && Alert(kOtherDiskAlert, nil) != kItemOtherDiskImport)
    {
        DisposePtr(buffer);
        return false;
    }
    move = Alert(kMoveAlert, nil) == kItemMove;

    if (move && sameDisk && src.locked)
    {
        if (!AskToCopyInstead(P("This file is locked, so it can\xD5t be moved.")))
        {
            DisposePtr(buffer);
            return false;
        }
        move = false;
    }

    if (!AskForDetails(src.name, &wav, &details))
    {
        DisposePtr(buffer);
        return false;
    }

    working = Message_BeginWorking(P("Importing\xC9"));
    err = ChooseSongName(folder, &src, move && sameDisk, &id, songName);
    if (err == noErr)
    {
        FillSongInfo(&info, id, &details, &wav, fingerprint, &src);
        if (move && sameDisk)
            err = MoveIntoLibrary(folder, &src, &info, songName);
        else
            err = CopyIntoLibrary(folder, &src, &info, songName,
                                  buffer, bufferSize);
    }
    Message_EndWorking(working);

    /* E4: open elsewhere, or on a locked disk: nothing was changed. */
    if (move && sameDisk && IsCantChangeError(err)
        && AskToCopyInstead(P("This file is in use or its disk is locked, "
                              "so it can\xD5t be moved.")))
    {
        working = Message_BeginWorking(P("Importing\xC9"));
        err = CopyIntoLibrary(folder, &src, &info, songName, buffer, bufferSize);
        Message_EndWorking(working);
        move = false;
    }
    else if (move && sameDisk && IsCantChangeError(err))
    {
        DisposePtr(buffer);
        return false;
    }

    DisposePtr(buffer);
    if (err == dskFulErr)
    {
        Message_Show(P("There is not enough space on the iPod Library\xD5s "
                       "disk for this song."), noErr);
        return false;
    }
    if (err != noErr)
    {
        Message_Show(P("The song could not be imported."), err);
        return false;
    }
    imported = true;

    /* From another disk, "move" = copy, then delete the original (E4: if
       that fails, the original is left where it was). */
    if (move && !sameDisk
        && HDelete(src.vRefNum, src.dirID, src.name) != noErr)
        Message_Show(P("The song was imported, but the original file "
                       "couldn\xD5t be removed from its disk, so it was "
                       "left in place."), noErr);
    else if (move && !sameDisk)
        FlushVol(nil, src.vRefNum);

    return imported;
}
