# Plan: Part F, preserving the build and test environment

Status: approved (2026-10-08), decisions X1-X8 as recommended.

## Aim

In a year, on this Mac or a replacement, get from "found a bug" to
"built, tested and running in Mini vMac" in under an hour, without
rediscovering how anything was set up. The first steps are in
`RESTART.md` at the top of the repository.

## What MacMusic depends on outside the repository (2026-10-08)

| Dependency | Where | Notes |
|---|---|---|
| Retro68 (gcc, Rez, MakeAPPL, LaunchAPPL, hfsutils) | source `~/Retro68` at `f3b44a4` (2026-09-26), built into `~/Retro68-build` with `build-toolchain.bash --no-ppc --no-carbon` | the toolchain records `RETRO68_ROOT=/Users/jack/Retro68-build/toolchain`, overridable in CMake |
| A local LaunchAPPL change | uncommitted in `~/Retro68/LaunchAPPL/Client/MiniVMac.cc` | runs Mini vMac's executable directly instead of `open -nWa` |
| Mini vMac 37.03 | `~/minivmac`, source `6860a9a`, configured as below, built with `xcodebuild` | |
| The Mac Plus ROM | `~/minivmac/vMac.ROM` | can't be downloaded legitimately: keep it |
| System 6.0.7 disk, autoquit disk | `~/minivmac/System607-hfs.img` (+ `.bak`), `autoquit-1.1.1.dsk` | |
| Disk links | `~/minivmac/disk1.dsk` → the System disk, `disk2.dsk` → `testdisks/MusicTest.dsk`, `disk3.dsk` → `testdisks/Transfer.dsk` | |
| LaunchAPPL settings | `~/.LaunchAPPL.cfg` | |
| Test disks | `testdisks/` (ignored by git) | used by `tools/run-persistent.sh` |
| BlueSCSI template | `~/MacPlusProjects/BlueSCSI-backups/Scratch.before-0692d4f.hda` (31 MB) | holds the Plus's SCSI driver; needed for Scratch images |
| Song work folder | `~/MacPlusProjects/library-work` (1.3 GB) | remade from the Music folder by `tools/library-image` |
| Host tools | macOS 26.4.1 (arm64), CMake 4.4.3, Apple clang 21, Python 3.14; `afconvert`, `mdls`, `afinfo` | |
| A second Retro68 clone | `~/MacPlus/Retro68` (2 GB) | not used (X7) |

Mini vMac was configured with:

    ./setup_t -maintainer "egon.rath@gmail.com" -homepage "https://github.com/egrath" \
      -n "minivmac-plus" -e xcd -t mcar -m Plus \
      -hres 512 -vres 342 -depth 0 -magnify 1 -mf 2 -sound 1 \
      -sony-sum 1 -sony-tag 1 -speed 4 -ta 2 -chr 0 -drc 1 -sss 4 \
      -fullscreen 0 -var-fullscreen 1 -api cco > setup.sh
    . ./setup.sh
    xcodebuild

## Steps

- **F1, `docs/environment.md`:** every dependency above with versions and
  commits; how to rebuild each from nothing; how to restore them from
  the archive instead (F4); a "first day back" list.
- **F2, `tools/patches/retro68-launchappl.patch`:** the LaunchAPPL change,
  with why it's needed and how to apply it.
- **F3, `tools/check-environment.sh`:** checks every piece (the ROM by
  checksum, LaunchAPPL patched or not) and prints ✓ or ✗ with a pointer
  into `environment.md`. Changes nothing.
- **F4, `tools/make-environment-archive.sh`:** writes
  `~/MacPlusProjects/environment-1.0/`: the built toolchain (465 MB), the
  Mini vMac folder (79 MB, with the ROM, System disks and configuration),
  the test disks, the BlueSCSI template, `~/.LaunchAPPL.cfg`, a
  `git bundle` of the repository with its tags, a README and SHA256SUMS.
  About 0.6 GB. For the owner's use only: the ROM and System disk are
  Apple's copyright.
- **F5, the proof:** restore the archive into a temporary folder, clone
  the bundle at `v1.0`, build with the restored toolchain, run the host
  and smoke tests with a separate LaunchAPPL settings file, and check the
  app is byte-identical to the released `MacMusic-1.0.bin` (MacBinary
  dates aside). Needs the scripts to honour `RETRO68` and `MINIVMAC`
  locations and `run-smoke.sh` a LaunchAPPL settings file, with
  today's defaults unchanged.
- **F6:** pointers in `CLAUDE.md` and `README.md`; a CHANGELOG note under
  "After 1.0" (developer tools only).

## Decisions (2026-10-08)

- **X1:** the archive is `~/MacPlusProjects/environment-1.0/`, covered by
  the periodic external-drive backups.
- **X2:** Retro68's source isn't archived; its commit and the patch are
  recorded, and the built toolchain is archived.
- **X3:** the song work folder isn't archived (it can be remade).
- **X4:** the scripts honour `RETRO68` and `MINIVMAC` locations, and
  `run-smoke.sh` a LaunchAPPL settings file; defaults unchanged.
- **X5:** F5 must rebuild the released 1.0 app byte for byte.
- **X6:** the repository's off-Mac copy is the git bundle in the archive;
  a remote (such as a private GitHub repository) is the owner's decision,
  not part of this plan.
- **X7:** `~/MacPlus/Retro68` is noted as unused and left alone.
- **X8:** documents and scripts only, so no new app version; committed in
  steps after the owner's check: F2 first, then F1 and F3, then F4-F6.

## Tests

- `check-environment.sh` passes here, and fails clearly when pointed at
  an empty folder.
- The archive's SHA256SUMS verify.
- The F5 restore builds the identical app and passes both test suites.
