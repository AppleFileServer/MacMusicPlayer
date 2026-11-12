# Coming back to MacMusic

A refresher for picking the project up again after a long gap. Five
minutes to read; about an hour to be fixing bugs.

## What this is

MacMusic 1.1 (October 2026; 1.0 a week before; 1.2 on the way): a 2004 iPod as a
Macintosh Plus program, in C, built on this Mac with the **Retro68** cross-compiler and tested
in the **Mini vMac** emulator and on the real Plus (BlueSCSI). Everything
about it is in this folder; `README.md` is the overview.

**The latest release is 1.1** (2026-10-13, tag `v1.1`): faster start-up,
printing, importing several songs. Its files, manuals included, are in
`~/MacPlusProjects/releases/MacMusic-1.1/` (checked against its
`SHA256SUMS` on 2026-10-25).

**MacMusic 1.2 is built and waiting for its checks** (2026-11-12;
`docs/plans/release-1.2.md`, approved 2026-11-11; asked for as 1.1.1 by
the Converter session, named 1.2 by the owner because it changes the
song and cache formats). Everything is committed and pushed, up to
`ee1077a`:

- **What's in it** (`CHANGELOG.md`, decisions N12-1 to N12-9): a click
  anywhere in a box of the song details window selects it; Albums › All
  lists songs by album, then track (PROVISIONAL); songs at 22,254 Hz,
  the Plus's own rate, as the MacMusic Converter 1.1 makes them, are
  read, played at true speed, edited and imported (22,050 Hz as before;
  the cache is version 2, and 1.1's is still read); songs over about 27
  minutes get their times right; up to 500 albums, artists, genres and
  composers (was 256).
- **Done:** the User's Guide (third edition) and its Read Me, *MacMusic
  Utilities* Chapter 2 for the Converter 1.1 (with its captures of
  2026-11-11), the *MacMusic 1.2 Update* card
  (`tools/utilities-manual/make-update-card-1.2.py`), the version (1.2),
  and the checklist's section 11 for 1.2.
- **The candidate:** `~/MacPlusProjects/releases/MacMusic-1.2-candidate-ee1077a/`
  (both test scripts passed when it was made).
- **The Music disk for the Plus:**
  `~/MacPlusProjects/library-work-1.2/HD50_512 Music 1984 MB.hda`
  (1,984 MiB, volume "Music", SCSI ID 5, to replace 1.1's Scratch image
  on the card): the candidate's MacMusic and a library of the owner's 376
  songs up to 10 minutes, at 22,254 Hz, levelled to −16 LUFS by the
  Converter's own engine, with full details. Made with
  `tools/library-image` (`--ffprobe`, `--converter`, `--trim 2`); its
  `wav` and `songs` folders are kept, so it can be remade quickly.

**Left for 1.2**, in order (`release-1.2.md` step F):

1. **The owner's checks:** section 11 of `docs/testing/runs/1.2.md`, in
   Mini vMac (`tools/run-persistent.sh`; the MusicTest disk has a
   22,254 and a 22,050 Hz test WAV) and on the Plus from the Music disk,
   including the **floppy read-write-verify** while a song plays (11.15;
   the issues log, 2026-11-11: the sound goes odd while a disk is read).
   Anything found goes in `docs/issues-found.md`.
2. **Then:** close I4 in `docs/outstanding-issues.md` (accepted for
   22,050 Hz songs, fixed for 22,254 Hz ones); the `CHANGELOG.md`
   heading's date; tag `v1.2`; `tools/make-release.sh` for the final
   files; and the extras below (the BlueSCSI disk, both editions of the
   PDFs, the 1.2 Update card's, `SHA256SUMS`).
3. **The MacMusic Converter Utility 1.1 is already released**
   (2026-11-12, tag `v1.1` in its repository; installed in
   Applications), by the owner's choice ahead of MacMusic 1.2: its WAVs
   are 22,254 Hz, so until 1.2 is released only the candidate (and the
   Music disk) plays them. Its release notes say so.

**Also open:**

- **1.1's Plus checks** (`docs/testing/runs/1.1.md`): start-up and
  importing passed 2026-11-11; printing on the ImageWriter still to come;
  the 1 MB Plus won't be tested (it means taking SIMMs out).
- **Open issues:** I6, I8 and I9 in `docs/outstanding-issues.md`; a 1.x
  idea in its roadmap (removing a library from the File menu's list).
- **The utilities:** the Macintosh Transfer Utility 2.0 (its own
  session, commit 80ec7d3) takes MacMusic WAVs at 22,254 Hz only; 1.0
  takes only 22,050 Hz. *MacMusic Utilities* Chapter 3 still describes
  1.0 (T3 in `LOOSE-ENDS.md`).
- **On the BlueSCSI card** (2026-11-12): the Music disk on ID 5, copied
  and checked; 1.1's Scratch image and the old Timing 3 image are gone.
- **The manuals stay 7½ x 9 in.** An A5 edition of the User's Guide was
  planned and proofed both ways on 2026-10-25, then not pursued:
  `docs/plans/manual-a5.md` says how, if it's ever wanted.
- Everything outside MacMusic (the utilities, their manual, their
  issues) is in `~/MacPlusProjects/LOOSE-ENDS.md`.

## 1. Check the tools are still here

    tools/check-environment.sh

Every line should show ✓. If anything shows ✗, the quick fix is the
environment archive:

    cd ~/MacPlusProjects/environment-1.0 && ./restore.sh

It puts back whatever is missing (and leaves the rest alone), then runs
the check again. Its README covers the one awkward case, Homebrew having
upgraded the libraries the toolchain needs. Otherwise follow the pointer
the check prints into `docs/environment.md`; rebuilding Retro68 from
source is the slow last resort.

## 2. Build and test what you had

    cmake -S . -B build \
      -DCMAKE_TOOLCHAIN_FILE="$HOME/Retro68-build/build-target/cmake/retro68.toolchain.cmake"
    cmake --build build
    tests/run-host.sh        # seconds; "host tests: all passed"
    tests/run-smoke.sh       # about 20 s; Mini vMac must be closed

If all three work, the environment is back. `git log --oneline -5` and
`git tag` show where you left off (the last release is the newest
`v...` tag).

## 3. See it running

    tools/run-persistent.sh

Mini vMac opens with the System disk and the MusicTest disk; open
MacMusic there. Its library and settings are kept between runs. Set
Mini vMac to 1x speed (Control-S, then 1) for realistic timing.

## 4. Fix the bugs

1. **Write each bug down** in `docs/issues-found.md`: what happened, how
   to make it happen, how serious.
2. **Open Claude Code in this folder** and say something like: "read
   RESTART.md and docs/issues-found.md, then plan a fix for issue ...".
   It reads `CLAUDE.md` for the project's rules: plan first, small steps,
   nothing committed without your OK, ask before quitting Mini vMac, no
   screenshots unless asked.
3. **Each fix:** plan → change → `tests/run-host.sh` and
   `tests/run-smoke.sh` → your check in `tools/run-persistent.sh` →
   commit, with a line in `CHANGELOG.md`.

## 5. Release the fixes

1. Set the version in `CMakeLists.txt` (`project(MacMusic VERSION 1.1.1)`
   for fixes; see `docs/plans/release-1.0.md`, Part E).
2. `tools/make-release.sh --scratch ~/MacPlusProjects/library-work`
   makes a **candidate** in `~/MacPlusProjects/releases/`.
3. Run `docs/testing/manual-checklist.md` on it (copy it to
   `docs/testing/runs/<version>.md`), in Mini vMac and on the Plus.
4. Tag it (`git tag -a v1.1.1`) and run `tools/make-release.sh` again
   for the final files.
5. **What `make-release.sh` doesn't make** (1.1 added them by hand):
   - `HD40_512 MacMusic.hda`, a 20 MB BlueSCSI disk with the release's
     app and Read Me: the template's first 96 blocks, then an HFS volume,
     as `tools/library-image` makes the Scratch image (a throwaway
     script did it for 1.0 and 1.1);
   - the manuals' PDFs, if they changed: `tools/manual/make-book.py`
     (the guide, then again with `MANUAL_ASSETS=/nonexistent` for the
     plain edition, then once more so `docs/manual/book.html` is the
     own-use one), *MacMusic Utilities* if it changed
     (`tools/utilities-manual/make-book.py`, after `make-figures.py`),
     `tools/utilities-manual/make-card.py`, and for a version with
     changes a user would notice, an update card (`make-update-card.py`
     for 1.1, `make-update-card-1.2.py` for 1.2; MD10 in
     `docs/plans/manual-1.1-part-d.md`); each built with
     `tools/utilities-manual/build-pdf.sh`;
   - then `shasum -a 256` of each into the release's `SHA256SUMS`.
6. **A test disk of real songs at 22,254 Hz** (as for 1.2's Music disk):
   `tools/library-image/library_image.py` with `--ffprobe` (the
   Converter's ffprobe) for the tags, `--converter` (an `mmc-convert`
   built from the Converter's source) with `--tools-bin` and `--trim 2`,
   and `image --size-mib 1984 --volume-name Music --app` the release's
   `.bin`.

## Where to look

| For | Read |
|---|---|
| The tools and how they were set up | `docs/environment.md` |
| How MacMusic is meant to behave | `docs/user-guide.md` |
| Why the code does what it does | `docs/decisions.md` (codes like D9, N4, G1) |
| Known problems and ideas for later | `docs/outstanding-issues.md` |
| What changed when | `CHANGELOG.md`, `git log` |
