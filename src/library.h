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
#define kMaxPlaylistEntries 1000    /* songs read from one playlist file */
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

/* A song whose audio has this length and fingerprint (D31), or kNone. */
short Library_FindAudioMatch(long audioLength, unsigned long fingerprint);

/* ---- Playlists (DECISION P1: membership by song ID only) ---- */

/* The song with this stable ID, or kNone. */
short Library_SongForID(long id);

/* The playlist with this name (ignoring case, as HFS does), or kNone. */
short Library_FindPlaylist(ConstStringPtr name);

/* True if the playlist file already lists this song's ID (P5). */
Boolean Library_PlaylistContains(short playlist, short song);

/* Creates an empty playlist file (TEXT/iPdM) in Playlists. dupFNErr if
   the name is taken; dirFulErr if there are kMaxPlaylists already. The
   new playlist's index is returned; existing indexes do not change. */
OSErr Library_AddPlaylist(ConstStringPtr name, short *playlist);

/* Appends "<id><tab><title><CR>" to the playlist file. */
OSErr Library_AppendToPlaylist(short playlist, short song);

/* Removes every line for song ID id from every playlist file (Stage 5b-2
   export: the song leaves the library). Other lines are kept exactly;
   files without the ID are not rewritten. Returns the first error. */
OSErr Library_RemoveFromPlaylists(long id);

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

/* Per-song details for Now Playing and playback (song = library index,
   valid until the next scan; use the ID to keep a song across scans). */
long Library_SongID(short song);
ConstStringPtr Library_SongArtistName(short song);   /* may be empty */
ConstStringPtr Library_SongAlbumName(short song);    /* may be empty */
long Library_SongSeconds(short song);
long Library_SongAudioLength(short song);           /* bytes */

/* Where a song's audio is on disk, for playback. False if there is no
   scanned library. */
Boolean Library_SongFile(short song, short *vRefNum, long *dirID,
                         ConstStringPtr *fileName, long *audioOffset);

#endif
