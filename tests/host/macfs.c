/* An in-memory HFS volume for the host tests: the File Manager calls
   MacMusic's portable modules make, over folders and files held here.
   Behaviour follows Inside Macintosh where the modules depend on it:
   names compare without regard to case, a folder lists in name order,
   PBGetCatInfo returns a file's or folder's ID in ioDirID, a file open
   for writing can't be opened for writing again, a locked file or volume
   can't be changed. Files can also stand for a file on this Mac's disk,
   read-only (for the library-image check's real songs). */

#include <ctype.h>
#include <stdio.h>

#include "testfs.h"

#define kVRefNum        (-1)
#define kRootID         2
#define kFirstID        16
#define kMaxOpen        24
#define kMaxWDs         16
#define kWDBase         (-32000)

typedef struct
{
    long id, parent;
    Str63 name;
    Boolean isDir, locked, used;
    OSType type, creator;
    unsigned char *data;
    long length, capacity;
    char *hostPath;         /* read-only file on this Mac's disk */
    unsigned long modDate;  /* set from the clock when the file changes */
    unsigned short finderFlags;
} Node;

typedef struct
{
    long node;              /* index into gNodes, or -1 */
    long mark;
    Boolean write;
    FILE *host;
} OpenFile;

static Node *gNodes;
static long gNodeCount, gNodeCapacity, gNextID;
static OpenFile gOpen[kMaxOpen + 1];            /* refNums 1..kMaxOpen */
static long gWDs[kMaxWDs];                      /* dirIDs; 0 = unused */
static long gSystemFolder;
static Boolean gMounted, gVolumeLocked;
static long gFreeBytes;
static Str31 gVolumeName;
static unsigned long gVolumeCreated;
/* The clock that sets files' modification dates: it moves on a second at
   each change unless stopped (a Plus whose battery has run down starts
   from the same time at every power-on). */
static unsigned long gClock;
static Boolean gClockStopped;

static unsigned long Now(void)
{
    return gClockStopped ? gClock : gClock++;
}

/* ---- Names and nodes ---------------------------------------------------- */

static void ToPascal(const char *c, StringPtr p)
{
    size_t n = strlen(c);

    if (n > 63)
        n = 63;
    p[0] = (unsigned char)n;
    memcpy(p + 1, c, n);
}

static Boolean SameName(ConstStringPtr a, ConstStringPtr b)
{
    int i;

    if (a[0] != b[0])
        return false;
    for (i = 1; i <= a[0]; i++)
        if (toupper(a[i]) != toupper(b[i]))
            return false;
    return true;
}

/* HFS catalog order: case-insensitive. */
static int CompareNames(ConstStringPtr a, ConstStringPtr b)
{
    int i, n = a[0] < b[0] ? a[0] : b[0];

    for (i = 1; i <= n; i++)
        if (toupper(a[i]) != toupper(b[i]))
            return toupper(a[i]) - toupper(b[i]);
    return a[0] - b[0];
}

static Boolean ValidName(ConstStringPtr name, Boolean isDir)
{
    int i;

    if (name == nil || name[0] == 0 || name[0] > 31)
        return false;
    for (i = 1; i <= name[0]; i++)
        if (name[i] == ':')
            return false;
    (void)isDir;
    return true;
}

static long FindByID(long id)
{
    long i;

    for (i = 0; i < gNodeCount; i++)
        if (gNodes[i].used && gNodes[i].id == id)
            return i;
    return -1;
}

static long FindChild(long dirID, ConstStringPtr name)
{
    long i;

    for (i = 0; i < gNodeCount; i++)
        if (gNodes[i].used && gNodes[i].parent == dirID
            && SameName(gNodes[i].name, name))
            return i;
    return -1;
}

static long NewNode(long parent, ConstStringPtr name, Boolean isDir)
{
    Node *n;

    if (gNodeCount == gNodeCapacity)
    {
        gNodeCapacity = gNodeCapacity ? gNodeCapacity * 2 : 64;
        gNodes = realloc(gNodes, gNodeCapacity * sizeof(Node));
    }
    n = &gNodes[gNodeCount];
    memset(n, 0, sizeof(*n));
    n->used = true;
    n->id = parent == 1 ? kRootID : gNextID++;    /* 1: the root's parent */
    n->parent = parent;
    memcpy(n->name, name, name[0] + 1);
    n->isDir = isDir;
    n->modDate = Now();
    return gNodeCount++;
}

