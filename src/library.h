#ifndef LIBRARY_H
#define LIBRARY_H

#include <MacTypes.h>

#include "libfolder.h"

/* The music library: built by scanning the iPod Library folder's song
   files (song_format.h) and its Playlists folder. Only metadata is held
   in memory; audio stays on disk. Artists, albums, genres and composers
   are indexed by number; kNone in a filter field means "any". */

#define kMaxSongs       500
#define kMaxGroups      256     /* per kind: artists, albums, genres... */
#define kMaxPlaylists   64
#define kNone           (-1)

typedef struct
{
    short genre;
    short artist;
    short album;
    short composer;
    short playlist;
} LibFilter;

void LibFilter_Clear(LibFilter *filter);

/* Empties the library and frees its memory. */
void Library_Clear(void);

/* Rebuilds the library from a folder. Returns false only if memory ran
   out; unreadable or unrecognised files are skipped and counted. */
Boolean Library_Scan(const LibraryFolder *folder);

short Library_SkippedCount(void);   /* files skipped by the last scan */
long Library_NextSongID(void);      /* one more than the largest ID */

/* Each List function writes item indexes, in display order, to out
   (at most max) and returns how many it wrote. */
short Library_ListGenres(short *out, short max);
short Library_ListArtists(const LibFilter *filter, short *out, short max);
short Library_ListAlbums(const LibFilter *filter, short *out, short max);
short Library_ListComposers(short *out, short max);
short Library_ListPlaylists(short *out, short max);
short Library_ListSongs(const LibFilter *filter, short *out, short max);
short Library_ListAudiobooks(short *out, short max);

ConstStringPtr Library_GenreName(short genre);
ConstStringPtr Library_ArtistName(short artist);
ConstStringPtr Library_AlbumName(short album);
ConstStringPtr Library_ComposerName(short composer);
ConstStringPtr Library_PlaylistName(short playlist);
ConstStringPtr Library_SongTitle(short song);

#endif
