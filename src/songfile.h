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
    long sampleRate;            /* kSongSampleRate or kSongSampleRatePlus */
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

/* Stage 7d: rewrites the chunk's flags field in place (two bytes),
   after checking that the file's chunk is the song with this ID. The
   rest of the file is not touched. Works while the song is playing (the
   audio has its own read-only path to the file). */
OSErr SongFile_WriteFlags(short vRefNum, long dirID, ConstStringPtr name,
                          long id, short flags);

/* Stage 8c: rewrites the whole chunk in place from info (the song's
   details after editing), after checking that the file's chunk is the
   song with info->id. The audio is not touched; the file's length and
   RIFF size follow the new chunk. Works while the song is playing. */
OSErr SongFile_WriteInfo(short vRefNum, long dirID, ConstStringPtr name,
                         const SongInfo *info);

/* The audio fingerprint (D31) of an open file. buffer needs
   kSongFingerprintSample bytes. */
OSErr SongFile_Fingerprint(short refNum, long audioOffset, long audioLength,
                           Ptr buffer, unsigned long *fingerprint);

#endif
