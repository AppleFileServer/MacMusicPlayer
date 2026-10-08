# MacMusic environment archive

Everything outside the MacMusic repository that building, testing and
releasing it need, saved as it was when this archive was made (see
`MANIFEST.txt` for the date, the commit and the versions). It exists so
the project can be picked up again after a long gap, or on another Mac,
without setting the tools up from scratch.

**Private:** the Mac Plus ROM, the System 6.0.7 disk and the SCSI driver
in `bluescsi-template.hda` are Apple's copyright. Keep this archive for
your own use; don't share or publish it.

## What's here

| File | What | Restored to |
|---|---|---|
| `MacMusic.bundle` | the whole repository, every commit and tag | `~/MacPlusProjects/MacMusicPlayer` (cloned) |
| `toolchain.tar.gz` | Retro68's built toolchain (gcc, Rez, LaunchAPPL, hfsutils) and its CMake file | `~/Retro68-build` |
| `homebrew-libs.tar.gz` | the 14 Homebrew libraries the toolchain loads (boost, gmp, mpfr, libmpc, gettext, zstd) | `/opt/homebrew/...`, only where missing |
| `minivmac.tar.gz` | Mini vMac (app, configuration, source), the Plus ROM, the System 6.0.7 disks, the autoquit disk, the disk links | `~/minivmac` |
| `testdisks.tar.gz` | the persistent test disks | the repository's `testdisks/` |
| `LaunchAPPL.cfg` | LaunchAPPL's settings | `~/.LaunchAPPL.cfg` |
| `bluescsi-template.hda` | a working BlueSCSI image's first 96 blocks (partition map and driver), for Scratch images | stays here; `tools/make-release.sh` finds it |
| `restore.sh` | puts it all back | |
| `MANIFEST.txt`, `SHA256SUMS` | what was saved, and checksums | |

## Restoring

You need macOS on Apple silicon with Xcode's Command Line Tools
(`xcode-select --install`) and Homebrew with CMake and Python 3
(`brew install cmake python`).

    cd ~/MacPlusProjects/environment-1.0
    ./restore.sh

It checks the archive's checksums, puts back each missing piece (leaving
anything already there alone, and saying so), then runs
`tools/check-environment.sh`. Then follow `RESTART.md` in the repository:
build, `tests/run-host.sh`, `tests/run-smoke.sh`.

Elsewhere than the defaults: set `RETRO68`, `MINIVMAC` or
`LAUNCHAPPL_CFG`, and `--project DIR` for the repository; `restore.sh`
adjusts the paths the toolchain file, the disk links and LaunchAPPL's
settings record.

## If the toolchain doesn't start

"Library not loaded" or a crash from gcc, Rez or LaunchAPPL usually means
Homebrew has upgraded or removed one of its libraries (see
`docs/environment.md` §3 in the repository).

- **Removed:** `./restore.sh` puts the archived ones back.
- **Upgraded:** `./restore.sh --replace-libs` puts the archived versions
  over the newer ones (keeping those as `*.before-restore`). Other
  Homebrew programs that need the newer versions may then misbehave, so
  afterwards `brew pin boost gmp mpfr libmpc gettext zstd` to keep things
  steady, or rebuild Retro68 against the new libraries instead
  (`docs/environment.md` §4).
