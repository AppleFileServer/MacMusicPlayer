#ifndef AUDIO_H
#define AUDIO_H

#include <MacTypes.h>

/* Real audio output (Stage 5b): one Sound Manager sampled sound looping
   over a ring buffer, filled from the main loop (decisions R1-R9; see
   audio.c). Plays 8-bit unsigned mono at the Mac Plus hardware rate of
   22254.5 Hz, about 0.9% fast for 22050 Hz songs (R3). Stage 6a-4: the
   play position comes from the Sound Manager's own read pointer. */

/* Where a song's audio is. vRefNum/dirID/name locate the file; the audio
   starts at offset and is length bytes (= samples) long. */
typedef struct
{
    short vRefNum;
    long dirID;
    ConstStringPtr name;
    long offset;
    long length;
} AudioSource;

/* Finds the Sound Manager, sets up the channel and ring, and checks that
   the Sound Manager's read pointer can be followed (6a-4). Returns false
   if sound is unavailable (playback stays silent, A8). */
Boolean Audio_Init(void);
Boolean Audio_IsAvailable(void);

/* Starts playing source from sample startSample, as a new stream: stops
   anything playing first. tag identifies the song to the caller (its
   place in the play queue). Returns an error if the file can't be opened
   or read. Sound starts after about 4 s of audio is buffered (G2). */
OSErr Audio_Play(const AudioSource *source, long startSample, short tag);

/* Stage 5c, gapless playback (DECISION G1): adds the next song to the
   stream. Its audio follows the previous song's last sample in the
   ring, with no gap. Call whenever Audio_WantsNext() is true (G3: as
   soon as a song starts). A song whose file can't be opened is dropped,
   reported once by Audio_TakeFileError (R8). */
void Audio_Queue(const AudioSource *source, short tag);
Boolean Audio_WantsNext(void);
Boolean Audio_TakeFileError(void);

/* The tag of the song being heard now. */
short Audio_CurrentTag(void);

/* Stops playback and closes the file. */
void Audio_Stop(void);

/* Pause: records the current song's position (Audio_Position) and stops.
   Resume: plays source (the same song) from the recorded position, as a
   new stream. The Sound Manager's pauseCmd is not used: it did not work
   on the Plus (5a). Audio_Stop forgets the recorded position. */
long Audio_Pause(void);
long Audio_PausedPosition(void);
OSErr Audio_Resume(const AudioSource *source, short tag);

/* Stage 7c: while paused, moves the recorded position (a seek). */
void Audio_SetPausedPosition(long sample);

/* Follows the Sound Manager's read position and keeps the ring filled;
   restarts from the end of the valid audio after an underrun. Call often
   from the main loop while playing. */
void Audio_Service(void);

/* The single source of the play position: samples of the current song
   the Sound Manager has read (0 to its length), as of the last
   Audio_Service. */
long Audio_Position(void);

/* True once every song in the stream has played. */
Boolean Audio_Ended(void);

/* Underruns so far: times the main loop was held up longer than the
   ring lasts, each followed by a restart from the end of the audio. */
long Audio_Underruns(void);

/* Stops sound and releases everything; call before quitting. */
void Audio_Shutdown(void);

#endif
