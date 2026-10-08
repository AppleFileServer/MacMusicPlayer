# Plan: MacMusic 1.0, and changes after it

Status: approved (2026-10-08); decisions at the end.

## Progress

| Step | Done | Commit |
|---|---|---|
| The slow 9d/9e test runs explained | LaunchAPPL runs at about 1x; 9d was songs playing through, 9e the Clicker's hang | (testing notes, issues log) |
| Part A0, housekeeping | H1-H5; H6 not needed | `d7e18fc` |
| Part A, the version | "1.0" in one place, `vers`, About copyright | `91189cd` |
| Part B, the tests | B1 host tests, B2 smoke test, B3 manual checklist | `8107863`, `e032a5c`, `e50e8e0` |
| Additions approved on the way | Fix 2 follow-up (Import alert text); Extras > Clock (E1-E5) | `81b7907`, `1c888f9` |
| Part C, documentation | README, user guide and Read Me tool, CHANGELOG, decisions index, CLAUDE.md, issues logs | (this commit) |
| Part D, the release | to do | |

## Aim

- **Make the current MacMusic version 1.0:** a fixed, documented,
  reproducible point that can be rebuilt exactly, installed on a Mac Plus
  and handed to someone else.
- **Keep the project easy to pick up again later:** for fixes found in
  use, or for new features, without losing what 1.0 is.

## Where the project is now

- **Code:** Stages 1-10 complete, plus Fixes 1 and 2. 63 commits on
  `main`; one old tag, `pre-stage1`; no remote repository. The working
  tree is clean.
- **Version:** "0.1", written in three places:
  - the About box (`resources/MacMusic.r`);
  - Settings > About (`src/ui_lists.c`);
  - the Finder's signature resource (`resources/finder-icons.r`).

  There is no `vers` resource, so the Finder's Get Info shows no version.
- **Documentation:**
  - `CLAUDE.md` (build and run, now partly out of date: "Current state"
    describes Stage 1);
  - the issues log;
  - the stage plans from Stage 6 on;
  - the legal statement;
  - the `library-image` README.

  There is no project README, no user guide, and no changelog. The plans
  for Stages 1-5 were never added to `docs/`; their decisions live in the
  code's DECISION and PROVISIONAL comments and the commit history.
- **Tests:** each stage was tested with throwaway diagnostic builds,
  which were not kept, plus manual checks in Mini vMac. The only tests
  in the project are the `library-image` check program and the test
  library scripts. A later change therefore has nothing ready to run to
  show it hasn't broken earlier work.
- **Hardware:** 1.0's library behaviour has been tested in Mini vMac; the
  307-song Scratch image is on the Plus, with results still to be
  recorded. It has the app from before Fixes 1 and 2.
- **Open items (none blocking):**
  - **Issues:** I4 (0.9% fast), I6 (open a library from the Finder), I8
    (an unexplained stall, seen once), I9 (undocumented Sound Manager
    state).
  - **Unexplained slow test runs:** the 9d and 9e test runs.
  - **Deferred:** the Clicker, On-The-Go, the Extras audit, using
    ratings, "Move to Library", fast scanning.
  - **The licence** the legal statement refers to: not yet written.

## The plan

### Part A0: housekeeping (from a read-only audit, 2026-10-08)

Done before Part A, so 1.0 ships tidied. H1 comes first, because it may
change the memory settings 1.0 ships with.

- **H1, memory headroom (the one real finding).**
  - **The partition:** MacMusic asks MultiFinder for 512 KB but accepts
    384 KB (its `SIZE` resource).
  - **A rough tally:**
    - the audio ring, 148 KB, and the stack, 24 KB;
    - code about 94 KB, and globals about 47 KB (6.5 KB data, 41 KB
      zeroed);
    - the library's tables, about 40 KB for 307 songs and about 65 KB at
      the 500-song limit;
    - smaller items: the LCD, the window, menus, the sound channel, and
      the Legal text (4 KB);
    - 32 KB copy buffers during an import or export.
  - **That's about 365 KB with 307 songs before an import:** only about
    20 KB spare at 384 KB.
  - **The step:** a diagnostic measures the free memory (FreeMem,
    MaxMem) at the worst case, at both partition sizes. The worst case is
    500 songs, an import or an edit under way, Legal open, and a rescan.
    Then the `SIZE` minimum is set from the result (probably 448 or
    512 KB), and the figures are recorded in the issues log.
