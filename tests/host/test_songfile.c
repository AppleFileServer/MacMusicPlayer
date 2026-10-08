/* Song files (songfile.c, song_format.h): the "iPod" chunk added to a
   WAV, read back, checked, rewritten and removed. */

#include "hosttest.h"
#include "fileopen.h"
#include "bytes.h"

static unsigned char gA[300000], gB[300000];

static long Root(void) { return TestFS_Root(); }

static Boolean Read(const char *name, SongInfo *info)
{
    return SongFile_Read(TestFS_VRefNum(), Root(), PS(name), info);
}

/* Rewrites a file with bytes changed by edit, then reads it as a song. */
static Boolean ReadAfter(void (*edit)(unsigned char *, long *))
{
    SongInfo info;
    long n = FileBytes(Root(), "IPOD0007", gA, sizeof gA);

    edit(gA, &n);
    TestFS_WriteFile(Root(), "IPOD0007", kSongFileType, kAppCreator, gA, n);
    return Read("IPOD0007", &info);
}

static long ChunkStart(const unsigned char *b, long n) { return (long)Get32(b + n - 8); }
static void CutLastByte(unsigned char *b, long *n)    { (void)b; (*n)--; }
static void BadEndMark(unsigned char *b, long *n)     { b[*n - 1] = 'X'; }
static void BadRIFFSize(unsigned char *b, long *n)    { (void)n; b[4]++; }
static void BadSeconds(unsigned char *b, long *n)
{ b[ChunkStart(b, *n) + 8 + kSongOffsetDuration + 3]++; }
static void BadVersion(unsigned char *b, long *n)
{ b[ChunkStart(b, *n) + 8 + kSongOffsetVersion + 1] = 9; }
static void BadRate(unsigned char *b, long *n)
{ b[ChunkStart(b, *n) + 8 + kSongOffsetSampleRate + 3]++; }
static void LongString(unsigned char *b, long *n)
{ b[ChunkStart(b, *n) + 8 + kSongOffsetStrings] = 64; }
static void ZeroID(unsigned char *b, long *n)
{ memset(b + ChunkStart(b, *n) + 8 + kSongOffsetID, 0, 4); }
static void BadChunkID(unsigned char *b, long *n)
{ b[ChunkStart(b, *n)] = 'X'; }
static void Unchanged(unsigned char *b, long *n) { (void)b; (void)n; }

