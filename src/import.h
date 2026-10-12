#ifndef IMPORT_H
#define IMPORT_H

#include <MacTypes.h>

#include "libfolder.h"

/* File > Import Song...: asks for a WAV file with Standard File, checks
   it, asks for its details and puts it into the library as a song file
   (song_format.h). Stage 5b-2: the file is moved into the library, the
   details added at its end, unless the user chooses Just Copy; from
   another disk it is copied, then the original deleted. 1.1 (N11-25):
   then the Open dialog comes back for the next song, until it's
   cancelled; cancelling a song skips it. Returns true if any song was
   added (the caller then reads the library again, once: N11-26). */
Boolean Import_Run(const LibraryFolder *folder);

#endif
