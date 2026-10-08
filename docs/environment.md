# MacMusic's build and test environment

Everything outside this repository that building, testing and releasing
MacMusic depend on, as set up for 1.0 (2026-10-08), and how to get each
piece back. `tools/check-environment.sh` checks them all and points
here (by section number) for anything missing. `RESTART.md` is the
short version.

There are two ways back:
- **Restore** from the environment archive, `~/MacPlusProjects/environment-1.0/`,
  with its `restore.sh` (the archive is made by
  `tools/make-environment-archive.sh`; its README says more). Quick, and
  gives back exactly what 1.0 was built and tested with: restored into an
  empty folder on 2026-10-08, it rebuilt the released 1.0 app byte for
  byte and passed both test suites.
- **Rebuild** from nothing (sections 2-8). Slow, and the newest macOS,
  Xcode or Homebrew may need changes to the steps.

Paths below are the defaults. The scripts also take `RETRO68` (the
Retro68 build folder, default `~/Retro68-build`) and `MINIVMAC` (the Mini
vMac folder, default `~/minivmac`).

## 1. The pieces

| § | Piece | Where | 1.0 used |
|---|---|---|---|
| 2 | This Mac's own tools | the system, Homebrew | macOS 26.4.1 (Apple silicon), Xcode 26.6, Command Line Tools 26.4.1, CMake 4.4.3, Apple clang 21, Python 3.14, Homebrew 7.0.7 |
| 3 | Homebrew libraries the toolchain loads | `/opt/homebrew/opt/...` | boost 1.92.0, gmp 6.3.0, mpfr 4.2.2, libmpc 1.4.1, gettext 1.0, zstd 1.5.7_1 |
| 4 | Retro68 (gcc 16.1.0, Rez, MakeAPPL, LaunchAPPL, hfsutils) | source `~/Retro68`, built into `~/Retro68-build` | commit `f3b44a4` (2026-09-26) plus `tools/patches/retro68-launchappl.patch` |
| 5 | Mini vMac, as a Mac Plus | `~/minivmac` | 37.03, commit `6860a9a` (2026-09-17) |
| 6 | The Plus ROM, System 6.0.7 and the autoquit disk | `~/minivmac` | see the checksums in §6 |
| 7 | The persistent test disks | `testdisks/` here (not in git) | `MusicTest.dsk` (40 MB), `Transfer.dsk` (8 MB) |
| 8 | For Scratch images: the BlueSCSI template and the songs | `~/MacPlusProjects/` | see §8 |

Not used: `~/MacPlus/Retro68` is a second Retro68 clone at the same
commit; nothing builds from it. `build-xcode/` here is an old,
ignored build folder.

## 2. This Mac's own tools

