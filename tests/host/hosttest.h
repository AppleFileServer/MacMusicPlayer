/* The host tests' checks and helpers (tests/host). Each test_*.c is a
   program of its own (so each module's statics start fresh), run by
   tests/run-host.sh. */
#ifndef HOSTTEST_H
#define HOSTTEST_H

#include <stdio.h>

#include "testfs.h"
#include "library.h"
#include "songfile.h"

extern int gChecks, gFailures;
extern const char *gTest;

/* Starts a test: empties the volume and resets the stand-ins. */
void Test(const char *name);

/* A check: quiet when it passes, the file, line and condition when not. */
#define CHECK(cond) CheckAt((cond) != 0, __FILE__, __LINE__, #cond)
#define CHECK_EQ(a, b) CheckEq((long)(a), (long)(b), __FILE__, __LINE__, \
                               #a " == " #b)
#define CHECK_STR(p, c) CheckStr((p), (c), __FILE__, __LINE__, #p)
void CheckAt(int ok, const char *file, int line, const char *what);
void CheckEq(long a, long b, const char *file, int line, const char *what);
void CheckStr(ConstStringPtr p, const char *c, const char *file, int line,
              const char *what);

/* Prints the totals; the program's exit status. */
int TestsDone(const char *program);

/* Pascal string from a C string (one of 8 rotating buffers). */
StringPtr PS(const char *c);

/* A song file like Import makes: a WAV of samples bytes (each byte from
   the ID and position, so songs' audio differs), the details chunk
   added. Fields left 0/nil are empty. fileName nil: "IPODnnnn". */
typedef struct
{
    long id;
    const char *title, *artist, *album, *genre, *composer;
    short track, flags;
    long samples;
    const char *fileName;
} SongSpec;
OSErr MakeSong(long dir, const SongSpec *spec);

/* A new library folder "MacMusic Library" in the root, with Playlists. */
LibraryFolder NewLibrary(void);

/* A file's bytes (up to max) and length; -1 if absent. */
long FileBytes(long dir, const char *name, unsigned char *out, long max);

/* Writes a playlist file with this text. */
void WritePlaylist(const LibraryFolder *f, const char *name, const char *text);

/* A playlist file's text as a C string (CR shown as '|'). */
const char *PlaylistText(const LibraryFolder *f, const char *name);

#endif
