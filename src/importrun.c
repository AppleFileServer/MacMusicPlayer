#include <Memory.h>

#include "importrun.h"
#include "library.h"

/* 1.1 (N11-25): the songs a run of Import Song has imported, in a handle
   that grows with each (about 70 bytes a song) and is freed when the run
   ends. If it can't grow, the song is still counted for the limit; only
   the duplicate warning for it is lost. */
typedef struct
{
    long audioLength;
    unsigned long fingerprint;
    Str63 title;
} RunSong;

static Handle gRunH;
static short gStored;               /* songs in gRunH */
static short gCount;                /* songs imported in the run */

void ImportRun_Begin(void)
{
    ImportRun_End();
}

void ImportRun_End(void)
{
    if (gRunH != nil)
        DisposeHandle(gRunH);
    gRunH = nil;
    gStored = gCount = 0;
}

void ImportRun_Add(long audioLength, unsigned long fingerprint, ConstStringPtr title)
{
    RunSong *song;
    short length = title[0] > 63 ? 63 : title[0];

    gCount++;
    if (gRunH == nil)
        gRunH = NewHandle(0);
    if (gRunH == nil)
        return;
    SetHandleSize(gRunH, (long)(gStored + 1) * sizeof(RunSong));
    if (MemError() != noErr)
        return;
    song = (RunSong *)*gRunH + gStored++;
    song->audioLength = audioLength;
    song->fingerprint = fingerprint;
    song->title[0] = (unsigned char)length;
    BlockMove(title + 1, song->title + 1, length);
}

short ImportRun_Count(void)
{
    return gCount;
}

Boolean ImportRun_LibraryFull(void)
{
    return Library_SongCount() + gCount >= kMaxSongs || Library_OverLimitCount() > 0;
}

Boolean ImportRun_FindDuplicate(long audioLength, unsigned long fingerprint,
                                Str255 title)
{
    short match = Library_FindAudioMatch(audioLength, fingerprint), i;
    const RunSong *song;

    if (match != kNone)
    {
        BlockMove(Library_SongTitle(match), title, Library_SongTitle(match)[0] + 1);
        return true;
    }
    for (i = 0; i < gStored; i++)
    {
        song = (const RunSong *)*gRunH + i;
        if (song->audioLength == audioLength && song->fingerprint == fingerprint)
        {
            BlockMove(song->title, title, song->title[0] + 1);
            return true;
        }
    }
    return false;
}
