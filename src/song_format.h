#ifndef SONG_FORMAT_H
#define SONG_FORMAT_H

#include <MacTypes.h>

/* iPod song file format, version 2 (project-specific, Stage 5b-2).

   A song file is the imported WAV file itself, unchanged except that an
   "iPod" chunk is added at the very end and the RIFF size field (bytes
   4-7) is updated to cover it. The audio is never moved or rewritten,
   so import can move the file into the library instead of copying it,
   and export restores the original file byte for byte by cutting the
   chunk off and putting back the RIFF size.

   File layout:
     0               "RIFF" <size, LE> "WAVE"   size = file length - 8
     12              the original WAV's chunks, untouched
     originalLength  one zero pad byte, only if originalLength is odd
                     (RIFF chunks start on even offsets)
     chunkStart      "iPod" <payload length, LE>
     chunkStart + 8  payload; its length is even, so the chunk ends the
                     file with no RIFF pad byte

   Payload, integers big-endian (bytes.h):

   Offset Size Field
     0     2   chunk version (1)
     2     4   song ID (> 0, unique in the library)
     6     2   flags (bit 0: audiobook; bits 1-3: rating, Stage 7d)
     8     2   track number (0 = none)
    10     4   duration in seconds, = round(audio length / sample rate)
    14     4   encoding 'PCMu': linear PCM, unsigned samples
    18     4   sample rate in Hz: 22050, or 22254 (1.2, N12-4)
    22     2   bits per sample (8)
    24     2   channels (1)
    26     4   audio offset: first byte of the WAV's "data" chunk body
    30     4   audio length in bytes (= the "data" chunk size)
    34     4   audio fingerprint (SongFingerprint, DECISION D31)
    38     4   original file length, before import
    42     4   original RIFF size field, before import
    46     4   original Finder type
    50     4   original Finder creator
    54     -   title, artist, album, genre, composer, original file name:
               each a length byte (0-63) then that many Mac Roman
               characters
     -     -   zero padding to an even length
     -     4   chunkStart (the offset of "iPod"), so a reader can find
               the chunk from the end of the file
     -     4   end mark 'iPod'

   The data fork holds everything; the resource fork is unused. */

#define kSongChunkID            'iPod'
#define kSongChunkVersion       1

#define kSongOffsetVersion      0
#define kSongOffsetID           2
#define kSongOffsetFlags        6
#define kSongOffsetTrack        8
#define kSongOffsetDuration     10
#define kSongOffsetEncoding     14
#define kSongOffsetSampleRate   18
#define kSongOffsetBits         22
#define kSongOffsetChannels     24
#define kSongOffsetAudioOffset  26
#define kSongOffsetAudioLength  30
#define kSongOffsetFingerprint  34
#define kSongOffsetOrigLength   38
#define kSongOffsetOrigRIFFSize 42
#define kSongOffsetOrigType     46
#define kSongOffsetOrigCreator  50
#define kSongOffsetStrings      54

#define kSongStringCount        6       /* the five tags + original name */
#define kSongTagCount           5       /* title ... composer */
#define kSongStringFileName     5
#define kSongMaxString          63
#define kSongTrailerSize        8       /* chunkStart + end mark */
#define kSongMinPayload         (kSongOffsetStrings + kSongStringCount \
                                 + kSongTrailerSize)
#define kSongMaxChunk           1024    /* whole chunk, header included */

#define kSongFlagAudiobook      0x0001

/* Stage 7d (N4): the rating, in flags bits 1-3. 0 = unrated, 1-5 = one
   to five stars; 6 and 7 are invalid and read as unrated. Files from
   before 7d have these bits clear, so the chunk version is unchanged. */
#define kSongFlagRatingMask     0x000E
#define kSongFlagRatingShift    1
#define kSongMaxRating          5

static inline short SongFlags_Rating(short flags)
{
    short rating = (flags & kSongFlagRatingMask) >> kSongFlagRatingShift;

    return rating <= kSongMaxRating ? rating : 0;
}

static inline short SongFlags_WithRating(short flags, short rating)
{
    return (short)((flags & ~kSongFlagRatingMask)
                   | ((rating << kSongFlagRatingShift) & kSongFlagRatingMask));
}

/* DECISION D31 (revised in 5b-2): duplicates are found by audio length
   plus a fingerprint: two running 32-bit sums,
       a += byte; b += a;   fingerprint = (b << 16) ^ a
   over up to three kSongFingerprintSample-byte samples of the audio: the
   start, the middle and the end (the whole audio if it is shorter than
   three samples). Reading 24 KB instead of the whole file keeps import
   fast; audio that differs only elsewhere is not detected as a
   duplicate, which is accepted. */
#define kSongFingerprintSample  8192L

typedef struct
{
    UInt32 a;
    UInt32 b;
} SongChecksum;

static inline void SongChecksum_Init(SongChecksum *c)
{
    c->a = 0;
    c->b = 0;
}

static inline void SongChecksum_Add(SongChecksum *c,
                                    const unsigned char *bytes, long count)
{
    UInt32 a = c->a, b = c->b;

    while (count-- > 0)
    {
        a += *bytes++;
        b += a;
    }
    c->a = a;
    c->b = b;
}

static inline UInt32 SongChecksum_Value(const SongChecksum *c)
{
    return (c->b << 16) ^ c->a;
}

/* The audio format accepted: 8-bit unsigned mono PCM at either of two
   rates. 1.0 and 1.1 took 22050 Hz only. 1.2 (N12-4) also takes 22254
   Hz, the Macintosh Plus's own rate (it plays 22254.5 samples a second),
   as the MacMusic Converter 1.1 makes: such songs play at true speed and
   pitch. Every sample is played at the hardware rate whatever the song's
   rate; a song's rate decides only its times (N12-5). */
#define kSongEncodingPCMu       'PCMu'
#define kSongSampleRate         22050L  /* 1.0's rate */
#define kSongSampleRatePlus     22254L  /* the Plus's own (N12-4) */
#define kSongBitsPerSample      8
#define kSongChannels           1

static inline Boolean SongRate_Valid(long rate)
{
    return rate == kSongSampleRate || rate == kSongSampleRatePlus;
}

/* 1.2 (N12-8): samples at a rate, as ticks (sixtieths of a second),
   rounded down. In two parts, as samples * 60 overflows a long beyond
   about 26.8 minutes of sound (1.0 to 1.1 got such a song's times
   wrong). */
static inline long SongRate_Ticks(long samples, long rate)
{
    return samples / rate * 60 + samples % rate * 60 / rate;
}

/* A song's length in whole seconds, rounded, as stored at offset 10. */
static inline long SongRate_Seconds(long samples, long rate)
{
    return (samples + rate / 2) / rate;
}

/* Finder codes (project-specific, unregistered). */
#define kAppCreator             'iPdM'
#define kSongFileType           'iPdS'
#define kPrefsFileType          'pref'
#define kPlaylistFileType       'TEXT'  /* P4: readable in TeachText */
#define kLibraryMarkerType      'iPdL'  /* "MacMusic Library Info" (5b-3) */

#endif
