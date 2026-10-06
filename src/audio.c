#include <Files.h>
#include <Memory.h>
#include <OSUtils.h>
#include <Traps.h>
#include <Sound.h>
#include <string.h>

#include "audio.h"
#include "fileopen.h"

/* How it works (chosen by the Stage 5a tests, decisions R1-R9):

   The Sound Manager plays one sampled sound whose loop covers the whole
   ring, started once per stream, so there are no buffer boundaries (the
   request-based paths left a gap at every buffer on the Plus). The main
   loop reads the files into the ring ahead of the play position.

   Stage 6a-4 (simplified design): the play position is the Sound
   Manager's own read pointer into the ring, never a time calculation.
   The earlier "ticks since start x 370" was shown in Mini vMac at 1x to
   drift: under heavy Finder disk work the Sound Manager falls whole ticks
   behind the tick count, never to catch up. Sound Manager 2.0 has no call
   that reports a looped sound's position (SndChannelStatus's times stay
   0), but its sampled-sound player keeps a pointer to the next sample it
   will read, in the channel's private data; measured against the samples
   in the hardware sound buffer it stayed exactly in step, under load as
   well. It is undocumented, so Audio_Init checks it before relying on it,
   and sound is reported as unavailable if it doesn't behave.

   Ring slot for stream sample X = (X - gOrigin) mod kRingBytes. gConsumed
   counts the samples the Sound Manager has read, advanced by the
   pointer's movement since the last look (with wrap-around). The main
   loop writes up to gConsumed + kRingBytes - kGuard. If it finds the
   Sound Manager has read past what was written (the app was held up
   longer than the ring lasts), it stops the sound, counts an underrun
   and restarts from the end of the valid audio: heard as a pause.

   Stage 5c (gapless, DECISION G1): the ring carries a stream, not one
   song. Sample positions count from the start of the stream; a short
   list of songs (gEntries) says where each starts in it. When the main
   loop finishes reading one song it opens the next and writes its first
   sample straight after the last one, so the sound never stops between
   songs. After the stream's last song the ring is filled with silence. */

/* Constants missing from Retro68's headers (Inside Macintosh). */
enum { kQuietCmd = 3, kFreqCmd = 42, kSoundCmd = 80 };
#define kInitMono           0x0080
#define kRate22254          0x56EE8BA3L     /* hardware rate, Fixed */
#define kMiddleC            60
#define kSoundDispatchTrap  0xA800
#define kUnimplementedTrap  0xA89F

#define kSamplesPerTick     370L
#define kRingBytes          (400L * kSamplesPerTick)   /* 148000, ~6.65 s (R2) */
#define kGuard              (8L * kSamplesPerTick)     /* write margin */
#define kMinRefill          8192L                       /* R4: no tiny reads */
#define kStartFill          (240L * kSamplesPerTick)   /* G2: about 4 s */
#define kMaxEntries         8           /* songs in the ring at once */
#define kNotStarted         0x7FFFFFFFL /* streamStart before reading */
#define kSilence            0x80

/* The Sound Manager 2.0 sampled-sound player's read pointer: in the
   private block that the channel's first modifier stub's userInfo points
   to, at this byte offset (found by the 6a-4 probe). */
#define kPrivateBlockWord   2           /* stub long holding the block */
#define kReadPointerOffset  148
#define kCheckTicks         12          /* Audio_Init's pointer check */

static Boolean gAvailable;
static SndChannelPtr gChan;
static Ptr gBlock;                      /* SoundHeader + ring */
static SoundHeader *gHeader;
static unsigned char *gRing;

/* A song in the stream. Its sample first+k is at stream position
   streamStart+k; streamStart is kNotStarted until reading reaches it. */
typedef struct
{
    long streamStart;
    long first;                 /* first sample played (resume point) */
    long length;                /* the song's samples */
    long offset;                /* first audio byte in the file */
    short vRefNum;
    long dirID;
    Str63 name;
    short tag;
} Entry;

static Entry gEntries[kMaxEntries];     /* oldest first */
static short gEntryCount;
static short gReading;                  /* entry being read; gEntryCount
                                           once everything is read */
static short gRefNum;                   /* its open file, 0 if none */
static Boolean gFileFailed;             /* a queued song was dropped */

