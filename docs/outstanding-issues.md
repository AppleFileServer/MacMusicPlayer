# Outstanding issues

Known problems and deferred work, newest stage last. Update this file when
an issue is fixed (move it to "Resolved" with the commit) or found.

Status as of commit `b1bf831` (Stage 5b-3).

## Open

### I1. Audio repeats while another program's window is in front (5b-1)
- **Seen:** with a Finder window in front of the iPod (MultiFinder), the
  last few seconds of audio loop until the iPod window is clicked again.
- **Cause (measured):** the app's `SIZE` resource (Retro68's default) says
  it cannot run in the background, so it gets no time when switched out,
  and MultiFinder also suspends its VBL task (57 runs in 2,818 ticks). The
  Sound Manager keeps looping the ring with nothing refilling or silencing
  it. On return, playback catches up to the right position.
- **Planned fix (5b-1 plan):** own `SIZE` resource allowing background
  running with suspend/resume events; keep the silencing working in the
  background (VBL task record outside the app heap, or stop the sound when
  starved). Decision outstanding: keep playing in the background
  (recommended) or pause.

### I2. Main loop runs only about once a second during playback
- **Seen:** event-loop log: about one pass per 60 ticks while a song plays,
  although `WaitNextEvent` asks for 1 tick (every 31 ticks when idle, as
  asked). The iPod may respond slowly to clicks and the wheel.
- **Cause:** unknown. Suspects: `WaitNextEvent` giving the Finder long
  turns under MultiFinder; the ring reads; the once-a-second redraw.
- **Impact now:** no longer starves the audio since `6e6fcde` (whole-ring
  refill); remains a responsiveness issue.
- **Next step:** a timing build that measures each part of a pass.

### I3. Silence after about 6 s of a held menu or open dialog
- **Seen/expected:** the ring (148,000 bytes, about 6.6 s) is refilled only
  from the main loop; a menu held or dialog open longer than that is heard
  as silence (by design, R6: silence rather than repeated audio).
- **Fix (if wanted):** refill at interrupt time with asynchronous reads
  (the former "5c" idea), which would also help I1.

### I4. Songs play about 0.9% fast (R3)
- The Plus hardware plays 22,254.5 samples/s; songs are 22,050. Times and
  the progress bar are kept in song time, so they stay correct.
- **Fix (if wanted):** repeat one sample in every 108 while filling the
  ring. Accepted as-is for now.

### I5. Gap between songs (R5) — next stage
- Each natural song change stops the sound, refills the whole ring
  (up to 148,000 bytes) and restarts it.
- See the Stage 5c plan.

### I6. Open a library from the Finder (L4, deferred from 5b-3)
- Double-clicking "iPod Library Info" could launch the app with that
  library selected (System 6 passes the document at launch).

### I7. Hardware test image is out of date
- `HD50_512 Scratch 30 MB.hda` on the BlueSCSI card has the app as of
  `6e6fcde` and a version-1 library; this version (version-2 songs) won't
  read it. Rebuild the image (plan first) before the next Mac Plus test.

### I8. Unexplained 600-tick stall, seen once (5b diagnostic)
- One AudioTest run lost about 10 s at a song change; three later runs,
  with timestamps, did not. Not reproduced since. Watch for a long silence
  at a song change.

## Roadmap items (not defects)
- Stage 7: Now Playing volume, scrubber and rating screens.
- Stage 8: On-The-Go playlist (D17).
- Settings menu.

## Resolved
- Ring ran dry after 20–25 s (stutter): `6e6fcde`.
- Slow import / duplicate copies on disk: `eaaff53` (Stage 5b-2).
- No way to use an existing library: `b1bf831` (Stage 5b-3).
- `run-persistent.sh` could not launch Mini vMac (-10825): `8eccbce`.
