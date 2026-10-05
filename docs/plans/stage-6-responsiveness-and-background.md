# Stage 6 plan: responsiveness and playing in the background

Status: proposed, not started. Revised after review (2026-10-05): order
changed so measurement comes first, the background safeguard corrected,
background playback layered across the stage, interrupt-time safety rules
and success criteria added; then sub-stage 6b-2 added (pause instead of
skipping when playback is starved, decision S5). Decisions S1-S5
decided 2026-10-05 (see the end); Stage 6 started.

Covers outstanding issues I2 (main loop runs about once a second while
playing), I1 (audio repeats with another program's window in front) and
I3 (silence after a long menu hold). See `docs/outstanding-issues.md`.

Recommended before Stage 7 (Now Playing volume, scrubber and rating):
those are driven by turning the wheel while a song plays, and should be
built on the final timing design rather than reworked afterwards.

## Goals and success criteria

- **Responsiveness (I2):** while a song plays, a main-loop pass normally
  takes 1-2 ticks, so the wheel and buttons respond as quickly as when
  stopped. Now Playing shows a new (gapless) song within a tick or two.
- **Background playback (I1):** the app keeps playing under MultiFinder
  with another program in front. With TeachText or the Finder in front
  and idle for a minute: no break at all. During a long busy period in
  the front program (a large Finder copy): silence at worst, **never a
  repeated passage**. On return, the time and position are correct.
- **Menu holds and dialogs (I3):** with 6c, playback continues through
  them; without it, silence after about 6.6 s, never repeats. With 6b-2
  (if S5 chooses pause), the song then continues from where the sound ran
  out instead of skipping the stalled time.
- **No new instability:** the Mac must never crash because of the app's
  interrupt-time code, with or without other programs running.

## Background playback: what is needed (three levels)

The Sound Manager already keeps playing the looped ring in the background
(seen in the 5b-1 test); what stops is the code that feeds and protects
it.

1. **Cooperative (6a-2, 6b):** the app's own `SIZE` resource lets it run
   in the background and receive suspend/resume events. MultiFinder gives
   a background program time whenever the front program is idle, which
   most System 6 programs mostly are; the main loop keeps refilling the
   ring, and the 6.6 s ring covers short busy periods. Limit: a front
   program busy for longer than the ring gets the app no time at all.
2. **Interrupt-time safeguard (6b):** so that the worst case is silence,
   not repeats. The 5b-1 test showed the silencing VBL task stops when the
   app is switched out (57 runs in 2,818 ticks), apparently MultiFinder
   disabling VBL tasks whose code is in the switched-out application's
   heap. Options, to be tested (not assumed):
   - the silencing routine placed in **system-heap memory** (a known
     System 6 technique for code that must run whatever program is in
     front), with its data pointers fixed when installed;
   - a **Time Manager** task (`InsTime`/`PrimeTime`, supported by the
     Plus ROM).
   Note (review): "stop the sound when the app gets no time" is not a
   possible fallback, because in that situation none of the app's code
   runs. The safeguard must itself run at interrupt time.
3. **Interrupt-time refill (6c):** asynchronous reads chained from
   interrupt time, paced by the level-2 timer, keep the ring full
   whichever program is in front and however busy it is. The main loop
   opens each song's file in advance (the gapless queue from 5c already
   knows the next song), so the interrupt code only issues reads.
   Required for reliable background playback; see the Plus caveat below.

Practical costs to keep in view:
- **Processor:** playback costs the Plus a share of its processor time
  (about 55% measured in the 5a tests); the front program runs more
  slowly while music plays in the background.
- **Memory:** the app's partition is Retro68's default 1 MB. For use
  alongside other programs, measure what it needs (the ring is 148,000
  bytes) and set a smaller preferred size in the new `SIZE` resource.

## Interrupt-time code under MultiFinder: why it is safe, and the rules

It is safe because MultiFinder is cooperative and does not change how
interrupts work: all programs share one processor and one address space,
and interrupts (VBL, Time Manager, disk completions) run underneath
whichever program is in front; the cursor, disk drivers and the Sound
Manager rely on this. Every running program's partition stays resident,
so memory the code uses does not disappear or move. The app already runs
a VBL task (the R6 silencing) under MultiFinder alongside the Finder.

