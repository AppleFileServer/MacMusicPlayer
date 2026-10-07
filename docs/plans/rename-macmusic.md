# Plan: rename MacMusicPlayer to MacMusic

Status: **done (2026-10-07)**, decisions R1-R6. Tested with a
migration diagnostic and manually on the MusicTest setup. Decided 2026-10-07 (8b, S-11): the app is renamed **MacMusic**.

## What doesn't change

- **The Finder codes:** creator `iPdM`, and the file types of songs
  (`iPdS`), prefs (`pref`), playlists (`TEXT`) and the library marker
  (`iPdL`). Every existing library, song file, playlist and marker keeps
  working, and the Finder still associates them with the app.
- **File formats:** songs, playlists, the marker and the prefs' contents
  (prefs version 2) are unchanged.
- **The default library folder name**, "iPod Library", and the marker's
  name, "iPod Library Info". They describe the simulated iPod, not the
  app.
- **The simulated iPod** itself: its screens and its "iPod" menu title.

## Where the old name appears

| Place | Now | After |
|---|---|---|
| `CMakeLists.txt` | `project(MacMusicPlayer)`, `add_application(MacMusicPlayer ...)` | `MacMusic`: the build makes `build/MacMusic.APPL` (and `.bin`, `.dsk`) |
| Resource file | `resources/MacMusicPlayer.r` | `resources/MacMusic.r` (renamed with git, so history is kept) |
| Apple menu | "About MacMusicPlayer…" | "About MacMusic…" |
| About box | "MacMusicPlayer" | "MacMusic", still "Version 0.1" (R4) |
| Window title | "MacMusicPlayer" | "MacMusic" |
| Prefs file | "MacMusicPlayer Prefs" in the System Folder | "MacMusic Prefs", migrated (below) |
| Library alert (`libsetup.c`) | "MacMusicPlayer keeps its songs in a folder…" | "MacMusic keeps…" |
| Playlist message (`playlists.c`) | "…MacMusicPlayer can handle." | "…MacMusic can handle." |
| Marker text (`libfolder.c`) | "…an iPod Library for MacMusicPlayer…" | "…for MacMusic…" in new markers; existing markers keep their text (R5) |
| Comments (`main.c`, `prefs.h`) | the old name | the new name |
| `src/main.cpp` | an unused leftover from the project template, not built ("MacMusicPlayer development build") | R3 |
| `tools/run-persistent.sh` | installs `build/MacMusicPlayer.bin` as `:MacMusicPlayer` | installs `build/MacMusic.bin` as `:MacMusic`, and removes the old `:MacMusicPlayer` from the test disk |
| `CLAUDE.md` | name, build output, run command | the new name and paths |
| `docs/` | current documents and historical plans | R1 |
| Project folder and git repository | `~/MacPlusProjects/MacMusicPlayer` | R2 |

## The prefs file: carrying settings over

Existing users (your Mini vMac system disk, and the real Plus) have
"MacMusicPlayer Prefs" holding the library location and the volume.
Without migration, the renamed app would ask for a library again.

On startup, before the prefs are read:
- **Only the old file exists:** it's renamed to "MacMusic Prefs" (a
  rename, so nothing is copied or lost), then read as usual.
- **Both exist:** the new file is used, and the old one is left alone.
- **Neither exists:** as now, with no prefs.
- **The rename fails** (for example a locked System disk): the old file
  is read, and the next save writes "MacMusic Prefs".

The old file is never deleted (in the spirit of L1).

## Steps

1. **Code and resources, one commit:** CMake, the resource file, the
   strings above, the prefs migration, and R3.
2. **Tools and `CLAUDE.md`, the same commit:** the build output's name
   changes there, so they must change together.
3. **Tests:**
   - a throwaway diagnostic for the migration cases: only old, both,
     neither (and the library and volume read correctly after the
     rename);
   - the build output is `build/MacMusic.APPL`, and LaunchAPPL runs it;
   - **manual, on the persistent MusicTest setup:** the renamed app
     opens your existing library and volume with no Choose Library
     prompt. The window title, the Apple menu and the About box show
     MacMusic. The System Folder now has "MacMusic Prefs" and no
     "MacMusicPlayer Prefs".
4. **Documents (R1),** in the same or a following commit.
5. **Later, on the real Plus:** a new Scratch image with `MacMusic`
   replacing `MacMusicPlayer`, made when you ask (as before). Its prefs
   migrate the same way on first launch.

## Decisions (2026-10-07)

- **R1:** the current-facing documents (`CLAUDE.md`, the issues log, the
  Stage 8 plans) are updated to MacMusic. The completed stage plans
  (Stage 6 and 7) are left as written, as history.
- **R2:** the project folder and repository stay
  `~/MacPlusProjects/MacMusicPlayer` for now. Renaming them is a possible
  separate step later, with the Mini vMac disk links recreated.
- **R3:** `src/main.cpp`, the unused template leftover, is deleted.
- **R4:** the About box keeps "Version 0.1".
- **R5:** existing library markers keep their text; only new markers name
  MacMusic.
- **R6:** the old prefs file is renamed, not copied, and never deleted.
