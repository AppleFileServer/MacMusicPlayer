# MacMusic decisions index

The decision codes used in MacMusic's code comments and plans, one line
each, grouped by the stage that made them. Codes were numbered within
each stage or plan, so the same code can mean different things in
different stages (G1 is gapless playback in Stage 5c, and the icons'
place in Stage 10c). A comment's surrounding text, or the "Stage" it
names, says which is meant.

- **DECISION:** a project choice, not known iPod behaviour.
- **PROVISIONAL:** a best guess at iPod behaviour or appearance that
  isn't documented. It may change if better information turns up.
- **Where** gives the main file. For Stages 6 onwards the plan in
  `docs/plans/` has the reasoning. Stages 1-5 had no plan documents in
  the repository, so their decisions are recorded only here and in the
  code.

## Stages 1-5: the iPod screens, library, playlists and sound

The interface (D: Stages 1-4)

| Code | Decision | Where |
|---|---|---|
| D1 | PROVISIONAL: the highlight stops at the first and last items, no wrapping | `ui_nav.c` |
| D2 | PROVISIONAL: Menu on the main menu does nothing | `ui_nav.c` |
| D3 | Extras: decided 2026-10-08, the Clock only (E1-E5 below) | `ui_menus.c` |
| D4 | PROVISIONAL: Music includes Audiobooks (inferred from the 4G's scroll bar) | `ui_menus.c` |
| D5 | PROVISIONAL: the scroll bar's thumb size | `lcd_draw.c` |
| D6 | PROVISIONAL: going back keeps the previous screen's highlight and scroll | `ui_nav.c` |
| D7 | PROVISIONAL: entering a menu highlights its first item | `ui_nav.c` |
| D8 | PROVISIONAL: the Mac keyboard mapping (an emulator convenience) | `input.h` |
| D9 | Genres, Artists and Albums start with an "All" row | `ui_lists.c` |
| D10, D16 | PROVISIONAL: text that doesn't fit ends with an ellipsis, 5 px from the edge | `lcd_draw.c`, `lcd_layout.h` |
| D11, D14 | Names sort case-insensitively (the ROM's RelString), ignoring a leading "The " | `library.c` |
| D12 | PROVISIONAL: a screen's title is the item chosen to reach it | `ui_lists.c` |
| D13 | An album's songs are in track order (also when shuffling by album) | `library.c`, `shuffle.c` |
| D17 | The On-The-Go playlist is deferred (`docs/plans/deferred-on-the-go.md`) | |
| D18 | PROVISIONAL: audiobooks appear only under Audiobooks | `library.c` |
| D19 | PROVISIONAL: albums are grouped by name alone | `library.c` |
| D20 | An empty artist, album, genre or composer makes no group | `library.c` |
| D21 | Create New uses the name typed in the Save dialog ("MacMusic Library" offered) | `libfolder.c` |
| D22 | Not Now continues with an empty library | `libsetup.c` |
| D23 | One alert reports files the scan skipped | `libsetup.c` |
| D25 | A song's title may not be blank | `songdialog.c` |
| D26 | Every restart (library change, rescan) returns to the main menu | `main.c` |
| D27 | WAV chunks may come in any order; unknown ones are skipped; WAVE_FORMAT_EXTENSIBLE is refused | `wav.c` |
| D28 | Import's defaults come from the WAV's INFO tags, else the file name | `import.c` |
| D31 | Duplicate audio is found by length plus a fingerprint of three 8 KB samples | `song_format.h`, `import.c` |
| D32 | PROVISIONAL: the Apple logo's shape; it shows for at least 1 s at start-up | `main.c`, `lcd_draw.c` |

Playback (N: Stage 4)

| Code | Decision | Where |
|---|---|---|
| N1 | PROVISIONAL: the main menu lists Now Playing only while something plays | `ui_lists.c`, `ui_nav.c` |
| N2 | A song row plays its list, from that song | `ui_lists.h`, `ui_nav.c` |
| N3 | A list row plays every song it leads to, from the first | `ui_lists.c` |
| N4 | At the natural end of the last song, playback stops (unless Repeat says otherwise) | `playback.c` |
| N5 | PROVISIONAL: Previous restarts the song after 3 s, else goes to the song before | `playback.c` |
| N6 | Next on the last song wraps to the first | `playback.c` |
| N7 | Play/Pause plays the highlighted row, or pauses and resumes | `ui_nav.c` |
| N8 | An empty artist or album leaves its Now Playing line blank | `lcd_draw.c` |
| N9 | PROVISIONAL: the pause icon's shape | `lcd_draw.c` |

Playlists (P: Stage 3C)

| Code | Decision | Where |
|---|---|---|
| P1 | A playlist lists songs by ID only (the title beside it is for reading) | `library.c` |
| P2 | Playlists are listed by name | `library.c` |
| P4 | A playlist is a TEXT file, readable in TeachText | `song_format.h` |
| P5 | Adding a song already in the playlist asks first | `playlists.c` |
| P6 | The Playlists menu has one "Add to <name>" item per playlist | `playlists.h` |
| P7 | Only song rows can be added to playlists | `ui_nav.h` |
| P8 | IDs not in the library are skipped, but kept in the file | `library.c` |
| P9 | New playlists are "Untitled Playlist" (2, 3...); names 1-31 characters, no colon | `playlists.c` |
| P10 | Any file in the Playlists folder is a playlist | `library.c` |
| P11 | Playlist changes rebuild the screens without an iPod restart (no logo) | `main.c` |

Sound (R and A: Stage 5b; G: Stage 5c)

| Code | Decision | Where |
|---|---|---|
| R1-R9 | The audio design chosen by the Stage 5a tests: one looping sampled sound over a ring filled from the main loop; R2 sizes the ring at 148,000 bytes (about 6.6 s) | `audio.c` |
| R3 | The Plus plays 22,254.5 samples/s, about 0.9% fast for 22,050 Hz songs; accepted (issue I4) | `audio.h` |
| R4 | No tiny disk reads: at least 8 KB per refill | `audio.c` |
| R7 | The main loop wakes every tick while playing, to keep the ring filled | `main.c` |
| R8 | A song whose file can't be read is skipped, with one message | `playback.c`, `audio.c` |
| A5 | Import, Export and Rescan pause the audio and resume it after | `main.c` |
| A6 | With sound, the audio position is the clock | `playback.c` |
| A8 | Without working sound, one alert, then silent playback on the clock | `main.c` |
| G1 | Gapless playback: the next song follows in the same ring | `audio.c` |
| G2 | Sound starts after about 4 s of audio is buffered | `audio.c` |
| G3 | The next song is queued as soon as the audio can take it | `playback.c` |

Import, export and libraries (E, L: Stages 5b-2, 5b-3)

| Code | Decision | Where |
|---|---|---|
| E1 | Export restores the original WAV, named as it was (or with "-n") | `export.c` |
| E4 | A file that can't be moved (locked, open elsewhere) is offered as a copy instead | `import.c`, `fileops.c` |
| E5 | A chunk left by an earlier import is replaced, not doubled | `import.c`, `wav.c` |
| E7 | A song that goes from the library while playing: playback moves on | `main.c` |
| L1 | Choose Library: Find Existing (the default) or Create New | `libsetup.c` |
| L3 | Create New inside a library asks whether to use that library | `libfolder.c` |

The click wheel (W: Stage 5c-1)

| Code | Decision | Where |
|---|---|---|
| W1-W3 | The wheel's markings: black on a white patch, 7 px tall, MENU in hand-drawn capitals | `ipod_shell.c` |

## Stage 6: responsiveness and the background (`stage-6-responsiveness-and-background.md`)

| Code | Decision |
|---|---|
| S2 | Keep playing in the background; starvation behaves as a pause |
| S4 | Interrupt-time refill dropped (the remaining starvation case is an accepted limitation) |
| S5 | After starvation, pause rather than skip |
| S6 | Keep the ring fed inside the app's own modal loops (menus, dialogs, drags) |
| S7 | A 24 KB stack (issue I10) |

## Stage 7: Now Playing (`stage-7-now-playing.md`)

| Code | Decision |
|---|---|
| N1 | Volume uses the Plus hardware's 8 levels (Stage 8e-1: one wheel turn from the quietest to full) |
| N2 | It changes the system volume; the level is remembered and saved when volume mode ends |
| N3 | One wheel turn is the whole song in the scrubber; the audio jumps once turning stops |
| N4 | The rating is kept in the song file's flags (bits 1-3), written when rating mode ends |
| N5 | PROVISIONAL: the modes time out after 2 s (volume) and 5 s (scrubber, rating) |
| N7 | The modes' look follows the 4G; exact pixels PROVISIONAL |

## Stage 8: library management (`stage-8-library-and-settings.md`, `stage-8a-multiple-libraries.md`)

| Code | Decision |
|---|---|
| L1 | Removing a song from the library means exporting it; removing from a playlist only removes the entry |
| L2 | Edit Song works from Now Playing too |
| L3 | The Import window is reused for editing; every field can be changed |
| L4 | After an edit, stay on the same screens where possible |
| L7 | Remove from Playlist, in the Playlists menu |
| L11 | Editing a title rewrites it in the playlist files too |
| M1 | One active library at a time |
| M2 | A Recent Libraries list in the File menu |
| M3 | At most 500 songs; Import refuses the 501st with a message |
| M4 | Song IDs are never reused: each library keeps a "next song ID" in its marker |

## Stage 8b and Stage 9: Settings (`stage-8b-settings.md`, `stage-9-settings.md`)

| Code | Decision |
|---|---|
| S-2 | Audiobooks are never shuffled |
| S-3 | With Repeat One, Next and Previous still move to another song |
| S-5 | Reset All Settings leaves the volume alone |
| S-6 | About shows Songs as "n of 500" and Available as the library disk's free space |
| S-7 | Extras to be audited later (done: D3, E1-E5) |
| S-9 | The main menu's Backlight inverts the screen, a novelty; not saved |
| S-10 | No Backlight timer |
| S-11 | The legal statement names the project MacMusic |
| T1 | Defaults: Shuffle Off, Repeat Off (Clicker On, kept in the prefs but unused) |
| T2 | Select steps a setting's value in place, saved at once |
| T3 | With Shuffle on, the chosen song plays first |
| T4 | A change to Shuffle applies to the next list played |
| T5 | Repeat All with Shuffle reshuffles each time round |
| T6 | Legal is the statement as plain text in Geneva 9 |
| T7 | A change to Repeat during playback may take effect one song later (the next song is already queued) |
| (9e) | The Clicker was dropped (2026-10-07) |

## The rename (`rename-macmusic.md`)

| Code | Decision |
|---|---|
| R2 | The project folder and repository keep the name MacMusicPlayer |
| R5 | Existing library markers keep their text |
| R6 | The old "MacMusicPlayer Prefs" is renamed, never copied or deleted |

## Stage 10: fast-forward, rewind and the status icons (`stage-10-fast-forward.md`)

| Code | Decision |
|---|---|
| F1 | PROVISIONAL: a hold is half a second (30 ticks) |
| F2 | Option-right and Option-left are held Next and Previous |
| F3 | The song plays on while held; one jump on release |
| F4 | 4x for the first 2 s held, then 16x, worked out from the time held |
| F5 | Fast-forward stops at the song's end (the next plays on); rewind at 0:00 |
| F6 | Holds work on Now Playing only |
| F7 | Paused stays paused, at the new position |
| G1 | PROVISIONAL: the shuffle and repeat icons at the top right, below the battery |
| G2 | The shuffle icon shows whenever the playing songs are shuffled |
| G3 | Shuffle by Albums shows the same icon |
| G4 | Repeat One and Repeat All show the same loop icon (reversed in review) |

## Fixes and 1.0

| Code | Decision | Where |
|---|---|---|
| K1 | Fix 1: the library marker and prefs keep the Finder's generic icons | `finder-icons.r` |
| K2 | Fix 1: the "has bundle" flag is set after each build by `tools/set-bundle-bit.py` | `CMakeLists.txt` |
| N1 (Fix 2) | An old "iPod Library Info" marker is renamed "MacMusic Library Info" | `libfolder.c` |
| V1-V9 | 1.0: the version, numbering, tests, branches, backup, licence, real-Plus check, release files, user guide | `release-1.0.md` |
| H1 | 1.0: the partition is 640 KB preferred, 512 KB minimum (measured) | `MacMusic.r` |
| H5 | 1.0: accessors only the tests use are marked as such | `audio.h`, `library.h`, `ui_nav.h` |
| E1 | The Clock shows the time and date in the Mac's own format | `ui_nav.c` |
| E2 | The abbreviated date, or the short one if it doesn't fit | `ui_nav.c` |
| E3 | Hours and minutes only, redrawn on the minute | `ui_nav.c` |
| E4 | PROVISIONAL: the Clock's layout, in the LCD's font | `lcd_layout.h` |

## 1.1 (`release-1.1.md`)

| Code | Decision | Where |
|---|---|---|
| N11-1 | The Create New Library prompt is "New library in:" | `libfolder.c` |
| N11-2 | A cache of the song details, read at start-up instead of the files | `libcache.c` |
| N11-3 | Kept in an invisible folder "MacMusic Cache" in the library, file "Song Details" (1.0 skips folders) | `libcache.c` |
| N11-4 | A file comes from the cache if its name, date and length match; the folder is always listed | `library.c` |
| N11-5 | Before Edit Song or a rating changes a song file, its cache entry is marked stale (a stopped clock can't mislead) | `library.c`, `editsong.c` |
| N11-6 | Rescan Library reads every file, and writes a new cache | `libsetup.c`, `main.c` |
| N11-7 | A damaged, unknown or unreadable cache is ignored and rewritten; a locked disk has none | `libcache.c` |
| N11-8 | PROVISIONAL: "Loading the library…" when songs are read from their files: under the logo, or across the foot of the screen at Rescan | `main.c`, `lcd_draw.c` |
| N11-9 | The cache is in memory only during a scan; short of memory, the scan is done without it | `library.c` |
| N11-10 | The cache's format: a header, then packed entries, big-endian, with a check-sum | `libcache.c` |
| N11-11 | Print Library: Songs; Albums, with their songs or not; Artists, with their albums or not | `print.c`, `printlayout.c` |
| N11-12 | PROVISIONAL: each page's header (library, list, date), column heads, "Page n of m" | `printlayout.c`, `print.c` |
| N11-13 | PROVISIONAL: Times 10 for the lists, Helvetica bold for the heads; Geneva if missing | `print.c` |
| N11-14 | The columns follow the page's width (A4, Letter, landscape) | `printlayout.c` |
| N11-15 | File > Page Setup… and Print Library… (Command-P), before Quit | `MacMusic.r`, `main.c` |
| N11-16 | The first print with no Page Setup saved opens Page Setup; it's kept in the prefs' resource fork | `print.c` |
| N11-17 | The music pauses while it prints, as for Import | `main.c` |
| N11-18 | Command-period cancels a print | `print.c` |
| N11-19 | Printing problems in plain words | `print.c` |
| N11-20 | Not enough memory to print: said, and nothing else affected | `print.c` |
| N11-21 | The printer is whichever the Chooser has: ImageWriter, LaserWriter, AppleTalk | `print.c` |
| N11-22 | MacMusic runs on a 1 MB Plus: MultiFinder for ordinary libraries, the Finder alone for the largest | `MacMusic.r` |
| N11-23 | A merge sort by title, and each song's group noted as the lists are built | `library.c` |
| N11-24 | Printing is a code segment of its own, loaded only to print | `segments.map`, `main.c` |
| N11-25 | Import Song repeats: after each song "Import another song?" (Import Another, Done), then the Open dialog again; cancelling a song skips it; disk or library full ends the run; Move or Copy asked for each song; System 6's Cancel kept | `import.c` |
| N11-26 | The library read once, after the whole run; the run keeps its own list for the duplicate warning and the limit | `importrun.c`, `main.c` |
| N11-28 | `EventAvail` three times at start-up, before the library is read, so MultiFinder brings MacMusic's window in front | `main.c` |
| N11-29 | "Loading the library…" at the start of every library read, not only once a song file is read | `library.c`, `main.c` |
| N11-30 | Built with -O2 by default (the Release build type) | `CMakeLists.txt` |
| N11-32 | Each name's place in the group lists found by binary search, then checked for an identical name among those comparing equal | `library.c` |
| — | PROVISIONAL: audiobooks print after the songs under their own heading; an album of several artists shows "Various Artists" | `printlayout.c` |

## 1.2 (`release-1.2.md`)

| Code | Decision | Where |
|---|---|---|
| N12-1 | A click between a text field's frame and its text (the 3-pixel margin the Dialog Manager ignores) is moved inside the field, so that field is selected | `fieldclick.c`, `songdialog.c` |
| N12-2 | N12-1 in the song details window only (DITL 131), whose fields stand in a column | `songdialog.c` |
| N12-4 | Song files at 22050 or 22254 Hz (the Plus's own rate, as the Converter 1.1 makes) are accepted; any other rate, 22255 included, is refused | `song_format.h`, `songfile.c`, `wav.c`, `import.c` |
| N12-5 | Each song keeps its own rate: read from its file, held in the library, used for its times and seeking, and written back when Edit Song or a rating rewrites the chunk. Playback plays every sample at the hardware rate, whatever the song's rate | `songfile.c`, `library.c`, `playback.c` |
| N12-6 | The cache becomes version 2, with each song's rate in the entry's spare four bytes. 1.2 still reads 1.1's version 1 (all its songs 22050 Hz), so upgrading costs no scan; 1.1 ignores version 2 and reads every file | `libcache.c` |
| N12-7 | 1.0 and 1.1 skip a 22254 Hz song with their "not recognised" alert; its file is left as it is, and comes back when 1.2 opens the library | (documented, nothing changed) |
| N12-8 | Samples become ticks in two parts (SongRate_Ticks), so a song longer than about 26.8 minutes has the right times and progress bar (1.0 to 1.1 overflowed a long) | `song_format.h`, `playback.c` |
| N12-9 | As many artists, albums, genres and composers as songs (kMaxGroups = kMaxSongs, 500; was 256), so a group table can't fill and no song is left out of its group's list; 4.9 KB more globals | `library.h` |
| N12-10 | The release floppy also carries "Sample Song", a MacMusic WAV to try importing (28 s of Vangelis's "Chariots of Fire" at 22,254 Hz), taken from the private assets, never the repository; without it, the floppy is as before | `tools/make-release.sh` |
| N12-11 | A filter's artists or albums are found in one pass over the songs (each group marked), not by checking every group against every song: the same lists; printing the Albums and Artists lists for a library of 376 songs, 272 albums and 204 artists looks at 256,000 songs, not 62 million | `library.c` |
| N12-12 | While the pages are prepared (spooled, before the printer's own box), a box says "Preparing the pages to print..." and "To cancel, press Command-period."; Command-period, checked before each list and page, stops the job as Cancel does | `print.c` |
| N12-3 | PROVISIONAL: Albums > All lists its songs by album (D14 order, no album last), then track (D13), then title; Music > Songs and Composers stay by title. Playing from the list follows it (N3) | `library.c`, `ui_lists.c` |

## *MacMusic Utilities*, the companion manual (`manual-utilities.md`)

| Code | Decision | Where |
|---|---|---|
| UB1 | For the owner's own use only, with the USB drive and its contents | the plan |
| UB2 | The reader's world is a modern Mac and the Plus; the User's Guide's voice, no 1987 pretence | `docs/utilities-manual/style-addendum.md` |
| UB3 | One series with the User's Guide: its design and tools | the style addendum |
| UB4 | The owner's screenshots, cropped to the window, in greyscale; originals kept private | the style addendum |
| UB5 | Getting Started, then a chapter per program; a part per program if a chapter grows too big | `docs/utilities-manual/outline.md` |
| UB6 | The Backup Utility's chapter waits for the program, which gets its own plan | the plan |
| UB7 | Every statement checked against the program; CHECK marks what isn't seen | the plan |
| UB8 | PROVISIONAL: the USB drive's contents | the plan |
| UB9 | Where the book's files live | the plan |
| UB10 | The phases | the plan |

The cover (UC: *MacMusic Utilities*, Phase 8)

| Code | Decision | Where |
|---|---|---|
| UC1 | A real Mini vMac screen: the owner's screenshot of a MacMusic Utilities disk holding three stand-in programs (props, kept outside the repository) | `tools/utilities-manual/make-cover.py` |
| UC2 | The utilities' 1-bit icons: a compact Macintosh with a note, the double arrow, a clock | the stand-ins (scratchpad) |
| UC3 | The icons named in full: MacMusic Converter Utility, MacMusic Transfer Utility, Macintosh Backup Utility | the stand-ins |
| UC4 | The own edition's cover: the User's Guide's MacDraw composition, titled *MacMusic Utilities*, in the private assets only | `tools/utilities-manual/make-cover.py` |
| UC5 | The plain edition's cover: MacMusic's icon, the title and the screen, as the User's Guide's | `make-book.py` (Phase 8) |
