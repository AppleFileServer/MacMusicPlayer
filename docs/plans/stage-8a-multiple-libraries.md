# Stage 8a: multiple libraries: design investigation

Status: **decided (2026-10-07): Option A.** A document only; no code
changed. Part of Stage 8 (`stage-8-library-and-settings.md`, decision
L8). Decisions M1-M4 are at the end; the follow-up work is in 8d.

## The question

Should MacMusicPlayer work with more than one iPod Library, and if so,
how? The options are:
- **A:** one active library at a time;
- **B:** several libraries shown as one;
- **C:** one library, with its volume as the limit.

## How the app identifies and uses a library today

| What | How it works now | Where |
|---|---|---|
| Which library | Prefs hold one location: volume name, volume creation date and folder ID. Both name and date must match a mounted volume, so renaming the disk loses it (the app then asks again) | `prefs.c`, `libfolder.c` |
| Recognising one | The folder holds the marker "iPod Library Info" (5b-3) | `libfolder.c` |
| Switching | File → Choose Library… stops playback and restarts the iPod on the new library | `main.c` |
| Song identity | A song ID, unique **only within its library**: a new song gets one more than the largest ID the scan found | `library.c`, `import.c` |
| Duplicates | Found by audio length and fingerprint, within the library (D31) | `library.c` |
| Playlists | Files in the library's Playlists folder; one line per song: ID, tab, title (P1) | `library.c` |
| Play queue | Song IDs, re-found after every rescan | `playback.c` |
| Ratings, details | In each song file (7d, 8c) | `songfile.c` |
| Settings | The prefs: the library location and the volume (7b) | `prefs.c` |
| Limits | 500 songs, 256 each of artists, albums, genres and composers, 64 playlists | `library.h` |

So **everything except the prefs lives inside the library folder.** A
library is self-contained: its songs, playlists and ratings travel with
it, and nothing outside it refers to its songs.

## The practical limits on a Plus

- **Song size:** 8-bit mono at 22,050 Hz is about 1.3 MB a minute, so a
  4-minute song is about 5.3 MB.
- **Volume size:** HFS under System 6 is limited to 2 GB per volume. A
  full 2 GB library holds about **380 four-minute songs** (about 25
  hours), below the app's 500-song limit.
- **Your BlueSCSI images today:** 20 to 40 MB each. The Scratch image
  (30 MB, 19 MB free) holds about 3 more songs. In practice the limit is
  the size of the disk image you create, not the app or HFS. BlueSCSI
  can serve larger images, up to System 6's 2 GB per volume.
- **Memory:** in the 512 KB partition, the ring (148 KB) and stack
  (24 KB) are fixed. The library's tables for 500 songs need roughly
  80 KB (song records, about 28 KB, plus the names). There is room for
  one full library, but not for several kept in memory at once without a
  larger partition.
- **Scan time:** each song costs a file open, three small reads and a
  close, all at Plus disk speed. Several libraries scanned together would
  multiply the startup time and every rescan.
- **Disks are mounted at startup.** On System 6, SCSI disks that weren't
  there at boot aren't mounted later without the user's intervention, so
  a library's disk is either present for the whole session or not.

## Option A: one active library at a time

The iPod sees one library. Others can exist on other disks or in other
folders, and the user switches between them.

- **What already exists:** this is how the app works now. You can create
  libraries on several disks, and Choose Library… switches, with a
  restart.
- **What A would add (optional):** remembering the libraries used before,
  so switching is a menu choice rather than finding the folder again:
  - **"Recent Libraries":** a short list in the File menu or the Choose
    Library dialog, each entry with its name and disk;
  - **missing disks:** a library whose disk isn't mounted is shown but
    can't be chosen;
  - **storage:** each entry is a location, as stored now (name, creation
    date, folder ID), kept in the prefs;
  - **the prefs format:** gains a list of locations; version 2 files are
    still read.
- **Effect on everything else:** none. IDs, playlists, the play queue,
  ratings, editing (8c), rescans and import/export are unchanged, because
  each library is self-contained. Switching stops playback, as now.
- **Cost:** small, and only if the convenience list is wanted. It could
  go in 8d (library administration) or later.
- **Risk:** low.

## Option B: several libraries shown as one

The app scans several library folders and shows their songs together in
Songs, Artists, Albums and so on.

- **Song references:** a song ID is no longer enough, because two
  libraries can both have song 12. Every reference becomes (library, ID):
  - the play queue;
  - the UI's lists;
  - "Library_SongForID";
  - the playlists.

  Each library would need its own permanent identity, for example a
  number stored in its marker file, because volume reference numbers
  change between boots.
