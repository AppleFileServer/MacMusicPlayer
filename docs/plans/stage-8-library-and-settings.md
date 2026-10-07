# Stage 8 plan: library management and settings

Status: approved (2026-10-07), decisions L1-L11 made. 8a decided:
Option A (`stage-8a-multiple-libraries.md`, M1-M4).
No code has been changed.

Replaces the earlier Stage 8 (On-The-Go), now deferred
(`deferred-on-the-go.md`). Multiple libraries and metadata editing can
change how playlists and playback refer to songs, so they are settled
first.

## Aims

- Settle how many libraries the app works with (8a) and what the iPod's
  Settings menu will contain (8b), as documents, before more code
  depends on either.
- Let a song's details be revised after import (8c).
- Give the library proper Mac-side administration (8d).

## Principles

- **The iPod UI is for playing and browsing; the Mac menu bar is for
  administration.** Editing, choosing and rescanning the library, and
  library information are Mac menus and dialogs, never screens on the
  simulated iPod.
- **Removing versus deleting a song (L1).** This is how songs move in and
  out of the library. It is the route established in 5b-2, so it needs no
  change in behaviour or code.
  - **Removing a song from the library** means exporting it (File →
    Export Song…). The app strips the iPod metadata, which restores the
    original WAV byte for byte, and moves the file out of the library to
    a folder chosen in the Finder. The song's playlist entries go with
    it. To another disk the move is a verified copy, after which the
    library's file is removed: the song still exists, as the exported
    WAV.
  - **Deleting a song** means removing the file itself from the disk.
    **MacMusicPlayer never does this.** The user exports the song, then
    drags the exported, metadata-stripped WAV to the Wastebasket.
  - **Removing a song from a playlist** removes only that playlist's
    entry; the song stays in the library (8d).
- Small, testable steps, each runnable. Undocumented iPod behaviour is
  marked PROVISIONAL or DECISION in the code. Nothing is committed until
  approved.

## Where the app is now

- **Menus:**
  - **File:** Import Song…, Export Song…, Choose Library…, Quit.
  - **Edit:** the standard items, used only by desk accessories.
  - **Playlists:** New Playlist…, then the playlists; choosing one adds
    the highlighted song to it.
- **Library:** one at a time, remembered in the prefs; Choose Library…
  creates one or picks an existing one (5b-3). Song IDs are unique within
  a library: a new song gets one more than the largest ID found by the
  scan.
- **Scanning:** at startup and after import, export or a library change.
  Each of these is an iPod "restart" (logo, then the main menu, D26),
  because a rescan invalidates every list position. Unreadable files are
  skipped, and an alert gives their number (D23).
- **Song details** are in each song file's "iPod" chunk at the end of the
  file:
  - title, artist, album, genre and composer, each up to 63 characters;
  - the track number;
  - the audiobook flag;
  - the rating (7d).

  They are set in the Import Song window and can't be changed afterwards.
- **Playlist files** have one line per song: its ID, a tab and its title.
  Membership is by ID only (P1); the title is there so the file reads
  sensibly in TeachText.
- The iPod's **Settings** item is a stub (D3).

## Order (L10)

1. **8a**, multiple libraries: investigation (a document).
2. **8b**, Settings menu audit (a document).
3. **8c**, editing a song's details (code).
4. **8d**, library administration (code).

8a and 8b are committed when approved. 8c and 8d are each built,
tested (diagnostics, then manual in Mini vMac at 1×) and committed
separately. 8c comes before 8d because 8d's Rescan reuses 8c's
stay-on-screen rebuild.

## 8a: multiple libraries: design investigation (no code)

A written comparison, decided before any implementation (L8).

**Questions for each option:**
- **Song references:** stable song IDs, playlists (which list IDs) and
  playback queues (IDs).
- **Interactions:** metadata editing (8c), rescanning, the prefs, and
  import and export.
- **The Plus's limits:** memory (500 songs in the library's tables now),
  scan time at Plus speed, and HFS volumes of up to 2 GB under System 6.
- **Missing disks:** what the user sees when a library's disk isn't
  present.

