# Changelog

MacMusic's releases, newest first. Version numbers follow
`docs/plans/release-1.0.md` (Part E): 1.0.x for fixes, 1.x for new
features that keep reading 1.0's libraries, 2.0 for a change needing a
library conversion. The decision codes in brackets are explained in
`docs/decisions.md`.

## 1.1 (in progress; docs/plans/release-1.1.md)

- **The Create New Library prompt (N11-1):** the Save dialog's prompt is
  now "New library in:", which fits; 1.0's "Create new MacMusic Library
  in:" was cut to "Create new MacMusic". From the issues log, 2026-10-08.
- **Faster reading of the library (N11-23):** the songs are sorted by
  title with a merge sort, and each song's artist, album, genre and
  composer found by a binary search, instead of 1.0's insertion sort and
  search from the start. The same lists in the same order (a new host
  test, `test_sorting`, checks them against 1.0's way); a full read of
  500 songs at Plus speed takes 12 s instead of 49 s. Each song's group
  is noted as the lists are built, with no string comparisons beyond
  1.0's.
- **The library's cache (N11-2 to N11-10):** an invisible folder
  "MacMusic Cache" in the library holds "Song Details": each song's file
  name, date and length, and what the scan reads from it. A file that
  matches is taken from the cache, anything else read from its file, so
  the songs, their order and the alerts are 1.0's; a damaged cache is
  ignored, and if memory is short the scan is done without it. Before
  Edit Song or a rating changes a song file, its entry is marked stale,
  so a Plus whose clock has stopped can't be misled. Rescan Library
  reads every file. Starting with 500 songs at Plus speed: 7.5 s. A new
  host test, `test_cache`.

## After 1.0

No changes to MacMusic itself yet.

- **Developer tools (Part F, 2026-10-08):** `RESTART.md`;
  `docs/environment.md`; `tools/check-environment.sh`;
  `tools/make-environment-archive.sh` and the archive's `restore.sh`; the
  Retro68 LaunchAPPL patch kept in `tools/patches/`; the scripts honour
  `RETRO68`, `MINIVMAC` and `LAUNCHAPPL_CFG`.

## 1.0 (2026-10-08)

The first release, built over Stages 1-10 (2026-10-01 to 10-07) and the
1.0 work (10-08).

### The iPod

- **Stage 1:** the iPod's body and 160x128 screen, drawn offscreen, with
  the main menu (Music, Extras, Settings, Shuffle Songs) and the Music
  menu.
- **Stage 2:** scrolling by dragging round the click wheel; the wheel's
  buttons; the Mac keyboard as iPod buttons (D8).
- **Stage 5c-1:** the wheel's markings (W1-W3).
- **Stage 6a-3:** Now Playing redrawn about 11 times faster.
- **Stage 7:** Now Playing's modes: volume over one wheel turn (8e-1),
  the scrubber and five-star ratings (N1-N7).
- **Stage 9:** Settings: About, Shuffle (Songs, Albums), Repeat (One,
  All), Legal, Reset All Settings; Backlight on the main menu (S-9,
  T1-T7). The planned Clicker was dropped.
- **Stage 10:** fast-forward and rewind by holding Next or Previous
  (F1-F8); the shuffle and repeat icons on Now Playing (G1-G4).
- **1.0:** Extras holds the Clock, showing the Mac's date and time
  (D3, E1-E5).

### The library

- **Stage 3:** the MacMusic Library folder; importing WAV songs (song
  file format version 2: the WAV with an "iPod" chunk); playlists as
  text files (P1-P11).
- **Stage 5b-2:** Import moves the file into the library (or copies it);
  Export gives the original WAV back exactly; duplicate audio is noticed
  (D31).
- **Stage 5b-3:** finding an existing library, and a marker file
  ("MacMusic Library Info") in every library (L1, L3).
- **Stage 8:** Edit Song (L2-L4, L11); safeguards: at most 500 songs,
  song IDs never reused (M3, M4); Rescan Library, Library Info and
  Recent Libraries (M2); Remove from Playlist (L7); at most 10
  playlists, Delete and Rename Playlist (8e-2 to 8e-4).

### Sound

- **Stage 4:** the play queue and Now Playing, on a clock (N1-N9).
- **Stage 5b:** real sound through the Sound Manager: one looping sound
  over a ring buffer filled from the main loop (R1-R9).
- **Stage 5c:** gapless playback (G1-G3).
- **Stage 6:** the position taken from the Sound Manager itself (6a-4);
  playing on in the background under MultiFinder (6a-2); the sound kept
  fed in menus, dialogs and window drags (6b-2); a 24 KB stack (6b-3).

### The Mac side

- **The rename:** MacMusicPlayer became MacMusic (2026-10-07); its prefs
  file is renamed on first launch (R6).
- **Fix 1:** MacMusic's Finder icons (the application and its song
  files).
- **Fix 2:** "MacMusic Library" is the library's default name; an old
  "iPod Library Info" marker is renamed.
- **1.0, Part A0:** memory measured; the partition is 640K preferred,
  512K minimum (it was 512K and 384K, which couldn't hold 500 songs).
- **1.0, Part A:** the version "1.0" set in one place, with `vers`
  resources and a copyright line in the About box.

### For developers

- `tools/library-image`: a library of real songs, and a BlueSCSI disk
  image for the real Plus.
- `tests/run-host.sh`: host tests of the library, song file, playlist,
  prefs, settings and shuffle code (567 checks).
- `tests/run-smoke.sh`: a scripted tour of the iPod in Mini vMac (79
  checks).
- `docs/testing/manual-checklist.md`: the release checklist.
