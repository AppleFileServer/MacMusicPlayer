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
   ring, started once per song, so there are no buffer boundaries (the
   request-based paths left a gap at every buffer on the Plus). The Plus
   plays exactly 370 samples per VBL whatever rate is requested, so the
   play position is (ticks since start) * 370. The main loop reads the
   file into the ring ahead of that position.

   Ring slot for absolute sample X = (X - gOrigin) mod kRingBytes.
     - main loop writes samples up to played + kRingBytes - kGuard
     - a VBL task silences samples up to played - kLag after they have
       played (R6), so if the main loop is held up (a menu held down, a
       dialog) the loop plays silence rather than repeating old audio.
   kLag < kGuard keeps the two apart: a slot is silenced before the main
   loop reuses it for the sample kRingBytes later.

   Stage 5c (gapless, DECISION G1): the ring carries a stream, not one
   song. Sample positions count from the start of the stream; a short
   list of songs (gEntries) says where each starts in it. When the main
   loop finishes reading one song it opens the next and writes its first
   sample straight after the last one, so the sound never stops between
   songs. Nothing is written after the stream's end: those slots have
   already been silenced, a lap earlier, by the VBL task. */

/* Constants missing from Retro68's headers (Inside Macintosh). */
enum { kQuietCmd = 3, kFreqCmd = 42, kSoundCmd = 80 };
#define kInitMono           0x0080
#define kRate22254          0x56EE8BA3L     /* hardware rate, Fixed */
#define kMiddleC            60
#define kVBLType            1               /* vType */
#define kSoundDispatchTrap  0xA800
#define kUnimplementedTrap  0xA89F

#define kSamplesPerTick     370L
#define kRingBytes          (400L * kSamplesPerTick)   /* 148000, ~6.65 s (R2) */
#define kGuard              (8L * kSamplesPerTick)     /* write margin */
#define kLag                (3L * kSamplesPerTick)     /* silence this far behind */
#define kMinRefill          8192L                       /* R4: no tiny reads */
#define kStartFill          (240L * kSamplesPerTick)   /* G2: about 4 s */
#define kMaxEntries         8           /* songs in the ring at once */
#define kNotStarted         0x7FFFFFFFL /* streamStart before reading */
#define kSilence            0x80

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

/* Shared with the VBL task: plain longs, written by the main loop only
   while gActive is false (or atomically). */
static volatile Boolean gActive;
static volatile long gOrigin;           /* absolute sample at ring[0] */
static volatile long gStartSample;      /* sample playing at gStartTick */
static volatile unsigned long gStartTick;
static volatile long gSilenced;         /* samples silenced so far (absolute) */

static long gWritten;                   /* stream samples written */
static long gUnderruns;

static VBLTask gTask;

static unsigned long Ticks(void)
{
    return *(volatile unsigned long *)0x016A;   /* low-memory Ticks */
}

/* Absolute samples played. Safe at interrupt time. */
static long Played(void)
{
    return gStartSample + (long)(Ticks() - gStartTick) * kSamplesPerTick;
}

static long Slot(long sample)
{
    return (sample - gOrigin) % kRingBytes;
}

/* VBL task (R6): runs every tick at interrupt time. Only memory writes:
   no Toolbox, no File Manager, no allocation. Re-arms itself. */
static void SilenceTask(void)
{
    long upTo, slot, n;

    gTask.vblCount = 1;
    if (!gActive)
        return;

    upTo = Played() - kLag;
    if (upTo - gSilenced > kRingBytes)          /* far behind: one lap is enough */
        gSilenced = upTo - kRingBytes;
    while (gSilenced < upTo)
    {
        slot = Slot(gSilenced);
        n = upTo - gSilenced;
        if (n > kRingBytes - slot)
            n = kRingBytes - slot;
        memset(gRing + slot, kSilence, n);
        gSilenced += n;
    }
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

    gTask.qType = kVBLType;
    gTask.vblAddr = (ProcPtr)SilenceTask;
    gTask.vblCount = 1;
    gTask.vblPhase = 0;
    if (VInstall(&gTask) != noErr)
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

OSErr Audio_Play(const AudioSource *source, long startSample, short tag)
{
    SndCommand c;
    OSErr err;

    if (!gAvailable)
        return notOpenErr;

    Audio_Stop();
    if (startSample < 0 || startSample > source->length)
        startSample = 0;

    /* A new stream: the ring may hold the last one's audio. */
    memset(gRing, kSilence, kRingBytes);
    gOrigin = 0;
    gWritten = 0;
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

    /* G2: about 4 s before starting, not the whole ring, so playback
       starts sooner; the main loop fills the rest. */
    err = Fill(kStartFill);
    if (err != noErr && gWritten == 0)
    {
        Audio_Stop();
        return err;
    }

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
    if (err != noErr)
    {
        Audio_Stop();
        return err;
    }

    gStartSample = 0;
    gSilenced = 0;
    gStartTick = Ticks();
    gActive = true;
    gFileFailed = false;                /* only the first song, reported
                                           through the return value */
    return noErr;
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
    played = Played();
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

void Audio_Service(void)
{
    long played;

    if (!gActive || gEntryCount == 0)
        return;

    played = Played();

    /* Songs that have been heard in full are forgotten. */
    while (gEntryCount > 1 && gReading > 0
           && gEntries[1].streamStart <= played)
    {
        RemoveEntry(0);
        gReading--;
    }

    if (gReading >= gEntryCount)
        return;                         /* all read: the stream runs out */

    /* Fell behind: what was missed has been heard as silence (R6), so
       carry on from where playback is now, keeping time. */
    if (played > gWritten)
    {
        gUnderruns++;
        gWritten = played + kSamplesPerTick;
    }

    /* Fix 1 (after 5b): top up all the free space, however long since
       the last pass. The main loop may come round only about once a
       second while playing, and a capped refill (two 8 KB reads, 16 KB)
       fell behind the 22 KB a second played, so the ring ran dry after
       20-25 s. Small amounts wait until kMinRefill is free. */
    if (played + kRingBytes - kGuard - gWritten >= kMinRefill)
        Fill(played + kRingBytes - kGuard);
}

short Audio_CurrentTag(void)
{
    if (!gActive || gEntryCount == 0)
        return -1;
    return gEntries[CurrentEntry(Played())].tag;
}

long Audio_Position(void)
{
    const Entry *e;
    long played, position;

    if (!gActive || gEntryCount == 0)
        return 0;
    played = Played();
    e = &gEntries[CurrentEntry(played)];
    position = e->first + (played - e->streamStart);
    if (position < e->first)
        position = e->first;
    return position < e->length ? position : e->length;
}

Boolean Audio_Ended(void)
{
    return gActive && gReading >= gEntryCount && Played() >= gWritten;
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
    VRemove(&gTask);
    SndDisposeChannel(gChan, true);
    gChan = nil;
    DisposePtr(gBlock);
    gBlock = nil;
    gAvailable = false;
}