**Options:**
- **A, one active library at a time:** libraries can live on different
  disks or folders, and the iPod sees only the active one. This is
  closest to the current design, since Choose Library already switches.
  The likely work is remembering the known libraries and switching
  quickly. **Your current leaning.**
- **B, several libraries shown as one:**
  - every song needs a reference to its library as well as its ID, since
    IDs are only unique within one library;
  - playlists would have to name the library for each entry;
  - missing disks, duplicates across libraries and longer scans all need
    answers.

  Considerably more complex.
- **C, one library:** keep the current design and accept one volume, up
  to 2 GB, as the practical limit.

**Output:** a short document comparing A, B and C, with a
recommendation, for your decision. If the choice changes anything 8c or
8d depend on, this plan is revised before they start.

**Result (2026-10-07):** Option A, one active library at a time, in
`stage-8a-multiple-libraries.md`. 8c is unaffected. 8d gains a Recent
Libraries list (M2), the 500-song check (M3) and a song ID counter that
never reuses IDs (M4).

## 8b: Settings menu audit (no code)

1. List every Settings item of the 4G (iPod software 3.x), from Apple's
   Click Wheel user guide.
2. For each item, record:
   - what the 4G did;
   - whether it makes sense on a Mac Plus;
   - whether it's technically possible here;
   - the verdict: replicate faithfully, adapt, omit, or defer.
3. Produce a definitive **MacMusicPlayer Settings specification**:
   - the items, their order and values;
   - where each value is stored: in the prefs, or with the library for
     library-specific ones (following 8a's decision);
   - the settings that already exist elsewhere, such as the volume (7b)
     and Shuffle Songs.

The specification is for your approval (L9). Building the Settings
screens is a later stage.

## 8c: editing a song's details

**The command (L2):** Edit → Edit Song…, after a divider below the
standard items. It's enabled when the highlight is on a song in any song
list, and on Now Playing, where it edits the song playing.

**The window (L3):** the Import Song window, reused, so everything that
can be set on import can be revised later:
- the fields are Title, Artist, Album, Genre, Composer and Track, plus
  the Audiobook checkbox, filled with the song's current details;
- the buttons are Save and Cancel, and Return presses Save;
- the checks are the same as on import: up to 63 characters per field,
  and a title is required.

**Saving:**
- The song file's chunk is rebuilt with the new details and written in
  place at the end of the file, with the RIFF size updated. The audio is
  never touched, and the song keeps its ID and rating.
- The change is written at once, so it survives rescans and restarts.
- As in 7d, the song's ID is checked before writing. On any error the
  file is left as it was and a message is shown.
- The playlist files' title text is updated too (L11): every playlist
  that lists the song has the title on its lines rewritten. Files that
  don't list it are left alone, as for export.

**While the song plays:** like the rating (7d), the save uses its own
path to the file. Unlike 7d it can change the file's length, so a
diagnostic must check this while the song plays before it's relied on.
If it misbehaves, the fallback is to pause the audio around the save, as
import does (A5).

**After saving, stay on the same screen if possible (L4).** The library
is rescanned and the lists rebuilt, without the restart to the main
menu, and the play queue carries on by song ID. Each screen in the
iPod's navigation is then checked, from the top:
- **A list that still exists** is rebuilt, and its highlight follows the
  same item (by name, or by ID for a song). If the item has left the
  list, for example because the song's artist changed, the highlight
  goes to the nearest row.
- **A list that no longer exists**, because its album, artist, genre or
  composer has no songs left, is closed and its parent is shown.
- **Now Playing** stays, with the new details.

This is new UI code, so a diagnostic covers each case. Restarting to the
main menu, as import does, is only a fallback if it proves unworkable.
Import, export and Choose Library keep their restart (D26).

**Tests:**
- host tests for rebuilding the chunk: every field, maximum lengths, the
  RIFF size, and an edited song exported byte-identical to the original
  WAV;
- a diagnostic editing a song while it plays, then rescanning;
- a diagnostic for the playlist title update (only the edited song's
  lines change);
- a diagnostic for each stay-on-screen case;
- manual.

## 8d: library administration

**The File menu (L5):** Import Song…, Export Song…, then a divider, then
Choose Library…, Rescan Library, Library Info… and Reveal Library in
Finder, then Quit.

**Recent Libraries (M2):** a submenu or list beside Choose Library…,
remembering the libraries used before. Each entry shows the library's
folder name and its disk. A library whose disk isn't mounted is shown but
can't be chosen. Choosing one switches to it, as Choose Library… does
(playback stops, the iPod restarts). The locations are kept in the prefs,
which gain a list; version 2 prefs files are still read. How many to keep
is decided in 8d.

**The 500-song limit (M3):**
- Import checks the number of songs first. With 500 already, it stops
  with a message before touching the file, for example: "The iPod Library
  is full (500 songs). Export a song to make room."
- The scan reports songs over the limit separately from unreadable files
  (they can be there if files were copied in from the Finder), so the
  message says which it is.

**Song IDs are never reused (M4):**
- Each library keeps a "next song ID" counter, stored with the library
  (in or beside its marker file; the exact form is decided in 8d). It
  only ever goes up.
- A new song gets the counter's value, or one more than the highest ID
  present if that is higher (for songs copied in from elsewhere).
- If the counter can't be read or written (a library from before 8d, a
  locked disk), the app falls back to the current rule (highest ID + 1)
  and creates the counter when it can.

