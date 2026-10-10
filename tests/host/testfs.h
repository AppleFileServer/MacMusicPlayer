/* The host tests' stand-in Macintosh (tests/host): an in-memory HFS
   volume (macfs.c) and the other Toolbox calls MacMusic's portable
   modules make (mactoolbox.c), with controls for the tests. */
#ifndef TESTFS_H
#define TESTFS_H

#include "Files.h"
#include "Memory.h"
#include "TextUtils.h"
#include "OSUtils.h"
#include "StandardFile.h"

/* The volume: "Test Disk", with a System Folder. Reset empties it. */
void TestFS_Reset(void);
short TestFS_VRefNum(void);
long TestFS_Root(void);
long TestFS_SystemFolder(void);
ConstStringPtr TestFS_VolumeName(void);
unsigned long TestFS_VolumeCreated(void);
short TestFS_NewWD(long dirID);         /* a working directory refNum */

/* Unmounted: every call on the volume fails with nsvErr. Locked: nothing
   on it can be changed (vLckdErr, or permErr opening for writing). */
void TestFS_SetMounted(Boolean mounted);
void TestFS_LockVolume(Boolean locked);
void TestFS_SetFreeBytes(long bytes);

/* Folders and files by C-string name. */
long TestFS_MakeDir(long parent, const char *name);
OSErr TestFS_WriteFile(long dir, const char *name, OSType type,
                       OSType creator, const void *data, long length);
long TestFS_ReadFile(long dir, const char *name, void *buffer, long max);
Boolean TestFS_Exists(long dir, const char *name);
OSType TestFS_Type(long dir, const char *name);
void TestFS_Lock(long dir, const char *name, Boolean locked);
void TestFS_Delete(long dir, const char *name);
int TestFS_ListDir(long dir, char names[][64], int max);
void TestFS_AddHostFile(long dir, const char *name, const char *hostPath,
                        OSType type, OSType creator);
int TestFS_OpenFiles(void);             /* files left open: should be 0 */

/* 1.1: files' modification dates come from a clock that moves on a
   second at each change, or stays put (a Plus with a flat battery). */
void TestFS_SetClock(unsigned long seconds, Boolean stopped);
unsigned long TestFS_ModDate(long dir, const char *name);
void TestFS_SetModDate(long dir, const char *name, unsigned long date);
unsigned short TestFS_FinderFlags(long dir, const char *name);

/* Standard File: the reply the next SFPGetFile or SFPPutFile gives. */
void TestSF_SetReply(Boolean good, short vRefNum, OSType type,
                     const char *name);
/* Alert: the item the next alerts return; the last alert's ID. */
void TestAlert_SetAnswer(short item);
short TestAlert_LastID(void);
/* Message_Show: how many messages, and the last one's text. */
int TestMessage_Count(void);
const char *TestMessage_Last(void);
/* Memory: a limit on the bytes all handles and pointers may hold (0
   for none), for out-of-memory paths. */
void TestMem_SetLimit(long bytes);
/* Random: a fixed sequence from seed. */
void TestRandom_Seed(unsigned long seed);

#endif
