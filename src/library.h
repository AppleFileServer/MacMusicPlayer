#ifndef LIBRARY_H
#define LIBRARY_H

#include <MacTypes.h>

#include "libfolder.h"
#include "songfile.h"

/* The music library: built by scanning the MacMusic Library folder's song
   files (song_format.h) and its Playlists folder. Only metadata is held
   in memory; audio stays on disk. Artists, albums, genres and composers
   are indexed by number; kNone in a filter field means "any". */

#define kMaxSongs       500
#define kMaxGroups      256     /* per kind: artists, albums, genres... */
#define kMaxPlaylists   10      /* 8e-2: a MacMusic limit, not the 4G's */
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

/* Stage 8d (M3): songs in the library now, at most kMaxSongs; and song
   files the last scan left out because the library was already full
   (copied in from the Finder, for example). */
short Library_SongCount(void);
short Library_OverLimitCount(void);

/* Stage 8e-2: playlist files the last scan left out because there were
   already kMaxPlaylists (the first ones by name are used). */
short Library_PlaylistsOverLimitCount(void);

/* Stage 8d-2, for Library Info: the songs' audio in bytes, and the
   number of playlists. */
long Library_TotalAudioBytes(void);
short Library_PlaylistCount(void);

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

/* Stage 8e-4: renames a playlist's file; its number stays the same.
   dupFNErr if another playlist (or file there) has the name already. */
OSErr Library_RenamePlaylist(short playlist, ConstStringPtr name);

/* Stage 8e-3: deletes a playlist's file (only the playlist: its songs
   stay in the library). Playlist numbers after it move down by one. */
OSErr Library_DeletePlaylist(short playlist);

/* Stage 8d-4 (L7): removes one entry from a playlist file: the nth
   (0-based) line for song ID id, so a song listed twice keeps its other
   line. Other lines are kept exactly. */
OSErr Library_RemoveFromPlaylist(short playlist, long id, short nth);

/* Stage 8c (L11): rewrites every playlist line for song ID id with this
   title (playlist files show the title as text; membership stays by ID).
   Files without the ID are not rewritten. Returns the first error. */
OSErr Library_RetitleInPlaylists(long id, ConstStringPtr title);

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

/* Stage 7d: the song's rating, 0 (unrated) to 5. Setting it writes the
   song file (SongFile_WriteFlags), then updates the library; on an error
   the library keeps the old rating. */
/* Stage 9c, for Shuffle by album: the song's album (a group number, or
   kNone), track number, and whether it is an audiobook. */
short Library_SongAlbum(short song);
short Library_SongTrack(short song);
Boolean Library_SongIsAudiobook(short song);

short Library_SongRating(short song);
OSErr Library_SetSongRating(short song, short rating);

/* Stage 8c: the song's details changed (its file already rewritten from
   info): the library takes the new title, artist, album, genre, composer,
   track and flags without a rescan. Song indexes and IDs are unchanged;
   artist, album, genre and composer numbers may all change, and pointers
   into the old names must not be kept. False if memory ran out (the
   caller then rescans). */
Boolean Library_UpdateSong(short song, const SongInfo *info);

/* The artist, album, genre or composer with exactly this name (as the
   lists group them), or kNone. */
short Library_FindArtist(ConstStringPtr name);
short Library_FindAlbum(ConstStringPtr name);
short Library_FindGenre(ConstStringPtr name);
short Library_FindComposer(ConstStringPtr name);

/* Where a song's audio is on disk, for playback. False if there is no
   scanned library. */
Boolean Library_SongFile(short song, short *vRefNum, long *dirID,
                         ConstStringPtr *fileName, long *audioOffset);

#endif
