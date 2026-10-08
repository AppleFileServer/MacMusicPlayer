/* The host tests' stand-ins for the Toolbox calls, other than the File
   Manager (macfs.c), that MacMusic's portable modules make: the Memory
   Manager, string utilities, SysEnvirons, Random, Standard File and
   alerts, with controls for the tests (testfs.h). */

#include <ctype.h>
#include <stdio.h>

#include "testfs.h"
#include "Quickdraw.h"
#include "message.h"
#include "modal.h"

/* ---- Memory Manager ----------------------------------------------------- */

/* A handle is the address of its block's master pointer; the size is
   kept beside it. */
typedef struct { Ptr master; Size size; } Block;

static OSErr gMemErr;
static long gLimit, gInUse;

void TestMem_SetLimit(long bytes) { gLimit = bytes; }

static Boolean Fits(Size more)
{
    return gLimit == 0 || gInUse + more <= gLimit;
}

Handle NewHandle(Size size)
{
    Block *b;

    if (!Fits(size))
    {
        gMemErr = memFullErr;
        return nil;
    }
    b = malloc(sizeof(Block));
    b->master = malloc(size > 0 ? size : 1);
    b->size = size;
    gInUse += size;
    gMemErr = noErr;
    return &b->master;
}

Handle NewHandleClear(Size size)
{
    Handle h = NewHandle(size);

    if (h != nil)
        memset(*h, 0, size);
    return h;
}

void SetHandleSize(Handle h, Size size)
{
    Block *b = (Block *)h;

    if (size > b->size && !Fits(size - b->size))
    {
        gMemErr = memFullErr;
        return;
    }
    b->master = realloc(b->master, size > 0 ? size : 1);
    gInUse += size - b->size;
    b->size = size;
    gMemErr = noErr;
}

Size GetHandleSize(Handle h) { return ((Block *)h)->size; }

void DisposeHandle(Handle h)
{
    Block *b = (Block *)h;

    if (b == nil)
        return;
    gInUse -= b->size;
    free(b->master);
    free(b);
}

void HLock(Handle h) { (void)h; }
void HUnlock(Handle h) { (void)h; }
void HNoPurge(Handle h) { (void)h; }
void MoveHHi(Handle h) { (void)h; }
OSErr MemError(void) { return gMemErr; }

/* A pointer block carries its size just before it. */
Ptr NewPtr(Size size)
{
    Size *p;

    if (!Fits(size))
    {
        gMemErr = memFullErr;
        return nil;
    }
    p = malloc(sizeof(Size) + (size > 0 ? size : 1));
    *p = size;
    gInUse += size;
    gMemErr = noErr;
    return (Ptr)(p + 1);
}

Ptr NewPtrClear(Size size)
{
    Ptr p = NewPtr(size);

    if (p != nil)
        memset(p, 0, size);
    return p;
}

void DisposePtr(Ptr p)
{
    Size *s = (Size *)p - 1;

    if (p == nil)
        return;
    gInUse -= *s;
    free(s);
}

void BlockMove(const void *src, void *dest, Size count)
{
    memmove(dest, src, count);
}

/* ---- Strings ------------------------------------------------------------ */

Boolean EqualString(ConstStringPtr a, ConstStringPtr b, Boolean caseSens,
                    Boolean diacSens)
{
    int i;

    (void)diacSens;
    if (a[0] != b[0])
        return false;
    for (i = 1; i <= a[0]; i++)
        if (caseSens ? a[i] != b[i] : toupper(a[i]) != toupper(b[i]))
            return false;
    return true;
}

void NumToString(long n, StringPtr s)
{
    char b[32];

    snprintf(b, sizeof b, "%ld", n);
    s[0] = (unsigned char)strlen(b);
    memcpy(s + 1, b, s[0]);
}

void StringToNum(ConstStringPtr s, long *n)
{
    char b[300];

    memcpy(b, s + 1, s[0]);
    b[s[0]] = 0;
    *n = atol(b);
}

/* library.c calls the ROM's _RelString directly: case-insensitive, with
   both lengths in one long. */
