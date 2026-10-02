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
    kFolderError        /* see *error */
};

/* Asks where to create (or which existing) iPod Library folder to use,
   with SFPutFile, then creates it and its Playlists folder. */
short Folder_Choose(LibraryLocation *location, LibraryFolder *folder,
                    short *error);

#endif
