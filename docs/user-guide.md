# MacMusic 1.2 user guide

MacMusic turns a Macintosh Plus into a 2004 iPod: a window with the
iPod's screen and click wheel, its menus, and its Now Playing screen.
Your songs live in a **MacMusic Library**, a folder on any disk.

## Getting started

### What you need

- A Macintosh Plus with System 6.0.7 (or Mini vMac emulating one). Under
  MultiFinder, MacMusic asks for 640K of memory and runs in 512K. On a
  Plus with 1 MB, MultiFinder leaves about 590K: enough for 500 songs
  with ordinary details. For a library of 500 songs whose every detail
  is long, use the Finder alone (Set Startup, in the Special menu).
- Songs as WAV files: **8-bit, mono, at 22,254 Hz or 22,050 Hz**.
  22,254 Hz is the Plus's own rate, which the MacMusic Converter 1.1
  makes: those songs play at their true speed and pitch. Other WAV
  formats are refused with a message saying what's needed. On a modern
  Mac, `afconvert -f WAVE -d UI8@22254 -c 1 --mix song.mp3 song.wav`
  makes one.

### The first time

Double-click MacMusic. After the Apple logo, it asks for a library:

- **Create New...**: choose where, and a name ("MacMusic Library" is
  offered). MacMusic makes the folder, with a **Playlists** folder and a
  file called **MacMusic Library Info** inside.
- **Find Existing...**: open the "MacMusic Library Info" file (or any
  song or playlist) inside a library you already have.
- **Not Now**: carry on with no library.

To hear MacMusic straight away, import the **Sample Song** from the
MacMusic floppy, if yours has one: choose File > Import Song..., open
Sample Song, and click Import. It's 28 seconds of music, at the Plus's
own rate.

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
  are in track order, and **All** in an Albums list gives the songs album
  by album, each in track order. Audiobooks appear only under Audiobooks.
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

  Then MacMusic asks **Import another song?** Click **Import Another**
  to choose the next one, so several can be imported one after another,
  or **Done** when you've finished. Each song is in the library as soon
  as it's imported, so Done (or Cancel in the Open dialog) keeps every
  song imported so far. Cancel in a song's window skips just that song.
  The library is read once, at the end.
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
- **File > Rescan Library:** reads every song file in the library
  again, for example after changing it in the Finder.
- **File > Library Info...:** the number of songs and playlists, the
  songs' total size, the disk's free space, and any unreadable files.

Only MacMusic should change a library's files. Song files copied in from
another library may clash: two songs with the same ID can't both be
used, and the scan reports the one it skipped.

### Starting quickly

MacMusic keeps a note of each song's details in the library, in an
invisible folder called "MacMusic Cache", so it needn't open every song
file each time it starts. A song file that has changed, or is new, is
read again. While the library is being read, the screen says **Loading
the library…**. The first start with a library, and Rescan Library,
read every file; later starts read only the new or changed ones, and
are quicker.

The cache does no harm to MacMusic 1.0 or 1.1, which open the library as
before; starting 1.1 again after 1.2 reads every file once. If a song was
changed outside MacMusic and its details look wrong, choose Rescan
Library.

MacMusic 1.0 and 1.1 can't play songs at 22,254 Hz: they leave them out,
saying some files were not recognised. The files aren't changed, and the
songs are back the next time MacMusic 1.2 opens the library.

## Printing

- **File > Page Setup...:** the paper and its way round, for the printer
  chosen in the Chooser. MacMusic remembers it.
- **File > Print Library... (Command-P):** choose which lists to print:
  - **Songs:** every song by title, with its artist, album and length;
    the audiobooks follow, under their own heading;
  - **Albums:** each album's artist, its number of songs and their
    length; tick "with each album's songs" for its songs below it, in
    track order;
  - **Artists:** each artist's number of albums and songs; tick "with
    each artist's albums" for its albums below it.

  Then click Print, and the printer's own Print dialog appears. Each
  list starts on a new page, with the library's name, the list and the
  date at the top, and "Page 3 of 12" at the foot.
- The first time you print, Page Setup comes first, so you can choose
  the paper (A4 or US Letter).
- The music pauses while it prints, and carries on afterwards.
  **Command-period** stops printing.
- MacMusic prints on whichever printer is chosen in the Chooser: an
  ImageWriter on the printer port, or an ImageWriter or LaserWriter on
  AppleTalk. With background printing on (a LaserWriter, under
  MultiFinder), PrintMonitor prints while MacMusic plays.

## Living with other programs

MacMusic keeps playing in the background under MultiFinder, and while
menus, dialogs and desk accessories are open. If another program keeps
the Mac completely busy for more than about 6 seconds, you may hear a
moment repeat, then a pause, before the music carries on by itself.

## Limits and known issues

- **8-bit mono sound at 22 kHz,** the Plus's own sound.
- **Songs at 22,050 Hz play about 0.9% fast,** because the Plus's sound
  hardware runs at 22,254 samples a second. The times shown are right.
  Songs at 22,254 Hz, as the MacMusic Converter 1.1 makes them, play at
  their true speed.
- **The quietest volume isn't silent.** It's the Plus's lowest
  hardware level.
- **Starting takes a little while:** on a Macintosh Plus, about 13
  seconds for a library of 300 songs, and about 13 seconds again after
  importing. The first start with a library, and Rescan Library, read
  every song file and take longer.
- **The Mac's clock:** if the Plus forgets the time when it's switched
  off, its clock battery is flat. MacMusic copes, but after a song file
  has been changed outside MacMusic, choose Rescan Library.
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
