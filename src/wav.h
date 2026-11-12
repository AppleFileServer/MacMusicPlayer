#ifndef WAV_H
#define WAV_H

#include <MacTypes.h>

/* What the importer needs from a WAV file: where its audio is, and any
   LIST/INFO metadata (converted to Mac Roman, at most 63 characters). */
typedef struct
{
    long dataOffset;            /* first audio byte in the data fork */
    long dataLength;            /* audio bytes */
    long sampleRate;            /* 22050 or 22254 (1.2, N12-4) */
    Str63 title;                /* INAM */
    Str63 artist;               /* IART */
    Str63 album;                /* IPRD */
    Str63 genre;                /* IGNR */
    short track;                /* ITRK; 0 if absent */
    long ipodChunkAt;           /* offset of an "iPod" chunk (song_format.h)
                                   left by an earlier import; 0 if none */
} WavInfo;

/* Checks a WAV file. Returns true if it is PCM, mono, 8-bit unsigned, at
   22050 Hz or (1.2, N12-4) 22254 Hz, the Plus's own rate; otherwise false with a sentence explaining why in problem.
   An "iPod" chunk is skipped, and only accepted as the last chunk.
   The file (vRefNum may be a working directory) is opened read-only. */
Boolean Wav_Read(short vRefNum, long dirID, ConstStringPtr name,
                 WavInfo *info, Str255 problem);

#endif
