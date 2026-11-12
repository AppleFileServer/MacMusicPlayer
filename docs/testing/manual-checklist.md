# MacMusic manual test checklist

The checks only a person can make: sound, the feel of the controls,
how things look, the Finder, and the real Macintosh Plus. Run it before
each release, after the automatic tests pass. The automatic tests already
cover the library, playlists, prefs, shuffle and the iPod screens' logic
(`tests/run-host.sh`, `tests/run-smoke.sh`); this list doesn't repeat them.

Copy this file for each run (for example `docs/testing/runs/1.0.md`), tick
each box, and note anything wrong under **Problems found** at the end,
with the section and item number.

| | |
|---|---|
| Version | |
| Date | |
| Tested by | |
| Mini vMac (1x) | yes / no |
| Real Mac Plus | yes / no (memory: ____ ; MultiFinder: yes / no) |

## 0. Before starting

- [ ] 0.1 `tests/run-host.sh` ends with "host tests: all passed".
- [ ] 0.2 `tests/run-smoke.sh` ends with "smoke test: passed".
- [ ] 0.3 A clean build (see CLAUDE.md) gives no warnings.
- [ ] 0.4 Mini vMac is at 1x speed (Control-S, then 1) for every timing check below.

Start MacMusic with `tools/run-persistent.sh` (the persistent test disk),
unless a check says otherwise.

## 1. Starting up and the library

- [ ] 1.1 With no library remembered (delete "MacMusic Prefs" from the
      System Folder): the logo, then the alert offering Find Existing...,
      Create New... and Not Now.
- [ ] 1.2 Create New...: the Save dialog suggests "MacMusic Library"; the
      folder appears with a Playlists folder and "MacMusic Library Info".
- [ ] 1.3 Quit and relaunch: the same library opens without asking.
- [ ] 1.4 File > Choose Library... > Find Existing...: only library files
      are listed; opening a song file or a playlist chooses its library.
- [ ] 1.5 File menu: the recent libraries are listed below Library
      Info...; one on a disk that isn't mounted is dimmed; choosing another
      switches library (the logo shows, then the main menu).
- [ ] 1.6 File > Library Info...: Songs "n of 500", Playlists, Size, Free,
      Unreadable Files; the figures look right.
- [ ] 1.7 File > Rescan Library while a song plays: the song keeps its
      place or stops cleanly if its file has gone.
- [ ] 1.8 A library from before Fix 2 ("iPod Library Info"): it opens and
      the marker is renamed "MacMusic Library Info".

## 2. Importing, exporting and editing

- [ ] 2.1 File > Import Song... with a 22 kHz 8-bit mono WAV on the same
      disk: the alert says the file will be moved, with Just Copy;
      the details window (title from the file name) has its default
      button outlined; the song appears in Songs.
- [ ] 2.2 Import from another disk: the alert warns it will take longer;
      the original is deleted only after the copy.
- [ ] 2.3 Import the same audio again: "already in the MacMusic Library
      as ..." with Cancel and Import Anyway.
- [ ] 2.4 A WAV in another format (16-bit or stereo): refused with
      "Only mono, 22050 Hz, 8-bit PCM WAV files can be ...".
- [ ] 2.5 File > Export Song... on a highlighted song: the WAV comes back
      and plays in another program (or re-imports as the same audio);
      the song leaves the library and its playlists.
- [ ] 2.6 Edit > Edit Song... on a song row and on Now Playing: the
      fields are filled in; Save changes the title, artist and album, and
      the iPod stays on the same screen (lists re-sorted).
- [ ] 2.7 A locked song file: Edit Song and rating explain they can't
      save; nothing else changes.

## 3. The Playlists menu

- [ ] 3.1 New Playlist...: "Untitled Playlist" suggested; Create outlined;
      the playlist appears in Music > Playlists.
- [ ] 3.2 With 10 playlists, New Playlist... warns before asking for a
      name.
- [ ] 3.3 Add to <name> on a highlighted song; adding it again asks
      (Cancel / Add Again).
- [ ] 3.4 Remove from Playlist on a song inside a playlist: only that
      entry goes; the highlight stays on the row that took its place.
- [ ] 3.5 Rename Playlist...: the name changes in place; a taken name is
      refused.
- [ ] 3.6 Delete Playlist...: the alert has Cancel first; Delete removes
      only the playlist, not its songs.
- [ ] 3.7 Items are dimmed when they can't be used (no library, no
      song highlighted, not in a playlist).

## 4. The controls

- [ ] 4.1 Clicking Menu, Previous, Next, Play/Pause and the centre
      button on the wheel works, each as on an iPod.
- [ ] 4.2 Dragging round the wheel scrolls lists smoothly at 1x, one row
      per step, with no lag or skipped redraws.
- [ ] 4.3 Keyboard: up/down arrows move the highlight, Return or Enter
      selects, Delete is Menu, Space is Play/Pause, left/right are
      Previous/Next.
