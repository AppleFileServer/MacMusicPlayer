#include <Files.h>
#include <Memory.h>
#include <string.h>

#include "songfile.h"
#include "bytes.h"
#include "fileopen.h"

/* Read buffer for the end of a file; static to keep it off the stack. */
static unsigned char gTail[kSongMaxChunk];

/* Reads count bytes at offset; false unless all were read. */
static Boolean ReadAt(short refNum, long offset, long count, void *buffer)
{
    long n = count;

    return SetFPos(refNum, fsFromStart, offset) == noErr
        && FSRead(refNum, &n, buffer) == noErr
        && n == count;
}

static OSErr WriteAt(short refNum, long offset, long count, const void *buffer)
{
    long n = count;
    OSErr err = SetFPos(refNum, fsFromStart, offset);

    if (err == noErr)
        err = FSWrite(refNum, &n, buffer);
    if (err == noErr && n != count)
        err = ioErr;
    return err;
}

/* Checks the chunk at p (length bytes, header included) and fills info. */
static Boolean ParseChunk(const unsigned char *p, long length,
                          long chunkStart, long fileLength, SongInfo *info)
{
    const unsigned char *payload = p + 8;
    long size = (long)GetLE32(p + 4);
    long stringsEnd, at, expectedSeconds;
    short i;

    if (Get32(p) != kSongChunkID || size != length - 8
        || size < kSongMinPayload || (size & 1)
        || Get16(payload + kSongOffsetVersion) != kSongChunkVersion)
        return false;

    if (Get32(payload + kSongOffsetEncoding) != kSongEncodingPCMu
        || Get32(payload + kSongOffsetSampleRate) != kSongSampleRate
        || Get16(payload + kSongOffsetBits) != kSongBitsPerSample
        || Get16(payload + kSongOffsetChannels) != kSongChannels)
        return false;

    info->id = (long)Get32(payload + kSongOffsetID);
    info->flags = (short)Get16(payload + kSongOffsetFlags);
    info->track = (short)Get16(payload + kSongOffsetTrack);
    info->seconds = (long)Get32(payload + kSongOffsetDuration);
    info->audioOffset = (long)Get32(payload + kSongOffsetAudioOffset);
    info->audioLength = (long)Get32(payload + kSongOffsetAudioLength);
    info->fingerprint = Get32(payload + kSongOffsetFingerprint);
    info->originalLength = (long)Get32(payload + kSongOffsetOrigLength);
    info->originalRIFFSize = (long)Get32(payload + kSongOffsetOrigRIFFSize);
    info->originalType = Get32(payload + kSongOffsetOrigType);
    info->originalCreator = Get32(payload + kSongOffsetOrigCreator);

    /* The chunk follows the original file (plus a pad byte if its length
       was odd), and the audio lies within the original file. */
    if (info->id <= 0 || info->track < 0
        || info->originalLength < 12
        || chunkStart != info->originalLength + (info->originalLength & 1)
        || chunkStart + length != fileLength
        || info->audioLength <= 0 || info->audioOffset < 20
        || info->audioOffset > info->originalLength - info->audioLength)
        return false;

    expectedSeconds = (info->audioLength + kSongSampleRate / 2) / kSongSampleRate;
    if (info->seconds != expectedSeconds)
        return false;

    stringsEnd = size - kSongTrailerSize;
    at = kSongOffsetStrings;
    for (i = 0; i < kSongStringCount; i++)
    {
        if (at >= stringsEnd || payload[at] > kSongMaxString
            || at + 1 + payload[at] > stringsEnd)
            return false;
        BlockMove(payload + at, info->text[i], payload[at] + 1);
        at += 1 + payload[at];
    }
    /* An HFS file name: 1 to 31 characters. */
    i = info->text[kSongStringFileName][0];
    return i >= 1 && i <= 31;
}

