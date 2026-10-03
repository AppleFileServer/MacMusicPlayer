#include <Events.h>

#include "playback.h"
#include "library.h"
#include "song_format.h"

/* PROVISIONAL (N5, spec unknown #5): Previous restarts the current song
   once more than this has played; up to and including it, Previous goes
   to the previous song. */
#define kRestartThresholdTicks  (3 * 60)

#define kMaxQueue   kMaxSongs

static long gQueue[kMaxQueue];          /* song IDs */
static short gQueueLength;
static short gPosition;
static PlayState gState = kPlayStopped;

/* Elapsed time = gElapsedBase + (TickCount() - gResumedAt) while playing. */
static long gElapsedBase;
static unsigned long gResumedAt;
static long gLastSecond;

PlayState Playback_State(void)  { return gState; }
Boolean Playback_IsActive(void) { return gState != kPlayStopped; }
short Playback_Position(void)   { return gPosition; }
short Playback_QueueLength(void){ return gQueueLength; }

short Playback_CurrentSong(void)
{
    if (gState == kPlayStopped || gQueueLength == 0)
        return kNone;
    return Library_SongForID(gQueue[gPosition]);
}

long Playback_ElapsedTicks(void)
{
    if (gState == kPlayPlaying)
        return gElapsedBase + (long)(TickCount() - gResumedAt);
    return gElapsedBase;
}

long Playback_LengthTicks(void)
{
    short song = Playback_CurrentSong();

    if (song == kNone)
        return 0;
    /* Exact length from the audio: bytes * 60 / 22050. */
    return Library_SongAudioLength(song) * 60 / kSongSampleRate;
}

/* Starts the current position from 0:00, keeping play/pause state. */
static void RestartSong(void)
{
    gElapsedBase = 0;
    gResumedAt = TickCount();
    gLastSecond = 0;
}

void Playback_Stop(void)
{
    gState = kPlayStopped;
    gQueueLength = 0;
    gPosition = 0;
    gElapsedBase = 0;
}

void Playback_Start(const short *songs, short count, short start)
{
    short i;

    if (count <= 0 || start < 0 || start >= count)
        return;
    if (count > kMaxQueue)
        count = kMaxQueue;
    if (start >= count)
        start = 0;

    for (i = 0; i < count; i++)
        gQueue[i] = Library_SongID(songs[i]);
    gQueueLength = count;
    gPosition = start;
    gState = kPlayPlaying;
    RestartSong();
}

void Playback_TogglePause(void)
{
    if (gState == kPlayPlaying)
    {
        gElapsedBase = Playback_ElapsedTicks();
        gState = kPlayPaused;
    }
    else if (gState == kPlayPaused)
    {
        gResumedAt = TickCount();
        gState = kPlayPlaying;
    }
}

/* DECISION N6: Next on the last song wraps to the first song in the
   list. (Only the natural end of the last song stops playback, N4.) The
   play/pause state is kept (PROVISIONAL: unknown for the 4G). */
void Playback_Next(void)
{
    if (gState == kPlayStopped)
        return;
    gPosition = (gPosition + 1) % gQueueLength;
    RestartSong();
}

/* PROVISIONAL (N5): more than 3 seconds in, start the current song over;
   3 seconds or less, go to the previous song (on the first song, start it
   over). This matches the manual: "Press Previous/Rewind" starts over,
   "twice" plays the previous song. */
void Playback_Previous(void)
{
    if (gState == kPlayStopped)
        return;
    if (Playback_ElapsedTicks() <= kRestartThresholdTicks && gPosition > 0)
        gPosition--;
    RestartSong();
}

short Playback_Tick(void)
{
    short result = 0;
    long elapsed, second;

    if (gState == kPlayStopped)
        return 0;

    /* A song that has left the library is skipped. */
    if (Playback_CurrentSong() == kNone)
    {
        Playback_LibraryChanged();
        if (gState == kPlayStopped)
            return kTickStopped;
        result |= kTickSongChanged;
    }

    elapsed = Playback_ElapsedTicks();
    if (gState == kPlayPlaying && elapsed >= Playback_LengthTicks())
    {
        /* DECISION N4: at the natural end of the last song, stop. */
        if (gPosition + 1 >= gQueueLength)
        {
            Playback_Stop();
            return result | kTickStopped;
        }
        gPosition++;
        RestartSong();
        return result | kTickSongChanged | kTickTimeChanged;
    }

    second = elapsed / 60;
    if (second != gLastSecond)
    {
        gLastSecond = second;
        result |= kTickTimeChanged;
    }
    return result;
}

void Playback_LibraryChanged(void)
{
    long currentID;
    short i, n = 0, newPosition = 0;
    Boolean currentKept = false;

    if (gState == kPlayStopped)
        return;

    currentID = gQueue[gPosition];
    for (i = 0; i < gQueueLength; i++)
    {
        if (Library_SongForID(gQueue[i]) == kNone)
            continue;
        if (i == gPosition)
        {
            newPosition = n;
            currentKept = true;
        }
        else if (!currentKept && i < gPosition)
            newPosition = n + 1;
        gQueue[n++] = gQueue[i];
    }
    gQueueLength = n;

    if (n == 0)
    {
        Playback_Stop();
        return;
    }
    if (newPosition >= n)
        newPosition = 0;
    gPosition = newPosition;
    if (!currentKept || gQueue[gPosition] != currentID)
        RestartSong();
}
