/* Checks a folder of MacMusic song files with MacMusic's own library code
   (src/library.c, src/songfile.c), built for this Mac: the File Manager
   and Memory Manager calls those files make are stood in for here, over
   the host file system, and the Mac headers by inc/.

   Usage: check <library folder> <song count>
   The folder holds the song files (IPOD0001...) and a Playlists folder.
   Prints the scan's counts and some songs; exits non-zero on a problem.

   dirID 100 is the library folder, 101 its Playlists folder; 0 means the
   name is a host path. */

#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#include "MacTypes.h"
#include "Files.h"
#include "library.h"
#include "songfile.h"

#define kMaxFiles   1024

static const char *gRoot;
static char gNames[2][kMaxFiles][64];   /* [0] the library, [1] Playlists */
static int gCount[2];

static void ListFolder(int k, const char *dir)
{
    DIR *d = opendir(dir);
    struct dirent *e;
    int i, j;
    char t[64];

    gCount[k] = 0;
    if (d == NULL)
        return;
    while ((e = readdir(d)) != NULL && gCount[k] < kMaxFiles)
        if (e->d_name[0] != '.')
            snprintf(gNames[k][gCount[k]++], 64, "%s", e->d_name);
    closedir(d);

    /* HFS lists a folder in name order. */
    for (i = 1; i < gCount[k]; i++)
        for (j = i; j > 0 && strcasecmp(gNames[k][j - 1], gNames[k][j]) > 0; j--)
        {
            strcpy(t, gNames[k][j]);
            strcpy(gNames[k][j], gNames[k][j - 1]);
            strcpy(gNames[k][j - 1], t);
        }
}

static void HostPath(char *out, long dirID, ConstStringPtr name)
{
    if (dirID == 0)
        sprintf(out, "%.*s", name[0], name + 1);
    else
        sprintf(out, "%s%s/%.*s", gRoot, dirID == 101 ? "/Playlists" : "",
                name[0], name + 1);
}

/* ---- File Manager stand-ins ------------------------------------------ */

OSErr PBGetCatInfoSync(CInfoPBRec *pb)
{
    int k = pb->hFileInfo.ioDirID == 101;
    int i = pb->hFileInfo.ioFDirIndex;
    const char *n;
    char p[512];
    struct stat st;

    if (i < 1 || i > gCount[k])
        return fnfErr;
    n = gNames[k][i - 1];
    pb->hFileInfo.ioNamePtr[0] = strlen(n);
    memcpy(pb->hFileInfo.ioNamePtr + 1, n, strlen(n));
    sprintf(p, "%s%s/%s", gRoot, k ? "/Playlists" : "", n);
    stat(p, &st);
    pb->hFileInfo.ioFlAttrib = S_ISDIR(st.st_mode) ? 0x10 : 0;
    pb->hFileInfo.ioFlFndrInfo.fdType =
        strcmp(n, "iPod Library Info") == 0 ? 'iPdL' : 'iPdS';
    return noErr;
}

static FILE *gFiles[8];

static OSErr HostOpen(long dirID, ConstStringPtr name, SignedByte perm,
                      short *refNum)
{
    char p[512];
    FILE *f;
    int i;

    HostPath(p, dirID, name);
    f = fopen(p, perm == fsRdPerm ? "rb" : "r+b");
    if (f == NULL)
        return fnfErr;
    for (i = 1; i < 8; i++)
        if (gFiles[i] == NULL)
        {
            gFiles[i] = f;
            *refNum = i;
            return noErr;
        }
    fclose(f);
    return -42;                         /* tmfoErr */
}

OSErr PBHOpenSync(HParamBlockRec *pb)
{
    return HostOpen(pb->fileParam.ioDirID, pb->ioParam.ioNamePtr,
                    pb->ioParam.ioPermssn, &pb->ioParam.ioRefNum);
}

OSErr OpenDataFork(short vRefNum, long dirID, ConstStringPtr name,
                   SignedByte perm, short *refNum)
{
    (void)vRefNum;
    return HostOpen(dirID, name, perm, refNum);
}

OSErr GetEOF(short r, long *eof)
{
    long pos = ftell(gFiles[r]);

    fseek(gFiles[r], 0, SEEK_END);
    *eof = ftell(gFiles[r]);
    fseek(gFiles[r], pos, SEEK_SET);
    return noErr;
}

OSErr SetEOF(short r, long eof)
{
    fflush(gFiles[r]);
    return ftruncate(fileno(gFiles[r]), eof) ? ioErr : noErr;
}

OSErr SetFPos(short r, short mode, long offset)
{
    long eof;

    if (mode == fsFromLEOF)
    {
        fseek(gFiles[r], offset, SEEK_END);
        return noErr;
    }
    GetEOF(r, &eof);
    if (offset > eof)
        return eofErr;
    fseek(gFiles[r], offset, SEEK_SET);
    return noErr;
}

OSErr FSRead(short r, long *count, void *buf)
{
    long want = *count;

    *count = fread(buf, 1, want, gFiles[r]);
    return *count < want ? eofErr : noErr;
}

