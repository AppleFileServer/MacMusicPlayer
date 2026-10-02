#ifndef PREFS_H
#define PREFS_H

#include "libfolder.h"

/* The prefs file, "MacMusicPlayer Prefs" in the System Folder (System 6
   has no Preferences folder). It holds only the library location.

   Format, version 1, big-endian:
     0  4  magic 'MMPf'
     4  2  version (1)
     6  1  volume name length (1-27), then the name
     n  4  volume creation date
   n+4  4  library folder directory ID */

Boolean Prefs_Load(LibraryLocation *location);
OSErr Prefs_Save(const LibraryLocation *location);

#endif
