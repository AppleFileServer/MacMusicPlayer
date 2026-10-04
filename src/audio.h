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

/* Starts playing source from sample startSample. Stops anything playing
   first. Returns an error if the file can't be opened or read. */
OSErr Audio_Play(const AudioSource *source, long startSample);

/* Stops playback and closes the file. */
void Audio_Stop(void);

/* Keeps the ring filled. Call often from the main loop while playing. */
void Audio_Service(void);

/* Samples played so far (0 to length): what the hardware has consumed. */
long Audio_Position(void);

/* True once the whole song has played. */
Boolean Audio_Ended(void);

/* Times the main loop fell behind (heard as silence, R6). */
long Audio_Underruns(void);

/* Stops sound and releases everything; call before quitting. */
void Audio_Shutdown(void);

#endif
