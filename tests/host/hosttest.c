/* The host tests' checks and helpers (hosttest.h). */

#include "hosttest.h"
#include "fileopen.h"

int gChecks, gFailures;
const char *gTest = "";

void Test(const char *name)
{
    gTest = name;
    TestFS_Reset();
    TestMem_SetLimit(0);
    TestAlert_SetAnswer(1);
    TestRandom_Seed(1);
    Library_Clear();
}

static void Fail(const char *file, int line, const char *what,
                 const char *detail)
{
    gFailures++;
    printf("  FAIL %s (%s:%d): %s%s\n", gTest, file, line, what, detail);
}

void CheckAt(int ok, const char *file, int line, const char *what)
{
    gChecks++;
    if (!ok)
        Fail(file, line, what, "");
}

void CheckEq(long a, long b, const char *file, int line, const char *what)
{
    char detail[80];

    gChecks++;
    if (a != b)
    {
        snprintf(detail, sizeof detail, " (%ld, expected %ld)", a, b);
        Fail(file, line, what, detail);
    }
}

void CheckStr(ConstStringPtr p, const char *c, const char *file, int line,
              const char *what)
{
    char detail[340];

    gChecks++;
    if (p == nil || p[0] != strlen(c) || memcmp(p + 1, c, p[0]) != 0)
    {
        snprintf(detail, sizeof detail, " (\"%.*s\", expected \"%s\")",
                 p ? p[0] : 0, p ? (const char *)p + 1 : "", c);
        Fail(file, line, what, detail);
    }
}

int TestsDone(const char *program)
{
    CHECK_EQ(TestFS_OpenFiles(), 0);        /* nothing left open */
    printf("%-14s %4d checks, %d failed\n", program, gChecks, gFailures);
    return gFailures ? 1 : 0;
}

StringPtr PS(const char *c)
{
    static Str255 buffers[8];
    static int next;
    StringPtr p = buffers[next++ % 8];
    size_t n = strlen(c);

    if (n > 255)
        n = 255;
    p[0] = (unsigned char)n;
    memcpy(p + 1, c, n);
    return p;
}

static void SetText(Str63 out, const char *c)
{
    size_t n = c ? strlen(c) : 0;

    if (n > 63)
        n = 63;
    out[0] = (unsigned char)n;
    if (n)
        memcpy(out + 1, c, n);
}

OSErr MakeSong(long dir, const SongSpec *spec)
{
    static unsigned char wav[200000];
    static char fingerprintBuffer[kSongFingerprintSample];
    char name[32];
    long n = spec->samples > 0 ? spec->samples : 2206, i, c;
    SongInfo info;
    short refNum;
    OSErr err;

    if (spec->fileName)
        snprintf(name, sizeof name, "%s", spec->fileName);
    else
        snprintf(name, sizeof name, "IPOD%04ld", spec->id);
    memcpy(wav, "RIFF\0\0\0\0WAVEfmt \20\0\0\0\1\0\1\0\x22\x56\0\0"
                "\x22\x56\0\0\1\0\x08\0data", 40);
    c = 36 + n;
    wav[4] = c; wav[5] = c >> 8; wav[6] = c >> 16; wav[7] = 0;
    wav[40] = n; wav[41] = n >> 8; wav[42] = n >> 16; wav[43] = 0;
    for (i = 0; i < n; i++)
        wav[44 + i] = (unsigned char)(0x80 + ((i * 7 + spec->id * 31) & 0x3F));
    err = TestFS_WriteFile(dir, name, kSongFileType, kAppCreator, wav, 44 + n);
    if (err != noErr)
        return err;

    memset(&info, 0, sizeof info);
    info.id = spec->id;
    info.flags = spec->flags;
    info.track = spec->track;
    info.audioOffset = 44;
    info.audioLength = n;
    info.seconds = (n + 22050 / 2) / 22050;
    info.originalLength = 44 + n;
    info.originalRIFFSize = 36 + n;
    info.originalType = 'WAVE';
    info.originalCreator = 'TVOD';
    SetText(info.text[0], spec->title);
    SetText(info.text[1], spec->artist);
    SetText(info.text[2], spec->album);
    SetText(info.text[3], spec->genre);
    SetText(info.text[4], spec->composer);
    SetText(info.text[5], "original.wav");

    err = OpenDataFork(TestFS_VRefNum(), dir, PS(name), fsRdWrPerm, &refNum);
    if (err != noErr)
        return err;
    err = SongFile_Fingerprint(refNum, 44, n, fingerprintBuffer,
                               &info.fingerprint);
    if (err == noErr)
        err = SongFile_AddChunk(refNum, &info);
    FSClose(refNum);
    return err;
}

LibraryFolder NewLibrary(void)
{
    LibraryFolder f;

    f.vRefNum = TestFS_VRefNum();
    f.folderID = TestFS_MakeDir(TestFS_Root(), "MacMusic Library");
    f.playlistsID = TestFS_MakeDir(f.folderID, "Playlists");
    return f;
}

long FileBytes(long dir, const char *name, unsigned char *out, long max)
{
    return TestFS_ReadFile(dir, name, out, max);
}

void WritePlaylist(const LibraryFolder *f, const char *name, const char *text)
{
    TestFS_WriteFile(f->playlistsID, name, kPlaylistFileType, kAppCreator,
                     text, (long)strlen(text));
}

const char *PlaylistText(const LibraryFolder *f, const char *name)
{
    static char text[4096];
    long n = TestFS_ReadFile(f->playlistsID, name, text, sizeof text - 1), i;

    if (n < 0)
        return "(none)";
    if (n > (long)sizeof text - 1)
        n = sizeof text - 1;
    text[n] = 0;
    for (i = 0; i < n; i++)
        if (text[i] == '\r')
            text[i] = '|';
    return text;
}
