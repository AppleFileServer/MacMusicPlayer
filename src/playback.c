#include <Events.h>

#include "playback.h"
#include "library.h"
#include "song_format.h"
#include "audio.h"
#include "settings.h"
#include "shuffle.h"

/* PROVISIONAL (N5, spec unknown #5): Previous restarts the current song
   once more than this has played; up to and including it, Previous goes
   to the previous song. */
#define kRestartThresholdTicks  (3 * 60)

#define kMaxQueue   kMaxSongs

static long gQueue[kMaxQueue];          /* song IDs */
static short gQueueLength;
static short gPosition;
static PlayState gState = kPlayStopped;

/* Without sound: elapsed time = gElapsedBase + (TickCount() - gResumedAt)
   while playing. With sound, the audio position is the clock (A6): from
   the Sound Manager's read pointer (6a-4), and while paused the position
   Audio_Pause recorded. */
static long gElapsedBase;
static unsigned long gResumedAt;
static long gLastSecond;
static Boolean gFileError;              /* a song could not be played (R8) */
static short gQueuedUpTo;               /* last position handed to audio */
static short gShuffleMode;              /* the queue's order (9d, T5) */

/* Stage 9d: each song handed to the audio stream is tagged with a fresh
   sequence number, not its queue position, because with Repeat the same
   position comes round again (One) or goes back to 0 (All). This table
   gives the queue position (and song ID) for each recent sequence number;
   the stream holds at most a few songs, far fewer than kSeqSlots. */
#define kSeqSlots   16
#define kSeqWrap    20000               /* restarted with a new stream */
static short gSeqPosition[kSeqSlots];
static long gSeqID[kSeqSlots];
static short gNextSeq = 1;
static short gCurrentSeq;               /* the song gPosition is playing */

static short NewSeq(short position)
{
    short seq = gNextSeq++;

    gSeqPosition[seq % kSeqSlots] = position;
    gSeqID[seq % kSeqSlots] = gQueue[position];
    return seq;
}

/* Audio position in song time: samples at the song's own rate (N12-5:
   22050 or 22254 Hz), so the times and progress bar reach the song's
   length exactly at its end, though a 22050 Hz song plays 0.9% fast on
   the Plus (R3). */
static long SamplesToTicks(long samples, long rate)
{
    return SongRate_Ticks(samples, rate);   /* N12-8: no overflow */
}

/* The current song's rate (22050 if there's none, as 1.0's). */
static long CurrentRate(void)
{
    short song = Playback_CurrentSong();

    return song == kNone ? kSongSampleRate : Library_SongSampleRate(song);
}

PlayState Playback_State(void)  { return gState; }
Boolean Playback_IsActive(void) { return gState != kPlayStopped; }
short Playback_Position(void)   { return gPosition; }
short Playback_QueueLength(void){ return gQueueLength; }
Boolean Playback_IsShuffled(void) { return gShuffleMode != kShuffleOff; }

short Playback_CurrentSong(void)
{
    if (gState == kPlayStopped || gQueueLength == 0)
        return kNone;
    return Library_SongForID(gQueue[gPosition]);
}

long Playback_ElapsedTicks(void)
{
    if (Audio_IsAvailable() && gState == kPlayPlaying
        && Audio_CurrentTag() != gCurrentSeq)
        return Playback_LengthTicks();  /* moved on; Tick catches up */
    if (Audio_IsAvailable())
        return SamplesToTicks(gState == kPlayPlaying ? Audio_Position()
                                                     : Audio_PausedPosition(),
                              CurrentRate());
    if (gState == kPlayPlaying)
        return gElapsedBase + (long)(TickCount() - gResumedAt);
    return gElapsedBase;
}

Boolean Playback_TakeFileError(void)
{
    Boolean error = gFileError;

    gFileError = false;
    return error;
}

/* Where the song at queue position is, for the audio stream. */
static Boolean SourceAt(short position, AudioSource *source)
{
    short song = Library_SongForID(gQueue[position]);

    if (song == kNone
        || !Library_SongFile(song, &source->vRefNum, &source->dirID,
                             &source->name, &source->offset))
        return false;
    source->length = Library_SongAudioLength(song);
    return true;
}

/* Stage 9d (T5): Repeat All, at the end of a shuffled queue: shuffled
   again for the next time round, in the mode it was first shuffled
   with. Positions noted for the songs still in the stream are found
   again by song ID. */
