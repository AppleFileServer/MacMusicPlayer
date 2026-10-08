# MacMusic manual: issues noticed

A running list of problems noticed while the manual is written, for the
owner to clean up at the end (2026-10-08 on). Each entry says where it
shows, what's wrong, and what would fix it. Strike an entry through, or
delete it, once it's dealt with.

## Screenshots that don't match the text

The tutorial (Chapter 1) imports Song.wav, titled "Chariots of Fire" by
Vangelis (album "Chariots of Fire"), and makes a playlist called "Film
Themes". These pictures show something else:

| Figure | Where | Shows | Should show |
|---|---|---|---|
| c1-songs (S23) | Lesson 8 | the Songs list with other songs | Songs with only "Chariots of Fire" |
| c1-now-playing (S10) | Lesson 8 (labelled) | "Good Luck Babe", 2 of 3, Shuffle and Repeat icons on | "Chariots of Fire", Vangelis, 1 of 1; Shuffle and Repeat off |
| c1-paused (S14) | Lesson 8 | "Good Luck Babe", paused | "Chariots of Fire", paused |
| c1-import-open (S30) | Lesson 7 | tone.wav selected; an "iPod Library" folder | Song.wav selected; "MacMusic Library" |
| c1-import-info (S32) | Lesson 7 | "tone.wav (0:10)", title "tone" | "Song.wav", title "Song", its real length |
| c1-new-playlist (S36) | Lesson 9 | Add to "Pop", "Quiet" and others | no playlists yet (no Add to items) |
| c1-add-to (S40) | Lesson 9 | Add to "Pop" highlighted, among five | Add to "Film Themes", the only one |
| c1-playlist (Playlist Songs) | Introduction, Lesson 9 | "Pop", with "Wet Dream" | "Film Themes" with "Chariots of Fire" (the introduction could keep a fuller playlist, but not "Wet Dream") |
| c1-hard-disk | Lesson 2 | a MacMusic Library folder already there | only MacMusic and Song.wav, before the library exists |
| c1-import-menu, c1-quit (S29, S44) | Lessons 7, 10 | the File menu lists "iPod Library (MusicTest)" | the library's current name, "MacMusic Library" |
| c1-start | Lesson 3 | a Finder window overlapping the MacMusic window | (fine as is; optional) the window clear of other windows |

### Chapter 2

Chapter 2 isn't tied to the tutorial song, so its pictures can show any
music; these are the ones that still jar:

| Figure | Where | Shows | Better |
|---|---|---|---|
| c2-choose, c2-recent (S33, S43); c2-open-existing (S35, not printed in Chapter 2) | Using More Than One Library | the old name, "iPod Library (MusicTest)" | "MacMusic Library" |
| c2-songs, c2-playlist-songs, c2-rename, c2-delete (S23, D16, S41, S42) | Finding Music, Playing Songs, Playlists | "Wet Dream" in the lists | another song |
| c2-songs vs c2-playlist-songs | Finding Music, Playlists | "Good Luck Babe" and "Good Luck, Babe" both appear | one spelling |
| c2-background (D27) | Other Applications | the TeachText letter with a postal address (F7: left as is) | neutral text, if the book is ever shared |
| c2-da-crop (D26) | Other Applications | small at a column's width | a shot with the Calculator nearer the MacMusic window would crop tighter |

### Chapter 3

| Figure | Where | Shows | Better |
|---|---|---|---|
| c2-export-menu (D17), c1-add-to (S40) | File Menu, Playlists Menu | the library as "MacMusic Library (MusicTest)" (good); the Playlists menu with "Pop" highlighted | fine as they are; a retake with every item enabled would show the menu at its fullest |
| the six Now Playing states | MacMusic's Screens | "2 of 3" in five, "1 of 3" in the volume shot | one place in the list throughout |

## The program

- **The Edit menu and desk accessories:** Undo, Cut, Copy, Paste and
  Clear are always dimmed (the menu resource starts them disabled and
  nothing enables them when a desk accessory is in front), so a desk
  accessory can't use them, from the menu or with Command-key
  equivalents. Seen in S38; found in `src/main.c`. The manual says only
  that they're dimmed. A candidate for 1.0.1: enable them while a desk
  accessory's window is in front. (Worth logging in
  `docs/issues-found.md`.)
- **Song file names:** MacMusic names song files "IPOD" and a number
  (IPOD0001). The manual describes them only as "named by number", to
  keep "iPod" out of the text; renaming would need a library
  conversion's care, so it's for a later version, if at all.
- **The arrow keys on Now Playing:** the Up Arrow makes the music
  softer and the Down Arrow louder (they follow the click wheel's
  direction for lists). The manual describes it as it is; it may feel
  backwards. A possible change for 1.0.1, if you agree.
- **The Create New Library prompt is cut off** ("Create new MacMusic"),
  seen in c1-create (Lesson 4). Already logged in `docs/issues-found.md`
  for 1.0.1; retake c1-create after the fix.

## The layout and tools

- ~~**`build-pdf.sh` emptied `build-manual/`**~~ Fixed 2026-10-08: it
  clears only what it copies in. (The sample spread's PDF, deleted
  earlier, can be remade with `make-sample.py`.)
- **Dialog boxes on task pages:** at a column's width (101pt) a dialog
  box can't be read, so Chapter 2's task columns show only MacMusic's
  screens and menus; the dialogs and alerts are pictured in Appendix A.
  If you'd like some on the task pages, a task could take two columns.
- ~~**Figure sources on the Desktop.**~~ Done 2026-10-08: the 28
  Desktop screenshots moved to `~/MacPlus/Screenshots/`, `figures.txt`
  updated, the figures rebuilt identical.
- **Lesson 5's labels:** "Close box" sits just under the lesson's
  heading in the margin; fine, but tight if the heading grows.

## The text

- ~~**The sound output port (MU6, MU6a), to check.**~~ Confirmed by
  the owner 2026-10-08: a plug in the port silences the built-in
  speaker; a mono 3.5 mm miniplug, with an adaptor for stereo inputs.
