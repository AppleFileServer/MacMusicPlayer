# Outstanding issues

Known problems and deferred work, newest stage last. Update this file when
an issue is fixed (move it to "Resolved" with the commit) or found.

Status as of Stage 6a-4 (`99601ce`, playback position from the Sound
Manager).

## Open

### I1. Switching in and out of the background (rest of 5b-1 / Stage 6b-1)
- **Now:** since `82c7079` (6a-2, `SIZE` resource) the app runs in the
  background under MultiFinder, so playback continues with another
  program in front; since `99601ce` (6a-4) heavy Finder disk work
  (Duplicate, Empty Wastebasket) no longer breaks it (tested in Mini vMac
  at 1x).
- **Still to do (6b-1):** suspend/resume events are accepted but not
  handled; the window's active/inactive appearance and redrawing on
  switching are as before. Check Standard File, alerts and desk
  accessories with background running enabled.
- **Accepted limitation:** if the front program gives the app no time at
  all for longer than the ring (about 6.6 s), the Sound Manager may
  replay stale ring audio until the app runs again; it then detects the
  underrun and restarts from the end of the valid audio (heard as a
  pause).

### I3. Long menu hold or open dialog: stale audio, then recovery
- **Now (since 6a-4):** the ring is refilled only from the main loop, and
  the VBL silencing task (R6) has been removed. A menu held or a dialog
  open for longer than the ring (about 6.6 s) can replay stale ring audio
  until it closes; then the underrun is detected and playback restarts
  from where the valid audio ended. Before 6a-4 the same situation was
  heard as silence (R6). Accepted with the 6a-4 design.
- **Fix (if wanted):** interrupt-time refill (Stage 6c). Not part of
  6a-4; decide separately.

### I4. Songs play about 0.9% fast (R3)
- The Plus hardware plays 22,254.5 samples/s; songs are 22,050. Times and
  the progress bar are kept in song time, so they stay correct.
- **Fix (if wanted):** repeat one sample in every 108 while filling the
  ring. Accepted as-is for now.

### I6. Open a library from the Finder (L4, deferred from 5b-3)
- Double-clicking "iPod Library Info" could launch the app with that
  library selected (System 6 passes the document at launch).

### I7. Hardware test image is out of date
- `HD50_512 Scratch 30 MB.hda` on the BlueSCSI card has the app as of
  `6e6fcde` and a version-1 library; versions from Stage 5b-2 on
  (version-2 songs) won't read it. Rebuild the image (plan first) before
  the next Mac Plus test. Needed to confirm I9 on real hardware.

### I8. Unexplained 600-tick stall, seen once (5b diagnostic)
- One AudioTest run lost about 10 s at a song change; three later runs,
  with timestamps, did not. Not reproduced since. Watch for a long silence
  at a song change.

### I9. The play position relies on undocumented Sound Manager 2.0 state
- 6a-4 reads the sampled-sound player's read pointer: the channel's first
  modifier stub, its `userInfo` block, byte offset 148. Validated at
  startup (inside the ring, moving forward at the playback rate) and at
  every start; if it doesn't behave, sound is reported unavailable and
  the app plays silently (A8) rather than guessing.
- **Not yet confirmed on the real Mac Plus** (same System 6.0.7 software
  as in Mini vMac; see I7).
- **Other systems:** on System 7's Sound Manager 3.0 the layout is likely
  different; the startup check would then fail and the app would play
  without sound. Untested.
- Sound Manager quirk found while implementing: after `quietCmd`, every
  other start of the looped sound does nothing; starts are therefore
  verified by the pointer moving, and retried.

### I10. Small stack margin
- Under MultiFinder the app gets an 8 KB stack (`ApplLimit` to
  `CurStackBase`). Measured deepest use: about 6.4 KB during startup
  with a newly created library, about 2.5 KB in normal use; playback
  does not go deeper. Not a known cause of any failure, but a thin margin
  since interrupt routines also use the app's stack.
- **Fix (if wanted):** reserve a larger stack at startup
  (`SetApplLimit` below the default before the heap grows).

## Testing notes
- Test timing-sensitive changes in Mini vMac at **1x** (Control-S): at
  faster speeds problems such as the slow redraw (I2) and the Sound
  Manager falling behind under disk load did not show.
- Diagnostic builds that log to a folder the Finder has open make the
  Finder refresh that window, slowing the app (this produced the original
  I2 reading). Log to a closed folder, and size log buffers for the
  longest line (an overflow there caused crashes in Stage 6a-2 testing).

## Roadmap items (not defects)
- Stage 6: 6b-1 (suspend/resume handling), 6b-2 and 6c are in
  `docs/plans/stage-6-responsiveness-and-background.md`; their scope needs
  revisiting after 6a-4 (the VBL task and tick model they assumed are
  gone).
- Stage 7: Now Playing volume, scrubber and rating screens.
- Stage 8: On-The-Go playlist (D17).
- Settings menu.

## Resolved
- Ring ran dry after 20–25 s (stutter): `6e6fcde`.
- Gap between songs (I5, R5): Stage 5c, gapless stream; sound also
  starts after about 4 s of buffering instead of the whole ring.
- Slow import / duplicate copies on disk: `eaaff53` (Stage 5b-2).
- No way to use an existing library: `b1bf831` (Stage 5b-3).
- `run-persistent.sh` could not launch Mini vMac (-10825): `8eccbce`.
- App starved while another program's window is in front (main part of
  I1): `82c7079` (6a-2): the app now runs in the background.
- Main loop about once a second while playing (I2): the original reading
  was a logging artefact, but at Mac Plus speed (Mini vMac 1x) the Now
  Playing redraw really did take about 56 ticks a second; fixed in
  `f34544c` (6a-3, redraw about 11x faster, pixel-identical).
- Stutter or silence that never recovered after heavy Finder disk work:
  the tick-based play position drifted from the Sound Manager's real
  position, and the R6 silencing then blanked audio before it was played.
  `99601ce` (6a-4): position from the Sound Manager's read pointer; tick
  model and VBL silencing removed.
