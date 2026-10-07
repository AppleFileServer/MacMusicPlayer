# Outstanding issues

Known problems and deferred work, newest stage last. Update this file when
an issue is fixed (move it to "Resolved" with the commit) or found.

Status as of the end of Stage 6 (`206cffe`): complete in Mini vMac;
the real Mac Plus checks are deferred until the BlueSCSI card is available
(I7, I9).

## Open

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

## Accepted limitations
- **Starvation by other programs:** if another program gives the app no
  time at all for longer than the ring (about 6.6 s, e.g. a very busy
  front program), the Sound Manager may replay stale ring audio until the
  app runs again; the underrun is then detected and playback restarts
  from the end of the valid audio (heard as a pause). The app's own menus,
  dialogs, window drags and wheel holds no longer cause this (6b-2).
  Interrupt-time refill (former 6c) would remove it; dropped from Stage 6
  (S4), possible later.

## Testing notes
- Test timing-sensitive changes in Mini vMac at **1x** (Control-S): at
  faster speeds problems such as the slow redraw (I2) and the Sound
  Manager falling behind under disk load did not show.
- Diagnostic builds that log to a folder the Finder has open make the
  Finder refresh that window, slowing the app (this produced the original
  I2 reading). Log to a closed folder, and size log buffers for the
  longest line (an overflow there caused crashes in Stage 6a-2 testing).

## Roadmap items (not defects)
- Stage 6 hardware checks (when the card is available): rebuild the
  Scratch image (I7), then on the real Plus the read-pointer check (I9),
  the Finder stress test, the modal-loop tests and responsiveness.
- Press-and-hold fast-forward and rewind (N8, deferred from Stage 7).
- Ratings (7d) are recorded but not used for anything else yet. Ideas
  for later, not planned now: use them in Shuffle Songs, or to create
  playlists (e.g. songs rated 4 stars or more).
- Stage 8: library management and settings (song metadata editing,
  library administration, a multiple-libraries investigation, and a
  Settings menu audit): `docs/plans/stage-8-library-and-settings.md`.
- On-The-Go playlist (D17): deferred until after Stage 8, possibly a
  later stage. The 4G guide's description and a draft plan are in
  `docs/plans/deferred-on-the-go.md`.

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
- Switching in and out of the background (I1): verified in 6b-1 (title
  bar, redrawing, desk accessories, dialogs, quitting while playing); no
  code changes needed.
- Playback starved inside the app's own modal loops (I3, menus, dialogs,
  alerts, Standard File, window drags, wheel holds): `fa6ac35` (6b-2).
- Small stack margin (I10): 24 KB stack, about 18 KB spare: `206cffe`
  (6b-3).
- Stage 7, Now Playing volume, scrubber and rating: `cede6c4`,
  `3016fe2`, `de22d4d` and the 7d commit.
- Responsiveness at Plus speed (6b-4, Mini vMac 1x): a main-loop pass
  takes 1-2 ticks while playing; the Now Playing redraw about 6 ticks a
  second; no underruns.