**Rescan Library:** rescans now, which is useful after files were added
or changed in the Finder. It stays on the same screen where possible,
using 8c's rebuild.

**Library Info…:** a dialog showing:
- the numbers of songs and playlists;
- the total size of the songs;
- the free space on the library's disk;
- the files that couldn't be read: their number, and names up to a
  limit.

No path is shown (L6).

**Reveal Library in Finder (L6), feasibility first:**
- The System 6 Finder has no documented way for a program to make it
  open a folder; Apple events arrived with System 7.
- 8d starts with a short investigation of what System 6.0.7's Finder and
  MultiFinder allow.
- Nothing that relies on undocumented Finder behaviour is built without
  your approval.
- If it can't be done reliably, the item is left out and the plan comes
  back to you.

**Remove from Playlist (L7):** a Playlists menu item, enabled when the
highlight is on a song inside a playlist. It removes that one entry from
the playlist file, and the song stays in the library.

**Missing or corrupt songs:** the scan already skips unreadable files,
and playback skips songs whose files have gone (R8). 8d lists them in
Library Info. Under L1 the app never deletes or "repairs" them.

**Tests:**
- a diagnostic for Rescan, with files added and removed in between;
- a diagnostic for Library Info's counts;
- a diagnostic for Recent Libraries, including a library whose disk is
  missing;
- a diagnostic for the 500-song check (import refused at 500, nothing
  moved) and the scan's over-limit report;
- a diagnostic for ID allocation: export the highest-numbered song, then
  import: the new song gets a new ID, not the exported one's;
- a diagnostic for Remove from Playlist: only that entry goes, and the
  rest of the file is unchanged;
- manual.

## Not in Stage 8

- The On-The-Go playlist (deferred; research and draft in
  `deferred-on-the-go.md`).
- Building the Settings screens (after 8b's specification is approved).
- Using ratings in shuffle or playlists (issues log).
- Press-and-hold fast-forward and rewind (N8).

## Decisions

**Made (2026-10-07):**
- **L1:** removing a song means exporting it (metadata stripped, moved to
  the Finder). Deleting means the user drags the exported file to the
  Wastebasket; the app never deletes a song. A note only: no change in
  behaviour.
- **L2:** Edit Song… works from Now Playing too.
- **L3:** the Import Song window is reused; every field editable on
  import can be edited later.
- **L4:** after an edit, stay on the same screen if possible.
- **L5:** the library items stay in the File menu.
- **L6:** Reveal Library in Finder, not a path, subject to the
  feasibility check in 8d.
- **L7:** Remove from Playlist, in the Playlists menu.
- **L8:** investigate options A, B and C before deciding (leaning
  towards A).
- **L9:** a Settings specification, item by item, for approval.
- **L10:** the design documents first: 8a, 8b, then 8c, 8d.
- **L11:** when a title is edited, the title text in the playlist files
  that list the song is updated too (membership stays by ID).
