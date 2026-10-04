#ifndef LIBFOLDER_H
#define LIBFOLDER_H

#include <MacTypes.h>

/* Where the iPod Library folder lives, in a form that survives restarts
   on System 6: volume reference numbers change between boots and the
   Alias Manager needs System 7, so the volume is remembered by name and
   creation date, and the folder by its HFS directory ID. */
typedef struct
{
    Str31 volumeName;
    unsigned long volumeCreated;
    long folderID;
} LibraryLocation;

/* A located library: the mounted volume and the folder's directory ID. */
typedef struct
{
    short vRefNum;
    long folderID;
    long playlistsID;   /* its "Playlists" subfolder */
} LibraryFolder;

/* Finds a remembered library. Creates its Playlists folder if missing.
   Returns false if the volume is not mounted or the folder has gone. */
Boolean Folder_Resolve(const LibraryLocation *location, LibraryFolder *folder);

enum
{
    kFolderChosen,
    kFolderCancelled,
    kFolderNameIsFile,  /* a file, not a folder, has that name */
    kFolderNotLibrary,  /* the folder found is not an iPod Library */
    kFolderError        /* see *error */
};

/* Stage 5b-3: every library holds a marker file, "iPod Library Info"
   (type kLibraryMarkerType), so that even an empty library can be
   picked in the Open dialog. Created with a new library and added to
   any library found without one. */

/* Create New: asks where to create a new iPod Library folder, with
   SFPutFile, then creates it and its Playlists folder. A typed name that
   is an existing folder is reused; Save inside a folder that is already
   a library asks whether to use that one (DECISION L3). */
short Folder_Choose(LibraryLocation *location, LibraryFolder *folder,
                    short *error);

/* Use Existing: asks for a file inside an existing library with
   SFGetFile (only the marker, song files and playlists are listed) and
   uses the library it is in. */
short Folder_FindExisting(LibraryLocation *location, LibraryFolder *folder,
                          short *error);

#endif