static Boolean IsOpen(long node)
{
    int r;

    for (r = 1; r <= kMaxOpen; r++)
        if (gOpen[r].node == node)
            return true;
    return false;
}

/* The folder a vRefNum (volume or working directory) and dirID name. */
static OSErr ResolveDir(short vRefNum, long dirID, long *dir)
{
    if (!gMounted)
        return nsvErr;
    if (vRefNum <= kWDBase && vRefNum > kWDBase - kMaxWDs)
    {
        long wd = gWDs[kWDBase - vRefNum];

        if (wd == 0)
            return nsvErr;
        *dir = dirID != 0 ? dirID : wd;
    }
    else if (vRefNum == kVRefNum || vRefNum == 0)
        *dir = dirID != 0 ? dirID : kRootID;
    else
        return nsvErr;
    if (*dir != 1 && (FindByID(*dir) < 0 || !gNodes[FindByID(*dir)].isDir))
        return dirNFErr;
    return noErr;
}

/* The item vRefNum/dirID/name names: a file or folder. */
static OSErr Lookup(short vRefNum, long dirID, ConstStringPtr name, long *node)
{
    long dir;
    OSErr err = ResolveDir(vRefNum, dirID, &dir);

    if (err != noErr)
        return err;
    if (name == nil || name[0] == 0)
        return (*node = FindByID(dir)) >= 0 ? noErr : fnfErr;
    *node = FindChild(dir, name);
    return *node >= 0 ? noErr : fnfErr;
}

static long FreeBytesLeft(void)
{
    long used = 0, i;

    for (i = 0; i < gNodeCount; i++)
        if (gNodes[i].used)
            used += gNodes[i].length;
    return gFreeBytes - used;
}

/* ---- Test set-up -------------------------------------------------------- */

void TestFS_Reset(void)
{
    gClock = 3000000000UL;          /* 1999 */
    gClockStopped = false;
    long i;
    int r;

    for (r = 1; r <= kMaxOpen; r++)
        if (gOpen[r].host != NULL)
            fclose(gOpen[r].host);
    for (i = 0; i < gNodeCount; i++)
    {
        free(gNodes[i].data);
        free(gNodes[i].hostPath);
    }
    gNodeCount = 0;
    gNextID = kFirstID;
    memset(gOpen, 0, sizeof(gOpen));
    for (r = 0; r <= kMaxOpen; r++)
        gOpen[r].node = -1;
    memset(gWDs, 0, sizeof(gWDs));
    gMounted = true;
    gVolumeLocked = false;
    gFreeBytes = 20L * 1024 * 1024;
    ToPascal("Test Disk", gVolumeName);
    gVolumeCreated = 0xB0C0FFEE;

    NewNode(1, gVolumeName, true);              /* the root, ID 2 */
    gSystemFolder = TestFS_MakeDir(kRootID, "System Folder");
}

short TestFS_VRefNum(void)              { return kVRefNum; }
long TestFS_Root(void)                  { return kRootID; }
long TestFS_SystemFolder(void)          { return gSystemFolder; }
ConstStringPtr TestFS_VolumeName(void)  { return gVolumeName; }
unsigned long TestFS_VolumeCreated(void) { return gVolumeCreated; }
void TestFS_SetMounted(Boolean mounted) { gMounted = mounted; }
void TestFS_LockVolume(Boolean locked)  { gVolumeLocked = locked; }
void TestFS_SetFreeBytes(long bytes)    { gFreeBytes = bytes; }

short TestFS_NewWD(long dirID)
{
    int i;

    for (i = 0; i < kMaxWDs; i++)
        if (gWDs[i] == 0 || gWDs[i] == dirID)
        {
            gWDs[i] = dirID;
            return (short)(kWDBase - i);
        }
    return 0;
}

