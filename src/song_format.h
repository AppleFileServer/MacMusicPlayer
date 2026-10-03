#ifndef SONG_FORMAT_H
#define SONG_FORMAT_H

/* iPod song file format, version 1 (project-specific).

   All integers big-endian, read and written field by field (bytes.h).

   Offset Size Field
     0     4   magic 'IPOD'
     4     2   format version (1)
     6     4   header length: offset of the first audio byte
    10     4   song ID (> 0, unique in the library)
    14     2   flags (bit 0: audiobook)
    16     2   track number (0 = none)
    18     4   duration in seconds, = round(audio length / sample rate)
    22     4   encoding 'PCMu': linear PCM, unsigned samples
    26     4   sample rate in Hz (22050)
    30     2   bits per sample (8)
    32     2   channels (1)
    34     4   audio length in bytes
    38     -   title, artist, album, genre, composer: each a length byte
               (0-63) followed by that many Mac Roman characters
     s     4   audio checksum, present only if flags bit 1 is set
               (s = first byte after the strings; see SongChecksum)
     ...       (header length may exceed these: room for later fields)
   header length: audio samples

   Files without the checksum flag remain valid; they just cannot be
   matched as duplicates (DECISION D31).

   The data fork holds everything; the resource fork is unused. */

#define kSongMagic              'IPOD'
#define kSongFormatVersion      1

#define kSongOffsetMagic        0
#define kSongOffsetVersion      4
#define kSongOffsetHeaderLength 6
#define kSongOffsetID           10
#define kSongOffsetFlags        14
#define kSongOffsetTrack        16
#define kSongOffsetDuration     18
#define kSongOffsetEncoding     22
#define kSongOffsetSampleRate   26
#define kSongOffsetBits         30
#define kSongOffsetChannels     32
#define kSongOffsetAudioLength  34
#define kSongOffsetStrings      38

#define kSongStringCount        5
#define kSongMaxString          63
#define kSongMinHeaderSize      (kSongOffsetStrings + kSongStringCount)
#define kSongMaxHeaderSize      1024

#define kSongFlagAudiobook      0x0001
#define kSongFlagHasChecksum    0x0002

/* Audio checksum: two running 32-bit sums over the audio bytes,
       a += byte; b += a;   checksum = (b << 16) ^ a
   computed incrementally over the copy, starting from a = b = 0. */
typedef struct
{
    unsigned long a;
    unsigned long b;
} SongChecksum;

static inline void SongChecksum_Init(SongChecksum *c)
{
    c->a = 0;
    c->b = 0;
}

static inline void SongChecksum_Add(SongChecksum *c,
                                    const unsigned char *bytes, long count)
{
    unsigned long a = c->a, b = c->b;

    while (count-- > 0)
    {
        a += *bytes++;
        b += a;
    }
    c->a = a;
    c->b = b;
}

static inline unsigned long SongChecksum_Value(const SongChecksum *c)
{
    return (c->b << 16) ^ c->a;
}

/* The only audio format accepted. */
#define kSongEncodingPCMu       'PCMu'
#define kSongSampleRate         22050L
#define kSongBitsPerSample      8
#define kSongChannels           1

/* Finder codes (project-specific, unregistered). */
#define kAppCreator             'iPdM'
#define kSongFileType           'iPdS'
#define kPrefsFileType          'pref'

#endif