- **Playlists:** a playlist holding songs from several libraries would
  need the library on each line, which changes the playlist file format.
  It also needs a home: which library is it stored in? A playlist kept in
  one library would point into another, so the libraries would no longer
  be self-contained.
- **A missing disk:** its songs disappear from every list and from
  playlists that span libraries, then reappear when the disk is back.
  Playlists must keep those entries rather than lose them. The UI needs a
  rule for showing them, or not.
- **Duplicates:** the same song in two libraries appears twice; import
  can only check the library it imports into. Option B would need a
  policy here.
- **Import:** needs a choice of library to import into, every time.
- **Memory and limits:** the 500-song limit would have to rise. Every
  library's records would be in memory at once, so the partition would
  probably have to grow beyond 512 KB, which depends on how much memory
  your Plus has.
- **Scan time:** all libraries are scanned at startup and on every
  rescan, at Plus disk speed.
- **Effect on Stage 8:**
  - 8c (editing) and 8d (Library Info, Rescan, Remove from Playlist)
    would all have to handle (library, ID);
  - L4's stay-on-screen rebuild gets harder;
  - the plan would need revising before 8c.
- **Cost:** large. Touches the library, playback, playlists, prefs, UI
  lists, import and export.
- **Risk:** high. It changes the file formats that existing libraries
  use (playlists, the marker) and the app's core identity rules.

## Option C: one library, with the volume as its limit

- Keep the current design and accept one volume as the practical limit:
  up to about 380 four-minute songs on a 2 GB volume.
- **Effect:** none; no work.
- **Difference from A:** in practice, very little. The app already lets
  you switch libraries, so C is A without the convenience list. C just
  means not adding it.

## Comparison

| | A, one at a time | B, combined | C, single |
|---|---|---|---|
| Works today | Yes (switching via Choose Library…) | No | Yes |
| New work | Small, optional (recent list) | Large | None |
| File format changes | Prefs only (if the list is added) | Playlists, marker, prefs | None |
| Libraries self-contained | Yes | No | Yes |
| Songs available at once | One library (about 380 per 2 GB disk) | All mounted libraries | One library |
| Memory | As now | Grows with total songs | As now |
| Startup and rescan time | One library | All libraries | One library |
| Effect on 8c and 8d | None | Every reference changes | None |
| Risk | Low | High | None |

## Recommendation

**Option A, one library at a time**, which matches your leaning.

- It is what the app already does, so it costs nothing in risk, and every
  library stays self-contained.
- The only question is whether switching should be more convenient (M2).
- B's benefit is having more songs available at once than one disk
  holds (about 380 on a 2 GB volume). That would need a higher song limit
  and more memory than the current partition, and it changes the core of
  the app and its file formats. The disk sizes you use today don't need
  it.
- If B is ever wanted, A doesn't stand in its way. A's location list is
  the first step B would need anyway.

Under A, 8c and 8d proceed as planned, and 8b's Settings specification
stores app-wide settings in the prefs, with anything library-specific in
the library folder.

## Points clarified before deciding

- **Libraries can have different names, on the same volume.** Create New
  uses the folder name typed in its Save dialog ("iPod Library" is only
  the default, D21), so for example "Main Library" and "Back Catalog" can
  sit side by side. Each has its own marker, playlists, ratings and IDs.
  The prefs remember the folder by its folder ID, so renaming it in the
  Finder is safe. A library can't be created inside another (L3).
- **Moving a song between libraries** means exporting it from one and
  importing it into the other. Export restores the original WAV, so the
  song's iPod details and rating don't go with it. A "Move to Library…"
  command that keeps them is noted as a possible later item (issues log).
- **The 500-song limit had a gap:** import didn't check the count, so a
  501st song was moved in, then skipped by the scan and reported as an
  unreadable file. Fixed in 8d (M3).
- **Song IDs and slots are separate:** the limit counts the song files
  present, so exporting a song frees its slot at once, and an ID (a
  32-bit number) costs nothing. Before 8d, a new song got one more than
  the highest ID present, so exporting the highest-numbered song let its
  ID be given to the next import (M4).

## Decisions (2026-10-07)

- **M1:** Option A, one active library at a time.
- **M2:** add a "Recent Libraries" list for quicker switching, in 8d,
  beside Choose Library…. Each entry shows the library's folder name and
  disk; a library whose disk isn't mounted is shown but can't be chosen.
- **M3:** keep the 500-song limit, with import warning and stopping
  before the 501st song, so the file is never moved into a full library.
  The scan reports songs over the limit separately from unreadable files.
- **M4:** song IDs are never reused. Each library keeps a "next song ID"
  counter that only goes up, so an ID given out once is never given to
  another song, even after the first is exported. This protects any
  reference that survives an export (a playlist restored from a backup or
  edited in TeachText, or a future On-The-Go list).
