# Deferred: the On-The-Go playlist

Status: **deferred (2026-10-07), not scheduled.** Taken out of Stage 8,
which became library management and settings
(`stage-8-library-and-settings.md`): multiple libraries and metadata
editing may change how playlists and playback refer to songs, so this
waits until that is settled. The research and draft below are kept for
when it is picked up; decisions O1-O9 were never made.

## What the 4G iPod does (source)

From Apple's iPod (Click Wheel) User's Guide (June 2004, revised March
2005), "Making Playlists on iPod", p. 18:

- **Add a song:** "Highlight a song and press and hold the Select button
  until the song title flashes."
- **Add a list:** "You can also queue entire lists of songs at once. For
  example, to queue an album, highlight the album title and press and
  hold the Select button until the album title flashes."
- **View:** "Select Music > Playlists > On-The-Go to view your list of
  songs."
- **Play:** "Select Music > Playlists > On-The-Go and select a song."
- **Remove a song:** "Highlight a song in the playlist, and hold down the
  Select button until the song title flashes."
- **Clear:** "Select Music > Playlists > On-The-Go > Clear Playlist."
- **Save:** "Select Music > Playlists > On-The-Go > Save Playlist > Save
  Playlist. The first playlist is saved as 'New Playlist 1' in the
  Playlists menu. The On-The-Go playlist is cleared."
- **Computer:** with automatic transfer, the playlist goes to iTunes when
  iPod is connected (not applicable here).

Not documented, so PROVISIONAL in code: how long the hold is, how the
flash looks, where On-The-Go sits in the Playlists list, whether Clear and
Save ask for confirmation, whether a song can be added twice, and what
happens to a playing On-The-Go queue when the list changes.

## Where the app is now

- **Select** acts the moment the button goes down (mouse on the drawn
  Select button; Return/Enter on the keyboard). There is no press-and-hold
  detection, and Select can't simply be held: a hold must not also open
  or play the highlighted item.
- **Playlists:** each file in the library's Playlists folder is a
  playlist, with one line per song: its ID, a tab and the title (P1, P10).
  The Playlists screen lists them sorted by name. Selecting one shows its
  songs, and selecting a song plays the playlist from there.
- Song IDs survive rescans. Export removes a song's lines from every
  playlist (5b-2).

## Plan: three steps, each built, tested and committed separately

### 8a: press-and-hold Select (input only)
- Select acts when the button is **released**, not pressed. Held past a
  threshold (O1), it becomes a new event, "Select held", and the release
  does nothing more.
- While the button is held, the audio is kept fed (as for the wheel,
  6b-2). The hold is timed by ticks, which stay correct under MultiFinder.
- Keyboard (emulator mapping, D8): O2.
- The flash (O3): the highlighted row is inverted a few times, as the
  4G's "title flashes".
- In 8a, "Select held" only flashes the row; nothing is added yet. This
  checks that the change to Select doesn't upset any existing screen
  (lists, Now Playing modes, the scrubber's Select).
- Tests: a diagnostic for press vs hold timing, and every screen's Select
  unchanged; manual at 1x for feel.

### 8b: On-The-Go list, adding and playing
- **Storage (O4):** a text file "On-The-Go" in the library folder, in the
  same format as playlist files. It is not a song file, so the scan skips
  it without counting it. It lives with the library, survives quitting,
  and isn't in Playlists, so it can't clash with a user playlist of the
  same name.
- **Where it shows (O5):** "On-The-Go" in the Playlists list. Its
  position is provisional (O5), and it's shown even when empty (O6).
- **Adding:** "Select held" on a song adds it to the end. On a list
  (album, artist, genre, composer or playlist) it adds that list's songs, in the order the
  list shows them. Not on menu rows (Music, Settings and so on), Shuffle
  Songs, or On-The-Go itself.
- **Duplicates (O7):** a song already in On-The-Go is added again
  (a queue, as the guide's wording suggests).
- **Limit:** 1000 entries (kMaxPlaylistEntries); beyond that, adding
  stops and the row doesn't flash.
- **Playing:** selecting a song in On-The-Go plays the list from there,
  as for any playlist. The play queue is a copy made when play starts
  (as now), so later changes don't alter what is playing (O8).
- **Library changes:** export removes the song from On-The-Go too.
  Songs no longer in the library are dropped when the list is read, as
  for playlists.
- Tests: host tests for the file (append and read back); a diagnostic
  adding songs and lists, rescanning and relaunching; manual.

### 8c: removing, Clear Playlist and Save Playlist
- **Remove:** "Select held" on a song inside On-The-Go removes that line
  (that entry only, if the song is in the list twice).
- **Clear Playlist** and **Save Playlist** rows at the end of the
  On-The-Go list, shown only when it has songs. Each leads to a two-row
  confirmation screen ("Cancel" first, then the action), as the guide's
  "Save Playlist > Save Playlist" suggests (O9).
- **Save:** creates a playlist file "New Playlist n" (the lowest n not
  already used) with On-The-Go's songs, then clears On-The-Go. If it
  can't be written (disk full, 64 playlists), On-The-Go is kept and a
  message is shown.
- Tests: a diagnostic for remove, clear and save (file contents, names
  "New Playlist 1" then 2, the 64-playlist limit); manual.

### Not in Stage 8
- Using ratings in shuffle or playlists (noted in the issues log).
- Press-and-hold fast-forward and rewind (N8). 8a's hold detection is
  for Select only, though the same approach would serve later.

## Decisions needed

- **O1, hold time:** about 1 second (60 ticks)? PROVISIONAL.
- **O2, keyboard:** the Plus's System 6 doesn't report key releases by
  default, so a held Return can't be timed simply. Proposal:
  Option-Return = "Select held". (Alternative: count auto-repeats of
  Return, which depends on the Keyboard control panel's settings.)
- **O3, flash:** invert the row 3 times over about half a second?
  PROVISIONAL artwork.
- **O4, storage:** the "On-The-Go" file in the library folder, as above?
- **O5, position:** On-The-Go last in the Playlists list (believed to be
  the 4G's place, unconfirmed), or first?
- **O6, empty list:** On-The-Go always shown, even when empty?
- **O7, duplicates:** allowed (recommended), or ignored?
- **O8, while playing:** changes to On-The-Go don't affect a queue already
  playing from it (recommended, simplest), or are followed live?
- **O9, confirmation:** a "Cancel / Clear Playlist" (and "Cancel / Save
  Playlist") screen before acting?