- [ ] 4.4 Holding Next or Previous on the wheel (half a second) fast-
      forwards or rewinds on Now Playing, faster after two seconds;
      elsewhere a hold is just a press. Option-left/right do the same
      from the keyboard.

## 5. Sound (by ear)

- [ ] 5.1 Songs play cleanly, with no clicks, stutter or drift, for
      several minutes.
- [ ] 5.2 Album tracks play on with no gap between songs.
- [ ] 5.3 Pause and resume: the song continues from the same place.
- [ ] 5.4 Volume: one full turn of the wheel on Now Playing goes from the
      quietest level (not quite silent, accepted) to full; the arrows
      step one level; the level is the same after relaunching.
- [ ] 5.5 Fast-forward and rewind: the music keeps playing while held,
      and carries on from the new place when released.
- [ ] 5.6 The scrubber: the song jumps to the new place once turning
      stops.

## 6. How it looks

- [ ] 6.1 Now Playing: "n of m", title, artist, album, progress bar,
      elapsed and remaining times; the play, pause, fast-forward and
      rewind icons at the top left.
- [ ] 6.2 With Shuffle on, the shuffle icon at the top right below the
      battery; Repeat One and Repeat All icons beside it; neither when off.
- [ ] 6.3 Rating mode: five bullets turning into stars; volume mode: the
      bar; each goes back to the progress bar after a few seconds.
- [ ] 6.4 Lists: titles truncated cleanly, scroll bar when longer than
      the screen, arrows on rows that open more.
- [ ] 6.5 Settings > About: Songs "n of 500", Available, Version 1.0.
      Settings > Legal: readable small text (Geneva 9) that scrolls.
- [ ] 6.6 Backlight: the screen inverts and back.
- [ ] 6.7 Apple > About MacMusic...: "Version 1.0" and "© 2026 Jack
      McAteer", nothing cut off.
- [ ] 6.8 Extras > Clock: the time and date match the menu-bar clock
      (or the Alarm Clock desk accessory), centred and readable; the time
      changes on the minute while the screen is left open; 24-hour time
      when the control panel is set to it.

## 7. Living with other programs (MultiFinder)

- [ ] 7.1 With another program in front, the song keeps playing.
- [ ] 7.2 Holding a menu open, dragging the window, or leaving a dialog
      or Standard File open for 10 s doesn't stop or stutter the sound.
- [ ] 7.3 A desk accessory opens and closes without upsetting playback.
- [ ] 7.4 Quit while playing: the sound stops cleanly; the volume and
      settings are kept.

## 8. The Finder

- [ ] 8.1 MacMusic has its iPod icon; song files have the note icon
      (rebuild the desktop if the old icons show).
- [ ] 8.2 Get Info on MacMusic: version "1.0, © 2026 Jack McAteer";
      under MultiFinder, memory 640K suggested.
- [ ] 8.3 With MacMusic's memory set to 512K in Get Info, a large
      library still opens and plays.

## 9. The real Macintosh Plus

On the Plus, from the BlueSCSI Scratch image (`tools/library-image`).

- [ ] 9.1 The disk mounts; MacMusic opens its library (Find Existing the
      first time, if the prefs remember another disk).
- [ ] 9.2 The first scan's time with the full library (record it): ______
- [ ] 9.3 Sections 4, 5 and 6 on the Plus: the wheel, the sound, the
      screen.
- [ ] 9.4 Finder stress: while a song plays, duplicate a large file and
      empty the Wastebasket; the sound may pause but recovers by itself.
- [ ] 9.5 Under MultiFinder: the menus, a window drag and a dialog, as
      7.2.
- [ ] 9.6 An hour of playing with Shuffle and Repeat All: no stall, no
      crash.

## 10. New in 1.1 (`docs/plans/release-1.1.md`)

Starting and the cache (Part B):
- [ ] 10.1 With the 500-song test library (or the 307-song Scratch
      library): the first start shows "Loading the library…" under the
      logo; record its time: ______
- [ ] 10.2 Quit and start again: the message from the start (N11-29), a
      quicker start; record it: ______. The songs, artists, albums and
      playlists as before.
- [ ] 10.3 Rate a song and edit another's details; quit, start: both
      changes kept, the message shown.
- [ ] 10.4 File > Rescan Library: the band "Loading the library…" across
      the foot of the screen; the screen comes back as it was.
- [ ] 10.5 Change a song file in the Finder (replace it with a copy from
      another library), start: the new details shown.
- [ ] 10.6 The library on a locked disk: it opens, every start reads
      every file, no error.
- [ ] 10.7 Open a 1.1 library in MacMusic 1.0: all songs, no "skipped"
      alert. Then in 1.1 again.
- [ ] 10.8 1.0's MacMusic Prefs opened by 1.1, and 1.1's by 1.0.

Printing (Part C), with the printer chosen in the Chooser:
- [ ] 10.9 File > Page Setup: the driver's own dialog, A4 offered;
      chosen paper kept after quitting.
