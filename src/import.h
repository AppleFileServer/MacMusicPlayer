#ifndef IMPORT_H
#define IMPORT_H

#include <MacTypes.h>

#include "libfolder.h"

/* File > Import Song...: asks for a WAV file with Standard File, checks
   it, asks for its details and copies it into the library as a new song
   file. The source is never modified. Returns true if a song was added
   (the caller then rescans the library). */
Boolean Import_Run(const LibraryFolder *folder);

#endif
