# Outstanding issues

Known problems and deferred work, newest stage last. Update this file when
an issue is fixed (move it to "Resolved" with the commit) or found.

Status at 1.0 (2026-10-08): Stages 1-10, Fixes 1-2 and the 1.0 work
complete; host tests, smoke test and manual checklist in place. Problems
found while using MacMusic go in `docs/issues-found.md` first, and are
moved here when they are looked into.

Status at 1.1 (2026-10-13): Parts 0, A-D of `docs/plans/release-1.1.md`
done; released without the full manual checklist (the owner's choice;
`docs/testing/runs/1.1.md`).

## Known issues in 1.1

- **Printing hasn't been tried on paper** (Part C): checked in Mini vMac
  only, which has no printer. The page's fonts, columns, header and
  footer are PROVISIONAL (N11-12, N11-13).
- **On the owner's 4 MB Plus** (2026-11-11): the first start and
  Rescan Library, which read every song file, are fast enough; their
  times weren't recorded. Later starts take about 13 s for 316 songs.
- **A 1 MB Plus can't be tested:** the owner's Plus is the only one, and
  taking its SIMMs out isn't wanted. 1.1's Part 0 measured a 1 MB
  configuration in Mini vMac instead ("Accepted limitations" below).

## Known issues in 1.0 (and still in 1.1)

What a user may notice, in short (details below; also in the user
guide):
- Songs play about 0.9% fast (I4).
- The quietest volume level isn't silent (accepted in Stage 9d: the
  Plus's lowest hardware level, 0, still sounds).
- A large library is slow to scan: about 1.5-2 minutes for 500 songs at
  Plus speed (H1; "Fast scanning" below). **Fixed in 1.1:** a faster
  sort, faster grouping, the library's cache and an optimised build; on
  the real Plus a 316-song library starts in about 13 s (N11-31; the
  first start, or Rescan Library, longer).
- A program that keeps the Mac completely busy for over 6 s can make the
  sound repeat a moment and pause (accepted limitation below).
- Sound relies on undocumented System 6 Sound Manager state, checked at
  start-up; untested on Macs other than the Plus (I9).
- Opening a library from the Finder (double-clicking its marker) isn't
  supported (I6).
- Extras holds only the Clock; ratings are recorded but not used.

## Open

### I4. Songs play about 0.9% fast (R3)
- The Plus hardware plays 22,254.5 samples/s; songs are 22,050. Times and
  the progress bar are kept in song time, so they stay correct.
- **Fix (if wanted):** repeat one sample in every 108 while filling the
  ring. Accepted as-is for now.
- **1.2 (`docs/plans/release-1.2.md`, step D):** MacMusic will accept
  songs made at 22,254 Hz, the Plus's own rate, as the MacMusic
  Converter 1.1 makes them; they play at true speed. I4 is then closed:
  accepted for 22,050 Hz songs, fixed for 22,254 Hz ones.

### I6. Open a library from the Finder (L4, deferred from 5b-3)
- Double-clicking "MacMusic Library Info" could launch the app with that
  library selected (System 6 passes the document at launch).

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
- **Real Mac Plus, System 6.0.7: confirmed working** (2026-10-07,
  `0692d4f`): songs imported and played correctly, so the startup check
  passed. Kept open because the state is undocumented.
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

- **Memory (H1, measured 2026-10-08):** the `SIZE` resource asks for
  640 KB and accepts 512 KB (was 512 and 384). It only matters under
  MultiFinder; under the Finder alone MacMusic gets all free memory.
  About 262 KB is in use before a library is read (code, the 148 KB
  audio ring, the LCD and window). The peak below is: playing, the 32 KB
  import buffer, the Edit Song window and Legal loaded, a rescan done,
  plus a further 32 KB block.

  | Partition | Free, no library | 500 songs, typical names | 500 songs, all tags 63 characters |
  |---|---|---|---|
  | 384 KB | 46 KB | out of memory | out of memory |
  | 512 KB | 174 KB | fits, 54 KB spare at the peak | out of memory |
  | 640 KB | 302 KB | fits | fits, 53 KB spare at the peak |

  Running out shows "There is not enough memory to read the MacMusic
  Library" and doesn't crash. Typical names are the 307-song library's
  averages (title 16, artist 13, album 15, genre 6 characters).
- **Scanning is slow at Plus speed:** 500 small song files took 94 s
  to scan in Mini vMac at 1x (137 s with every tag 63 characters).
  See "Fast scanning" in the roadmap. 1.1's Part 0 found most of it was
  the title sort and the group search, not the disk; fixed in 1.1.
- **A 1 MB Plus (measured for 1.1, Part 0; British System 6.0.7 by Easy
  Install):** 858K free under the Finder alone, 593K under MultiFinder.
  MacMusic, asking for 640K, is offered 598K there and given about 556K:
  500 songs with typical names load with about 215K to spare; 500 with
  every detail 63 characters don't ("There is not enough memory to read
  the MacMusic Library"), so the largest libraries need the Finder alone.
  Printing takes 2-3K more (ImageWriter, LaserWriter), given back after.

## Testing notes
- Test timing-sensitive changes in Mini vMac at **1x** (Control-S): at
  faster speeds problems such as the slow redraw (I2) and the Sound
  Manager falling behind under disk load did not show.
