# MacMusicPlayer

## Project

MacMusicPlayer is a classic Macintosh 68K application targeting the Macintosh Plus.

It is built with the Retro68 toolchain and tested in Mini vMac.

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

The resulting application is:

    build/MacMusicPlayer.APPL

## Run

Launch with:

    ~/Retro68-build/toolchain/bin/LaunchAPPL \
      ~/MacPlusProjects/MacMusicPlayer/build/MacMusicPlayer.APPL

## Current state

The application successfully:

- Builds with Retro68.
- Produces a valid APPL.
- Launches in Mini vMac.
- Creates a Macintosh window.
- Runs a classic Macintosh event loop.

## Development approach

Make small changes and keep the application runnable after each change.

When changing the UI or event handling, preserve the existing working event loop unless there is a specific reason to replace it.