long TestFS_MakeDir(long parent, const char *name)
{
    Str63 p;
    long n;

    ToPascal(name, p);
    n = FindChild(parent, p);
    if (n < 0)
        n = NewNode(parent, p, true);
    return gNodes[n].id;
}

OSErr TestFS_WriteFile(long dir, const char *name, OSType type,
                       OSType creator, const void *data, long length)
{
    Str63 p;
    long n;

    ToPascal(name, p);
    n = FindChild(dir, p);
    if (n < 0)
        n = NewNode(dir, p, false);
    if (gNodes[n].isDir)
        return dupFNErr;
    free(gNodes[n].data);
    gNodes[n].data = malloc(length > 0 ? length : 1);
    memcpy(gNodes[n].data, data, length);
    gNodes[n].length = gNodes[n].capacity = length;
    gNodes[n].type = type;
    gNodes[n].creator = creator;
    return noErr;
}

void TestFS_AddHostFile(long dir, const char *name, const char *hostPath,
                        OSType type, OSType creator)
{
    Str63 p;
    long n;
    FILE *f = fopen(hostPath, "rb");

    ToPascal(name, p);
    n = NewNode(dir, p, false);
    gNodes[n].hostPath = strdup(hostPath);
    gNodes[n].type = type;
    gNodes[n].creator = creator;
    gNodes[n].locked = true;
    if (f != NULL)
    {
        fseek(f, 0, SEEK_END);
        gNodes[n].length = ftell(f);
        fclose(f);
    }
}

long TestFS_ReadFile(long dir, const char *name, void *buffer, long max)
{
    Str63 p;
    long n;

    ToPascal(name, p);
    n = FindChild(dir, p);
    if (n < 0 || gNodes[n].isDir || gNodes[n].hostPath != NULL)
        return -1;
    if (max > gNodes[n].length)
        max = gNodes[n].length;
    memcpy(buffer, gNodes[n].data, max);
    return gNodes[n].length;
}

Boolean TestFS_Exists(long dir, const char *name)
{
    Str63 p;

    ToPascal(name, p);
    return FindChild(dir, p) >= 0;
}

OSType TestFS_Type(long dir, const char *name)
{
    Str63 p;
    long n;

    ToPascal(name, p);
    n = FindChild(dir, p);
    return n >= 0 ? gNodes[n].type : 0;
}

void TestFS_Lock(long dir, const char *name, Boolean locked)
{
    Str63 p;
    long n;

    ToPascal(name, p);
    n = FindChild(dir, p);
    if (n >= 0)
        gNodes[n].locked = locked;
}

void TestFS_Delete(long dir, const char *name)
{
    Str63 p;
    long n;

    ToPascal(name, p);
    n = FindChild(dir, p);
    if (n >= 0)
        gNodes[n].used = false;
}

int TestFS_OpenFiles(void)
{
    int r, count = 0;

    for (r = 1; r <= kMaxOpen; r++)
        if (gOpen[r].node >= 0)
            count++;
    return count;
}

int TestFS_ListDir(long dir, char names[][64], int max)
{
    long i, j;
    int count = 0;
    long order[1024];
    long t;

    for (i = 0; i < gNodeCount && count < 1024; i++)
        if (gNodes[i].used && gNodes[i].parent == dir)
            order[count++] = i;
    for (i = 1; i < count; i++)
        for (j = i; j > 0 && CompareNames(gNodes[order[j - 1]].name,
                                          gNodes[order[j]].name) > 0; j--)
        {
            t = order[j];
            order[j] = order[j - 1];
            order[j - 1] = t;
        }
    for (i = 0; i < count && i < max; i++)
        snprintf(names[i], 64, "%.*s", gNodes[order[i]].name[0],
                 gNodes[order[i]].name + 1);
    return count;
}

/* ---- The File Manager --------------------------------------------------- */

/* The nth (1-based) item of a folder, in name order. */
static long NthChild(long dir, short index)
{
    long i, best = -1, prev = -1;
    short k;

    for (k = 0; k < index; k++)
    {
        best = -1;
        for (i = 0; i < gNodeCount; i++)
            if (gNodes[i].used && gNodes[i].parent == dir
                && (prev < 0 || CompareNames(gNodes[i].name,
                                             gNodes[prev].name) > 0)
                && (best < 0 || CompareNames(gNodes[i].name,
                                             gNodes[best].name) < 0))
                best = i;
        if (best < 0)
            return -1;
        prev = best;
    }
    return best;
}

