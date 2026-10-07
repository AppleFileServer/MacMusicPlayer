#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <MacTypes.h>

/* Simulated playback (Stage 4): a queue of songs and a clock driven by
   TickCount. No audio yet. The queue holds stable song IDs, resolved to
   the current library whenever needed, so it survives rescans. */

typedef enum
{
    kPlayStopped = 0,
    kPlayPlaying,
    kPlayPaused
} PlayState;

/* Playback_Tick results. */
#define kTickTimeChanged    0x01    /* the displayed second changed */
#define kTickSongChanged    0x02    /* moved on to another song */
#define kTickStopped        0x04    /* reached the end and stopped */

/* Plays songs[start], then the rest of songs in order (N2). songs are
   library song indexes; count may be 0 (does nothing). */
void Playback_Start(const short *songs, short count, short start);

void Playback_Stop(void);
void Playback_TogglePause(void);
void Playback_Next(void);       /* on the last song: wraps to the first (N6) */
void Playback_Previous(void);

/* Stage 7c: moves the current song to ticks from its start (clamped to
   the song). Playing: the audio restarts there, as a new stream (with
   the usual pre-fill). Paused: it resumes from there. */
void Playback_Seek(long ticks);

/* Stage 6b-2: only keeps the audio fed (refill, underrun recovery, queueing
   the next song); no other state changes and no drawing, so it can be
   called from inside modal loops (menus, dialogs, window drags). */
void Playback_ServiceAudio(void);

/* Advances the clock; returns kTick... flags. Call often while playing. */
short Playback_Tick(void);

/* Re-resolves the queue after a rescan: songs no longer in the library
   are dropped; stops if none remain. */
void Playback_LibraryChanged(void);

/* True once after a song file could not be played and was skipped (R8). */
Boolean Playback_TakeFileError(void);

PlayState Playback_State(void);
Boolean Playback_IsActive(void);        /* playing or paused */
short Playback_CurrentSong(void);       /* library song index, or kNone */
short Playback_Position(void);          /* 0-based place in the queue */
short Playback_QueueLength(void);
long Playback_ElapsedTicks(void);
long Playback_LengthTicks(void);        /* of the current song */

#endif
