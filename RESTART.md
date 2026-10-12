# Coming back to MacMusic

A refresher for picking the project up again after a long gap. Five
minutes to read; about an hour to be fixing bugs.

## What this is

MacMusic 1.1 (October 2026; 1.0 a week before): a 2004 iPod as a
Macintosh Plus program, in C, built on this Mac with the **Retro68** cross-compiler and tested
in the **Mini vMac** emulator and on the real Plus (BlueSCSI). Everything
about it is in this folder; `README.md` is the overview.

**The latest release is 1.1** (2026-10-13, tag `v1.1`): faster start-up,
printing, importing several songs. Its files, manuals included, are in
`~/MacPlusProjects/releases/MacMusic-1.1/`. Where it was left
(2026-10-13, put aside):

- **Not yet checked on the real Plus** (it was released without the
  full manual checklist, `docs/testing/runs/1.1.md`): printing on the
  ImageWriter, the time of a first start or Rescan Library, a 1 MB Plus.
  Anything found goes in `docs/issues-found.md`, for 1.1.1.
- **Open issues, for review after 1.1:** I4, I6, I8 and I9 in
  `docs/outstanding-issues.md`. The issues log has nothing open.
- **On the BlueSCSI card:** `HD60_512 MacMusic Timing 3.hda`, a timing
  image no longer needed, can be deleted (it shares SCSI ID 6 with the
  Pokémon project's disk).
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
     own-use one), `tools/utilities-manual/make-card.py`, and for a
     version with changes a user would notice, an update card like
     `make-update-card.py` (MD10 in `docs/plans/manual-1.1-part-d.md`);
     each built with `tools/utilities-manual/build-pdf.sh`;
   - then `shasum -a 256` of each into the release's `SHA256SUMS`.

## Where to look

| For | Read |
|---|---|
| The tools and how they were set up | `docs/environment.md` |
| How MacMusic is meant to behave | `docs/user-guide.md` |
| Why the code does what it does | `docs/decisions.md` (codes like D9, N4, G1) |
| Known problems and ideas for later | `docs/outstanding-issues.md` |
| What changed when | `CHANGELOG.md`, `git log` |
