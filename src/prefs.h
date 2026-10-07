#ifndef PREFS_H
#define PREFS_H

#include "libfolder.h"

/* The prefs file, "MacMusic Prefs" in the System Folder (System 6
   has no Preferences folder): the library location and (Stage 7b) the
   volume level. Before the app was renamed it was "MacMusicPlayer
   Prefs"; that file is renamed on first use (rename plan, R6).

   Format, version 2, big-endian:
     0  4  magic 'MMPf'
     4  2  version (2)
     6  1  1 if a library location follows, else 0
     7  1  volume level 0-7, or 0xFF if none saved
     8  1  volume name length (1-27), then the name   } only with a
     n  4  volume creation date                        } library
   n+4  4  library folder directory ID                  }

   Version 1 (still read): magic, version 1, then the library location
   from offset 6 as above, and no volume. */

Boolean Prefs_Load(LibraryLocation *location);
OSErr Prefs_Save(const LibraryLocation *location);     /* keeps the volume */

/* Stage 7b: the saved volume level 0-7, or -1 if none. */
short Prefs_LoadVolume(void);
OSErr Prefs_SaveVolume(short level);                    /* keeps the library */

#endif