static Boolean gActive;                 /* a stream is playing */
static long gOrigin;                    /* stream sample at ring[0] */
static long gWritten;                   /* stream samples written */
static long gPadded;                    /* ring silent up to here after the end */
static long gUnderruns;
static long gPausedPosition;            /* song sample, while paused */
static Boolean gEndedEarly;             /* an underrun restart failed */

/* The play position (6a-4). */
static unsigned char * volatile *gReadPointer;  /* the Sound Manager's */
static long gConsumed;                  /* stream samples read by it */
static long gLastOffset;                /* its ring offset at the last look */
static unsigned long gLastLook;         /* when (ticks) */
static short gBadLooks;                 /* consecutive looks outside the ring */

static unsigned long Ticks(void)
{
    return *(volatile unsigned long *)0x016A;   /* low-memory Ticks */
}

static long Slot(long sample)
{
    return (sample - gOrigin) % kRingBytes;
}

/* The channel's read pointer field, or nil if the private block isn't
   where Sound Manager 2.0 keeps it. */
static unsigned char * volatile *FindReadPointer(void)
{
    long *stub, block;

    if (gChan == nil || (stub = (long *)gChan->firstMod) == nil)
        return nil;
    block = stub[kPrivateBlockWord];
    if (block < 0x1000 || block > 0x00FFFFFFL || (block & 1))
        return nil;
    return (unsigned char * volatile *)(block + kReadPointerOffset);
}

/* The read pointer's offset into the ring, or -1 if it isn't in it. At
   the loop's wrap it can read as the ring's end for a moment; that is
   the start again. */
static long ReadOffset(void)
{
    long offset;

    if (gReadPointer == nil)
        return -1;
    offset = *gReadPointer - gRing;
    if (offset == kRingBytes)
        offset = 0;
    return offset >= 0 && offset < kRingBytes ? offset : -1;
}

static OSErr StartLoop(void);
static void Quiet(void);

/* Starts the ring playing from ring[0] and confirms it really started:
   the read pointer must move, from near the ring's start, within
   kStartWait ticks. Measured in the 6a-4 tests: after quietCmd, every
   other start does nothing (the Sound Manager apparently still treats
   the note as on, so freqCmd only changes its pitch), and the pointer
   stays where it was. Such a start is quieted and retried. Returns the
   pointer's offset, or -1 if the sound could not be started. */
#define kStartWait          4
#define kStartTries         3
#define kStartWindow        (4L * kSamplesPerTick)
static long StartVerified(void)
{
    unsigned long until;
    long first, offset;
    short tries;

    for (tries = 0; tries < kStartTries; tries++)
    {
        if (tries > 0)
            Quiet();
        if (StartLoop() != noErr)
            return -1;
        gReadPointer = FindReadPointer();
        first = ReadOffset();
        until = Ticks() + kStartWait;
        do
        {
            offset = ReadOffset();
            if (offset >= 0 && offset < kStartWindow && offset != first)
                return offset;
        } while (Ticks() < until);
    }
    Quiet();
    return -1;
}

/* 6a-4: plays the (silent) ring for kCheckTicks and checks the read
   pointer: inside the ring, and moved forward by about the samples the
   hardware plays in that time (370 a tick, give or take a few ticks for
   the start). The pointer is undocumented, so if it doesn't behave the
   app plays without sound rather than guessing positions. */
static Boolean CheckReadPointer(void)
{
    unsigned long until;
    long before, after, moved;

    memset(gRing, kSilence, kRingBytes);
    before = StartVerified();
    until = Ticks() + kCheckTicks;
    while (Ticks() < until)
        ;
    after = ReadOffset();
    Quiet();

    if (before < 0 || after < 0)
        return false;
    moved = after - before;
    if (moved < 0)
        moved += kRingBytes;
    return moved >= (kCheckTicks - 4) * kSamplesPerTick
        && moved <= (kCheckTicks + 4) * kSamplesPerTick;
}

Boolean Audio_IsAvailable(void)
{
    return gAvailable;
}