OSErr FSWrite(short r, long *count, const void *buf)
{
    *count = fwrite(buf, 1, *count, gFiles[r]);
    return noErr;
}

OSErr FSClose(short r)
{
    fclose(gFiles[r]);
    gFiles[r] = NULL;
    return noErr;
}

/* Writing calls the scan never makes, for the linker. */
OSErr HCreate(short v, long d, ConstStringPtr n, OSType c, OSType t)
{ (void)v; (void)d; (void)n; (void)c; (void)t; return noErr; }
OSErr HDelete(short v, long d, ConstStringPtr n)
{ (void)v; (void)d; (void)n; return noErr; }
OSErr RenameFile(short v, long d, ConstStringPtr a, ConstStringPtr b)
{ (void)v; (void)d; (void)a; (void)b; return noErr; }
OSErr FlushVol(void *n, short v) { (void)n; (void)v; return noErr; }

/* ---- Memory Manager and Toolbox stand-ins ----------------------------- */

static OSErr gMemErr;

Handle NewHandle(Size s)
{
    Handle h = malloc(sizeof(Ptr));

    *h = malloc(s ? s : 1);
    return h;
}

void SetHandleSize(Handle h, Size s)
{
    *h = realloc(*h, s ? s : 1);
    gMemErr = *h ? noErr : memFullErr;
}

OSErr MemError(void) { return gMemErr; }
void MoveHHi(Handle h) { (void)h; }
void HLock(Handle h) { (void)h; }
void HUnlock(Handle h) { (void)h; }
void DisposeHandle(Handle h) { free(*h); free(h); }
Ptr NewPtr(Size s) { return malloc(s ? s : 1); }
void DisposePtr(Ptr p) { free(p); }
void BlockMove(const void *s, void *d, Size n) { memmove(d, s, n); }

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

    sprintf(b, "%ld", n);
    s[0] = strlen(b);
    memcpy(s + 1, b, s[0]);
}

void StringToNum(ConstStringPtr s, long *n)
{
    char b[300];

    memcpy(b, s + 1, s[0]);
    b[s[0]] = 0;
    *n = atol(b);
}

long RelStringTrap(const unsigned char *a, const unsigned char *b, long lens)
{
    int la = lens >> 16, lb = lens & 0xFFFF, i;

    for (i = 0; i < la && i < lb; i++)
        if (toupper(a[i]) != toupper(b[i]))
            return toupper(a[i]) < toupper(b[i]) ? -1 : 1;
    return la == lb ? 0 : (la < lb ? -1 : 1);
}

/* ---- The check -------------------------------------------------------- */

static void Show(ConstStringPtr s)
{
    printf("%.*s", s[0], s + 1);
}

int main(int argc, char **argv)
{
    static SongInfo info;
    static short list[600];
    LibraryFolder folder;
    LibFilter all;
    Str63 name;
    char playlists[512];
    int expected, problems = 0;
    short i, n;

    if (argc != 3)
    {
        fprintf(stderr, "usage: check <library folder> <song count>\n");
        return 2;
    }
    gRoot = argv[1];
    expected = atoi(argv[2]);
    ListFolder(0, gRoot);
    snprintf(playlists, sizeof playlists, "%s/Playlists", gRoot);
    ListFolder(1, playlists);

    folder.vRefNum = 0;
    folder.folderID = 100;
    folder.playlistsID = 101;
    if (!Library_Scan(&folder))
    {
        printf("FAIL: the scan ran out of memory\n");
        return 1;
    }

    LibFilter_Clear(&all);
    printf("songs %d, skipped %d, over the limit %d, next ID %ld\n",
           Library_SongCount(), Library_SkippedCount(),
           Library_OverLimitCount(), Library_NextSongID());
    printf("artists %d, albums %d\n",
           Library_ListArtists(&all, list, 600),
           Library_ListAlbums(&all, list, 600));
    if (Library_SongCount() != expected || Library_SkippedCount() != 0
        || Library_OverLimitCount() != 0)
    {
        printf("FAIL: expected %d songs, none skipped\n", expected);
        problems++;
    }

    for (i = 1; i <= expected; i++)
    {
        sprintf((char *)name + 1, "IPOD%04d", i);
        name[0] = 8;
        if (!SongFile_Read(0, 100, name, &info) || info.id != i)
        {
            printf("FAIL: IPOD%04d does not read back as song %d\n", i, i);
            problems++;
        }
    }

    n = Library_ListSongs(&all, list, 600);
    printf("some songs (title / artist / album, seconds):\n");
    for (i = 0; i < n; i += n / 10 > 0 ? n / 10 : 1)
    {
        printf("  ");
        Show(Library_SongTitle(list[i]));
        printf(" / ");
        Show(Library_SongArtistName(list[i]));
        printf(" / ");
        Show(Library_SongAlbumName(list[i]));
        printf("  (%ld)\n", Library_SongSeconds(list[i]));
    }
    printf(problems ? "%d problem(s)\n" : "OK\n", problems);
    return problems ? 1 : 0;
}
