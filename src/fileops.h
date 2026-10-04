#ifndef FILEOPS_H
#define FILEOPS_H

#include <Files.h>

#include "songfile.h"

/* File Manager operations shared by import and export (Stage 5b-2). */

#define kLockedAttribute    0x01    /* ioFlAttrib bit 0 */

/* The volume and folder of a working directory (as Standard File gives). */
OSErr ResolveWD(short wdRefNum, short *vRefNum, long *dirID);

/* PBHGetFInfo into pb (cleared first). */
OSErr GetFileInfo(short vRefNum, long dirID, ConstStringPtr name,
                  HParamBlockRec *pb);
Boolean FileExists(short vRefNum, long dirID, ConstStringPtr name);
OSErr SetFileType(short vRefNum, long dirID, ConstStringPtr name,
                  OSType type, OSType creator);
OSErr RenameFile(short vRefNum, long dirID, ConstStringPtr name,
                 ConstStringPtr newName);

/* Moves a file to another folder on the same volume, keeping its name. */
OSErr MoveFile(short vRefNum, long dirID, ConstStringPtr name, long newDirID);

long FreeBytes(short vRefNum);

/* True for errors that mean a file can be read but not changed or moved
   (E4): locked file or disk, or open in another program. */
Boolean IsCantChangeError(OSErr err);

/* Adds (add) or removes the song chunk of a file in place, then flushes
   the volume. */
OSErr ChangeChunk(short vRefNum, long dirID, ConstStringPtr name,
                  const SongInfo *info, Boolean add);

#endif
