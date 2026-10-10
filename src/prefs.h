#ifndef PREFS_H
#define PREFS_H

#include "libfolder.h"

/* The prefs file, "MacMusic Prefs" in the System Folder (System 6
   has no Preferences folder): the library location, the volume level
   (Stage 7b), the recent libraries (8d-3) and the Settings (9a). Before the app was renamed it was "MacMusicPlayer
   Prefs"; that file is renamed on first use (rename plan, R6).

   Format, version 4 (Stage 9a), big-endian:
     0  4  magic 'MMPf'
     4  2  version (4)
     6  1  1 if a library location follows, else 0
     7  1  volume level 0-7, or 0xFF if none saved
     8  -  the library location, only with a library:
           1  volume name length (1-27), then the name
           4  volume creation date
           4  library folder directory ID
     -  1  number of recent libraries (0-kMaxRecent), then each:
           a location as above, then
           1  the library folder's name length (0-31), then the name
              (shown in the File menu when its disk isn't mounted)
     -  1  number of settings that follow (Stage 9a), then one byte
           each: Shuffle, Repeat, Clicker (settings.h)

   Versions 1-3 are still read: version 3 has no settings; version 2 no
   recent list either; version 1 has the location from offset 6 and no
   volume. */

Boolean Prefs_Load(LibraryLocation *location);
OSErr Prefs_Save(const LibraryLocation *location);     /* keeps the volume */

/* Stage 8d-3: the libraries used before, most recent first (the current
   one included), each with its folder's name. Load returns how many. */
#define kMaxRecent      5               /* PROVISIONAL */

typedef struct
{
    LibraryLocation location;
    Str31 folderName;
} RecentLibrary;

short Prefs_LoadRecent(RecentLibrary *recent);          /* kMaxRecent */
OSErr Prefs_SaveRecent(const RecentLibrary *recent, short count);

/* Stage 9a: the Settings values, one byte each. Load fills in as many
   as were saved (leaving the rest of values as they are); Save keeps
   everything else in the prefs. */
#define kMaxSettings    8
void Prefs_LoadSettings(unsigned char *values, short count);
OSErr Prefs_SaveSettings(const unsigned char *values, short count);

/* Stage 7b: the saved volume level 0-7, or -1 if none. */
short Prefs_LoadVolume(void);
OSErr Prefs_SaveVolume(short level);                    /* keeps the library */

/* 1.1 (N11-16): the prefs file's folder (a working directory refNum, 0
   if unknown) and name, for its resource fork. */
short Prefs_FileForResources(Str63 name);

#endif
