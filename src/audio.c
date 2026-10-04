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
   loop reuses it for the sample kRingBytes later. */

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
#define kSilence            0x80

static Boolean gAvailable;
static SndChannelPtr gChan;
static Ptr gBlock;                      /* SoundHeader + ring */
static SoundHeader *gHeader;
static unsigned char *gRing;

static short gRefNum;                   /* open song file, 0 if none */
static long gDataOffset, gLength;

/* Shared with the VBL task: plain longs, written by the main loop only
   while gActive is false (or atomically). */
static volatile Boolean gActive;
static volatile long gOrigin;           /* absolute sample at ring[0] */
static volatile long gStartSample;      /* sample playing at gStartTick */
static volatile unsigned long gStartTick;
static volatile long gSilenced;         /* samples silenced so far (absolute) */

static long gWritten;                   /* samples written into the ring */
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

/* Writes samples [gWritten, upTo) into the ring: file data, then silence
   after the end. Each read takes all it can up to the end of the ring,
   so a fill is at most two reads (before and after the wrap). */
static OSErr Fill(long upTo)
{
    long slot, n, count;
    OSErr err = noErr;

    while (gWritten < upTo)
    {
        slot = Slot(gWritten);
        n = upTo - gWritten;
        if (n > kRingBytes - slot)
            n = kRingBytes - slot;

        if (gWritten < gLength)
        {
            if (n > gLength - gWritten)
                n = gLength - gWritten;
            count = n;
            err = SetFPos(gRefNum, fsFromStart, gDataOffset + gWritten);
            if (err == noErr)
                err = FSRead(gRefNum, &count, gRing + slot);
            if (err == eofErr && count > 0)
                err = noErr;
            if (err != noErr || count <= 0)
                return err != noErr ? err : eofErr;
            n = count;
        }
        else
            memset(gRing + slot, kSilence, n);

        gWritten += n;
    }
    return err;
}

void Audio_Stop(void)
{
    Quiet();
    if (gRefNum != 0)
    {
        FSClose(gRefNum);
        gRefNum = 0;
    }
}

OSErr Audio_Play(const AudioSource *source, long startSample)
{
    SndCommand c;
    OSErr err;

    if (!gAvailable)
        return notOpenErr;

    Audio_Stop();
    err = OpenDataFork(source->vRefNum, source->dirID, source->name,
                       fsRdPerm, &gRefNum);
    if (err != noErr)
    {
        gRefNum = 0;
        return err;
    }
    gDataOffset = source->offset;
    gLength = source->length;
    if (startSample < 0 || startSample > gLength)
        startSample = 0;

    /* A full ring before starting. */
    gOrigin = startSample;
    gWritten = startSample;
    err = Fill(startSample + kRingBytes);
    if (err != noErr)
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

    gStartSample = startSample;
    gSilenced = startSample;
    gStartTick = Ticks();
    gActive = true;
    return noErr;
}

void Audio_Service(void)
{
    long played;

    if (!gActive)
        return;

    played = Played();
    if (played >= gLength)
        return;

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
    if (played + kRingBytes - kGuard - gWritten >= kMinRefill
        || gWritten >= gLength)
        Fill(played + kRingBytes - kGuard);
}

long Audio_Position(void)
{
    long played;

    if (!gActive)
        return 0;
    played = Played();
    return played < gLength ? played : gLength;
}

Boolean Audio_Ended(void)
{
    return gActive && Played() >= gLength;
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