- [ ] 10.10 With no Page Setup saved (a fresh MacMusic Prefs), Print
      Library opens Page Setup first.
- [ ] 10.11 The lists dialog: the sub-boxes dim with their list; Print
      dims with nothing ticked; the choice kept for the session.
- [ ] 10.12 Print Songs, Albums (with and without songs) and Artists
      (with and without albums) on the ImageWriter: columns, cut text,
      the header and "Page n of m", each list on a new page.
- [ ] 10.13 Command-period while printing: it stops, nothing more
      printed.
- [ ] 10.14 The printer switched off, or not chosen: a plain message,
      no hang.
- [ ] 10.15 Printing while a song plays: the music pauses, then carries
      on.
- [ ] 10.16 A LaserWriter on AppleTalk, background printing on: printed
      by PrintMonitor while MacMusic plays (when one is available).

A 1 MB Plus (Mini vMac's 1 MB build with the clean System, or the Plus
with 1 MB):
- [ ] 10.17 Under MultiFinder: 500 songs with ordinary details load;
      play, print.
- [ ] 10.18 Under the Finder alone: 500 songs with long details load.

The Create New Library prompt (Part A):
- [ ] 10.19 File > Choose Library > Create New Library: "New library
      in:" shown whole.

Starting and importing (Part D):
- [ ] 10.20 Under MultiFinder, with a Finder window where MacMusic's
      opens: MacMusic's window and logo come in front at once, before the
      library is read (N11-28).
- [ ] 10.21 Import Song: import a song; "Import another song?": Import
      Another; import two more; Done. All three in the library, read once
      (one "Loading the library…") (N11-25, N11-26). Again, with Cancel
      in the Open dialog instead of Done: the same.
- [ ] 10.22 In a run, Cancel in a song's window: that song skipped, the
      question asked; "Don't Import" at the duplicate warning, the same.
- [ ] 10.23 In a run, import a song, then the same WAV again (a copy):
      the duplicate warning names the song imported earlier in the run.
- [ ] 10.24 In a run, a file that isn't a WAV: its message, then the
      question. With the library at 499 songs: one more imported, then
      "The MacMusic Library is full" (no question), and the run ends.

## 11. New in 1.2 (`docs/plans/release-1.2.md`)

Test songs: a WAV from the MacMusic Converter 1.1 (22,254 Hz), and the
same song at 22,050 Hz (an older MacMusic WAV, or
`afconvert -f WAVE -d UI8@22050 -c 1 --mix song.mp3 old.wav`).

The song details window (N12-1, N12-2):
- [ ] 11.1 Edit Song: type in Composer, then click just inside the top or
      bottom edge of Genre's box (between the frame and the text): the
      typing moves to Genre. The same for each box, and Track.
- [ ] 11.2 Clicks on the text, Tab, Return and Cancel as before; Import
      Song's window the same.

Albums > All (N12-3):
- [ ] 11.3 Music > Albums > All: album by album, each in track order,
      songs with no album last. Music > Songs: by title.
- [ ] 11.4 Artists > an artist > All, and Genres > a genre > Albums >
      All: that artist's (genre's) songs album by album.
- [ ] 11.5 With Shuffle off, play a song from Albums > All: the next
      songs follow the list.

Songs at 22,254 Hz (N12-4 to N12-7):
- [ ] 11.6 Import the 22,254 Hz WAV: its length shown right in the Import
      window; title from the WAV.
- [ ] 11.7 Play it: the time reaches the song's length as it ends; seeking
      and scrubbing land in the right place. Beside the 22,050 Hz copy,
      it's a touch lower and slower (true pitch).
- [ ] 11.8 A 22,050 Hz song, then the 22,254, then a 22,050 (an album or
      a playlist): no gap or click between them.
- [ ] 11.9 Edit Song and a rating on the 22,254 Hz song: saved; after
      Rescan Library it still plays at true pitch with the right length.
- [ ] 11.10 A WAV at another rate (44,100, or 22,255): refused, "Only
      mono, 8-bit PCM WAV files at 22050 or 22254 Hz can be imported."
- [ ] 11.11 Library Info, Print Library (Songs) and Export of the
      22,254 Hz song: its length right; the exported WAV the same as the
      one imported.
- [ ] 11.12 The first start of 1.2 with a 1.1 library: as quick as 1.1's
      later starts (1.1's cache used; no full read).
- [ ] 11.13 The library in MacMusic 1.1: "files ... were not recognised"
      for the 22,254 Hz songs; the others as before. Back in 1.2: all the
      songs, the 22,254 Hz ones included.

On the Plus (the release's BlueSCSI disk):
- [ ] 11.14 11.1, 11.3, 11.6-11.8 on the Plus.
- [ ] 11.15 The floppy check (the issues log, 2026-11-11): a floppy
      read-write-verify while a song plays; how it sounds; the files
      verified.

## Problems found

| Item | What happened | How serious |
|---|---|---|
| | | |