- **H2, the one compiler warning:** `import.c` (`MakeSongFileName`)
  builds a short file name with the Str255 append function. It is
  harmless (names are short), but it is the only warning under
  -Wall -Wextra; build into a local Str255, then copy.
- **H3, comments no longer true:**
  - `playback.h` ("Simulated playback... No audio yet");
  - `ui_lists.c` ("playback is Stage 4");
  - `ui_nav.h` ("Stage 1 subset");
  - an unused constant, `kVBufA`, in `volume.c`.

  Also review the 76 "Stage N" notes in the headers: keep them as
  history, but correct any statement that no longer holds.
- **H4, duplicated code:** the dialog helpers `DrawDefaultOutline` and
  `ItemHandle` exist in both `playlists.c` and `songdialog.c`. Make one
  shared copy, in `modal.c`.
- **H5, accessors only the tests use:** `Audio_Underruns`,
  `Library_SongSeconds` and `UI_NowPlayingMode`. Keep them for Part B,
  each marked "for tests".
- **H6, optional memory trims, only if H1 shows a need:** about 9 KB of
  the 41 KB of fixed buffers is used only briefly and could be
  allocated when needed:
  - the stay-on-screen names (3 KB);
  - the playlist ID buffer (4 KB);
  - the shuffle work space (2 KB).

