#ifndef LIBSETUP_H
#define LIBSETUP_H

#include <MacTypes.h>

#include "libfolder.h"

/* Startup: finds the remembered MacMusic Library (asking for one if there is
   none, or it cannot be found) and scans it. Never fails: without a
   library the iPod shows empty lists. */
void LibSetup_Startup(void);

/* File > Choose Library...: asks for a library and remembers it. Returns
   true if one was chosen; the caller then rescans and resets the UI. */
Boolean LibSetup_ChooseLibrary(void);

/* Rebuilds the library from the current folder (empty if none). */
void LibSetup_Rescan(void);
/* 1.1 (N11-6): Rescan Library's full read, ignoring the cache. */
void LibSetup_RescanAll(void);

/* File > Library Info... (Stage 8d-2): Songs (of the 500-song limit,
   audiobooks included), Playlists, Size (the songs' audio), Free (on the
   library's disk), Unreadable Files (as the scan reports), and, only if
   a scan left files out, Songs Over the Limit and Playlists Over the
   Limit (8e-2). */
void LibSetup_ShowInfo(void);

/* Stage 8d-3: the libraries used before, most recent first (the current
   one included, up to kMaxRecent). Item gives the File menu text, "folder
   (disk)", whether the library can be chosen now (its disk mounted), and
   whether it is the current one. UseRecent switches to library i;
   false if it is already the current one or can't be found (a message
   then says so). The caller then rescans and restarts, as for Choose
   Library.... */
/* Appends a size in megabytes with one decimal place ("7.5 MB"). */
void LibSetup_AppendMB(Str255 text, long bytes);

short LibSetup_RecentCount(void);
void LibSetup_RecentItem(short i, Str255 text, Boolean *available,
                         Boolean *current);
Boolean LibSetup_UseRecent(short i);

Boolean LibSetup_HasLibrary(void);
const LibraryFolder *LibSetup_Folder(void);
/* 1.1: the library folder's name (for printing). */
void LibSetup_LibraryName(Str31 name);

#endif
