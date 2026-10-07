#ifndef EDITSONG_H
#define EDITSONG_H

#include <MacTypes.h>

/* Stage 8c: Edit > Edit Song...: shows the song details window (the
   Import Song window, L3) filled with the song's details and, on Save,
   rewrites them in the song's file and in the library, and the title in
   any playlist that lists the song (L11). The audio, the ID and the
   rating are never changed. Returns true if the library changed: the
   caller rebuilds the iPod's screens (UI_BeginLibraryEdit was called
   first). *rescan is set if the library could not be updated in memory
   and must be rescanned instead. */
Boolean EditSong_Run(short song, Boolean *rescan);

#endif