Boolean Audio_Init(void)
{
    if (NGetTrapAddress(kSoundDispatchTrap, kToolboxTrapType)
        == NGetTrapAddress(kUnimplementedTrap, kToolboxTrapType))
        return false;                           /* no Sound Manager 2.0 */

    gBlock = NewPtrClear(sizeof(SoundHeader) + kRingBytes);
    if (gBlock == nil)
        return false;
    gHeader = (SoundHeader *)gBlock;
    gRing = gHeader->sampleArea;
    memset(gRing, kSilence, kRingBytes);

    if (SndNewChannel(&gChan, sampledSynth, kInitMono, nil) != noErr)
    {
        gChan = nil;
        DisposePtr(gBlock);
        gBlock = nil;
        return false;
    }

    if (!CheckReadPointer())
    {
        SndDisposeChannel(gChan, true);
        gChan = nil;
        DisposePtr(gBlock);
        gBlock = nil;
        return false;
    }

    gAvailable = true;
    return true;
}

/* Starts the ring playing from ring[0], looping, until Quiet. */
static OSErr StartLoop(void)
{
    SndCommand c;
    OSErr err;

    gHeader->samplePtr = nil;
    gHeader->length = kRingBytes;
    gHeader->sampleRate = kRate22254;
    gHeader->loopStart = 0;
    gHeader->loopEnd = kRingBytes;
    gHeader->encode = stdSH;
    gHeader->baseFrequency = kMiddleC;

    c.cmd = kSoundCmd;
    c.param1 = 0;
    c.param2 = (long)gHeader;
    err = SndDoImmediate(gChan, &c);
    if (err == noErr)
    {
        c.cmd = kFreqCmd;                       /* play, looping, until quiet */
        c.param1 = 0;
        c.param2 = kMiddleC;
        err = SndDoImmediate(gChan, &c);
    }
    return err;
}

static void Quiet(void)
{
    SndCommand c;

    gActive = false;
    if (gChan != nil)
    {
        c.cmd = kQuietCmd;
        c.param1 = 0;
        c.param2 = 0;
        SndDoImmediate(gChan, &c);
    }
}

static long EntryEnd(const Entry *e)
{
    return e->streamStart + (e->length - e->first);
}

static void RemoveEntry(short i)
{
    for (; i < gEntryCount - 1; i++)
        gEntries[i] = gEntries[i + 1];
    gEntryCount--;
}

static void CloseFile(void)
{
    if (gRefNum != 0)
        FSClose(gRefNum);
    gRefNum = 0;
}

/* Starts reading entry gReading at stream position at: opens its file.
   Entries whose files can't be opened are dropped (R8) and the next one
   is tried. */
static void StartReading(long at)
{
    Entry *e;

    while (gReading < gEntryCount)
    {
        e = &gEntries[gReading];
        if (OpenDataFork(e->vRefNum, e->dirID, e->name, fsRdPerm, &gRefNum)
            == noErr)
        {
            e->streamStart = at;
            return;
        }
        gRefNum = 0;
        gFileFailed = true;
        RemoveEntry(gReading);
    }
}

/* Writes stream samples [gWritten, upTo) into the ring from the songs'
   files, moving from one song to the next without a gap. Each read takes
   all it can up to the end of the ring or of the song. Stops at the end
   of the last song. A read error ends that song where it is. */
static OSErr Fill(long upTo)
{
    Entry *e;
    long slot, n, count;
    OSErr err = noErr;

    while (gWritten < upTo && gReading < gEntryCount)
    {
        e = &gEntries[gReading];
        if (gWritten >= EntryEnd(e))
        {
            CloseFile();
            gReading++;
            StartReading(gWritten);
            continue;
        }

        slot = Slot(gWritten);
        n = upTo - gWritten;
        if (n > kRingBytes - slot)
            n = kRingBytes - slot;
        if (n > EntryEnd(e) - gWritten)
            n = EntryEnd(e) - gWritten;

        count = n;
        err = SetFPos(gRefNum, fsFromStart,
                      e->offset + e->first + (gWritten - e->streamStart));
        if (err == noErr)
            err = FSRead(gRefNum, &count, gRing + slot);
        if (err == eofErr && count > 0)
            err = noErr;
        if (err != noErr || count <= 0)
        {
            if (err == noErr)
                err = eofErr;
            e->length = e->first + (gWritten - e->streamStart);
            gFileFailed = true;
            continue;                   /* ends here; on to the next */
        }
        gWritten += count;
    }
    return err;
}

