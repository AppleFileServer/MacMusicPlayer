# MacMusic

MacMusic is a music player for the Apple Macintosh Plus that looks and
works like a fourth-generation (2004) monochrome iPod: a click wheel, the
iPod's menus and Now Playing screen, playlists, Shuffle and Repeat. It
plays its own library of 8-bit mono WAV songs through the Plus's built-in
sound, gaplessly, while you use other programs.

It is classic 68000 Macintosh software, written in C for System 6.0.7,
built with the Retro68 cross-compiler and tested in Mini vMac and on a
real Mac Plus. Version 1.0 (2026).

- **For using MacMusic,** see the [user guide](docs/user-guide.md).
- **For what changed,** see the [changelog](CHANGELOG.md).

## What it needs

- **A Macintosh Plus** running **System 6.0.7**, under the Finder or
  MultiFinder (MacMusic asks for 640K and runs in 512K). Other 68000 Macs
  with System 6 may work but are untested: playback relies on System 6's
  Sound Manager in a way that is checked at start-up (issue I9).
- **Or Mini vMac,** emulating a Mac Plus, at 1x speed for realistic timing.
- **Songs:** WAV files, 22,050 Hz, 8-bit, mono. Up to 500 songs and 10
  playlists in a library. On a Mac, `afconvert -f WAVE -d UI8@22050 -c 1
  --mix in.mp3 out.wav` makes one; `tools/library-image` converts a whole
  music folder.

## Building

You need [Retro68](https://github.com/autc04/Retro68), built in
`~/Retro68-build`, and CMake.

    cmake -S . -B build \
      -DCMAKE_TOOLCHAIN_FILE="$HOME/Retro68-build/build-target/cmake/retro68.toolchain.cmake"
    cmake --build build

The application is `build/MacMusic.bin` (MacBinary, for copying to a Mac)
and `build/MacMusic.APPL` (for LaunchAPPL). The version is set in one
place: `project(MacMusic VERSION ...)` in `CMakeLists.txt`.

## Running

- **Quickly,** in a fresh emulator each time (nothing is kept):

      ~/Retro68-build/toolchain/bin/LaunchAPPL build/MacMusic.APPL

- **With a persistent test disk** (the library and prefs are kept
  between runs): `tools/run-persistent.sh`. It copies the build onto
  `testdisks/MusicTest.dsk` and starts Mini vMac.
- **On a real Plus:** `tools/library-image` makes a BlueSCSI disk image
  holding MacMusic and a library of your own songs.

## Testing

| Command | What | Time |
|---|---|---|
| `tests/run-host.sh` | The library, song file, playlist, prefs, settings and shuffle code, built for this Mac over a stand-in Toolbox (`tests/host`) | seconds |
| `tests/run-smoke.sh` | MacMusic built with a scripted tour of the iPod (`tests/smoke`), run in Mini vMac with LaunchAPPL | about 20 s |
| [`docs/testing/manual-checklist.md`](docs/testing/manual-checklist.md) | What needs a person: sound, the controls, dialogs, the Finder, the real Plus | by hand, before a release |

Both scripts exit with status 0 only if everything passed. The smoke test
needs Mini vMac to be closed. Its build (`-DMACMUSIC_SMOKE_TEST=ON`) is
separate, and the normal build contains none of it.

## The source

| Files | What |
|---|---|
| `src/main.c` | Start-up, the event loop, the Mac menus |
| `src/ipod_shell.c`, `ipod_lcd.c`, `lcd_draw.c`, `lcd_layout.h` | The iPod's body and click wheel, and the 160x128 LCD drawn offscreen |
| `src/input.c` | The mouse and keyboard as iPod buttons and wheel |
| `src/ui_nav.c`, `ui_lists.c`, `ui_menus.c` | The iPod's screens: menus, lists, Now Playing, Settings, the Clock |
| `src/library.c`, `songfile.c`, `song_format.h` | The library: song files (a WAV with an "iPod" chunk), scanning, lists, playlists |
| `src/libfolder.c`, `libsetup.c`, `prefs.c`, `settings.c` | Finding and remembering libraries; the prefs file; Settings |
| `src/import.c`, `export.c`, `editsong.c`, `songdialog.c`, `wav.c`, `fileops.c` | Import, Export and Edit Song |
| `src/playlists.c` | The Playlists menu |
| `src/playback.c`, `audio.c`, `shuffle.c`, `volume.c` | The play queue, the Sound Manager ring buffer, shuffle, the Plus's volume |
| `src/modal.c`, `message.c` | Keeping the sound fed in dialogs and menus; alerts |
| `src/legal.c` | The Legal screen's text |
| `resources/` | Menus, dialogs, alerts, icons, the legal text, the version resources |
| `tests/` | The host tests and the smoke test |
| `tools/` | Below |

## Tools

| Tool | What |
|---|---|
| `tools/run-persistent.sh` | Starts Mini vMac with the build on the persistent test disk |
| `tools/library-image/` | A real-song library and BlueSCSI disk image for the Plus ([README](tools/library-image/README.md)) |
| `tools/make_test_library.py`, `install-test-library.sh` | A small generated test library, copied onto the test disk |
| `tools/make_test_wavs.py`, `install-test-wavs.sh` | Test WAV files for Import, copied onto the test disk |
| `tools/make-legal-r.py` | Makes `resources/legal.r` from `docs/legal/legal-statement.md` |
| `tools/set-bundle-bit.py` | Sets the app's "has bundle" Finder flag after each build (for its icons) |

## Documents

- [`docs/user-guide.md`](docs/user-guide.md): how to use MacMusic.
- [`docs/decisions.md`](docs/decisions.md): every decision code in the
  code (D9, N4, G1...), what it decided and where.
- [`docs/plans/`](docs/plans/): the plans for Stage 6 onwards, and for
  1.0 (`release-1.0.md`), with their reasoning.
- [`docs/outstanding-issues.md`](docs/outstanding-issues.md): known
  issues, accepted limitations, testing notes and the roadmap.
- [`docs/issues-found.md`](docs/issues-found.md): problems found while
  using MacMusic, to be fixed in later versions.
- [`docs/legal/legal-statement.md`](docs/legal/legal-statement.md): the
  legal statement shown on the iPod's Legal screen.

## Status and legal

MacMusic 1.0 is complete. Changes after 1.0 follow the steps in
`docs/plans/release-1.0.md` (Part E): fixes as 1.0.x, features as 1.x.

MacMusic is an independent project, not affiliated with, endorsed by,
sponsored by or produced by Apple Inc. Apple, Macintosh, Mac and iPod
belong to their respective owners. Copyright © 2026 Jack McAteer; see
the [legal statement](docs/legal/legal-statement.md). The licence is to
follow.
