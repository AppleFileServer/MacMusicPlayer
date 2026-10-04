#ifndef AUDIO_H
#define AUDIO_H

#include <MacTypes.h>

/* Real audio output (Stage 5b): one Sound Manager sampled sound looping
   over a ring buffer, filled from the main loop (decisions R1-R9; see
   audio.c). Plays 8-bit unsigned mono at the Mac Plus hardware rate of
   22254.5 Hz, about 0.9% fast for 22050 Hz songs (R3). */

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

/* Finds the Sound Manager and sets up the channel, ring and silencing
   task. Returns false if sound is unavailable (playback stays silent). */
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

/* Keeps the ring filled. Call often from the main loop while playing. */
void Audio_Service(void);

/* Samples of the current song played so far (0 to its length): what the
   hardware has consumed. */
long Audio_Position(void);

/* True once every song in the stream has played. */
Boolean Audio_Ended(void);

/* Times the main loop fell behind (heard as silence, R6). */
long Audio_Underruns(void);

/* Stops sound and releases everything; call before quitting. */
void Audio_Shutdown(void);

#endif
