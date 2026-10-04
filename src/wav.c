#include <Files.h>
#include <Memory.h>
#include <TextUtils.h>

#include "wav.h"
#include "bytes.h"
#include "fileopen.h"
#include "message.h"
#include "pstr.h"

/* WAV is RIFF: "RIFF" <size> "WAVE", then chunks of <id> <size> <data>,
   sizes little-endian, each chunk padded to an even length. Chunks may
   come in any order; unknown ones are skipped (DECISION D27). The RIFF
   size field is not required to be exact, but every chunk must lie
   within the file. */

#define kFormatPCM          1
#define kFormatExtensible   0xFFFE
#define kMaxInfoBytes       1024    /* LIST/INFO metadata read, at most */

static unsigned char gInfo[kMaxInfoBytes];

/* Latin-1 0xA0-0xFF to Mac Roman (D28); '?' where Mac Roman has no
   equivalent. Below 0xA0 only printable ASCII is kept. */
static const unsigned char kLatin1ToMacRoman[96] = {
    0xCA, 0xC1, 0xA2, 0xA3, '?',  0xB4, '|',  0xA4,  /* A0-A7 */
    0xAC, 0xA9, 0xBB, 0xC7, 0xC2, '-',  0xA8, 0xF8,  /* A8-AF */
    0xA1, 0xB1, '2',  '3',  0xAB, 0xB5, 0xA6, 0xE1,  /* B0-B7 */
    0xFC, '1',  0xBC, 0xC8, '?',  '?',  '?',  0xC0,  /* B8-BF */
    0xCB, 0xE7, 0xE5, 0xCC, 0x80, 0x81, 0xAE, 0x82,  /* C0-C7 */
    0xE9, 0x83, 0xE6, 0xE8, 0xED, 0xEA, 0xEB, 0xEC,  /* C8-CF */
    '?',  0x84, 0xF1, 0xEE, 0xEF, 0xCD, 0x85, 'x',   /* D0-D7 */
    0xAF, 0xF4, 0xF2, 0xF3, 0x86, '?',  '?',  0xA7,  /* D8-DF */
    0x88, 0x87, 0x89, 0x8B, 0x8A, 0x8C, 0xBE, 0x8D,  /* E0-E7 */
    0x8F, 0x8E, 0x90, 0x91, 0x93, 0x92, 0x94, 0x95,  /* E8-EF */
    '?',  0x96, 0x98, 0x97, 0x99, 0x9B, 0x9A, 0xD6,  /* F0-F7 */
    0xBF, 0x9D, 0x9C, 0x9E, 0x9F, '?',  '?',  0xD8   /* F8-FF */
};

/* Copies a Latin-1 INFO string (NUL-terminated or not) to Mac Roman,
   at most 63 characters. */
static void CopyInfoText(const unsigned char *text, long length, Str63 out)
{
    long i;
    unsigned char c;

    out[0] = 0;
    for (i = 0; i < length && out[0] < 63; i++)
    {
        c = text[i];
        if (c == 0)
            break;
        if (c >= 0xA0)
            c = kLatin1ToMacRoman[c - 0xA0];
        else if (c < 0x20 || c > 0x7E)
            continue;
        out[++out[0]] = c;
    }
}

/* Parses the sub-chunks of a LIST/INFO chunk held in gInfo. */
static void ParseInfo(long length, WavInfo *info)
{
    long at = 4;                    /* after "INFO" */
    long size, n;
    unsigned long id;
    Str63 number;

    while (at + 8 <= length)
    {
        id = Get32(gInfo + at);
        size = (long)GetLE32(gInfo + at + 4);
        at += 8;
        if (size < 0 || size > length - at)
            break;

        if (id == 'INAM')
            CopyInfoText(gInfo + at, size, info->title);
        else if (id == 'IART')
            CopyInfoText(gInfo + at, size, info->artist);
        else if (id == 'IPRD')
            CopyInfoText(gInfo + at, size, info->album);
        else if (id == 'IGNR')
            CopyInfoText(gInfo + at, size, info->genre);
        else if (id == 'ITRK')
        {
            CopyInfoText(gInfo + at, size, number);
            if (number[0] > 0 && number[0] <= 4)
            {
                StringToNum(number, &n);
                if (n > 0 && n <= 9999)
                    info->track = (short)n;
            }
        }

        at += size + (size & 1);
    }
}