OSErr PBGetCatInfoSync(CInfoPBRec *pb)
{
    HFileInfo *f = &pb->hFileInfo;
    long dir, n;
    OSErr err;

    if (f->ioFDirIndex > 0)
    {
        if ((err = ResolveDir(f->ioVRefNum, f->ioDirID, &dir)) != noErr)
            return pb->hFileInfo.ioResult = err;
        n = NthChild(dir, f->ioFDirIndex);
        if (n < 0)
            return f->ioResult = fnfErr;
    }
    else if (f->ioFDirIndex < 0)
    {
        if ((err = ResolveDir(f->ioVRefNum, pb->dirInfo.ioDrDirID, &dir))
            != noErr)
            return f->ioResult = err;
        n = FindByID(dir);
    }
    else if ((err = Lookup(f->ioVRefNum, f->ioDirID, f->ioNamePtr, &n))
             != noErr)
        return f->ioResult = err;

    if (f->ioNamePtr != nil && f->ioFDirIndex != 0)
        memcpy(f->ioNamePtr, gNodes[n].name, gNodes[n].name[0] + 1);
    f->ioFlAttrib = (SignedByte)((gNodes[n].isDir ? 0x10 : 0)
                                 | (gNodes[n].locked ? 0x01 : 0)
                                 | (IsOpen(n) ? 0x80 : 0));
    if (gNodes[n].isDir)
    {
        memset(&pb->dirInfo.ioDrUsrWds, 0, sizeof(DInfo));
        pb->dirInfo.ioDrUsrWds.frFlags = gNodes[n].finderFlags;
        pb->dirInfo.ioDrMdDat = gNodes[n].modDate;
        pb->dirInfo.ioDrDirID = gNodes[n].id;
        pb->dirInfo.ioDrParID = gNodes[n].parent;
        pb->dirInfo.ioDrNmFls = 0;          /* its valence: files and folders */
        for (dir = 0; dir < gNodeCount; dir++)
            if (gNodes[dir].used && gNodes[dir].parent == gNodes[n].id && dir != n)
                pb->dirInfo.ioDrNmFls++;
    }
    else
    {
        memset(&f->ioFlFndrInfo, 0, sizeof(FInfo));
        f->ioFlFndrInfo.fdType = gNodes[n].type;
        f->ioFlFndrInfo.fdCreator = gNodes[n].creator;
        f->ioFlFndrInfo.fdFlags = gNodes[n].finderFlags;
        f->ioDirID = gNodes[n].id;
        f->ioFlLgLen = gNodes[n].length;
        f->ioFlMdDat = gNodes[n].modDate;
        f->ioFlParID = gNodes[n].parent;
    }
    return f->ioResult = noErr;
}

/* Sets a folder's or file's Finder flags (the only part MacMusic sets). */
OSErr PBSetCatInfoSync(CInfoPBRec *pb)
{
    HFileInfo *f = &pb->hFileInfo;
    long n;
    OSErr err;

    if (gVolumeLocked)
        return f->ioResult = vLckdErr;
    if ((err = Lookup(f->ioVRefNum, f->ioDirID, f->ioNamePtr, &n)) != noErr)
        return f->ioResult = err;
    gNodes[n].finderFlags = gNodes[n].isDir ? pb->dirInfo.ioDrUsrWds.frFlags
                                            : f->ioFlFndrInfo.fdFlags;
    return f->ioResult = noErr;
}