- **Xcode** (for Mini vMac's `xcodebuild`) and its **Command Line Tools**
  (`xcode-select --install`): the host compiler for the host tests,
  `make`, `git`.
- **Homebrew** (https://brew.sh), then:

      brew install cmake boost gmp mpfr libmpc bison texinfo

  CMake 3.15 or later builds MacMusic. Bison and Texinfo are only for
  building Retro68.
- **Python 3** for the tools; `afconvert`, `mdls` and `afinfo` (part of
  macOS) for `tools/library-image`.

## 3. The Homebrew libraries: keep them, or the toolchain breaks

Retro68's tools are linked to Homebrew's libraries, not built into
them: LaunchAPPL and Rez load **boost** (filesystem, program_options,
wave, thread and others); gcc's compiler loads **gmp**, **mpfr**,
**libmpc**, **gettext** and **zstd**. A `brew upgrade` that replaces one
of these with an incompatible version can stop Rez, LaunchAPPL or gcc
starting ("Library not loaded" or a crash), with nothing in the project
changed. boost is the likeliest, as its library names carry no version.

They were pinned on 2026-10-08 (X9), which keeps them as they are:

    brew pin boost gmp mpfr libmpc gettext zstd

(`brew unpin ...` to allow upgrades again.) If they have been upgraded
and the toolchain no longer starts, either restore the toolchain from
the archive with its libraries (see its README), or rebuild Retro68
(§4) against the new ones.

## 4. Retro68

    git clone https://github.com/autc04/Retro68.git ~/Retro68
    cd ~/Retro68
    git checkout f3b44a4
    git submodule update --init
    git apply ~/MacPlusProjects/MacMusicPlayer/tools/patches/retro68-launchappl.patch
    mkdir ~/Retro68-build && cd ~/Retro68-build
    ~/Retro68/build-toolchain.bash --no-ppc --no-carbon

This takes an hour or more. The toolchain lands in
`~/Retro68-build/toolchain`; MacMusic's CMake uses
`~/Retro68-build/build-target/cmake/retro68.toolchain.cmake`, which
records the toolchain's location (`RETRO68_ROOT`). Built elsewhere,
pass `-DRETRO68_ROOT=<that toolchain folder>` to CMake as well.

The patch makes LaunchAPPL run Mini vMac's executable directly, so it
waits for it and can stop it at `--timeout`; the patch file says more.
`tools/check-environment.sh` tells a patched LaunchAPPL from one that
isn't.

## 5. Mini vMac

    git clone https://github.com/minivmac/minivmac.git ~/minivmac
    cd ~/minivmac
    git checkout 6860a9a
    gcc -o setup_t setup/tool.c
    ./setup_t -maintainer "egon.rath@gmail.com" -homepage "https://github.com/egrath" \
      -n "minivmac-plus" -e xcd -t mcar -m Plus \
      -hres 512 -vres 342 -depth 0 -magnify 1 -mf 2 -sound 1 \
      -sony-sum 1 -sony-tag 1 -speed 4 -ta 2 -chr 0 -drc 1 -sss 4 \
      -fullscreen 0 -var-fullscreen 1 -api cco > setup.sh
    . ./setup.sh
    xcodebuild

Put the built `minivmac.app` at `~/minivmac/minivmac.app` (`find . -name
'minivmac*' -perm -111` finds it). The options make a Mac Plus with
sound, 512x342 in one bit. For timing checks, set 1x with Control-S,
then 1 (LaunchAPPL's runs were measured at about 1x).

## 6. The ROM, System 6.0.7 and the autoquit disk

In `~/minivmac`:

| File | What | SHA-256 (start) |
|---|---|---|
| `vMac.ROM` | the Mac Plus ROM, 128 KB. Apple's copyright: it can't be downloaded legitimately, so keep the copy in the archive. | `dd908e2b65772a6b` |
| `System607-hfs.img` | the System 6.0.7 start-up disk, 20 MB; changes as it's used | |
| `System607-hfs.img.bak` | an earlier copy of it (2026-10-01) | `62c0fc4a19b7dcab` |
| `autoquit-1.1.1.dsk` | LaunchAPPL's helper disk, from the Mini vMac site's extras | `e5cd4597ac614fd6` |

And the disk links Mini vMac mounts at start-up:

    cd ~/minivmac
    ln -s System607-hfs.img disk1.dsk
    ln -s ~/MacPlusProjects/MacMusicPlayer/testdisks/MusicTest.dsk disk2.dsk
    ln -s ~/MacPlusProjects/MacMusicPlayer/testdisks/Transfer.dsk disk3.dsk

LaunchAPPL's settings, `~/.LaunchAPPL.cfg`:

    emulator = minivmac
    minivmac-dir = /Users/jack/minivmac
    minivmac-path = ./minivmac.app
    minivmac-rom = ./vMac.ROM
    system-image = /Users/jack/minivmac/System607-hfs.img
    autoquit-image = ./autoquit-1.1.1.dsk

## 7. The persistent test disks

`tools/run-persistent.sh` copies the build onto `testdisks/MusicTest.dsk`
and starts Mini vMac with it. `Transfer.dsk` is for moving files in and
out. Both are HFS disk images, ignored by git, and kept in the archive.
To make new ones:

    H=~/Retro68-build/toolchain/bin
    dd if=/dev/zero of=testdisks/MusicTest.dsk bs=1m count=40
    $H/hformat -l MusicTest testdisks/MusicTest.dsk
    dd if=/dev/zero of=testdisks/Transfer.dsk bs=1m count=8
    $H/hformat -l Transfer testdisks/Transfer.dsk

then `tools/run-persistent.sh` (MacMusic creates its library on first
launch), and `tools/install-test-library.sh` and
`tools/install-test-wavs.sh` for test songs and WAV files.

## 8. For Scratch images (the real Plus)

`tools/library-image` and `tools/make-release.sh --scratch` need:
- **A BlueSCSI template:** any BlueSCSI image that works on the Plus.
  Only its first 96 blocks are used: the partition map and the
  `Apple_Driver43` SCSI driver (Apple's, so private). Every Scratch image
  made since 1.0 starts with them, so any of those serves, for example
  `~/MacPlusProjects/releases/MacMusic-1.0/HD50_512 Scratch 1980 MB.hda`.
  The archive keeps a 48 KB copy, `bluescsi-template.hda`, which
  `tools/make-release.sh` uses first.
- **The songs:** `~/MacPlusProjects/library-work`, made from the Music
  folder by `tools/library-image/library_image.py all` (its README). It
  isn't archived; making it again takes a while but needs nothing else.

## 9. First day back

1. `tools/check-environment.sh`: all ✓, or fix what it reports.
2. Build, `tests/run-host.sh`, `tests/run-smoke.sh` (see `RESTART.md`).
3. `tools/run-persistent.sh`, Mini vMac at 1x.
4. Log what you found in `docs/issues-found.md`.

## 10. For the manual (`docs/manual/`)

Only for building the manual's PDF, not MacMusic. All in
`~/MacPlusProjects/manual-tools/` (or `MANUAL_TOOLS`), outside the
repository:

| Piece | What | 2026-10-08 |
|---|---|---|
| `chrome-headless-shell-mac-arm64/` | Chrome for Testing's headless browser, prints the pages | 155.0.8059.39, from https://googlechromelabs.github.io/chrome-for-testing/ |
| `paged.polyfill.js` | Paged.js (MIT): page layout, running feet, page numbers | 0.4.3, from https://unpkg.com/pagedjs@0.4.3/dist/paged.polyfill.js |
| `fonts/EBGaramond*.ttf`, `OFL.txt` | EB Garamond (SIL Open Font License) | from github.com/google/fonts, `ofl/ebgaramond` |
| `fonts/built/` | MacMusic Garamond 86 and 100, made by `tools/manual/condense-font.py` | |
| `venv/` | Python with fontTools, Pillow and websocket-client | fontTools 4.66.1, Pillow 12.3.0 |

To set it up again:

    T=~/MacPlusProjects/manual-tools; mkdir -p $T/fonts; cd $T
    # Chrome: the "chrome-headless-shell" mac-arm64 zip for the version above; unzip it here
    curl -LO https://unpkg.com/pagedjs@0.4.3/dist/paged.polyfill.js
    # EB Garamond: EBGaramond[wght].ttf, EBGaramond-Italic[wght].ttf, OFL.txt into fonts/
    python3 -m venv venv && venv/bin/pip install fonttools pillow websocket-client
    for s in 1.00 0.86; do venv/bin/python ~/MacPlusProjects/MacMusicPlayer/tools/manual/condense-font.py \
        "fonts/EBGaramond[wght].ttf" "fonts/EBGaramond-Italic[wght].ttf" fonts/built $s; done

Then, in the repository (each with `$T/venv/bin/python`):

1. `tools/manual/make-figures.py docs/manual/figures/figures.txt
   docs/manual/figures/png`: the figures, from the owner's screenshots in
   `~/MacPlus/Screenshots/`.
2. `tools/manual/make-cover.py`: MacMusic's icon, and the own-use
   edition's cover (below).
3. `tools/manual/make-book.py`: the whole *MacMusic User's Guide*,
   `build-manual/MacMusic-Users-Guide.pdf` (laid out twice, for the page
   numbers in the contents and the index). Each part can also be built
   on its own for review: `make-chapter1.py` (and 2, 3, `make-appendixes.py`),
   then `tools/manual/build-pdf.sh docs/manual/chapter1.html
   build-manual/chapter1.pdf`.

Helvetica comes with macOS.

The environment archive (`tools/make-environment-archive.sh`) holds this folder (`manual-tools.tar.gz`),
and `restore.sh` puts it back, checking that the venv still runs (if
Python has changed, it prints how to remake it). The screenshots and the
private assets below are not archived: back them up yourself.

The own-use edition (decision MM1a) also needs
`~/MacPlusProjects/manual-assets-private/` (or `MANUAL_ASSETS`), outside
the repository and never shared: the MacDraw cover scan and the rainbow
logo (see its README), and `cover-macmusic.jpg`, which `make-cover.py`
makes from them. Without that folder, `make-book.py` builds the plain
edition, `MacMusic-Users-Guide-plain.pdf`, with MacMusic's own icon.
