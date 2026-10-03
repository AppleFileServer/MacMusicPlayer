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
#include "song_format.h"
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

#define kCopyBufferSize     8192L

typedef struct
{
    Str63 text[kSongStringCount];   /* title, artist, album, genre, composer */
    short track;
    Boolean audiobook;
} SongDetails;

static ConstStringPtr const kFieldNames[kSongStringCount] = {
    P("The title"), P("The artist"), P("The album"), P("The genre"),
    P("The composer")
};

/* ---- Small helpers ---------------------------------------------------- */

static void SetWatchCursor(void)
{
    CursHandle watch = GetCursor(watchCursor);

    if (watch != nil)
        SetCursor(*watch);
}

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

/* Reads the source audio once to compute its checksum (D31). */
static OSErr ChecksumAudio(short vRefNum, ConstStringPtr name,
                           const WavInfo *wav, Ptr buffer,
                           unsigned long *checksum)
{
    SongChecksum sum;
    long remaining = wav->dataLength;
    long count;
    short refNum;
    OSErr err;

    err = OpenDataFork(vRefNum, 0, name, fsRdPerm, &refNum);
    if (err != noErr)
        return err;

    SongChecksum_Init(&sum);
    err = SetFPos(refNum, fsFromStart, wav->dataOffset);
    while (err == noErr && remaining > 0)
    {
        count = remaining < kCopyBufferSize ? remaining : kCopyBufferSize;
        err = FSRead(refNum, &count, buffer);
        if (err == eofErr && count > 0)
            err = noErr;
        if (err == noErr && count == 0)
            err = eofErr;
        if (err == noErr)
        {
            SongChecksum_Add(&sum, (unsigned char *)buffer, count);
            remaining -= count;
        }
    }
    FSClose(refNum);

    *checksum = SongChecksum_Value(&sum);
    return err;
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

    for (i = 0; i < kSongStringCount; i++)
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

/* ---- Writing the song file -------------------------------------------- */

/* Free bytes on a volume. */
static long FreeBytes(short vRefNum)
{
    HParamBlockRec pb;

    memset(&pb, 0, sizeof(pb));
    pb.volumeParam.ioVRefNum = vRefNum;
    if (PBHGetVInfoSync(&pb) != noErr)
        return 0;
    return (long)pb.volumeParam.ioVFrBlk * pb.volumeParam.ioVAlBlkSiz;
}

/* Builds the version-1 header (song_format.h); returns its length. */
static long BuildHeader(unsigned char *h, long id, const SongDetails *d,
                        long audioLength, unsigned long checksum)
{
    long at = kSongOffsetStrings;
    short i;

    for (i = 0; i < kSongStringCount; i++)
    {
        BlockMove(d->text[i], h + at, d->text[i][0] + 1);
        at += 1 + d->text[i][0];
    }
    Put32(h + at, checksum);
    at += 4;

    Put32(h + kSongOffsetMagic, kSongMagic);
    Put16(h + kSongOffsetVersion, kSongFormatVersion);
    Put32(h + kSongOffsetHeaderLength, at);
    Put32(h + kSongOffsetID, id);
    Put16(h + kSongOffsetFlags, kSongFlagHasChecksum
                                | (d->audiobook ? kSongFlagAudiobook : 0));
    Put16(h + kSongOffsetTrack, d->track);
    Put32(h + kSongOffsetDuration,
          (audioLength + kSongSampleRate / 2) / kSongSampleRate);
    Put32(h + kSongOffsetEncoding, kSongEncodingPCMu);
    Put32(h + kSongOffsetSampleRate, kSongSampleRate);
    Put16(h + kSongOffsetBits, kSongBitsPerSample);
    Put16(h + kSongOffsetChannels, kSongChannels);
    Put32(h + kSongOffsetAudioLength, audioLength);
    return at;
}

/* Creates IPODnnnn with a fresh ID: Library_NextSongID() (above every
   existing valid ID), then upwards while the file name is taken. */
static OSErr CreateSongFile(const LibraryFolder *folder, long *id, Str63 name)
{
    OSErr err;
    short attempts;

    *id = Library_NextSongID();
    for (attempts = 0; attempts < 1000; attempts++, (*id)++)
    {
        MakeSongFileName(*id, name);
        err = HCreate(folder->vRefNum, folder->folderID, name,
                      kAppCreator, kSongFileType);
        if (err != dupFNErr)
            return err;
    }
    return dupFNErr;
}

/* Copies the audio after the header, checking it matches the checksum
   taken earlier (the source must not have changed). */
static OSErr CopyAudio(short sourceVRefNum, ConstStringPtr sourceName,
                       const WavInfo *wav, short destRefNum, Ptr buffer,
                       unsigned long expected)
{
    SongChecksum sum;
    long remaining = wav->dataLength;
    long count, written;
    short sourceRefNum;
    OSErr err;

    err = OpenDataFork(sourceVRefNum, 0, sourceName, fsRdPerm, &sourceRefNum);
    if (err != noErr)
        return err;

    SongChecksum_Init(&sum);
    err = SetFPos(sourceRefNum, fsFromStart, wav->dataOffset);
    while (err == noErr && remaining > 0)
    {
        count = remaining < kCopyBufferSize ? remaining : kCopyBufferSize;
        err = FSRead(sourceRefNum, &count, buffer);
        if (err == eofErr && count > 0)
            err = noErr;
        if (err == noErr && count == 0)
            err = eofErr;
        if (err != noErr)
            break;

        SongChecksum_Add(&sum, (unsigned char *)buffer, count);
        written = count;
        err = FSWrite(destRefNum, &written, buffer);
        remaining -= count;
    }
    FSClose(sourceRefNum);

    if (err == noErr && SongChecksum_Value(&sum) != expected)
        err = ioErr;
    return err;
}

/* Reopens the new file and checks its length and header. */
static Boolean VerifySongFile(const LibraryFolder *folder, ConstStringPtr name,
                              const unsigned char *header, long headerLength,
                              long audioLength)
{
    unsigned char check[400];
    long length, count = headerLength;
    short refNum;
    Boolean ok;

    if (OpenDataFork(folder->vRefNum, folder->folderID, name, fsRdPerm, &refNum)
        != noErr)
        return false;

    ok = GetEOF(refNum, &length) == noErr
      && length == headerLength + audioLength
      && FSRead(refNum, &count, check) == noErr
      && count == headerLength
      && memcmp(check, header, headerLength) == 0;
    FSClose(refNum);
    return ok;
}

static Boolean WriteSong(const LibraryFolder *folder, short sourceVRefNum,
                         ConstStringPtr sourceName, const WavInfo *wav,
                         const SongDetails *details, unsigned long checksum,
                         Ptr buffer)
{
    unsigned char header[400];
    Str63 name;
    long id, headerLength, count;
    short refNum = 0;
    Boolean created = false;
    OSErr err;

    /* D30: the only size limit is free space on the library's disk. */
    headerLength = BuildHeader(header, 0, details, wav->dataLength, checksum);
    if (headerLength + wav->dataLength > FreeBytes(folder->vRefNum))
    {
        Message_Show(P("There is not enough space on the iPod Library\xD5s "
                       "disk for this song."), noErr);
        return false;
    }

    err = CreateSongFile(folder, &id, name);
    if (err == noErr)
    {
        created = true;
        headerLength = BuildHeader(header, id, details, wav->dataLength, checksum);
        err = OpenDataFork(folder->vRefNum, folder->folderID, name,
                           fsRdWrPerm, &refNum);
    }
    if (err == noErr)
    {
        count = headerLength;
        err = FSWrite(refNum, &count, header);
    }
    if (err == noErr)
    {
        SetWatchCursor();
        err = CopyAudio(sourceVRefNum, sourceName, wav, refNum, buffer, checksum);
        InitCursor();
    }
    if (err == noErr)
        err = SetEOF(refNum, headerLength + wav->dataLength);
    if (refNum != 0)
        FSClose(refNum);
    if (err == noErr)
        err = FlushVol(nil, folder->vRefNum);
    if (err == noErr
        && !VerifySongFile(folder, name, header, headerLength, wav->dataLength))
        err = ioErr;

    if (err != noErr)
    {
        /* Never leave a half-written song in the library. */
        if (created)
            HDelete(folder->vRefNum, folder->folderID, name);
        Message_Show(P("The song could not be imported."), err);
        return false;
    }
    return true;
}

/* ---- Import Song... --------------------------------------------------- */

Boolean Import_Run(const LibraryFolder *folder)
{
    SFReply reply;
    WavInfo wav;
    SongDetails details;
    Str255 problem;
    Point where;
    Ptr buffer;
    unsigned long checksum;
    short match;
    Boolean imported;
    OSErr err;

    /* Any file can be chosen: WAVs are recognised by their contents. */
    where.h = 82;
    where.v = 50;
    SFGetFile(where, P(""), nil, -1, nil, nil, &reply);
    if (!reply.good)
        return false;

    if (!Wav_Read(reply.vRefNum, 0, reply.fName, &wav, problem))
    {
        Message_Show(problem, noErr);
        return false;
    }

    buffer = NewPtr(kCopyBufferSize);
    if (buffer == nil)
    {
        Message_Show(P("There is not enough memory to import a song."), noErr);
        return false;
    }

    /* DECISION D31: warn if the same audio is already in the library. */
    SetWatchCursor();
    err = ChecksumAudio(reply.vRefNum, reply.fName, &wav, buffer, &checksum);
    InitCursor();
    if (err != noErr)
    {
        DisposePtr(buffer);
        Message_Show(P("The WAV file could not be read."), err);
        return false;
    }

    match = Library_FindAudioMatch(wav.dataLength, checksum);
    if (match != kNone)
    {
        ParamText(Library_SongTitle(match), P(""), P(""), P(""));
        if (Alert(kDuplicateAlert, nil) != kItemImportAnyway)
        {
            DisposePtr(buffer);
            return false;
        }
    }

    imported = AskForDetails(reply.fName, &wav, &details)
            && WriteSong(folder, reply.vRefNum, reply.fName, &wav, &details,
                         checksum, buffer);

    DisposePtr(buffer);
    return imported;
}
