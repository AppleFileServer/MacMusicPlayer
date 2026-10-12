#include <Dialogs.h>
#include <Files.h>
#include <Memory.h>
#include <StandardFile.h>
#include <TextUtils.h>
#include <ToolUtils.h>
#include <string.h>

#include "modal.h"
#include "import.h"
#include "importrun.h"
#include "bytes.h"
#include "fileopen.h"
#include "library.h"
#include "message.h"
#include "pstr.h"
#include "fileops.h"
#include "songfile.h"
#include "songdialog.h"
#include "wav.h"

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

/* 1.1 (N11-25): how one song of a run ended. */
typedef enum
{
    kImported,      /* in the library: on to the next */
    kSkipped,       /* not imported (cancelled, or a problem): on to the next */
    kEndRun         /* the Open dialog cancelled, or nothing more can go in */
} ImportOutcome;

/* 1.1 (N11-25): after each song of a run, "Import another song?" (143). */
enum
{
    kAnotherAlert = 143,
    kItemImportAnother = 1
};

/* Copy buffer: larger transfers are much faster than 8 KB ones; at least
   kSongFingerprintSample, which the fingerprint reads with it. */
#define kCopyBufferSize     32768L

/* ---- Small helpers ---------------------------------------------------- */

/* "IPOD" + the ID, at least four digits. */
static void MakeSongFileName(long id, Str63 name)
{
    Str255 digits, built;   /* built in a Str255: PStrAppend's size */
    short pad;

    NumToString(id, digits);
    built[0] = 0;
    PStrAppend(built, P("IPOD"));
    for (pad = digits[0]; pad < 4; pad++)
        PStrAppend(built, P("0"));
    PStrAppend(built, digits);
    BlockMove(built, name, built[0] + 1);
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

/* Shows the song window for a new song. Returns true with the details if
   the user clicks Import, false on Cancel. */
static Boolean AskForDetails(ConstStringPtr fileName, const WavInfo *wav,
                             SongDetails *details)
{
    short i;

    /* DECISION D28: defaults from the WAV's INFO tags; without a title
       tag, the file name minus ".wav". */
    if (wav->title[0] > 0)
        BlockMove(wav->title, details->text[0], wav->title[0] + 1);
    else
        TitleFromFileName(fileName, details->text[0]);
    BlockMove(wav->artist, details->text[1], wav->artist[0] + 1);
    BlockMove(wav->album, details->text[2], wav->album[0] + 1);
    BlockMove(wav->genre, details->text[3], wav->genre[0] + 1);
    for (i = 4; i < kSongTagCount; i++)
        details->text[i][0] = 0;
    details->track = wav->track > 0 ? wav->track : 0;
    details->audiobook = false;

    return SongDialog_Run(P("Import"), fileName,
                          (wav->dataLength + kSongSampleRate / 2)
                              / kSongSampleRate,
                          details);
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

    /* M4: IDs are never reused, so the library's counter wins if it is
       above every ID present. */
    *id = Library_NextSongID();
    if (Folder_ReadNextID(folder) > *id)
        *id = Folder_ReadNextID(folder);
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
static void ShowLibraryFull(void)
{
    Message_Show(P("The MacMusic Library is full (500 songs). Export a song "
                   "to make room."), noErr);
}

static Boolean AskToCopyInstead(ConstStringPtr reason)
{
    ParamText(reason, P(""), P(""), P(""));
    return Alert(kCantMoveAlert, Modal_DialogFilter) == kItemCopyInstead;
}

static ImportOutcome ImportOne(const LibraryFolder *folder)
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
    Boolean sameDisk, move;
    DialogPtr working;
    OSErr err;

    /* Stage 8d (M3): a full library is refused before anything happens;
       1.1 (N11-25): counting the songs imported earlier in the run. */
    if (ImportRun_LibraryFull())
    {
        ShowLibraryFull();
        return kEndRun;
    }

    /* Any file can be chosen: WAVs are recognised by their contents. */
    where.h = 82;
    where.v = 50;
    SFPGetFile(where, P(""), nil, -1, nil, nil, &reply, getDlgID,
               Modal_FileFilter);
    if (!reply.good)
        return kEndRun;

    if (ResolveWD(reply.vRefNum, &src.vRefNum, &src.dirID) != noErr)
    {
        Message_Show(P("The file could not be found."), noErr);
        return kSkipped;
    }
    BlockMove(reply.fName, src.name, reply.fName[0] + 1);

    if (src.vRefNum == folder->vRefNum
        && (src.dirID == folder->folderID || src.dirID == folder->playlistsID))
    {
        Message_Show(P("This file is already inside the MacMusic Library folder."),
                     noErr);
        return kSkipped;
    }

    if (!Wav_Read(src.vRefNum, src.dirID, src.name, &wav, problem))
    {
        Message_Show(problem, noErr);
        return kSkipped;
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
        return kEndRun;
    }

    /* DECISION D31: warn if the same audio is already in the library (1.1,
       N11-25: or was imported earlier in the run). */
    err = ReadSource(&src, &wav);
    if (err == noErr)
        err = FingerprintSource(&src, &wav, buffer, &fingerprint);
    if (err != noErr)
    {
        DisposePtr(buffer);
        Message_Show(P("The WAV file could not be read."), err);
        return kSkipped;
    }

    if (ImportRun_FindDuplicate(wav.dataLength, fingerprint, problem))
    {
        ParamText(problem, P(""), P(""), P(""));
        if (Alert(kDuplicateAlert, Modal_DialogFilter) != kItemImportAnyway)
        {
            DisposePtr(buffer);
            return kSkipped;
        }
    }

    /* Stage 5b-2: from another disk the file has to be copied, which is
       slow, so ask first. Either way the file is moved unless the user
       chooses Just Copy. */
    sameDisk = src.vRefNum == folder->vRefNum;
    if (!sameDisk && Alert(kOtherDiskAlert, Modal_DialogFilter) != kItemOtherDiskImport)
    {
        DisposePtr(buffer);
        return kSkipped;
    }
    move = Alert(kMoveAlert, Modal_DialogFilter) == kItemMove;

    if (move && sameDisk && src.locked)
    {
        if (!AskToCopyInstead(P("This file is locked, so it can\xD5t be moved.")))
        {
            DisposePtr(buffer);
            return kSkipped;
        }
        move = false;
    }

    if (!AskForDetails(src.name, &wav, &details))
    {
        DisposePtr(buffer);
        return kSkipped;
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
        return kSkipped;
    }

    DisposePtr(buffer);
    if (err == dskFulErr)
    {
        Message_Show(P("There is not enough space on the MacMusic Library\xD5s "
                       "disk for this song."), noErr);
        return kEndRun;
    }
    if (err != noErr)
    {
        Message_Show(P("The song could not be imported."), err);
        return kSkipped;
    }
    ImportRun_Add(wav.dataLength, fingerprint, details.text[0]);
    Folder_WriteNextID(folder, id + 1);

    /* From another disk, "move" = copy, then delete the original (E4: if
       that fails, the original is left where it was). */
    if (move && !sameDisk
        && HDelete(src.vRefNum, src.dirID, src.name) != noErr)
        Message_Show(P("The song was imported, but the original file "
                       "couldn\xD5t be removed from its disk, so it was "
                       "left in place."), noErr);
    else if (move && !sameDisk)
        FlushVol(nil, src.vRefNum);

    return kImported;
}

/* 1.1 (N11-25): songs imported one after another, each chosen in the Open
   dialog. After each, imported or skipped (Cancel in its details window,
   or one of the alerts), "Import another song?": Done, or Cancel in the
   Open dialog, ends the run, as does a full library or disk. Each song is
   in the library as soon as it's imported. DECISION (the owner,
   2026-10-13): the question is asked before the Open dialog comes back. */
Boolean Import_Run(const LibraryFolder *folder)
{
    short imported;

    ImportRun_Begin();
    while (ImportOne(folder) != kEndRun)
    {
        if (ImportRun_LibraryFull())
        {
            ShowLibraryFull();          /* no question: nothing more fits */
            break;
        }
        if (Alert(kAnotherAlert, Modal_DialogFilter) != kItemImportAnother)
            break;
    }
    imported = ImportRun_Count();
    ImportRun_End();
    return imported > 0;
}