**Checked and fine:**
- every alert and dialog resource is in use;
- no TODO or throwaway code is left;
- the 36 PROVISIONAL markers are documented decisions (listed in Part
  C's decisions index);
- the Clicker's unused place in the prefs is documented.

**Tests:**
- the H1 diagnostic;
- a clean build with no warnings;
- the existing behaviour unchanged (a short diagnostic of the dialogs
  touched by H4);
- the manual check.

Committed as one housekeeping commit (H6 separately, if done).

### Part A: settle what 1.0 is (code; small)

1. **One version number:**
   - "1.0" defined once, in `CMakeLists.txt`, and passed to both the
     compiler and Rez;
   - the About box, Settings > About and the signature resource all take
     it from there;
   - add `vers` resources (1 and 2), so the Finder's Get Info shows
     "1.0" and "MacMusic 1.0".
2. **The About box:** the version, with a short copyright line ("©
   2026 Jack McAteer", matching the legal statement); its text otherwise
   as now.
3. **No behaviour changes go into 1.0** beyond these. Anything found from
   now on goes in the log for 1.0.1 or later (Part E).

### Part B: a regression test suite kept in the project (V3)

So that any later change can be checked against 1.0 in minutes:

1. **Host tests,** run on this Mac with one command (`tests/run-host.sh`):
   - MacMusic's library, song file, playlist, prefs and shuffle code,
     built for the Mac as the `library-image` check is;
   - covers scanning, IDs, the 500-song and 10-playlist limits, renaming
     and deleting playlists, editing, the rating bits, both prefs
     formats, and the marker carry-over.
2. **An emulator smoke test** (`tests/run-smoke.sh`): a test build run
   with LaunchAPPL, through a scripted tour.
   - **What it drives:** browsing, playing, the Now Playing modes,
     Shuffle, Repeat, fast-forward and the Settings screens. The checks
     are like the stage diagnostics, but kept and maintained.
   - **Code changes:** the test build is the normal app plus one test
     file, compiled in only for this test (a CMake option), so the
     released app is unchanged.
3. **A manual checklist** (`docs/testing/manual-checklist.md`): the
   by-eye and by-ear checks the automatic tests can't do. That covers
   sound, the feel of the wheel at 1×, icons, alerts and the real Plus.
   It is run before each release.
4. **Investigate the slow 9d/9e test runs first.** If LaunchAPPL runs
   stay unreliable, the smoke test can't be trusted.

### Part C: documentation for 1.0

1. **README.md,** for the project:
   - what MacMusic is;
   - what it needs (System 6.0.7, a Mac Plus or Mini vMac);
   - how to build, run and test it;
   - the tools;
   - the layout of the source;
   - where the decisions and plans are.
2. **A user guide** in two forms:
   - `docs/user-guide.md`;
   - a TeachText "MacMusic Read Me" for the Mac, kept on the release
     disk.

   It covers:
   - setting up a library; importing, editing, exporting and removing
     songs (the remove-versus-delete rule, L1);
   - playlists; the iPod screens and controls, mouse and keyboard;
   - Now Playing's modes; fast-forward; Settings;
   - the limits: 500 songs, 10 playlists, 8-bit mono sound, System 6;
   - known issues.
3. **CHANGELOG.md:** 1.0 summarised by stage (from the plans and commit
   history), then a section per release after it.
4. **Decisions index:** `docs/decisions.md` lists the decision codes in
   the code (D…, N…, R…, L…, S…, T…, F…, G… and others) with a line each
   and where they are decided. It includes Stages 1-5, whose plans aren't
   in `docs/`.
5. **CLAUDE.md:** updated to 1.0. The "Current state" section, the build
   output name, the test commands, and the working rules this project
   has settled on:
   - plan first;
   - small steps;
   - no commit without approval;
   - ask before changing the SD card;
   - no screenshots or GUI control without asking.
6. **The issues log:** a "Known issues in 1.0" section, and the roadmap
   split into "after 1.0" items.
7. **The licence (V6):** the legal statement refers to "the accompanying
   licence", which is yours to write; 1.0 can be tagged without it, as
   long as it isn't distributed.

### Part D: build and record the release

1. **Run Part B's tests and the manual checklist,** in Mini vMac at 1×
   and on the real Plus (V7).
2. **Commit and tag** `v1.0`, an annotated tag with a short release note.
3. **Release files,** made by `tools/make-release.sh` into
   `~/MacPlusProjects/releases/MacMusic-1.0/`, outside the repository:
   - `MacMusic-1.0.bin`: the app as MacBinary, with the "has bundle"
     flag set;
   - `MacMusic-1.0.dsk`: an 800K floppy image with MacMusic and the
     Read Me, for Mini vMac or a real floppy;
   - the source as `MacMusic-1.0-source.zip` (`git archive v1.0`);
   - a `SHA256SUMS` file.

   Optionally, a Scratch image (V8) built with `tools/library-image`.
4. **Backup (V5):** covered by this Mac's periodic backups to an external
   drive; no separate step.

### Part E: after 1.0, how changes are made

- **Your issues log:** `docs/issues-found.md`, a simple table you add to
  while using MacMusic. For each entry: the date, what happened, steps to
  reproduce, how serious it is, and the version. I turn entries into
  fixes when you ask, using the same numbering as the issues log.
- **The usual steps, unchanged:** plan, build, diagnostic and tests,
  your manual check, your approval, then commit.
- **Version numbers (V2):**
  - **1.0.x:** fixes only, with no change to the file formats;
  - **1.x:** new features, which keep reading 1.0's libraries, prefs and
    song files;
  - **2.0:** a change that needs a library conversion.

  The version number is changed in one place (Part A).
- **Branches (V4):** `main` stays the latest release plus finished work.
  A larger feature can use a branch until it's approved. Each release is
  tagged, so any version can be rebuilt with `git checkout v1.0`.
- **Every change** runs the Part B tests, updates CHANGELOG.md, and adds
  a line to the issues log if it closes one.

## Order

1. Part B's investigation of the slow test runs (the H1 diagnostic
   needs reliable LaunchAPPL runs too).
2. Part A0, housekeeping (H1 first).
3. Part A.
4. Part B.
5. Part C.
6. Part D.

Each is committed separately after your approval. Part E is documented
in Part C and applies from then on.

## Decisions (2026-10-08)

- **V1:** the version is "1.0".
- **V2:** after 1.0: 1.0.x for fixes, 1.x for features, 2.0 for a change
  needing a library conversion.
- **V3:** the full test suite: host tests, the emulator smoke test and the
  manual checklist.
- **V4:** `main` plus a tag per release; branches only for larger
  features.
- **V5:** no separate off-site step: this Mac is backed up to an external
  drive periodically, which covers the repository and the release folder.
- **V6:** tag 1.0 now; add the licence when it is written.
- **V7:** the manual checklist must pass on the real Plus before tagging.
- **V8:** the release files are the app, the 800K floppy image, the source
  and checksums, and a BlueSCSI Scratch image (made with
  `tools/library-image`).
- **V9:** the user guide in both forms: Markdown, and a TeachText Read Me.
- **H1-H6:** Part A0 does H1-H5, and H6 only if H1 shows memory is short.