long RelStringTrap(const unsigned char *a, const unsigned char *b, long lens)
{
    int la = (int)(lens >> 16), lb = (int)(lens & 0xFFFF), i;

    for (i = 0; i < la && i < lb; i++)
        if (toupper(a[i]) != toupper(b[i]))
            return toupper(a[i]) < toupper(b[i]) ? -1 : 1;
    return la == lb ? 0 : (la < lb ? -1 : 1);
}

void PStrAppend(Str255 dest, ConstStringPtr s)
{
    int n = s[0];

    if (dest[0] + n > 255)
        n = 255 - dest[0];
    memcpy(dest + dest[0] + 1, s + 1, n);
    dest[0] += n;
}

/* ---- System ------------------------------------------------------------- */

OSErr SysEnvirons(short versionRequested, SysEnvRec *env)
{
    (void)versionRequested;
    memset(env, 0, sizeof(*env));
    env->sysVRefNum = TestFS_NewWD(TestFS_SystemFolder());
    return noErr;
}

static unsigned long gSeed = 1;

void TestRandom_Seed(unsigned long seed) { gSeed = seed ? seed : 1; }

short Random(void)
{
    gSeed = gSeed * 1103515245UL + 12345;
    return (short)(gSeed >> 16);
}

/* ---- Standard File, alerts, messages ------------------------------------ */

static SFReply gReply;
static short gAnswer = 1, gLastAlert;
static int gMessages;
static char gLastMessage[300];

void TestSF_SetReply(Boolean good, short vRefNum, OSType type,
                     const char *name)
{
    memset(&gReply, 0, sizeof(gReply));
    gReply.good = good;
    gReply.vRefNum = vRefNum;
    gReply.fType = type;
    gReply.fName[0] = (unsigned char)strlen(name);
    memcpy(gReply.fName + 1, name, gReply.fName[0]);
}

void SFPPutFile(Point where, ConstStringPtr prompt, ConstStringPtr origName,
                DlgHookProcPtr hook, SFReply *reply, short dlgID,
                ModalFilterProcPtr filter)
{
    (void)where; (void)prompt; (void)origName; (void)hook; (void)dlgID;
    (void)filter;
    *reply = gReply;
}

void SFPGetFile(Point where, ConstStringPtr prompt, FileFilterProcPtr fileFilter,
                short numTypes, const OSType *typeList, DlgHookProcPtr hook,
                SFReply *reply, short dlgID, ModalFilterProcPtr filter)
{
    (void)where; (void)prompt; (void)fileFilter; (void)numTypes;
    (void)typeList; (void)hook; (void)dlgID; (void)filter;
    *reply = gReply;
}

short LMGetSFSaveDisk(void) { return 0; }
long LMGetCurDirStore(void) { return 0; }

void TestAlert_SetAnswer(short item) { gAnswer = item; }
short TestAlert_LastID(void) { return gLastAlert; }

short Alert(short alertID, ModalFilterProcPtr filter)
{
    (void)filter;
    gLastAlert = alertID;
    return gAnswer;
}

void GetDialogItem(DialogPtr d, short item, short *type, Handle *h, Rect *r)
{
    (void)d; (void)item;
    *type = 0;
    *h = nil;
    memset(r, 0, sizeof(*r));
}

void GetDialogItemText(Handle item, Str255 text)
{
    (void)item;
    text[0] = 0;
}

pascal Boolean Modal_DialogFilter(DialogPtr dialog, EventRecord *event,
                                  short *item)
{
    (void)dialog; (void)event; (void)item;
    return false;
}

pascal Boolean Modal_FileFilter(DialogPtr dialog, EventRecord *event,
                                short *item)
{
    (void)dialog; (void)event; (void)item;
    return false;
}

void Message_Show(ConstStringPtr text, OSErr err)
{
    gMessages++;
    snprintf(gLastMessage, sizeof gLastMessage, "%.*s (%d)", text[0],
             text + 1, err);
}

int TestMessage_Count(void) { return gMessages; }
const char *TestMessage_Last(void) { return gLastMessage; }