Boolean SongFile_Read(short vRefNum, long dirID, ConstStringPtr name,
                      SongInfo *info)
{
    unsigned char head[12];
    long fileLength, tailLength, chunkStart;
    short refNum;
    Boolean ok;

    if (OpenDataFork(vRefNum, dirID, name, fsRdPerm, &refNum) != noErr)
        return false;

    ok = GetEOF(refNum, &fileLength) == noErr
      && fileLength >= 12 + 8 + kSongMinPayload
      && ReadAt(refNum, 0, 12, head)
      && Get32(head) == 'RIFF' && Get32(head + 8) == 'WAVE'
      && (long)GetLE32(head + 4) == fileLength - 8;

    /* The trailer, at the very end, says where the chunk starts. */
    if (ok)
    {
        tailLength = fileLength - 12 < kSongMaxChunk ? fileLength - 12
                                                     : kSongMaxChunk;
        ok = ReadAt(refNum, fileLength - tailLength, tailLength, gTail)
          && Get32(gTail + tailLength - 4) == kSongChunkID;
    }
    if (ok)
    {
        chunkStart = (long)Get32(gTail + tailLength - 8);
        ok = chunkStart >= 12 && fileLength - chunkStart <= tailLength
          && ParseChunk(gTail + tailLength - (fileLength - chunkStart),
                        fileLength - chunkStart, chunkStart, fileLength, info);
    }

    /* The audio is the WAV's "data" chunk. */
    if (ok)
        ok = ReadAt(refNum, info->audioOffset - 8, 8, head)
          && Get32(head) == 'data'
          && (long)GetLE32(head + 4) == info->audioLength;

    FSClose(refNum);
    return ok;
}

/* Opens a song file for writing and finds its chunk: the trailer gives
   the chunk's start, whose header and song ID must match. */
static OSErr OpenChunk(short vRefNum, long dirID, ConstStringPtr name,
                       long id, short *refNum, long *chunkStart)
{
    unsigned char bytes[8 + kSongOffsetID + 4];
    long fileLength;
    OSErr err;

    err = OpenDataFork(vRefNum, dirID, name, fsRdWrPerm, refNum);
    if (err != noErr)
        return err;

    err = GetEOF(*refNum, &fileLength);
    if (err == noErr && !(fileLength >= 12 + 8 + kSongMinPayload
                          && ReadAt(*refNum, fileLength - 8, 8, bytes)
                          && Get32(bytes + 4) == kSongChunkID))
        err = badFormat;
    if (err == noErr)
    {
        *chunkStart = (long)Get32(bytes);
        if (*chunkStart < 12 || *chunkStart > fileLength - 8 - kSongMinPayload
            || !ReadAt(*refNum, *chunkStart, sizeof(bytes), bytes)
            || Get32(bytes) != kSongChunkID
            || (long)Get32(bytes + 8 + kSongOffsetID) != id)
            err = badFormat;
    }
    if (err != noErr)
        FSClose(*refNum);
    return err;
}

OSErr SongFile_WriteFlags(short vRefNum, long dirID, ConstStringPtr name,
                          long id, short flags)
{
    unsigned char bytes[2];
    long chunkStart;
    short refNum;
    OSErr err;

    err = OpenChunk(vRefNum, dirID, name, id, &refNum, &chunkStart);
    if (err != noErr)
        return err;
    Put16(bytes, flags);
    err = WriteAt(refNum, chunkStart + 8 + kSongOffsetFlags, 2, bytes);
    FSClose(refNum);
    if (err == noErr)
        err = FlushVol(nil, vRefNum);
    return err;
}

OSErr SongFile_WriteInfo(short vRefNum, long dirID, ConstStringPtr name,
                         const SongInfo *info)
{
    long chunkStart;
    short refNum;
    OSErr err;

    err = OpenChunk(vRefNum, dirID, name, info->id, &refNum, &chunkStart);
    if (err != noErr)
        return err;

    /* The chunk always starts straight after the original file. */
    if (chunkStart != info->originalLength + (info->originalLength & 1))
        err = badFormat;
    if (err == noErr)
        err = SongFile_AddChunk(refNum, info);
    FSClose(refNum);
    if (err == noErr)
        err = FlushVol(nil, vRefNum);
    return err;
}