- Diagnostic builds that log to a folder the Finder has open make the
  Finder refresh that window, slowing the app (this produced the original
  I2 reading). Log to a closed folder, and size log buffers for the
  longest line (an overflow there caused crashes in Stage 6a-2 testing).
- **LaunchAPPL timing (measured 2026-10-08, for the 1.0 plan):** about
  4 s to start and stop Mini vMac, then emulated time runs at about 1x,
  never faster. macOS runs Mini vMac at low background priority (its
  window isn't in front); it needs about 18% CPU to keep up, but fell to
  0.68x in one 10-minute run while this Mac was busy. Playing audio
  didn't slow it, and gave no underruns.
  - The slow 9d run (over 10 minutes) was its songs playing through at
    1x, not a fault. The 9e runs' empty or partial logs were the Clicker's
    `bufferCmd` hang (sent with `SndDoImmediate`), gone with the Clicker.
  - **For tests:** use short songs and seek rather than play through;
    keep a run to 1-2 minutes; pass LaunchAPPL `--timeout` so a hang fails;
    end the log with a "DONE" line so a cut-off log is caught.

## After 1.0 (roadmap, not defects)
- **Future releases** (the owner, 2026-10-10): the whole MacMusic system
  as "B-1.0 Personal" and "B-1.1 Personal" (with a v1.1 Update Card),
  and public systems from 1.1 in American, British and other languages,
  for the Plus and possibly earlier Macs. For later review:
  `docs/plans/future-releases.md`.
- **Fast scanning:** done in 1.1 (`release-1.1.md`, N11-2 to N11-10 and
  N11-23).
- Real Mac Plus results for the 1980 MB Scratch image (2026-10-07: 307
  real songs, the `6436450` app, from before Fixes 1-2): to record here,
  whether the disk mounts, how long the first scan takes and whether
  playback stays smooth. For 1.0 these checks are section 9 of the
  manual checklist, on a new Scratch image with the 1.0 app (V7, V8).
- The software licence that the legal statement
  (`docs/legal/legal-statement.md`) refers to: to be written later, in
  the product documentation.
- Move a song between libraries with its details: today that means export
  then import, and export strips the iPod details and rating. A "Move to
  Library…" command that keeps them (noted in Stage 8a, not planned).
- **Removing a library from the File menu's list** (the owner,
  2026-11-11; a 1.x feature): today the list (8d-3, `kMaxRecent` 5)
  can't be edited. A library whose folder is deleted stays listed,
  dimmed, and only throwing away MacMusic Prefs clears the list, which
  also resets the Settings. Proposed: choosing a library that can't be
  found (`LibSetup_UseRecent`) asks "Remove it from the list?" (Remove /
  Keep). To remove a test library, delete its folder in the Finder,
  then choose it once and click Remove. A library on a floppy that
  isn't inserted stays unless the user says so. Considered instead:
  File > Clear Library List (keeps the current library). Either keeps
  the prefs format, so 1.0 and 1.1 still read it. Not planned yet.
- Ratings (7d) are recorded but not used for anything else yet. Ideas
  for later, not planned now: use them in Shuffle Songs, or to create
  playlists (e.g. songs rated 4 stars or more).
- The Clicker (Stage 9e): dropped, not planned. If it is ever tried again,
  first explain the Stage 9e test runs that stopped without a log.
- On-The-Go playlist (D17): deferred until after Stage 8, possibly a
  later stage. The 4G guide's description and a draft plan are in
  `docs/plans/deferred-on-the-go.md`.

## Resolved
- 1.1: slow start-up (H1, the issues log of 2026-10-09): a merge sort,
  groups noted as they're built, and the library's cache (`d69799c`,
  `66a3241`); the cut-off Create New Library prompt (`2ed64cf`).
- Ring ran dry after 20–25 s (stutter): `6e6fcde`.
- Gap between songs (I5, R5): Stage 5c, gapless stream; sound also
  starts after about 4 s of buffering instead of the whole ring.
- Slow import / duplicate copies on disk: `eaaff53` (Stage 5b-2).
- No way to use an existing library: `b1bf831` (Stage 5b-3).
- `run-persistent.sh` could not launch Mini vMac (-10825): `8eccbce`.
- App renamed from MacMusicPlayer to MacMusic (`docs/plans/rename-macmusic.md`):
  the build, window, menus, About box and prefs file ("MacMusicPlayer
  Prefs" is renamed to "MacMusic Prefs" on first launch). The project
  folder keeps its old name for now (R2).
- Stage 8, library management and settings: metadata editing (8c),
  library safeguards, Rescan and Library Info, Recent Libraries and
  Remove from Playlist (8d), plus the 8a and 8b design documents
  (`docs/plans/stage-8-library-and-settings.md`).
- Stage 10, fast-forward and rewind (N8) and the Shuffle and Repeat
  icons on Now Playing (`docs/plans/stage-10-fast-forward.md`).
- Stage 9, the Settings menu (`docs/plans/stage-9-settings.md`): About,
  Shuffle, Repeat, Legal, Reset All Settings and the main menu's
  Backlight; the Clicker (9e) dropped.
- Hardware test image out of date (I7): the Scratch image was rebuilt
  with the `0692d4f` app and the old-format library removed (2026-10-07);
  the app runs correctly on the real Mac Plus.
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
