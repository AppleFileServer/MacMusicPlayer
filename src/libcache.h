#ifndef LIBCACHE_H
#define LIBCACHE_H

#include <Files.h>

#include "libfolder.h"
#include "songfile.h"

/* 1.1 (docs/plans/release-1.1.md, N11-2 to N11-10): the library's cache
   of song details, so a scan needn't open every song file. It's the file
   "Song Details" in an invisible folder "MacMusic Cache" in the library
   (1.0 skips folders when it scans, so a 1.1 library opens in 1.0 as it
   is). For each song it holds the file's name, modification date and
   length, and what a scan reads from the file. A file whose name, date
   and length match its entry is taken from the cache; anything else is
   read from the file, as 1.0 does.

   The cache is never trusted over a file: a damaged or unknown cache is
   ignored (and rewritten), and before MacMusic changes a song file it
   marks the song's entry stale (LibCache_Forget), so that file is read
   again whatever the clock says (a Plus whose battery has run down gives
   the same dates in every session). */

/* Reads the folder's cache into memory, if it has a usable one; false if
   it has none, it's damaged or there isn't the memory. Held until
   LibCache_Unload. */
Boolean LibCache_Load(const LibraryFolder *folder);
void LibCache_Unload(void);

/* The loaded cache's song count (0 if none is loaded). */
short LibCache_Count(void);

/* The entry for this file, if its name, date and length all match: fills
   in what a scan reads (ID, flags, track, seconds, audio offset and
   length, fingerprint, the five details) and gives the entry's place in
   the cache file, for LibCache_Forget. */
Boolean LibCache_Find(ConstStringPtr name, unsigned long modDate, long length,
                      SongInfo *info, long *at);

/* Writes a new cache: Begin, an Entry for each song (its place in the
   file, or -1), End. The header is written last, so a cache left half
   written is seen as damaged. Errors (a locked disk) leave no cache. */
OSErr LibCache_BeginSave(const LibraryFolder *folder);
long LibCache_SaveEntry(ConstStringPtr name, unsigned long modDate, long length,
                        const SongInfo *info);
OSErr LibCache_EndSave(void);

/* Marks the entry at `at` stale (its length set to one no file has),
   before the song's file is changed. */
void LibCache_Forget(const LibraryFolder *folder, long at);

#endif
