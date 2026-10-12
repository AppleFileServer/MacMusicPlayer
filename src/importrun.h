#ifndef IMPORTRUN_H
#define IMPORTRUN_H

#include <MacTypes.h>

/* 1.1 (N11-25): what a run of File > Import Song... has imported so far.
   The library in memory is read again only at the end of the run (N11-26),
   so until then it doesn't hold the run's songs: the run's own list is
   checked too, for the duplicate warning (D31) and the 500-song limit
   (M3). Begin and End bracket a run; the list is freed at the end. */
void ImportRun_Begin(void);
void ImportRun_End(void);

/* A song the run has just imported. */
void ImportRun_Add(long audioLength, unsigned long fingerprint, ConstStringPtr title);

short ImportRun_Count(void);           /* songs imported in the run */

/* True if the library, with the run's songs, holds kMaxSongs (or had more
   song files than that at its last scan): no more can be imported. */
Boolean ImportRun_LibraryFull(void);

/* True if the same audio is in the library or was imported earlier in
   the run; title is then that song's title. */
Boolean ImportRun_FindDuplicate(long audioLength, unsigned long fingerprint,
                                Str255 title);

#endif
