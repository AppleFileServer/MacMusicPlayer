# MacMusic

## Project

MacMusic (formerly MacMusicPlayer; the folder keeps the old name) is a
classic Macintosh 68K application for the Macintosh Plus under System
6.0.7: a faithful 4th-generation monochrome iPod, with its own music
library and sound. Version 1.0 was released on 2026-10-08.

It is built with the Retro68 toolchain and tested in Mini vMac (at 1x)
and on a real Mac Plus. See `README.md` for the layout and tools.

## Important constraints

- Target classic 68K Macintosh systems, especially Macintosh Plus.
- Use the classic Macintosh Toolbox APIs available through Retro68.
- Do not use modern macOS APIs for the Macintosh application.
- Do not replace or modify the working Retro68 toolchain.
- Keep the application compatible with the existing Retro68 CMake setup.
- Prefer simple, well-understood classic Mac code over unnecessary abstractions.

## Build

From the project directory:

    rm -rf build
    cmake -S . -B build \
      -DCMAKE_TOOLCHAIN_FILE="$HOME/Retro68-build/build-target/cmake/retro68.toolchain.cmake"
    cmake --build build

The results are `build/MacMusic.APPL` (for LaunchAPPL) and
`build/MacMusic.bin` (MacBinary, with the "has bundle" flag set). The
version is set only by `project(MacMusic VERSION ...)` in
`CMakeLists.txt`. Since 1.1 it's optimised (-O2) by default. Builds are reproducible apart from the MacBinary dates
and header CRC.

## Run

- A fresh emulator each time:

      ~/Retro68-build/toolchain/bin/LaunchAPPL build/MacMusic.APPL

- The persistent test disk, which keeps the library and prefs:
  `tools/run-persistent.sh`. This is what the user checks with.

## Test

- `tests/run-host.sh`: the portable modules on this Mac (seconds).
- `tests/run-smoke.sh`: the scripted iPod tour in Mini vMac (about 20 s;
  Mini vMac must be closed).
- `docs/testing/manual-checklist.md`: before each release.

Run both scripts after any change; they must pass before a commit. Add
host or smoke tests for new behaviour where they can check it.

## Working rules

- **Plan before coding.** Lay out a plan with numbered decisions; wait
  for the user's approval. Mark undocumented iPod behaviour PROVISIONAL
  or DECISION in the code, and add new decision codes to
  `docs/decisions.md`.
- **Small, testable steps.** Keep the app runnable after each one.
  Preserve the working event loop unless there is a specific reason.
- **Throwaway diagnostics** go in the scratchpad, never in the project.
  Configure each copy fresh (`cmake -S . -B build`); never copy a build
  folder, as its cache points at the original source.
- **Never commit until the user approves.** The user checks each change
  in Mini vMac and says "check done ... commit".
- **Mini vMac:** ask the user to quit it rather than quitting it or
  killing the process yourself.
- **No screenshots and no Accessibility control** of the screen,
  mouse or keyboard unless the user explicitly asks.
- **The BlueSCSI SD card:** tell the user the plan before changing
  anything on it.
- **Timing:** LaunchAPPL's Mini vMac runs at about 1x, slower when this
  Mac is busy. Keep test runs short, seek rather than play through, and
  use `--timeout` (see the testing notes in `docs/outstanding-issues.md`).

## After 1.0

Changes follow `docs/plans/release-1.0.md`, Part E:
- the user logs problems in `docs/issues-found.md`;
- 1.0.x for fixes, 1.x for features that keep reading 1.0's libraries,
  2.0 for a change needing a library conversion;
- every change runs the tests, updates `CHANGELOG.md`, and updates the
  issues log if it closes an issue;
- each release is tagged (`v1.0`, ...).

## Documents

- `RESTART.md`: coming back after a long gap (start here).
- `docs/environment.md`: everything outside the repository the build and
  tests need; `tools/check-environment.sh` checks it, and the archive in
  `~/MacPlusProjects/environment-1.0/` (`tools/make-environment-archive.sh`)
  restores it. Remake the archive after changing the environment.
- `docs/decisions.md`: the decision codes and where they are decided.
- `docs/outstanding-issues.md`: open issues, known issues in 1.0,
  limitations, testing notes and the roadmap.
- `docs/plans/`: the plans from Stage 6 on, and for 1.0.
- `docs/user-guide.md`: the user guide (`tools/make-readme.py` makes the
  TeachText Read Me from it).