OSErr PBHOpenSync(HParamBlockRec *pb)
{
    long n;
    int r, i;
    Boolean write = pb->ioParam.ioPermssn != fsRdPerm;
    OSErr err = Lookup(pb->ioParam.ioVRefNum, pb->fileParam.ioDirID,
                       pb->ioParam.ioNamePtr, &n);

    pb->ioParam.ioRefNum = 0;
    if (err != noErr)
        return pb->ioParam.ioResult = err;
    if (gNodes[n].isDir)
        return pb->ioParam.ioResult = fnfErr;
    if (write && (gNodes[n].locked || gVolumeLocked))
        return pb->ioParam.ioResult = permErr;
    for (i = 1; i <= kMaxOpen; i++)
        if (write && gOpen[i].node == n && gOpen[i].write)
            return pb->ioParam.ioResult = opWrErr;
    for (r = 1; r <= kMaxOpen; r++)
        if (gOpen[r].node < 0)
            break;
    if (r > kMaxOpen)
        return pb->ioParam.ioResult = tmfoErr;
    gOpen[r].node = n;
    gOpen[r].mark = 0;
    gOpen[r].write = write;
    gOpen[r].host = gNodes[n].hostPath ? fopen(gNodes[n].hostPath, "rb") : NULL;
    pb->ioParam.ioRefNum = (short)r;
    return pb->ioParam.ioResult = noErr;
}

static OpenFile *FileFor(short refNum)
{
    if (refNum < 1 || refNum > kMaxOpen || gOpen[refNum].node < 0)
        return NULL;
    return &gOpen[refNum];
}

OSErr FSRead(short refNum, long *count, void *buffer)
{
    OpenFile *o = FileFor(refNum);
    Node *n;
    long want = *count, have;

    *count = 0;
    if (o == NULL)
        return rfNumErr;
    n = &gNodes[o->node];
    have = n->length - o->mark;
    if (have < 0)
        have = 0;
    if (want < have)
        have = want;
    if (o->host != NULL)
    {
        fseek(o->host, o->mark, SEEK_SET);
        have = (long)fread(buffer, 1, have, o->host);
    }
    else
        memcpy(buffer, n->data + o->mark, have);
    o->mark += have;
    *count = have;
    return have < want ? eofErr : noErr;
}

static OSErr Grow(Node *n, long length)
{
    if (length > n->capacity)
    {
        long capacity = length * 2;

        n->data = realloc(n->data, capacity);
        memset(n->data + n->capacity, 0, capacity - n->capacity);
        n->capacity = capacity;
    }
    if (length > n->length)
        memset(n->data + n->length, 0, length - n->length);
    n->length = length;
    return noErr;
}

OSErr FSWrite(short refNum, long *count, const void *buffer)
{
    OpenFile *o = FileFor(refNum);
    Node *n;
    long want = *count;

    *count = 0;
    if (o == NULL)
        return rfNumErr;
    if (!o->write)
        return wrPermErr;
    n = &gNodes[o->node];
    if (o->mark + want > n->length
        && o->mark + want - n->length > FreeBytesLeft())
        return dskFulErr;
    if (o->mark + want > n->length)
        Grow(n, o->mark + want);
    memcpy(n->data + o->mark, buffer, want);
    o->mark += want;
    *count = want;
    n->modDate = Now();
    return noErr;
}

OSErr FSClose(short refNum)
{
    OpenFile *o = FileFor(refNum);

    if (o == NULL)
        return rfNumErr;
    if (o->host != NULL)
        fclose(o->host);
    o->host = NULL;
    o->node = -1;
    return noErr;
}

OSErr GetEOF(short refNum, long *eof)
{
    OpenFile *o = FileFor(refNum);

    if (o == NULL)
        return rfNumErr;
    *eof = gNodes[o->node].length;
    return noErr;
}

OSErr SetEOF(short refNum, long eof)
{
    OpenFile *o = FileFor(refNum);
    Node *n;

    if (o == NULL)
        return rfNumErr;
    if (!o->write)
        return wrPermErr;
    n = &gNodes[o->node];
    if (eof > n->length && eof - n->length > FreeBytesLeft())
        return dskFulErr;
    if (eof > n->length)
        Grow(n, eof);
    n->length = eof;
    n->modDate = Now();
    if (o->mark > eof)
        o->mark = eof;
    return noErr;
}

OSErr GetFPos(short refNum, long *position)
{
    OpenFile *o = FileFor(refNum);

    if (o == NULL)
        return rfNumErr;
    *position = o->mark;
    return noErr;
}