static void Reshuffle(void)
{
    static short songs[kMaxQueue];
    short i, j, t;
    long playing = gQueue[gPosition];
    long last = gQueue[gQueuedUpTo];    /* the round's last song: queued
                                           ahead, perhaps not yet heard */

    for (i = 0; i < gQueueLength; i++)
        if ((songs[i] = Library_SongForID(gQueue[i])) == kNone)
            return;                     /* changed meanwhile: leave it */
    Shuffle_Order(songs, gQueueLength, 0, false, gShuffleMode);
    /* Not the round's last song straight away again (Songs mode). */
    if (gShuffleMode == kShuffleSongs && gQueueLength > 1
        && Library_SongID(songs[0]) == last)
    {
        j = 1 + (short)((unsigned short)Random() % (gQueueLength - 1));
        t = songs[0];
        songs[0] = songs[j];
        songs[j] = t;
    }
    for (i = 0; i < gQueueLength; i++)
        gQueue[i] = Library_SongID(songs[i]);

    for (j = 0; j < kSeqSlots; j++)
        for (i = 0; i < gQueueLength; i++)
            if (gQueue[i] == gSeqID[j])
            {
                gSeqPosition[j] = i;
                break;
            }
    for (i = 0; i < gQueueLength; i++)
        if (gQueue[i] == playing)
        {
            gPosition = i;
            break;
        }
}

/* Stage 9d: the queue position after position p as the Repeat setting
   has it, or -1 at the end. Off: the next, and the end after the last
   (N4). One: the same song again. All: after the last, the first (the
   queue reshuffled first if it was shuffled, T5). Next and Previous
   don't use this: they still move to another song (S-3). */
static short NextPosition(short p, Boolean reshuffle)
{
    short repeat = Settings_Get(kSettingRepeat);

    if (repeat == kRepeatOne)
        return p;
    if (p + 1 < gQueueLength)
        return p + 1;
    if (repeat != kRepeatAll)
        return -1;
    if (reshuffle && gShuffleMode != kShuffleOff)
        Reshuffle();
    return 0;
}

/* Stage 5c, DECISION G3: the song after the last one handed to the audio
   stream is queued as soon as the stream can take it, so its audio
   follows without a gap however slowly the main loop runs. With Repeat
   Off the last song in the queue has nothing after it, so its natural
   end stops (N4); with Repeat One or All there is always a next song. */
static void QueueAhead(void)
{
    AudioSource source;
    short next, tries = 0;

    while (gState == kPlayPlaying && Audio_WantsNext()
           && tries++ <= gQueueLength
           && (next = NextPosition(gQueuedUpTo, true)) >= 0)
    {
        gQueuedUpTo = next;
        if (SourceAt(gQueuedUpTo, &source))
            Audio_Queue(&source, NewSeq(gQueuedUpTo));
    }
}

/* Starts the current song's audio as a new stream: at sample, or (resume)
   where Audio_Pause recorded. */
static OSErr StartCurrent(long sample, Boolean resume)
{
    AudioSource source;
    OSErr err;

    if (!SourceAt(gPosition, &source))
        return fnfErr;
    if (gNextSeq > kSeqWrap)
        gNextSeq = 1;                   /* a new stream: nothing in it */
    gCurrentSeq = NewSeq(gPosition);
    err = resume ? Audio_Resume(&source, gCurrentSeq)
                 : Audio_Play(&source, sample, gCurrentSeq);
    if (err == noErr)
    {
        gQueuedUpTo = gPosition;
        QueueAhead();
    }
    return err;
}

/* Gapless songs change without a restart: when the audio has moved on to
   a later song, the current position follows it. True if it moved. */
static Boolean FollowAudio(void)
{
    short tag;

    if (gState != kPlayPlaying || !Audio_IsAvailable())
        return false;
    tag = Audio_CurrentTag();
    if (tag <= 0 || (short)(tag - gCurrentSeq) <= 0
        || gSeqPosition[tag % kSeqSlots] >= gQueueLength)
        return false;
    gCurrentSeq = tag;
    gPosition = gSeqPosition[tag % kSeqSlots];
    gLastSecond = -1;
    return true;
}

long Playback_LengthTicks(void)
{
    short song = Playback_CurrentSong();

    if (song == kNone)
        return 0;
    /* Exact length from the audio: bytes * 60 / the song's rate. */
    return SamplesToTicks(Library_SongAudioLength(song),
                          Library_SongSampleRate(song));
}

void Playback_Stop(void)
{
    Audio_Stop();
    gState = kPlayStopped;
    gQueueLength = 0;
    gPosition = 0;
    gElapsedBase = 0;
}

/* Starts the current position from 0:00, keeping play/pause state. With
   sound, a song that can't be played is skipped (DECISION R8): the next
   one in the queue is tried; if none can play, playback stops. */