void Audio_Stop(void)
{
    Quiet();
    CloseFile();
    gEntryCount = 0;
    gReading = 0;
    gPausedPosition = 0;
    gEndedEarly = false;
}

static void AddEntry(const AudioSource *source, long first, short tag)
{
    Entry *e = &gEntries[gEntryCount++];

    e->streamStart = kNotStarted;
    e->first = first;
    e->length = source->length;
    e->offset = source->offset;
    e->vRefNum = source->vRefNum;
    e->dirID = source->dirID;
    BlockMove(source->name, e->name, source->name[0] + 1);
    e->tag = tag;
}

/* Starts the sound with the ring holding the stream from position at
   onward: ring[0] is stream sample at, which is where the Sound Manager
   starts reading. G2: about 4 s are read first (not the whole ring), so
   playback starts sooner; the main loop fills the rest. The song being
   read (gReading, its file open) must be the one containing at. */
static OSErr BeginStream(long at)
{
    OSErr err;

    memset(gRing, kSilence, kRingBytes);
    gOrigin = at;
    gWritten = at;
    gPadded = at;
    err = Fill(at + kStartFill);
    if (err != noErr && gWritten == at)
        return err;

    gLastOffset = StartVerified();
    if (gLastOffset < 0)
        return notOpenErr;              /* no start, or the pointer has gone */
    gConsumed = at + gLastOffset;
    gLastLook = Ticks();
    gBadLooks = 0;
    gActive = true;
    return noErr;
}

OSErr Audio_Play(const AudioSource *source, long startSample, short tag)
{
    OSErr err;

    if (!gAvailable)
        return notOpenErr;

    Audio_Stop();
    if (startSample < 0 || startSample > source->length)
        startSample = 0;

    gFileFailed = false;

    AddEntry(source, startSample, tag);
    gReading = 0;
    err = OpenDataFork(source->vRefNum, source->dirID, source->name,
                       fsRdPerm, &gRefNum);
    if (err != noErr)
    {
        gRefNum = 0;
        Audio_Stop();
        return err;
    }
    gEntries[0].streamStart = 0;

    err = BeginStream(0);
    if (err != noErr)
        Audio_Stop();
    gFileFailed = false;                /* only the first song, reported
                                           through the return value */
    return err;
}

void Audio_Queue(const AudioSource *source, short tag)
{
    long played, at;

    if (!gActive || gEntryCount >= kMaxEntries)
        return;

    AddEntry(source, 0, tag);
    if (gReading < gEntryCount - 1)
        return;                         /* read when the songs before it are */

    /* Everything before it has been read: start it now, straight after
       the last song, or just ahead of playback if that has gone past the
       end (it was queued late). */
    played = gConsumed;
    at = gWritten;
    if (at < played + kSamplesPerTick)
        at = played + kSamplesPerTick;
    gWritten = at;
    StartReading(at);
}

Boolean Audio_WantsNext(void)
{
    return gActive && gEntryCount < kMaxEntries
        && gReading >= gEntryCount - 1;
}

Boolean Audio_TakeFileError(void)
{
    Boolean failed = gFileFailed;

    gFileFailed = false;
    return failed;
}

/* The entry being heard: the last one whose audio has started. */
static short CurrentEntry(long played)
{
    short i = gEntryCount - 1;

    while (i > 0 && gEntries[i].streamStart > played)
        i--;
    return i;
}

/* Advances gConsumed by the read pointer's movement since the last look,
   allowing for the wrap from the ring's end to its start. False if the
   position can't be trusted: the pointer has left the ring, or so long
   has passed that it may have gone right round (the Sound Manager reads
   at most 370 samples a tick, so less than a lap's worth of ticks means
   less than a lap; the tick count is used only for this bound, never
   for the position). */
static Boolean Look(void)
{
    long offset = ReadOffset(), moved;
    unsigned long now = Ticks();
    Boolean trusted = (long)(now - gLastLook) * kSamplesPerTick
                      < kRingBytes - kGuard;

    if (offset < 0)
        return false;
    moved = offset - gLastOffset;
    if (moved < 0)
        moved += kRingBytes;
    if (trusted)
        gConsumed += moved;
    gLastOffset = offset;
    gLastLook = now;
    return trusted;
}