int main(void)
{
    SongSpec s = { 7, "Here Comes the Sun", "The Beatles", "Abbey Road",
                   "Rock", "George Harrison", 7, kSongFlagAudiobook, 44100,
                   nil };
    SongInfo info, edited;
    long a, b, i;
    short refNum;
    char longTitle[64];

    Test("round trip");
    CHECK_EQ(MakeSong(Root(), &s), noErr);
    CHECK(Read("IPOD0007", &info));
    CHECK_EQ(info.id, 7);
    CHECK_EQ(info.track, 7);
    CHECK_EQ(info.flags, kSongFlagAudiobook);
    CHECK_EQ(info.seconds, 2);
    CHECK_EQ(info.audioOffset, 44);
    CHECK_EQ(info.audioLength, 44100);
    CHECK_EQ(info.originalLength, 44 + 44100);
    CHECK_EQ(info.originalRIFFSize, 36 + 44100);
    CHECK_EQ(info.originalType, 'WAVE');
    CHECK_EQ(info.originalCreator, 'TVOD');
    CHECK_STR(info.text[0], "Here Comes the Sun");
    CHECK_STR(info.text[1], "The Beatles");
    CHECK_STR(info.text[2], "Abbey Road");
    CHECK_STR(info.text[3], "Rock");
    CHECK_STR(info.text[4], "George Harrison");
    CHECK_STR(info.text[5], "original.wav");
    a = FileBytes(Root(), "IPOD0007", gA, sizeof gA);
    CHECK_EQ(GetLE32(gA + 4), a - 8);                   /* RIFF size */
    CHECK_EQ(ChunkStart(gA, a), 44 + 44100);
    CHECK(memcmp(gA + a - 4, "iPod", 4) == 0);
    CHECK_EQ((a - ChunkStart(gA, a)) % 2, 0);

    Test("odd original length: a pad byte, then the chunk");
    s.samples = 2205;
    CHECK_EQ(MakeSong(Root(), &s), noErr);
    a = FileBytes(Root(), "IPOD0007", gA, sizeof gA);
    CHECK_EQ(ChunkStart(gA, a), 44 + 2205 + 1);
    CHECK(Read("IPOD0007", &info));
    CHECK_EQ(info.seconds, 0);                          /* 0.1 s rounds down */
    s.samples = 44100;

    Test("damaged files are refused");
    MakeSong(Root(), &s);
    CHECK(ReadAfter(Unchanged));
    CHECK(!ReadAfter(CutLastByte));
    MakeSong(Root(), &s);  CHECK(!ReadAfter(BadEndMark));
    MakeSong(Root(), &s);  CHECK(!ReadAfter(BadRIFFSize));
    MakeSong(Root(), &s);  CHECK(!ReadAfter(BadSeconds));
    MakeSong(Root(), &s);  CHECK(!ReadAfter(BadVersion));
    MakeSong(Root(), &s);  CHECK(!ReadAfter(BadRate));
    MakeSong(Root(), &s);  CHECK(!ReadAfter(LongString));
    MakeSong(Root(), &s);  CHECK(!ReadAfter(ZeroID));
    MakeSong(Root(), &s);  CHECK(!ReadAfter(BadChunkID));
    TestFS_WriteFile(Root(), "plain", 'WAVE', 'TVOD', gA, 44 + 100);
    CHECK(!Read("plain", &info));
    CHECK(!Read("missing", &info));

    Test("removing the chunk restores the original WAV exactly");
    MakeSong(Root(), &s);
    CHECK(Read("IPOD0007", &info));
    CHECK_EQ(OpenDataFork(TestFS_VRefNum(), Root(), PS("IPOD0007"),
                          fsRdWrPerm, &refNum), noErr);
    CHECK_EQ(SongFile_RemoveChunk(refNum, &info), noErr);
    FSClose(refNum);
    b = FileBytes(Root(), "IPOD0007", gB, sizeof gB);
    CHECK_EQ(b, 44 + 44100);
    CHECK_EQ(GetLE32(gB + 4), 36 + 44100);
    CHECK(memcmp(gB, "RIFF", 4) == 0 && memcmp(gB + 36, "data", 4) == 0);
    for (i = 0; i < 44100 && gB[44 + i] == (unsigned char)(0x80 + ((i * 7 + 7 * 31) & 0x3F)); i++)
        ;
    CHECK_EQ(i, 44100);

    Test("the rating: two bytes rewritten, nothing else");
    MakeSong(Root(), &s);
    a = FileBytes(Root(), "IPOD0007", gA, sizeof gA);
    CHECK_EQ(SongFile_WriteFlags(TestFS_VRefNum(), Root(), PS("IPOD0007"), 7,
                                 SongFlags_WithRating(kSongFlagAudiobook, 4)),
             noErr);
    b = FileBytes(Root(), "IPOD0007", gB, sizeof gB);
    CHECK_EQ(b, a);
    for (i = 0; i < a; i++)
        if (gA[i] != gB[i]
            && i != ChunkStart(gA, a) + 8 + kSongOffsetFlags + 1)
            break;
    CHECK_EQ(i, a);
    CHECK(Read("IPOD0007", &info));
    CHECK_EQ(SongFlags_Rating(info.flags), 4);
    CHECK(info.flags & kSongFlagAudiobook);
    CHECK(SongFile_WriteFlags(TestFS_VRefNum(), Root(), PS("IPOD0007"), 8, 0)
          != noErr);                                    /* another song's ID */
    CHECK(Read("IPOD0007", &info));
    CHECK_EQ(SongFlags_Rating(info.flags), 4);

    Test("rating bits");
    CHECK_EQ(SongFlags_Rating(0), 0);
    CHECK_EQ(SongFlags_Rating(SongFlags_WithRating(0, 5)), 5);
    CHECK_EQ(SongFlags_Rating(6 << kSongFlagRatingShift), 0);    /* invalid */
    CHECK_EQ(SongFlags_Rating(7 << kSongFlagRatingShift), 0);
    CHECK_EQ(SongFlags_WithRating(kSongFlagAudiobook | 0x0E, 1),
             kSongFlagAudiobook | 0x02);

    Test("editing the details rewrites the chunk; the audio is untouched");
    MakeSong(Root(), &s);
    a = FileBytes(Root(), "IPOD0007", gA, sizeof gA);
    CHECK(Read("IPOD0007", &info));
    edited = info;
    memset(longTitle, 'x', 63);
    longTitle[63] = 0;
    edited.text[0][0] = 63;
    memcpy(edited.text[0] + 1, longTitle, 63);
    edited.text[4][0] = 0;
    edited.track = 12;
    CHECK_EQ(SongFile_WriteInfo(TestFS_VRefNum(), Root(), PS("IPOD0007"),
                                &edited), noErr);
    CHECK(Read("IPOD0007", &info));
    CHECK_STR(info.text[0], longTitle);
    CHECK_STR(info.text[4], "");
    CHECK_STR(info.text[1], "The Beatles");
    CHECK_EQ(info.track, 12);
    b = FileBytes(Root(), "IPOD0007", gB, sizeof gB);
    CHECK_EQ(GetLE32(gB + 4), b - 8);
    CHECK(memcmp(gA + 8, gB + 8, 36 + 44100) == 0);     /* after the RIFF size */
    edited.id = 99;
    CHECK(SongFile_WriteInfo(TestFS_VRefNum(), Root(), PS("IPOD0007"), &edited)
          != noErr);

    Test("fingerprints: equal for the same audio, different for other audio");
    {
        static char buffer[kSongFingerprintSample];
        unsigned long f7, f7again, f8;
        SongSpec t = s;

        MakeSong(Root(), &s);
        t.fileName = "copy";
        MakeSong(Root(), &t);
        t.id = 8;
        t.fileName = "other";
        MakeSong(Root(), &t);
        OpenDataFork(TestFS_VRefNum(), Root(), PS("IPOD0007"), fsRdPerm, &refNum);
        SongFile_Fingerprint(refNum, 44, 44100, buffer, &f7);
        FSClose(refNum);
        OpenDataFork(TestFS_VRefNum(), Root(), PS("copy"), fsRdPerm, &refNum);
        SongFile_Fingerprint(refNum, 44, 44100, buffer, &f7again);
        FSClose(refNum);
        OpenDataFork(TestFS_VRefNum(), Root(), PS("other"), fsRdPerm, &refNum);
        SongFile_Fingerprint(refNum, 44, 44100, buffer, &f8);
        FSClose(refNum);
        CHECK_EQ(f7, f7again);
        CHECK(f7 != f8);
        CHECK(Read("copy", &info));
        CHECK_EQ(info.fingerprint, f7);
    }

    return TestsDone("songfile");
}