long SongFile_BuildChunk(const SongInfo *info, long chunkStart,
                         unsigned char *out)
{
    unsigned char *payload = out + 8;
    long at = kSongOffsetStrings;
    short i;

    Put16(payload + kSongOffsetVersion, kSongChunkVersion);
    Put32(payload + kSongOffsetID, info->id);
    Put16(payload + kSongOffsetFlags, info->flags);
    Put16(payload + kSongOffsetTrack, info->track);
    Put32(payload + kSongOffsetDuration,
          (info->audioLength + kSongSampleRate / 2) / kSongSampleRate);
    Put32(payload + kSongOffsetEncoding, kSongEncodingPCMu);
    Put32(payload + kSongOffsetSampleRate, kSongSampleRate);
    Put16(payload + kSongOffsetBits, kSongBitsPerSample);
    Put16(payload + kSongOffsetChannels, kSongChannels);
    Put32(payload + kSongOffsetAudioOffset, info->audioOffset);
    Put32(payload + kSongOffsetAudioLength, info->audioLength);
    Put32(payload + kSongOffsetFingerprint, info->fingerprint);
    Put32(payload + kSongOffsetOrigLength, info->originalLength);
    Put32(payload + kSongOffsetOrigRIFFSize, info->originalRIFFSize);
    Put32(payload + kSongOffsetOrigType, info->originalType);
    Put32(payload + kSongOffsetOrigCreator, info->originalCreator);

    for (i = 0; i < kSongStringCount; i++)
    {
        BlockMove(info->text[i], payload + at, info->text[i][0] + 1);
        at += 1 + info->text[i][0];
    }
    if (at & 1)
        payload[at++] = 0;
    Put32(payload + at, chunkStart);
    Put32(payload + at + 4, kSongChunkID);
    at += kSongTrailerSize;

    Put32(out, kSongChunkID);
    PutLE32(out + 4, at);
    return 8 + at;
}

OSErr SongFile_AddChunk(short refNum, const SongInfo *info)
{
    unsigned char *chunk = gTail;
    unsigned char size[4];
    unsigned char pad = 0;
    long chunkStart = info->originalLength + (info->originalLength & 1);
    long length = SongFile_BuildChunk(info, chunkStart, chunk);
    OSErr err = noErr;

    if (info->originalLength & 1)
        err = WriteAt(refNum, info->originalLength, 1, &pad);
    if (err == noErr)
        err = WriteAt(refNum, chunkStart, length, chunk);
    if (err == noErr)
        err = SetEOF(refNum, chunkStart + length);
    if (err == noErr)
    {
        PutLE32(size, chunkStart + length - 8);
        err = WriteAt(refNum, 4, 4, size);
    }
    return err;
}

OSErr SongFile_RemoveChunk(short refNum, const SongInfo *info)
{
    unsigned char size[4];
    OSErr err = SetEOF(refNum, info->originalLength);

    if (err == noErr)
    {
        PutLE32(size, info->originalRIFFSize);
        err = WriteAt(refNum, 4, 4, size);
    }
    return err;
}

OSErr SongFile_Fingerprint(short refNum, long audioOffset, long audioLength,
                           Ptr buffer, unsigned long *fingerprint)
{
    SongChecksum sum;
    long starts[3], at, end, count;
    short i, samples;

    if (audioLength <= 3 * kSongFingerprintSample)
    {
        samples = 1;
        starts[0] = 0;
    }
    else
    {
        samples = 3;
        starts[0] = 0;
        starts[1] = (audioLength - kSongFingerprintSample) / 2;
        starts[2] = audioLength - kSongFingerprintSample;
    }

    SongChecksum_Init(&sum);
    for (i = 0; i < samples; i++)
    {
        at = starts[i];
        end = samples == 1 ? audioLength : at + kSongFingerprintSample;
        while (at < end)
        {
            count = end - at < kSongFingerprintSample ? end - at
                                                      : kSongFingerprintSample;
            if (!ReadAt(refNum, audioOffset + at, count, buffer))
                return ioErr;
            SongChecksum_Add(&sum, (unsigned char *)buffer, count);
            at += count;
        }
    }
    *fingerprint = SongChecksum_Value(&sum);
    return noErr;
}