OSErr SetFPos(short refNum, short mode, long offset)
{
    OpenFile *o = FileFor(refNum);
    long length, at;

    if (o == NULL)
        return rfNumErr;
    length = gNodes[o->node].length;
    at = mode == fsFromStart ? offset
       : mode == fsFromLEOF ? length + offset
       : mode == fsFromMark ? o->mark + offset : o->mark;
    if (at < 0)
        return posErr;
    if (at > length)
    {
        o->mark = length;
        return eofErr;
    }
    o->mark = at;
    return noErr;
}

OSErr FlushVol(ConstStringPtr volName, short vRefNum)
{
    (void)volName;
    (void)vRefNum;
    return gMounted ? noErr : nsvErr;
}

OSErr HCreate(short vRefNum, long dirID, ConstStringPtr name, OSType creator,
              OSType type)
{
    long dir, n;
    OSErr err = ResolveDir(vRefNum, dirID, &dir);

    if (err != noErr)
        return err;
    if (gVolumeLocked)
        return vLckdErr;
    if (!ValidName(name, false))
        return bdNamErr;
    if (FindChild(dir, name) >= 0)
        return dupFNErr;
    n = NewNode(dir, name, false);
    gNodes[n].type = type;
    gNodes[n].creator = creator;
    return noErr;
}

OSErr HDelete(short vRefNum, long dirID, ConstStringPtr name)
{
    long n, i;
    OSErr err = Lookup(vRefNum, dirID, name, &n);

    if (err != noErr)
        return err;
    if (gVolumeLocked)
        return vLckdErr;
    if (gNodes[n].locked)
        return fLckdErr;
    if (IsOpen(n))
        return fBsyErr;
    if (gNodes[n].isDir)
        for (i = 0; i < gNodeCount; i++)
            if (gNodes[i].used && gNodes[i].parent == gNodes[n].id)
                return fBsyErr;
    gNodes[n].used = false;
    return noErr;
}

OSErr PBHGetFInfoSync(HParamBlockRec *pb)
{
    long n;
    OSErr err = Lookup(pb->fileParam.ioVRefNum, pb->fileParam.ioDirID,
                       pb->fileParam.ioNamePtr, &n);

    if (err == noErr && gNodes[n].isDir)
        err = fnfErr;
    if (err != noErr)
        return pb->fileParam.ioResult = err;
    memset(&pb->fileParam.ioFlFndrInfo, 0, sizeof(FInfo));
    pb->fileParam.ioFlFndrInfo.fdType = gNodes[n].type;
    pb->fileParam.ioFlFndrInfo.fdCreator = gNodes[n].creator;
    pb->fileParam.ioFlAttrib = (SignedByte)((gNodes[n].locked ? 0x01 : 0)
                                            | (IsOpen(n) ? 0x80 : 0));
    pb->fileParam.ioDirID = gNodes[n].id;
    return pb->fileParam.ioResult = noErr;
}

OSErr PBHSetFInfoSync(HParamBlockRec *pb)
{
    long n;
    OSErr err = Lookup(pb->fileParam.ioVRefNum, pb->fileParam.ioDirID,
                       pb->fileParam.ioNamePtr, &n);

    if (err == noErr && gNodes[n].isDir)
        err = fnfErr;
    if (err == noErr && gVolumeLocked)
        err = vLckdErr;
    if (err == noErr && gNodes[n].locked)
        err = fLckdErr;
    if (err != noErr)
        return pb->fileParam.ioResult = err;
    gNodes[n].type = pb->fileParam.ioFlFndrInfo.fdType;
    gNodes[n].creator = pb->fileParam.ioFlFndrInfo.fdCreator;
    return pb->fileParam.ioResult = noErr;
}

OSErr PBHRenameSync(HParamBlockRec *pb)
{
    ConstStringPtr newName = (ConstStringPtr)pb->ioParam.ioMisc;
    long n, other;
    OSErr err = Lookup(pb->ioParam.ioVRefNum, pb->fileParam.ioDirID,
                       pb->ioParam.ioNamePtr, &n);

    if (err != noErr)
        return pb->ioParam.ioResult = err;
    if (gVolumeLocked)
        return pb->ioParam.ioResult = vLckdErr;
    if (gNodes[n].locked)
        return pb->ioParam.ioResult = fLckdErr;
    if (!ValidName(newName, gNodes[n].isDir))
        return pb->ioParam.ioResult = bdNamErr;
    other = FindChild(gNodes[n].parent, newName);
    if (other >= 0 && other != n)
        return pb->ioParam.ioResult = dupFNErr;
    memcpy(gNodes[n].name, newName, newName[0] + 1);
    return pb->ioParam.ioResult = noErr;
}

