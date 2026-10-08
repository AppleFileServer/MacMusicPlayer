# MacMusic 1.0 user guide

MacMusic turns a Macintosh Plus into a 2004 iPod: a window with the
iPod's screen and click wheel, its menus, and its Now Playing screen.
Your songs live in a **MacMusic Library**, a folder on any disk.

## Getting started

### What you need

- A Macintosh Plus with System 6.0.7 (or Mini vMac emulating one). Under
  MultiFinder, MacMusic asks for 640K of memory and runs in 512K.
- Songs as WAV files: **22,050 Hz, 8-bit, mono**. Other WAV formats are
  refused with a message saying what's needed. On a modern Mac,
  `afconvert -f WAVE -d UI8@22050 -c 1 --mix song.mp3 song.wav` makes one.

### The first time

Double-click MacMusic. After the Apple logo, it asks for a library:

- **Create New...**: choose where, and a name ("MacMusic Library" is
  offered). MacMusic makes the folder, with a **Playlists** folder and a
  file called **MacMusic Library Info** inside.
- **Find Existing...**: open the "MacMusic Library Info" file (or any
  song or playlist) inside a library you already have.
- **Not Now**: carry on with no library.

MacMusic remembers the library and opens it next time. Its preferences
are in "MacMusic Prefs" in the System Folder.

## The iPod

### The controls

| On the iPod | Mouse | Keyboard |
|---|---|---|
| Turn the wheel (scroll, volume) | drag round the wheel | up and down arrows |
| Select (centre button) | click the centre | Return or Enter |
| Menu (back) | click MENU at the top | Delete |
| Play/Pause | click the bottom | Space |
| Next / Previous | click the right / left | right / left arrows |
| Hold Next / Previous (fast-forward, rewind) | press and hold the right / left | Option-right / Option-left, held |

A hold starts after half a second without moving. Holds only work on Now
Playing; elsewhere a held button is just a press.

### The menus

- **Music:** Playlists, Artists, Albums, Songs, Genres, Composers,
  Audiobooks. Artists, Albums and Genres start with an **All** row.
  Songs are listed by title (ignoring a leading "The "); an album's songs
  are in track order. Audiobooks appear only under Audiobooks.
- **Extras:** the **Clock**, showing the Mac's date and time.
- **Settings:** About, Shuffle, Repeat, Legal, Reset All Settings.
- **Shuffle Songs:** plays every song (not audiobooks) in random order.
- **Backlight:** turns the screen negative, and back. Just for fun.
- **Now Playing:** appears at the end while something is playing.

Select on a song plays it, followed by the rest of that list. Play/Pause
on a list row (an artist, an album, a playlist) plays everything it
leads to. Next on the last song goes back to the first.

### Now Playing

Shows where the song is in the list ("3 of 12"), its title, artist and
album, a progress bar, and the elapsed and remaining time. Shuffle and
Repeat icons appear at the top right when they're on.

- **Turn the wheel** to change the volume. One full turn goes from the
  quietest level to the loudest; the arrow keys step one level. The
  level is remembered.
- **Select** steps through:
  1. **The scrubber:** turn the wheel to move through the song (one turn
     is the whole song). It jumps there when you stop turning.
  2. **The rating:** turn the wheel for one to five stars. Ratings are
     saved in the song file. MacMusic doesn't use them for anything
     yet.
  3. Back to the progress bar.

  The volume bar goes after 2 seconds, the scrubber and the rating after
  5.
- **Hold Next or Previous** to fast-forward or rewind: four times normal
  speed at first, sixteen after two seconds. The music keeps playing and
  carries on from the new place when you let go.
- **Previous** goes to the song before, or restarts the song once more
  than 3 seconds of it have played.

### Settings

- **About:** Songs ("n of 500"), Available (free space on the library's
  disk) and Version.
- **Shuffle:** Off, Songs (the list in random order, the chosen song
  first) or Albums (whole albums in random order, each in track order).
  Audiobooks are never shuffled.
- **Repeat:** Off, One (the same song again) or All (back to the start;
  reshuffled each time round if shuffling). Next and Previous still move
  to another song with Repeat One.
- **Legal:** the legal statement.
- **Reset All Settings:** Cancel or Reset. Shuffle and Repeat go back to
  Off; the volume is left as it is.

Settings are remembered. Shuffle applies to the next list you play;
a change to Repeat may wait until the song already lined up has
started.

## Managing the library

All of this is in the Mac's menu bar, above the iPod.

### Songs

- **File > Import Song...:** choose a WAV file. A window shows the
  details (title, artist, album, genre, composer, track, audiobook),
  filled in from the file where it can; change any you like, then
  Import. A file on the library's disk is **moved** into the library
  (Just Copy leaves the original where it is); from another disk it is
  copied, and the original deleted after the copy. Importing audio
  that's already in the library asks first.
- **Edit > Edit Song...:** the same window for the highlighted song, or
  the one playing on Now Playing. Saving keeps you on the same screen.
- **File > Export Song...:** gives the highlighted song back as its
  original WAV file, exactly as it was imported, and **removes it from the
  library** and from every playlist. This is how to remove a song.

A library holds at most **500 songs** (audiobooks included). Import
refuses a song when it's full.

### Playlists

- **Playlists > New Playlist...:** a name ("Untitled Playlist" is
  offered). At most **10 playlists**.
- **Playlists > Add to <name>:** adds the highlighted song (it asks if
  the song is already there).
- **Playlists > Remove from Playlist:** takes the highlighted entry out
  of the playlist you're in. The song stays in the library.
- **Rename Playlist...** and **Delete Playlist...** act on the playlist
  highlighted or shown. Deleting removes only the playlist, never its
  songs.

Playlists are text files in the library's Playlists folder, one song per
line. TeachText can open them.

### Libraries

- **File > Choose Library...:** Find Existing or Create New, as at the
  first start.
- The libraries you've used are listed in the **File** menu; choose one
  to switch. One on a disk that isn't there is dimmed.
- **File > Rescan Library:** reads the library folder again, for
  example after changing it in the Finder.
- **File > Library Info...:** the number of songs and playlists, the
  songs' total size, the disk's free space, and any unreadable files.

Only MacMusic should change a library's files. Song files copied in from
another library may clash: two songs with the same ID can't both be
used, and the scan reports the one it skipped.

## Living with other programs

MacMusic keeps playing in the background under MultiFinder, and while
menus, dialogs and desk accessories are open. If another program keeps
the Mac completely busy for more than about 6 seconds, you may hear a
moment repeat, then a pause, before the music carries on by itself.

## Limits and known issues

- **8-bit mono sound at 22 kHz,** the Plus's own sound.
- **Songs play about 0.9% fast,** because the Plus's sound hardware
  runs at 22,254 samples a second. The times shown are right.
- **The quietest volume isn't silent.** It's the Plus's lowest
  hardware level.
- **A large library is slow to scan:** about 1.5 to 2 minutes for 500
  songs, at every start and Rescan.
- **Extras has only the Clock.** The iPod's other extras (contacts,
  calendar, notes, games) are not included.
- **Ratings are recorded but not yet used** (for example, for smart
  playlists).
- **Tested on the Macintosh Plus and Mini vMac only.** If MacMusic can't
  use the sound on another Mac, it says so and plays silently.

## Legal

MacMusic is an independent project, not affiliated with, endorsed by,
sponsored by or produced by Apple Inc. Copyright © 2026 Jack McAteer. The
full statement is under Settings > Legal.
