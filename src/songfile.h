#ifndef SONGFILE_H
#define SONGFILE_H

#include <MacTypes.h>

#include "song_format.h"

/* Reading and writing the "iPod" chunk of a song file (song_format.h). */

typedef struct
{
    long id;
    short flags;
    short track;
    long seconds;
    long audioOffset;           /* first audio byte in the data fork */
    long audioLength;
    unsigned long fingerprint;
    long originalLength;        /* the file before import */
    long originalRIFFSize;
    OSType originalType;
    OSType originalCreator;
    Str63 text[kSongStringCount];   /* title ... composer, original name */
} SongInfo;

/* Reads and checks a song file (vRefNum may be a working directory).
   False if it is not a valid song file. */
Boolean SongFile_Read(short vRefNum, long dirID, ConstStringPtr name,
                      SongInfo *info);

/* Builds the chunk (header, payload and trailer) for a chunk starting
   at chunkStart; returns its length. out needs kSongMaxChunk bytes. */
long SongFile_BuildChunk(const SongInfo *info, long chunkStart,
                         unsigned char *out);

/* Adds the chunk to the end of a file open for writing, whose length is
   info->originalLength, and updates the RIFF size. */
OSErr SongFile_AddChunk(short refNum, const SongInfo *info);

/* Undoes SongFile_AddChunk: cuts the file back to its original length
   and restores the original RIFF size. */
OSErr SongFile_RemoveChunk(short refNum, const SongInfo *info);

/* The audio fingerprint (D31) of an open file. buffer needs
   kSongFingerprintSample bytes. */
OSErr SongFile_Fingerprint(short refNum, long audioOffset, long audioLength,
                           Ptr buffer, unsigned long *fingerprint);

#endif
