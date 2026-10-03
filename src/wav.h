#ifndef WAV_H
#define WAV_H

#include <MacTypes.h>

/* What the importer needs from a WAV file: where its audio is, and any
   LIST/INFO metadata (converted to Mac Roman, at most 63 characters). */
typedef struct
{
    long dataOffset;            /* first audio byte in the data fork */
    long dataLength;            /* audio bytes */
    Str63 title;                /* INAM */
    Str63 artist;               /* IART */
    Str63 album;                /* IPRD */
    Str63 genre;                /* IGNR */
    short track;                /* ITRK; 0 if absent */
} WavInfo;

/* Checks a WAV file. Returns true if it is PCM, mono, 22050 Hz, 8-bit
   unsigned; otherwise false with a sentence explaining why in problem.
   The file (vRefNum may be a working directory) is opened read-only. */
Boolean Wav_Read(short vRefNum, long dirID, ConstStringPtr name,
                 WavInfo *info, Str255 problem);

#endif