/* The pointer is no longer where Sound Manager 2.0 keeps it: stop, and
   play without sound from now on (the A8 path) rather than guess. */
static void Fault(void)
{
    Audio_Stop();
    gAvailable = false;
}

/* Fills the ring with silence from the end of the audio up to upTo, so
   that after the stream's last song the Sound Manager reads silence, not
   the previous lap's audio. */
static void Pad(long upTo)
{
    long slot, n;

    if (gPadded < gWritten)
        gPadded = gWritten;
    while (gPadded < upTo)
    {
        slot = Slot(gPadded);
        n = upTo - gPadded;
        if (n > kRingBytes - slot)
            n = kRingBytes - slot;
        memset(gRing + slot, kSilence, n);
        gPadded += n;
    }
}

/* Underrun: the Sound Manager has read past the audio written (the app
   was held up longer than the ring lasts) and has been playing stale
   ring contents. Stop, and restart from the end of the valid audio, with
   the usual pre-fill: heard as a pause. The song being read continues
   from exactly there, so the stream's songs and positions are kept. */
static void Underrun(void)
{
    OSErr err;

    gUnderruns++;
    Quiet();
    err = BeginStream(gWritten);
    if (err == notOpenErr)
        Fault();                        /* the read pointer has gone */
    else if (err != noErr)
        gEndedEarly = true;             /* the file couldn't be read */
}

void Audio_Service(void)
{
    Boolean trusted;

    if (!gActive || gEntryCount == 0)
        return;

    trusted = Look();
    if (ReadOffset() < 0)
    {
        if (++gBadLooks >= 2)
            Fault();                    /* twice in a row: it has gone */
        return;
    }
    gBadLooks = 0;

    /* Songs that have been heard in full are forgotten. */
    while (gEntryCount > 1 && gReading > 0
           && gEntries[1].streamStart <= gConsumed)
    {
        RemoveEntry(0);
        gReading--;
    }

    if (gReading >= gEntryCount)
    {
        /* All read: the stream runs out into silence. Held up too long
           here only means the end has been reached. */
        if (!trusted && gConsumed < gWritten)
            gConsumed = gWritten;
        Pad(gConsumed + kRingBytes - kGuard);
        return;
    }

    if (!trusted || gConsumed > gWritten)
    {
        Underrun();
        return;
    }

    /* Fix 1 (after 5b): top up all the free space, however long since
       the last pass. Small amounts wait until kMinRefill is free. */
    if (gConsumed + kRingBytes - kGuard - gWritten >= kMinRefill)
        Fill(gConsumed + kRingBytes - kGuard);
}

short Audio_CurrentTag(void)
{
    if (!gActive || gEntryCount == 0)
        return -1;
    return gEntries[CurrentEntry(gConsumed)].tag;
}

/* The single source of the play position (6a-4): the samples of the
   current song the Sound Manager has read, from its read pointer. */
long Audio_Position(void)
{
    const Entry *e;
    long position;

    if (!gActive || gEntryCount == 0)
        return 0;
    e = &gEntries[CurrentEntry(gConsumed)];
    position = e->first + (gConsumed - e->streamStart);
    if (position < e->first)
        position = e->first;
    return position < e->length ? position : e->length;
}

Boolean Audio_Ended(void)
{
    return gEndedEarly
        || (gActive && gReading >= gEntryCount && gConsumed >= gWritten);
}

long Audio_Pause(void)
{
    long position;

    if (gActive)
        Look();
    position = Audio_Position();
    Audio_Stop();
    gPausedPosition = position;
    return position;
}

long Audio_PausedPosition(void)
{
    return gPausedPosition;
}

OSErr Audio_Resume(const AudioSource *source, short tag)
{
    return Audio_Play(source, gPausedPosition, tag);
}

long Audio_Underruns(void)
{
    return gUnderruns;
}

void Audio_Shutdown(void)
{
    if (!gAvailable)
        return;
    Audio_Stop();
    SndDisposeChannel(gChan, true);
    gChan = nil;
    DisposePtr(gBlock);
    gBlock = nil;
    gAvailable = false;
}
