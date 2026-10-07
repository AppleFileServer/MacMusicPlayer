#ifndef LIBSETUP_H
#define LIBSETUP_H

#include <MacTypes.h>

#include "libfolder.h"

/* Startup: finds the remembered iPod Library (asking for one if there is
   none, or it cannot be found) and scans it. Never fails: without a
   library the iPod shows empty lists. */
void LibSetup_Startup(void);

/* File > Choose Library...: asks for a library and remembers it. Returns
   true if one was chosen; the caller then rescans and resets the UI. */
Boolean LibSetup_ChooseLibrary(void);

/* Rebuilds the library from the current folder (empty if none). */
void LibSetup_Rescan(void);

/* File > Library Info... (Stage 8d-2): Songs (of the 500-song limit,
   audiobooks included), Playlists, Size (the songs' audio), Free (on the
   library's disk), Unreadable Files (as the scan reports), and Over the
   Limit if any song files were left out. */
void LibSetup_ShowInfo(void);

Boolean LibSetup_HasLibrary(void);
const LibraryFolder *LibSetup_Folder(void);

#endif
