#ifndef EXPORT_H
#define EXPORT_H

#include <MacTypes.h>

#include "libfolder.h"

/* File > Export Song... (Stage 5b-2): asks where to save song (a library
   index) with Standard File, starting with its original file name, and
   turns its song file back into the original WAV there, byte for byte:
   moved on the same disk, copied then deleted from the library on
   another. The song's lines are removed from every playlist. Returns
   true if the song left the library (the caller then rescans). The
   caller must stop the song's audio first: its file is changed. */
Boolean Export_Run(const LibraryFolder *folder, short song);

#endif