static void RestartSong(void)
{
    gElapsedBase = 0;
    gResumedAt = TickCount();
    gLastSecond = 0;

    if (!Audio_IsAvailable())
        return;
    if (gState != kPlayPlaying)
    {
        Audio_Stop();                   /* paused: from 0:00 when resumed */
        return;
    }
    while (StartCurrent(0, false) != noErr)
    {
        gFileError = true;
        if (gPosition + 1 >= gQueueLength)
        {
            Playback_Stop();
            return;
        }
        gPosition++;
    }
}

void Playback_Start(const short *songs, short count, short start,
                    short shuffle)
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
    gShuffleMode = shuffle;
    gState = kPlayPlaying;
    RestartSong();
}

void Playback_TogglePause(void)
{
    if (gState == kPlayPlaying)
    {
        FollowAudio();
        if (Audio_IsAvailable())
            Audio_Pause();              /* records the position */
        gElapsedBase = Playback_ElapsedTicks();
        gState = kPlayPaused;
    }
    else if (gState == kPlayPaused)
    {
        gResumedAt = TickCount();
        gState = kPlayPlaying;
        /* Re-opened by the song's current index (found by ID), so this
           also works after a rescan (A5: import pauses and resumes). */
        if (Audio_IsAvailable() && StartCurrent(0, true) != noErr)
        {
            gFileError = true;
            RestartSong();              /* R8: skip to a playable song */
        }
    }
}

/* DECISION N6: Next on the last song wraps to the first song in the
   list. (Only the natural end of the last song stops playback, N4.) The
   play/pause state is kept (PROVISIONAL: unknown for the 4G). */
void Playback_Next(void)
{
    if (gState == kPlayStopped)
        return;
    FollowAudio();
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
    FollowAudio();
    if (Playback_ElapsedTicks() <= kRestartThresholdTicks && gPosition > 0)
        gPosition--;
    RestartSong();
}

void Playback_Seek(long ticks)
{
    short song;
    long length, sample, rate;

    if (gState == kPlayStopped)
        return;
    FollowAudio();
    song = Playback_CurrentSong();
    if (song == kNone)
        return;
    length = Library_SongAudioLength(song);
    rate = Library_SongSampleRate(song);
    if (ticks < 0)
        ticks = 0;
    /* In two parts, so a long song can't overflow. */
    sample = ticks / 60 * rate + ticks % 60 * rate / 60;
    if (sample > length)
        sample = length;
    gElapsedBase = SamplesToTicks(sample, rate);
    gResumedAt = TickCount();
    gLastSecond = -1;

    if (!Audio_IsAvailable())
        return;
    if (gState == kPlayPaused)
    {
        Audio_SetPausedPosition(sample);
        return;
    }
    if (StartCurrent(sample, false) != noErr)
    {
        gFileError = true;
        RestartSong();                  /* R8 */
    }
}

void Playback_ServiceAudio(void)
{
    if (gState != kPlayPlaying || !Audio_IsAvailable())
        return;
    Audio_Service();
    QueueAhead();
}

short Playback_Tick(void)
{
    short result = 0, next;
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

    if (gState == kPlayPlaying && Audio_IsAvailable())
    {
        Audio_Service();
        QueueAhead();
        if (Audio_TakeFileError())
            gFileError = true;          /* a queued song was skipped (R8) */
        if (FollowAudio())
            result |= kTickSongChanged | kTickTimeChanged;
    }

    elapsed = Playback_ElapsedTicks();
    if (gState == kPlayPlaying
        && (Audio_IsAvailable() ? Audio_Ended()
                                : elapsed >= Playback_LengthTicks()))
    {
        /* DECISION N4: at the natural end of the last song, stop (Repeat
           Off). With sound, the stream ran out early only if the songs
           after it could not be queued (gone from the library): try the
           next. Without sound, Repeat decides the next song here (9d). */
        next = NextPosition(Audio_IsAvailable() ? gQueuedUpTo : gPosition,
                            !Audio_IsAvailable());
        if (next < 0)
        {
            Playback_Stop();
            return result | kTickStopped;
        }
        gPosition = next;
        RestartSong();
        if (gState == kPlayStopped)     /* R8: nothing left that plays */
            return result | kTickStopped;
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
    long currentID, sample = 0;
    short i, n = 0, newPosition = 0, oldLength = gQueueLength;
    Boolean currentKept = false;

    if (gState == kPlayStopped)
        return;

    /* The audio stream knows songs by queue position, which may shift. */
    FollowAudio();
    if (gState == kPlayPlaying && Audio_IsAvailable())
        sample = Audio_Position();

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
    else if (n != oldLength && gState == kPlayPlaying && Audio_IsAvailable()
             && StartCurrent(sample, false) != noErr)
    {
        gFileError = true;
        RestartSong();                  /* R8 */
    }
}