OSErr PBDirCreateSync(HParamBlockRec *pb)
{
    long dir, n;
    OSErr err = ResolveDir(pb->fileParam.ioVRefNum, pb->fileParam.ioDirID,
                           &dir);

    if (err == noErr && gVolumeLocked)
        err = vLckdErr;
    if (err == noErr && !ValidName(pb->fileParam.ioNamePtr, true))
        err = bdNamErr;
    if (err == noErr && FindChild(dir, pb->fileParam.ioNamePtr) >= 0)
        err = dupFNErr;
    if (err != noErr)
        return pb->fileParam.ioResult = err;
    n = NewNode(dir, pb->fileParam.ioNamePtr, true);
    pb->fileParam.ioDirID = gNodes[n].id;
    return pb->fileParam.ioResult = noErr;
}

OSErr PBHGetVInfoSync(HParamBlockRec *pb)
{
    HVolumeParam *v = &pb->volumeParam;
    long dir, free = FreeBytesLeft();

    if (!gMounted)
        return v->ioResult = nsvErr;
    if (v->ioVolIndex > 1)
        return v->ioResult = nsvErr;
    if (v->ioVolIndex == 0 && ResolveDir(v->ioVRefNum, 0, &dir) != noErr)
        return v->ioResult = nsvErr;
    if (v->ioNamePtr != nil)
        memcpy(v->ioNamePtr, gVolumeName, gVolumeName[0] + 1);
    v->ioVRefNum = kVRefNum;
    v->ioVCrDate = gVolumeCreated;
    v->ioVAlBlkSiz = 512;
    if (free < 0)
        free = 0;
    v->ioVFrBlk = (unsigned short)(free / 512 > 65535 ? 65535 : free / 512);
    return v->ioResult = noErr;
}

OSErr PBGetWDInfoSync(WDPBRec *pb)
{
    long dir;
    OSErr err = ResolveDir(pb->ioVRefNum, 0, &dir);

    if (err != noErr)
        return pb->ioResult = err;
    pb->ioWDVRefNum = kVRefNum;
    pb->ioWDDirID = dir;
    return pb->ioResult = noErr;
}

OSErr PBCatMoveSync(CMovePBRec *pb)
{
    long n, dir;
    OSErr err = Lookup(pb->ioVRefNum, pb->ioDirID, pb->ioNamePtr, &n);

    if (err == noErr)
        err = ResolveDir(pb->ioVRefNum, pb->ioNewDirID, &dir);
    if (err == noErr && gVolumeLocked)
        err = vLckdErr;
    if (err == noErr && FindChild(dir, gNodes[n].name) >= 0)
        err = dupFNErr;
    if (err != noErr)
        return pb->ioResult = err;
    gNodes[n].parent = dir;
    return pb->ioResult = noErr;
}

/* ---- The clock and dates (1.1, the library's cache) --------------------- */

void TestFS_SetClock(unsigned long seconds, Boolean stopped)
{
    gClock = seconds;
    gClockStopped = stopped;
}

unsigned long TestFS_ModDate(long dir, const char *name)
{
    Str63 p;
    long n;

    ToPascal(name, p);
    n = FindChild(dir, p);
    return n < 0 ? 0 : gNodes[n].modDate;
}

void TestFS_SetModDate(long dir, const char *name, unsigned long date)
{
    Str63 p;
    long n;

    ToPascal(name, p);
    n = FindChild(dir, p);
    if (n >= 0)
        gNodes[n].modDate = date;
}

unsigned short TestFS_FinderFlags(long dir, const char *name)
{
    Str63 p;
    long n;

    ToPascal(name, p);
    n = FindChild(dir, p);
    return n < 0 ? 0 : gNodes[n].finderFlags;
}
