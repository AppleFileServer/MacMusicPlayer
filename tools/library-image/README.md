# library-image: a MacMusic library and BlueSCSI disk image from a music folder

Builds a MacMusic Library of real songs on this Mac, and a BlueSCSI hard disk
image holding MacMusic and that library, for testing on a real Macintosh
Plus. First used on 2026-10-07 to make the 307-song
`HD50_512 Scratch 1980 MB.hda` (whose library, made before Fix 2, is
still called "iPod Library").

## What it does

`library_image.py STEP --work DIR ...` runs one step, or `all` of them in
order. Each step reads the previous one's files in the work folder, so a
step can be re-run on its own (for example `songs` after editing
`pick.json`).

| Step | Makes | How |
|---|---|---|
| `pick` | `pick.json`, `song-list.txt` | Reads every song's length and tags (Spotlight's `mdls`; `afinfo` for a missing length). Keeps those shorter than `--max-seconds` (default 300). Tidies tags left by video downloads (below). Gives each artist one spelling. With `--count N`, takes N songs spread evenly across the artists, A to Z. Lists the songs in artist, album and track order: song IDs are their places in this list. |
| `convert` | `wav/NNN.wav` | `afconvert` to 8-bit unsigned, mono (both channels mixed) WAV at `--rate`: 22,254 Hz by default, the Plus's own rate, which MacMusic 1.2 plays at true speed; `--rate 22050` for a library MacMusic 1.0 and 1.1 can read. |
| `songs` | `songs/IPODnnnn` | MacMusic song files: each WAV with the "iPod" chunk added at its end, exactly as Import writes it (`src/song_format.h`, `SongFile_AddChunk`). Text is converted to Mac Roman and cut to 63 characters. |
| `check` | (a report) | Builds `check/check.c` with MacMusic's own `library.c` and `songfile.c` for this Mac, over the host tests' stand-in Toolbox (`tests/host`), and runs the library scan over the song files. All must load, none be skipped, and each read back with its ID. |
| `image` | `volume.dsk`, `--out` | An HFS volume (`hfsutils`) with `MacMusic:MacMusic` and `MacMusic:MacMusic Library`: the songs (type `iPdS`, creator `iPdM`), the "MacMusic Library Info" marker (with the next song ID) and an empty Playlists folder. Then the `.hda`: the partition map and SCSI driver from `--template`, resized, followed by the volume. |

**Tag tidying** (`pick`; the music files themselves are never changed):
- **Artist:** drop " - Topic" (YouTube's Topic channels) and a "VEVO"
  ending.
- **Title:** drop a repeated "Artist - " at the start, a " | Artist" at
  the end, and "Official Audio", "Official Music Video" or "Official
  Visualiser" endings, with or without brackets.
- **Album:** blank for "Unknown Album", or when a video tag only repeated
  the title. Real albums named after a song are kept.

Check `song-list.txt` (tidied entries are marked) before converting.

## Example: the 307-song Scratch image

```
tools/library-image/library_image.py all \
    --work ~/MacPlusProjects/library-work \
    --music ~/Music/Music/Media.localized/Music \
    --template ~/MacPlusProjects/BlueSCSI-backups/Scratch.before-0692d4f.hda \
    --out "~/MacPlusProjects/BlueSCSI-backups/HD50_512 Scratch 1980 MB.hda"
```

Build MacMusic first (`cmake --build build`): the image takes
`build/MacMusic.bin` unless `--app` says otherwise.

- **Partial libraries:** add `--count 100` for 100 songs spread across
  the artists.
- **Image size:** `--size-mib` sets the image size (default 1980). System
  6 HFS volumes can be at most 2 GB.
- **Volume name:** `--volume-name` sets it (default "Scratch").

## The template

The Mac Plus needs a SCSI driver in the disk's partition map to use the
disk. The `image` step doesn't make one. It takes the first 96 blocks of
an existing BlueSCSI image that works on the Plus:
- the driver descriptor;
- the partition map;
- an `Apple_Driver43` driver;

with an `Apple_HFS` partition starting at block 96. It sets the disk's
size and makes that partition run to the end.

The driver isn't kept in this project. Use an image you already have,
such as a backup of the original Scratch image.

## Notes

- **The 500-song limit:** MacMusic holds at most 500 songs; `pick` stops
  beyond that.
- **On the Plus:** the first time MacMusic runs it may not find the new
  library. Its prefs remember the old volume by name and creation date.
  Choose Library… → Find Existing, and open "MacMusic Library Info".
- **Copying to the card:** a BlueSCSI image's name gives its SCSI ID
  (`HD50`: ID 5). Remove any other image with the same ID before copying
  the new one to the card.