/* Builds "This file is stereo, 44,100 Hz, 16-bit." from fmt values. */
static void DescribeFormat(short channels, unsigned long rate, short bits,
                           Str255 problem)
{
    Str255 number;

    problem[0] = 0;
    PStrAppend(problem, P("Only mono, 22050 Hz, 8-bit PCM WAV files can be "
                          "imported. This file is "));
    if (channels == 1)
        PStrAppend(problem, P("mono, "));
    else if (channels == 2)
        PStrAppend(problem, P("stereo, "));
    else
    {
        NumToString(channels, number);
        PStrAppend(problem, number);
        PStrAppend(problem, P(" channels, "));
    }
    NumToString((long)rate, number);
    PStrAppend(problem, number);
    PStrAppend(problem, P(" Hz, "));
    NumToString(bits, number);
    PStrAppend(problem, number);
    PStrAppend(problem, P("-bit."));
}

static Boolean Fail(Str255 problem, ConstStringPtr text)
{
    problem[0] = 0;
    PStrAppend(problem, text);
    return false;
}

Boolean Wav_Read(short vRefNum, long dirID, ConstStringPtr name,
                 WavInfo *info, Str255 problem)
{
    unsigned char header[24];
    long fileLength = 0, count, at, size;
    unsigned long id;
    Boolean haveFormat = false, haveData = false;
    short refNum, channels = 0, bits = 0, blockAlign = 0;
    unsigned short format = 0;
    unsigned long rate = 0, byteRate = 0;
    OSErr err;

    info->dataOffset = 0;
    info->dataLength = 0;
    info->title[0] = info->artist[0] = info->album[0] = info->genre[0] = 0;
    info->track = 0;
    info->ipodChunkAt = 0;

    err = OpenDataFork(vRefNum, dirID, name, fsRdPerm, &refNum);
    if (err != noErr)
        return Fail(problem, P("The file could not be opened."));

    GetEOF(refNum, &fileLength);
    count = 12;
    err = FSRead(refNum, &count, header);
    if (err != noErr || count < 12
        || Get32(header) != 'RIFF' || Get32(header + 8) != 'WAVE')
    {
        FSClose(refNum);
        return Fail(problem, P("This is not a WAV file."));
    }

    /* Walk the chunks, reading only their headers and small bodies. */
    at = 12;
    while (at + 8 <= fileLength)
    {
        SetFPos(refNum, fsFromStart, at);
        count = 8;
        if (FSRead(refNum, &count, header) != noErr || count < 8)
            break;

        id = Get32(header);
        size = (long)GetLE32(header + 4);
        at += 8;
        if (size < 0 || size > fileLength - at)
        {
            FSClose(refNum);
            return Fail(problem, P("This WAV file is damaged or incomplete: "
                                   "a chunk runs past the end of the file."));
        }

        if (id == 'fmt ' && size >= 16)
        {
            count = 16;
            if (FSRead(refNum, &count, header) != noErr || count < 16)
                break;
            format = GetLE16(header);
            channels = GetLE16(header + 2);
            rate = GetLE32(header + 4);
            byteRate = GetLE32(header + 8);
            blockAlign = GetLE16(header + 12);
            bits = GetLE16(header + 14);
            haveFormat = true;
        }
        else if (id == 'data')
        {
            info->dataOffset = at;
            info->dataLength = size;
            haveData = true;
        }
        else if (id == 'iPod')
        {
            /* E5: an earlier import's chunk; it must end the file. */
            if (at + size + (size & 1) != fileLength)
            {
                FSClose(refNum);
                return Fail(problem, P("This WAV file is damaged: it has "
                                       "iPod information that is not at "
                                       "the end of the file."));
            }
            info->ipodChunkAt = at - 8;
        }
        else if (id == 'LIST' && size >= 4)
        {
            count = size < kMaxInfoBytes ? size : kMaxInfoBytes;
            if (FSRead(refNum, &count, gInfo) == noErr && count >= 4
                && Get32(gInfo) == 'INFO')
                ParseInfo(count, info);
        }

        at += size + (size & 1);
    }
    FSClose(refNum);

    if (!haveFormat)
        return Fail(problem, P("This WAV file has no format information."));

    /* 8-bit WAV samples are unsigned by definition, so PCM + 8 bits
       means unsigned. WAVE_FORMAT_EXTENSIBLE is rejected (D27). */
    if (format != kFormatPCM)
        return Fail(problem, format == kFormatExtensible
            ? P("This WAV file uses the extensible format, which can't be "
                "imported. Save it as plain PCM.")
            : P("This WAV file is not plain PCM audio (it may be compressed "
                "or floating-point). Only uncompressed PCM WAV files can be "
                "imported."));

    if (channels != 1 || rate != 22050 || bits != 8)
    {
        DescribeFormat(channels, rate, bits, problem);
        return false;
    }

    if (blockAlign != 1 || byteRate != 22050)
        return Fail(problem, P("This WAV file's format information is "
                               "inconsistent."));

    if (!haveData)
        return Fail(problem, P("This WAV file has no audio data."));
    if (info->dataLength == 0)
        return Fail(problem, P("This WAV file's audio is empty."));

    return true;
}