Rules the app's interrupt-time code must follow (Inside Macintosh):
- no calls that allocate or move memory; touch only non-relocatable
  memory (the ring is `NewPtr`) and the app's fixed-address globals;
- no QuickDraw, Dialog or other Toolbox calls that are not
  interrupt-safe; File Manager only in its asynchronous form (requests are
  queued behind the front program's own disk work);
- no dependence on A5 or on low-memory globals MultiFinder swaps per
  program (`CurrentA5`, `ApplZone`, `TheZone`...). Retro68 addresses the
  app's globals absolutely, not through A5, which is why the silencing
  task already works whatever program is current; only `Ticks` (shared)
  is read;
- the code must stay loaded: confirm the app is a single, locked code
  segment (Retro68's usual layout) before relying on it;
- keep it short;
- always remove every installed task before the app quits.

Caveats:
1. **A bug at interrupt time crashes the whole Mac,** losing unsaved work
   in other programs. Keep interrupt code minimal; test it heavily in
   Mini vMac before the Plus. If the app itself crashes, a task may be
   left installed; another reason to keep the interrupt-time part small
   and simple.
2. **MultiFinder's handling of tasks in an application's heap** (see
   level 2): which safeguard option keeps running in the background must
   be measured.
3. **"Asynchronous" reads are not truly asynchronous on the Plus.** It
   has no DMA: the SCSI driver moves the data with the processor, so a
   read started at interrupt time is carried out while still in the
   interrupt, holding up other interrupts (possibly the Sound Manager's
   feed to the sound hardware). Large reads could cause clicks. 6c must
   read in small, steady pieces: playback needs 370 bytes per tick, e.g.
   one 512-byte read per tick. Measure first (see 6c).

## Order of work (revised)

The `SIZE` change needed for background playback also changes how
MultiFinder schedules the app in the foreground, so it may make I2 better
or worse. Hence: measure, change `SIZE` alone, measure again, then fix.

### 6a-1: baseline measurement (no app behaviour change)
- A throwaway timing build logs, for each main-loop pass while playing:
  time waiting in `WaitNextEvent`, time in the audio refill (and bytes
  read), time redrawing, and the rest.
- Measure on both machines, each run with the iPod in front for about a
  minute while a song plays:
  - Mini vMac and the real Mac Plus;
  - under MultiFinder and under the plain Finder (System 6 Special > Set
    Startup); under the Finder the app cannot be switched out, so this
    separates MultiFinder scheduling from the app's own work;
  - with and without Finder windows open.
- Clue to check: each slow pass took about 60 ticks, and Now Playing
  redraws exactly once a second, so the redraw is a strong suspect.
- Logs: read from the MusicTest image; on the Plus, written to the
  Scratch volume and read from the SD card image afterwards without
  changing it. Needs the card's Scratch image rebuilt first (issue I7),
  planned with the user before touching the card.

### 6a-2: the `SIZE` resource alone (background level 1)
- Own `SIZE` resource: can run in the background, accepts suspend/resume
  events, preferred and minimum partition sizes from measurement.
- Basic background playback starts working here.
- Repeat the 6a-1 measurements to see the effect on I2.

### 6a-3: fix I2
- Fix what the measurements point to, after reporting them:
  - waiting for events (MultiFinder giving other programs long turns):
    sleep 0 while playing, or poll events without yielding;
  - the redraw: draw less, or only what changed;
  - the reads: smaller reads spread over passes (or 6c).

### 6b-1: suspend/resume handling and the safeguard (level 2)
- Suspend/resume: window drawn inactive/active and redrawn correctly; the
  app keeps asking for time every tick while playing in the background.
- The interrupt-time safeguard: test which of the system-heap routine and
  the Time Manager task keeps running when the app is switched out; use
  it for the R6 silencing.
- Check Standard File dialogs, alerts and desk accessories (Edit menu)
  still behave with background running enabled.
- Behaviour when starved is unchanged here (R6: keep time, silence); that
  is changed, if at all, only in 6b-2, so each can be tested on its own.

### 6b-2: pause instead of skipping when starved (decision S5)
A separate, testable step after 6b-1, before 6c (while starvation is
still easy to cause by holding a menu).

- **Behaviour now (R6, "keep time"):** when the main loop is stalled
  longer than the ring lasts (menu held, dialog, busy front program), the
  ring plays silence and the song's time keeps running; on return the
  song continues at the current time, so the stalled seconds are skipped,
  like a radio.
- **Proposed (S5, "pause"):** the song continues from where the sound ran
  out, losing nothing; the stall is heard as a pause.
- **Design (recommended): no change at interrupt time.** The silencing
  stays exactly as in 6b-1. The main loop already knows where the audio
  ran out: the end of the data it last wrote (`gWritten`), which with the
  gapless stream maps to a song (entry/tag) and a sample in it. On return,
  when `Audio_Service` finds playback has passed that point, it restarts
  the stream from there (as resume after pause does) instead of skipping
  ahead. Heard: silence during the stall, then the song carries on from
  the point it stopped. No Sound Manager call at interrupt time is needed.
- **Display:** while starved, Now Playing's elapsed time and progress bar
  stop at the point the audio ran out (elapsed is clamped to the written
  data), so they do not run on during the silence and jump back.
- **Details to handle:** the starvation point falling at or across a song
  boundary (continue in the right song, Now Playing following); the user
  pressing Pause, Next or Previous while starved (those act normally from
  the frozen position); a song file that fails on restart (R8 skip).
- **Alternative design (only if the recommended one shows a problem):**
  the interrupt-time task stops the sound itself (`quietCmd`) when the
  ring is about to run dry, records the position, and the main loop
  restarts from it. Requires first confirming that Sound Manager 2.0 on
  System 6.0.7 allows that call at interrupt time; adds risk to the
  interrupt code, so not preferred.
- **Tests:** a diagnostic that stalls the main loop for 10 s mid-song and
  at a song boundary, checking the restart point (to within a few
  samples), that no audio is skipped, and the frozen elapsed time; manual:
  hold a menu for 10 s, leave a dialog open, put a busy program in front.
- With 6c in place, starvation becomes rare, but S5 still decides what
  happens in the extreme cases.

### 6c: interrupt-time refill (level 3)
- **First, measure on the Plus** (throwaway build): the time a 512-byte
  read takes when issued at interrupt time, and whether reads of that
  size disturb the sound. Choose the read size and pacing from this.
- Then: asynchronous reads chained from the level-2 timer into the ring,
  in small pieces; the main loop opens files ahead (gapless queue) and
  keeps them open; song changes switch to the next pre-opened file.
- The main loop's refill stays as the fallback when interrupt reads are
  not running (e.g. no sound).

## Testing

- Diagnostics in Mini vMac (LaunchAPPL): the timing log; background
  playback with no repeats and the correct time on return; the safeguard
  running while switched out; pause-on-starvation restarting at the right
  sample (6b-2); interrupt-time refill keeping the ring full with the
  main loop deliberately stalled.
- Manual tests in Mini vMac (user), then on the Mac Plus:
  - wheel and clicks responsive while playing;
  - TeachText or the Finder in front for a minute: playback continues;
  - a large Finder copy while playing in the background: silence at
    worst, never a repeat;
  - a menu held and a dialog left open for 30 s (with 6b-2 and pause:
    the song continues from where it stopped; with 6c: no break);
  - window active/inactive appearance, dialogs and desk accessories;
  - quitting while playing, in the foreground and from the background.

## Decisions (decided 2026-10-05)

- **S1:** Stage 6 is done before Stage 7.
- **S2:** in the background the app keeps playing. If it is starved, the
  silence behaves as a pause: feeding restarts from the last point of
  audio played (as S5).
- **S3:** no real hardware for now: develop and test in Mini vMac only.
  Deferred until the BlueSCSI card is available: the Plus runs of 6a-1,
  the Plus read-time measurement at the start of 6c, rebuilding the
  Scratch image (I7), and the final tests on the Plus. Mini vMac's disk
  is effectively instant, so it cannot show the Plus caveat on
  interrupt-time reads: 6c uses small reads by design and must be
  re-checked on the Plus before it is relied on there.
- **S4:** interrupt-time refill (6c) is part of Stage 6.
- **S5 (inferred from S2 and the discussion):** pause: continue from
  where the sound ran out, using the 6b-2 recommended design (main-loop
  restart, no interrupt-time Sound Manager call).
