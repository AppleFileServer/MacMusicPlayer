# MacMusic 1.1: plan

Status: **revised from the owner's answers (2026-10-10); for approval.
No code changes until it's approved.** Decisions are numbered (N11-1 and
so on). Every decision is agreed; PROVISIONAL marks what's settled only
after it's seen or printed.

## What 1.1 is

A feature release under `release-1.0.md`, Part E: **1.x keeps reading
1.0's libraries, prefs and song files.** A library used by 1.1 must still
open in 1.0, and 1.0's prefs must open in 1.1.

It changes three things:

- **A. The issues log:** the cut-off prompt when creating a library.
- **B. Faster start-up:** MacMusic no longer reads every song file each
  time it opens (the issues log's second entry; known issue H1, "Fast
  scanning" on the roadmap).
- **C. Printing:** a printed directory of the library's songs, albums
  and artists, on an ImageWriter or a LaserWriter, from the Plus.

Everything else stays as 1.0. The utilities' own issues (T1, T2, B1, C1
in `~/MacPlusProjects/LOOSE-ENDS.md`) belong to their own projects and
releases, not to MacMusic 1.1.

## The owner's answers (2026-10-10)

- **Q1. Printers:** an ImageWriter on the Plus's printer port (mini-DIN 8)
  now; other printers, and printing over AppleTalk, to be possible later,
  and so built in now (N11-21).
- **Q2. Memory:** the owner's Plus has 4 MB and runs MultiFinder; but
  MacMusic must still run on a **1 MB** Plus, under MultiFinder if it
  can, and under the Finder alone if it must (N11-22).
- **Q3. Paper:** A4 by default (N11-16).
- **Q4. Other issues:** none in 1.1. The owner reviews every outstanding
  issue after this release.
- **The approach:** Parts A and B agreed, without the progress bar: a
  message that the library is loading instead (N11-8). Part C agreed,
  **using the Macintosh's standard printing, improvising as little as
  possible.** The rest agreed.

## Part 0: measure before building (diagnostics in the scratchpad only)

Throwaway builds, never in the project:

1. **Start-up time, at 1x in Mini vMac,** with the 500-song test library
   and the 307-song real one:
   - the full read as now (1.0 measured 94 s for 500);
   - listing the folder alone (one catalog call per file, no opening);
   - reading the folder's own modification date alone;
   - sorting and indexing after loading (`IndexSongs`), and 1.0's
     duplicate-ID check.

   This shows what each kind of cache would save, and whether the
   listing alone is quick enough.
2. **Printing's memory:** opening the Printing Manager and a print job
   with the ImageWriter and LaserWriter drivers (installed in the test
   System), measuring the heap at each step. Mini vMac has no printer,
   so this can't print; it measures and checks the dialogs.
3. **Can anything be printed without a printer?** For example, the
   LaserWriter driver's "PostScript file" option, for checking pages on
   this Mac. Otherwise printed pages are checked only on your printers.
4. **A 1 MB Plus (N11-22):** how much memory a 1 MB Plus leaves for
   MacMusic under MultiFinder and under the Finder alone, with System
   6.0.7, and whether 1.0's minimum (512K) fits. This needs a Mini vMac
   with 1 MB (Mini vMac's memory is chosen when it's built: a 1 MB
   variant, kept beside the 4 MB one; the environment archive is remade
   with it).

I report the numbers; the decisions below are then confirmed or changed.

## Part 0: the results (2026-10-10)

Measured in Mini vMac at 1x with LaunchAPPL (the Finder alone, 4 MB), with
throwaway builds in the scratchpad; 500 test songs, 40 artists, 90 albums.

**Start-up (a full read, 500 songs): 49 s**, made up of:

| Part | Time |
|---|---|
| Opening and reading the 500 files (`ScanSongs`) | 6-7 s |
| Building the group lists (`InsertName`) | 1 s |
| Finding each song's artist, album, genre and composer (`FindName`: a straight search through each list, for each song) | 14 s |
| Sorting the songs by title (`SortSongsByTitle`: an insertion sort, about 62,000 comparisons by the `RelString` trap) | 28 s |
| Sorting by ID | 0.03 s |

For comparison: a merge sort of the same titles took **2.7 s**; listing the
folder without opening any file, **0.1 s**; reading the folder's date, a
moment; reading a 60,000-byte file (a cache's size), a moment; 1.0's
duplicate-ID check, 0.25 s.

So **most of start-up is spent sorting and grouping in memory, not reading
the disk.** The cache (N11-2 to N11-10) saves only the 6-7 s of reading in
Mini vMac (whose disk takes no time; on the real Plus the reading may be a
larger share). **N11-23 (agreed 2026-10-10), added to Part B:** sort
the titles with a merge sort, and note each song's groups as the lists are
built, instead of searching for them afterwards. The same lists and the
same order as 1.0, in about 4 s instead of 43 s; with the cache, start-up
would take about 5 s for 500 songs.

**Printing (heap used beyond MacMusic's own 271K), the British drivers:**

| | ImageWriter | LaserWriter |
|---|---|---|
| `PrOpen` | +1.8K | +1.7K |
| `PrintDefault` | +8.3K | +9.8K |
| `PrOpenDoc` | +15.3K | +61.6K, then error -98 (no AppleTalk network in Mini vMac) |
| `PrClose` | all given back | all given back |
| Default paper | **A4** | **A4** |
| The job | spooled, then `PrPicFile` (not measured: it waits for the printer) | sent straight to the printer |

The British drivers default to A4, so N11-16's first Page Setup simply
shows A4. A LaserWriter job needs at least 62K more.

**A 1 MB Plus (Mini vMac built with 1 MB; a clean British System 6.0.7 by
the Installer's Easy Install: Macintosh Plus System Software and all the
printer software):**

| | System | Finder | Free for MacMusic |
|---|---|---|---|
| The Finder alone | 166K | | **858K** |
| MultiFinder | 271K | 160K | **593K** (the largest free block) |

MacMusic's SIZE asks for 640K and accepts 512K, so under MultiFinder on a
1 MB Plus it gets 593K. By 1.0's memory table (outstanding-issues.md, H1),
that holds 500 songs with typical names with room to spare, but only just
holds 500 songs whose every detail is 63 characters; printing to the
ImageWriter (about 16K, plus `PrPicFile`'s) fits, a LaserWriter job (62K+)
may not with a large library.

**MacMusic 1.0's code on that 1 MB Plus, under MultiFinder** (the same
measuring build, launched from the Finder): the Finder said "640K needed,
598K available" and offered to open it in the memory there was; MacMusic's
partition was then about 556K.

| 500 songs | Full read | After reading |
|---|---|---|
| Typical details (as the 4 MB runs) | 52 s, the same as with 4 MB | **about 215K to spare** (36K free in the heap, 184K more it can grow into) |
| Every detail 63 characters | stopped at the reading: **"There is not enough memory to read the MacMusic Library."** | no library |

So on a 1 MB Plus under MultiFinder, 1.0 holds a 500-song library with
ordinary names comfortably, but not one with every detail at the full 63
characters (as 1.0's own table predicted for 512K). Under the Finder alone
it has 858K, enough for either. N11-22 holds as written: MultiFinder for
ordinary libraries, the Finder alone for the largest; the User's Guide is
to say so. The cache (N11-9) must add no memory, and the faster sort
(N11-23) needs only a 1K work list.

**Printing on the 1 MB Plus, under MultiFinder** (the printer chosen in
the Chooser, as a user would; a one-page job, MacMusic's partition about
550K):

| | ImageWriter (printer port, AppleTalk off) | LaserWriter (AppleTalk on) |
|---|---|---|
| Most heap used beyond MacMusic's own, `PrOpen` to `PrCloseDoc` | **2.1K** | **3.0K** |
| After `PrClose` | all given back | all given back |
| Paper | A4 | A4 |
| The job | spooled (`PrPicFile` then prints it: not run, as it waits for the printer) | handed to **PrintMonitor** for background printing, which tried to print it and said "the LaserWriter named 'LaserWriter' could not be found" |

So printing on a 1 MB Plus costs MacMusic only a few K; the larger
figures on the 4 MB test System (15K, 62K) were its drivers using more
when more was free, or printing in the foreground. Background printing to
a LaserWriter runs PrintMonitor as a program of its own, which needs its
own room under MultiFinder: on the 1 MB Plus it found it, beside
MacMusic. Choosing the LaserWriter, the Chooser reminds the user to
"choose Page Setup and confirm the settings", as N11-16 already has
MacMusic do. `PrPicFile`'s need (the ImageWriter's printing pass) still
waits for the real ImageWriter.

## Part A: the cut-off prompt (the issues log, 2026-10-08)

- **N11-1.** File > Choose Library > Create New Library: Standard File's
  Save prompt shows about 20 characters, so "Create new MacMusic Library
  in:" is cut. "New library in:" (`libfolder.c`, `Folder_Choose`),
  with "MacMusic Library" still the suggested name. The manual's picture
  and text follow.

## Part B: faster start-up

### What 1.0 does

At each start, and at Rescan Library, `Library_Scan` lists the library
folder and opens every song file: three small reads (the WAV header, the
details at the end, the start of the sound) and a close. About 0.19 s a
song at Plus speed. Nothing is kept between runs.

### Decisions

- **N11-2. A cache of the song details**, written by MacMusic, read at
  start-up instead of the files. Each entry holds what the scan reads
  from a file now: its name, ID, length, the sound's offset and length,
  the fingerprint, track, flags and the five details (title, artist,
  album, genre, composer).
- **N11-3. Where it's kept.** 1.0 counts any unknown file in the library
  folder as "skipped" and says so, but ignores folders. So:
  **Agreed:** a folder **"MacMusic Cache"** in the library, holding one file,
  **"Song Details"** (MacMusic's creator, a type of its own). 1.0 then
  opens a 1.1 library without complaint.
  - **N11-3a.** Visible or invisible in the Finder? **Agreed:** invisible, so
    it isn't deleted or edited by hand. It's copied with the library
    either way.
- **N11-4. When the cache can be trusted.** Two levels; Part 0 decides:
  - **(a) The folder's date:** the library folder's modification date
    changes whenever a file is added, removed or renamed in it. If it's
    unchanged since the cache was written, the cache is used without
    listing the folder: start-up in a few seconds.
  - **(b) Each file's date and size:** otherwise, the folder is listed
    (no file opened), and a file whose name, modification date and size
    match its entry comes from the cache; only new or changed files are
    opened and read.

  **Agreed:** both: (a) first, then (b) when the folder's date has changed.
  A song file changed in place by another program, without its folder's
  date changing, would be missed by (a); Rescan Library (N11-6) catches
  that, and nothing but MacMusic writes song files in practice.
- **N11-5. Keeping it up to date.** MacMusic itself changes song files
  (Edit Song, ratings) and the folder (import, export). **Agreed:** the cache
  is rewritten after every scan, and at quit if anything changed since;
  a change MacMusic makes updates the entry in memory at once.
- **N11-6. Rescan Library** always reads every file, as now, and writes
  a new cache. It's the way to put anything right.
- **N11-7. When the cache isn't used:** if it's missing, the wrong
  version, damaged (a check-sum over it), for a different folder, or
  there isn't the memory to read it, MacMusic reads every file, as 1.0
  does, and writes a new one. On a locked disk, no cache is written, and
  every start is a full read, as now. A cache is never trusted over a
  file it disagrees with.
- **N11-8. Saying the library is loading (agreed: no progress bar).**
  While the library is read, at start-up and at Rescan Library,
  MacMusic's screen shows the Apple logo (at start-up) or the screen as it
  was (at Rescan), with the words **"Loading the library…"** under it in
  MacMusic's small screen font, and the watch cursor. Shown only when the
  library has to be read from its files (a full or partial read), not
  when the cache is used as it is, which takes a moment. PROVISIONAL: the
  wording and its place; the iPod shows nothing of the kind.
- **N11-9. Memory:** the cache is read into the same handles the scan
  fills, through a small buffer, so the peak stays within 1.0's (Part 0
  checks with 500 songs and long details).
- **N11-10. The format:** a header (magic, version, the folder's ID and
  modification date, the song count, a check-sum) and fixed-length
  entries with Pascal strings, big-endian, as the other MacMusic files.
  Described in `docs/` with the library's other files.
- **Agreed 2026-10-10, before building the cache (B2):**
  - **N11-4 (a) dropped:** the folder is always listed (0.1 s for 500
    songs); only (b), each file's name, date and length, is used.
  - **N11-5 revised:** the cache isn't updated in memory or written at
    quit. Before MacMusic changes a song file (Edit Song, a rating), it
    marks that song's entry stale in the cache file (one 4-byte write,
    made before the song is written), so the file is read again at the
    next scan whatever its date: a Plus whose battery has run down
    restarts its clock from the same time, and could give a changed file
    the date it had before. Dates and lengths then catch changes made
    outside MacMusic; Rescan Library reads everything.
  - **N11-7:** no check of which folder a cache belongs to: every entry
    is checked against its own file, so a library copied to another disk
    keeps its cache. The check-sum and a header written last stay.
  - **N11-9:** the cache file is held in memory during the scan only
    (about 55K for 500 songs with ordinary details). If it can't be, or
    the scan then runs short of memory, the scan is done again without
    it, so any library 1.0 can read, 1.1 can.
- **B2 as built (measured in Mini vMac at 1x, 500 songs):** first start
  (no cache) 12.3 s; a later start 7.5 s (0.1 s loading the cache, 1.4 s
  listing the folder, 6 s sorting and grouping, 3 s of it the title
  sort's calls to `RelString`); Rescan Library 12.8 s. 1.0: 49 s.

- **N11-23. Faster sorting and grouping (agreed 2026-10-10, from Part 0).**
  Most of a full read is spent in memory, not on the disk: the title sort
  (an insertion sort, 28 s for 500 songs at 1x) and the search for each
  song's groups (14 s). So: the titles sorted with a merge sort (2.7 s
  measured, with a work list of 1K), and each song's artist, album, genre
  and composer noted as the group lists are built, instead of searched
  for afterwards. The same lists in the same order as 1.0 (a host test
  compares them, title by title, with 1.0's sort on the 500-song and
  307-song libraries), with ties kept in 1.0's order.

## Part C: printing

### The standard way (agreed)

MacMusic prints through the **Printing Manager** exactly as *Inside
Macintosh* (Volume II, "The Printing Manager") and Apple's Technical
Notes describe for System 6, and nothing else:

- `PrOpen`; `PrintDefault` or `PrValidate` on the print record;
  `PrStlDialog` for Page Setup and `PrJobDialog` for Print, unchanged;
- `PrOpenDoc`, then `PrOpenPage` and `PrClosePage` for each page, drawn
  with ordinary QuickDraw calls (`DrawString`, `MoveTo`, `LineTo`) in
  the page rectangle the print record gives (`rPage`), then
  `PrCloseDoc`;
- if the job was spooled (`bJDocLoop == bSpoolLoop`), `PrPicFile` to
  print the spool file, as *Inside Macintosh* says;
- `PrError` after each call; `PrClose` at the end, so the driver is
  closed between jobs and its memory given back.

No printer-specific code, no PostScript, no changes to the driver's
dialogs, no reading or writing of the print record's private fields.

- **N11-21. Which printer:** whichever is chosen in the **Chooser**, as
  for any Macintosh program: the ImageWriter on the printer port now; an
  ImageWriter or LaserWriter over AppleTalk later, with no change to
  MacMusic. Background printing (PrintMonitor, under MultiFinder) works
  if the chosen driver offers it. Only the ImageWriter can be checked on
  real hardware now; the LaserWriter driver is tried in Mini vMac as far
  as its dialogs and memory (Part 0), and noted in the manual checklist
  as untested on a real LaserWriter.

### What's printed

- **N11-11. The lists:** three, chosen in a small MacMusic dialog (an
  ordinary Dialog Manager dialog, like MacMusic's others) before the
  standard print dialog. (Adding MacMusic's choices to the driver's own
  dialog is possible, by Technical Note 95, but changes the driver's
  dialog, which "improvise as little as possible" rules out.)
  - **Songs:** every song, by title (a leading "The" ignored, as on
    screen): Title, Artist, Album, Time.
  - **Albums:** every album, by name: Album, Artist, Songs, Time; with
    "List each album's songs" ticked, its songs below it in track order.
  - **Artists:** every artist: Artist, Albums, Songs; with "List each
    artist's albums" ticked, the albums below.

  Any one, or all three (each starting a new page). Audiobooks print in
  the lists with the rest, marked. **Also possible:** playlists (each
  playlist's songs, in order), Genres, Composers. Say if you want them.
- **N11-12. The page:** at the top, the library's name, the list's name
  and the date; at the foot, "Page 3 of 12". Columns with a thin rule
  under the headings; a long entry cut with an ellipsis to fit its
  column, never wrapped.
- **N11-13. Type:** Times 10 point for the lists and Helvetica
  bold for the headings, which both printers print well (the ImageWriter
  from the System's bitmap fonts, the LaserWriter from its own); Geneva
  if either font isn't installed. PROVISIONAL until printed on your
  printers.
- **N11-14. Paper and orientation:** whatever Page Setup says; portrait
  by default. The columns are laid out to the page's width (`rPage`), so
  A4, US Letter or landscape all work without special cases.

### The commands

- **N11-15. The File menu** gains **Page Setup…** and **Print Library…**
  (Command-P), as other Macintosh programs have them, before Quit. Both
  are dimmed with no library; Print Library is dimmed with no songs.
- **N11-16. A4 by default, and Page Setup remembered (agreed: A4).**
  The paper size belongs to the printer's driver, and the standard way to
  set it is Page Setup; a program that writes the paper size into the
  print record itself is improvising. So:
  - the first time you print (with no saved Page Setup), MacMusic opens
    Page Setup first, so A4 can be chosen once, then the print dialog;
  - the print record is then saved, as programs of the time saved it,
    and checked with `PrValidate` each time it's used, so a different
    printer chosen later is handled by its driver;
  - it's kept in the **resource fork of "MacMusic Prefs"**, as a resource
    of MacMusic's own. 1.0 reads only the prefs' data fork, so the prefs
    stay readable by 1.0, and 1.0's prefs by 1.1.

  (If your System's ImageWriter driver already defaults to A4, as a
  British System's does, that first Page Setup simply shows A4.)

### While it prints

- **N11-17. The music:** printing to an ImageWriter keeps the Plus busy
  for minutes, which would starve the sound (the accepted limitation:
  more than about 6.6 s and it repeats, then pauses). As Import
  and Export do (A5): the music pauses while it prints, and carries on
  afterwards. A LaserWriter under MultiFinder with background printing
  (PrintMonitor) spools quickly, then prints while MacMusic plays.
- **N11-18. Cancelling:** Command-period, as the driver offers; a
  cancelled job prints nothing more.
- **N11-19. Errors:** the Printing Manager's errors in MacMusic's
  message box, in plain words ("The printer couldn't be found. Check
  it's switched on and chosen in the Chooser.").
- **N11-20. Memory for printing:** the driver works in MacMusic's own
  memory. If there isn't enough, MacMusic says so before starting
  ("There isn't enough memory to print. Quit other programs, or give
  MacMusic more memory in Get Info."), and nothing else is affected.
  Part 0 measures what each driver needs.
- **N11-22. Running on a 1 MB Plus (agreed).** MacMusic 1.1 must run on
  a 1 MB Plus: under MultiFinder if a 1 MB Plus leaves room, and under
  the Finder alone if not (the Finder alone gives MacMusic all the free
  memory). So:
  - the SIZE resource's **minimum stays at or below what a 1 MB Plus
    under MultiFinder leaves** (Part 0, step 4), and its preferred size
    stays 640K unless Part 0 shows that's too little to print;
  - the cache adds no lasting memory (N11-9);
  - printing to the ImageWriter must work on a 1 MB Plus; if a large
    library and a LaserWriter job can't both fit there, MacMusic says
    so (N11-20) rather than failing;
  - the User's Guide says what works with 1 MB, and how.

- **N11-24. Printing in a code segment of its own (agreed 2026-10-10).**
  The printing code (Page Setup, the lists dialog, the layout and the
  page drawing) goes in its own CODE segment, loaded by the Segment
  Loader only when Page Setup or Print Library is chosen, and unloaded
  with `UnloadSeg` when the job ends, as *Inside Macintosh* (Volume II,
  "The Segment Loader") describes. A user who never prints never has it
  in memory; the driver's own memory is already given back at `PrClose`
  (Part 0). Built with Retro68's `--mac-segments` and a segment map, a
  change to MacMusic's CMake settings only, not to the toolchain. The
  rest of the program stays in its main segment for 1.1 (segmenting it
  all is for the smaller Macs, `future-releases.md`). To check: that the
  segment loads and unloads (the heap before and after a print, on the
  1 MB Plus), and that a print with the segment's disk locked or slow
  still works.

### Not in 1.1 (for printing)

Printing from the Now Playing screen, cover pages, PostScript tricks,
and choosing fonts or columns.

## Part D: importing several songs, and the real Plus's start-up (added 2026-10-12)

From the owner's check of the 1.1 test build on the real Plus (the issues
log, 2026-10-12): a plain start takes 32 s with the 307-song Scratch
library; an import takes 28 s, because every import ends with a full
library read; and MacMusic's window stays behind the Finder's until the
library has loaded. Part B's timings (12 s, then 7.5 s for 500 songs)
came from Mini vMac, which the Pokémon project has shown to be 2 to 20
times off the Plus. **Added to 1.1 by the owner, as 1.1 is still in
pre-release.**

### First: where the time goes (D0)

A throwaway timing build (`releases/MacMusic-1.1-timing/`, two copies:
as shipped, and compiled with optimisation, M2) writes the time of each
part of a library read to a "Timings" file, on the real Plus: listing
the folder, the cache, the scan, grouping, the title sort, the ID sort,
writing the cache. Its figures decide N11-27 to N11-30.

**The Plus's figures (2026-10-12),** the 316-song Scratch library, in
seconds (ticks / 60). The Timings lines were cut short by the timing
build's line buffer, losing "this whole read" and "since launch"; the
parts add up to the owner's stopwatch (32 s).

| Part of a plain start | As shipped (-O0) | Optimised (-O2) |
|---|---|---|
| Reading the cache | 1.5 | 0.5 |
| The scan (no song files read) | 8.6 | 6.6 |
| **Grouping** (artists, albums, genres, composers) | **21.9** | **15.1** |
| The title sort | 1.1 | 0.9 |
| The ID sort | 0.0 | 0.0 |
| **The whole read** | **≈ 33** | **≈ 23** |

Listing the folder alone (the timing build's extra pass, not part of a
read) took 1.5 s. After an import (one song read from its file), writing
the cache took another 3.4 s (2.1 s optimised).

What they show:
- **Grouping is two-thirds of the start.** For each song and each of the
  four kinds, `InsertName` compares the name with every name so far
  (`EqualString`, a ROM call each), then walks the list again for the
  place to insert it: tens of thousands of ROM calls. Mini vMac ran them
  fast enough to hide this.
- **The scan takes 7 s more than listing the folder,** with nothing read
  from the song files: matching the files to the cache, or something
  else per file; not yet found.
- **Optimisation alone saves about 30%** (33 s to 23 s): worth having, not
  enough alone.
- Sorting is no longer a cost (B1's merge sort, N11-23).

### Decisions

- **N11-25. Importing one song after another (agreed, the owner's
  option 1).** File > Import Song… works as now for the first song; after
  its details window, **the Open dialog comes back for the next song**.
  Each song is chosen by the owner, one at a time (no whole-folder
  import: the owner wants control of exactly which songs). Cancel in the
  Open dialog ends the run.
  - **Cancel in a song's details window skips that song** and brings the
    Open dialog back for the next; only Cancel in the Open dialog ends the
    run (agreed, 2026-10-12).
  - Each song is moved or copied into the library at once, as now, so a
    song already imported stays imported if a later one fails or the run
    is cancelled.
  - The music pauses for the whole run (as A5), and carries on after.
  - **The rest of a run (agreed, 2026-10-13):** "Don't Import" in the
    duplicate alert and Cancel in the other-disk alert skip the song, as
    Cancel in its details window does. **Move or Copy is asked for each
    song,** as in 1.0. A song that can't be read gets its message, then
    the Open dialog comes back. **"Disk full" and "library full" end the
    run** (every later song would fail the same way).
  - **The Open dialog keeps System 6's Cancel** (agreed, 2026-10-13): a
    "Done" button would need our own copy of the standard dialog. The
    User's Guide says the songs already imported stay.
  - The run's list of what it has imported is a small portable module,
    host-tested.
- **N11-26. One library read at the end of the run,** not one per song:
  only if anything was imported.
- **Two things a run must get right** (found in the code, 2026-10-12):
  - **Duplicates:** the "already in the library" check looks in the
    library in memory, which won't hold songs imported earlier in the
    same run; the run keeps its own list of the songs it has imported
    (audio length and fingerprint) and checks that too.
  - **The 500-song limit:** counted with the songs already imported in
    the run, so the run stops, with the usual message, when the library
    is full.
  - (The songs' IDs are already safe: each import records the next ID in
    the library's marker file, and the next import reads it.)
- **N11-27. Not done (decided from the timings, the owner,
  2026-10-12).** Adding the new songs in memory instead of a read at the
  end would still need the grouping and sorting again, which were most
  of the read.
- **N11-28. MacMusic comes to the front first (agreed: a fix; built
  2026-10-13).** Under
  MultiFinder a program comes fully to the front only once it calls the
  event routines; MacMusic reads its library before its first. It will
  call `EventAvail` a few times at start-up, before the library is read
  (the usual remedy of the time), so its window and logo are in front
  while it loads.
- **N11-29. "Loading the library…" from the start of every read (agreed,
  2026-10-13; built).** At a start, after an import run and on a change
  of library, under the logo; for Rescan Library across the foot of the
  screen, as before. Not only once a song file has to be read: on the
  Plus a read from the cache takes about 10 s, with nothing said.
- **N11-30. Compiler optimisation (M2; agreed from the timings,
  2026-10-12; built 2026-10-13).** MacMusic 1.1 is built with `-O2` (about 30% faster on
  the Plus), after a full run of the tests and the manual checklist (it
  changes every timing the tests were made with).
- **N11-31.** Part B's times in the documents (the User's Guide, the Read
  Me, outstanding-issues.md) replaced with the real Plus's: about 13 s
  from double-clicking to ready for about 300 songs, optimised (the
  second timing run). The User's Guide also explains importing several
  songs.
- **Nothing else in 1.1** (agreed, 2026-10-13): the issues log has nothing
  else open.

- **N11-32. Grouping by binary search (from the timings, 2026-10-12).**
  The name lists are already sorted, so `InsertName` finds a name's
  place by binary search with `CompareNames` (about 8 comparisons for
  300 names, instead of up to a few hundred `EqualString` calls and a
  second walk), then checks the names that compare equal there for an
  exact match (`EqualString`, as now: the groups don't change). Host
  tests: the same groups and order as the present code, on the test
  libraries and on names that differ only in case or accents.
- **N11-33. The scan's extra 7 s: tried, and reverted (the owner,
  2026-10-12).** The second timing run (below) found it: growing the
  lists a song at a time in a nearly full heap. A fix was built
  (`MaxApplZone` at start-up, the lists sized once per scan; commit
  `ea550e8`), but on the Plus it made little difference the owner could
  see. **Reverted,** for simplicity: N11-32's start, about 13 s with the
  optimised build, is acceptable, and Part D's other work comes first.
- **A second timing build** on the Plus after N11-32, with the scan
  broken down, its lines long enough to keep every figure (run
  2026-10-12, the optimised copy; below). Its figures give N11-31 its
  times; a third run (with N11-33) is not needed now that it's reverted.

**The second timing run (Optimised, 2026-10-12),** in seconds:

| | First run | With N11-32 |
|---|---|---|
| Reading the cache | 0.5 | 0.5 |
| The scan | 6.6 | 6.6 |
| Grouping | 15.1 | **2.5** |
| The sorts | 0.9 | 0.9 |
| The read | 23 | **10.5** |

From launch to ready: 14.4 s (the owner: the import took about 12 s,
against 28 s). The scan, at the start: catalog calls 1.6 s; cache
lookups 0.65 s (the next entry every time); the duplicate check 0.4 s;
adding the strings 1.4 s; **growing the song lists 2.4 s, the song list
moving 155 times with 5.9K free.**

## Not in 1.1 (agreed)

No other issue is fixed in 1.1: I4 (0.9% fast), I6 (opening a library
from the Finder), and the watch items I8 and I9 stay open. The owner
reviews all the outstanding issues after 1.1.

## Testing

- **Host tests (`tests/run-host.sh`), new:**
  - the cache: written and read back the same; a damaged, old-version or
    wrong-folder cache refused; entries matched by name, date and size;
    a changed file read again; a removed file dropped; a new file added;
  - the print layout, kept in a module apart from QuickDraw: which rows
    go on which page, the page count, the columns, the cut entries (with
    text widths supplied by the test).
- **The smoke test (`tests/run-smoke.sh`):** a second start with the
  same library, checking it used the cache and found the same songs, in
  the same order, as a full read.
- **The manual checklist,** new sections:
  - start-up on the real Plus: first start (full read), second start
    (the cache), after importing, after Rescan, after copying the library
    to another disk, with a locked disk;
  - a 1.1 library opened in 1.0, and 1.0's prefs in 1.1;
  - printing each list on the ImageWriter, Page Setup's options (A4,
    US Letter, landscape), the first print opening Page Setup, cancelling,
    the printer switched off or not chosen, the music pausing and
    resuming; a LaserWriter and AppleTalk when available;
  - a 1 MB Plus (Mini vMac's 1 MB variant, and the real Plus if it can
    be run with 1 MB): MultiFinder and the Finder alone, with the
    500-song library, scanning, the cache, playing and printing.

## Documentation

- `CHANGELOG.md` (1.1), `docs/decisions.md` (the N11 codes kept as
  decisions), `docs/outstanding-issues.md` (H1 resolved; the issues
  log's entries closed with "Fixed in 1.1").
- The *MacMusic User's Guide*: Print Library and Page Setup (a task
  topic in Chapter 2, the File menu in Chapter 3); start-up time in
  Appendix D and the known issues; the cache in "The MacMusic Library";
  the Read Me; the Quick Reference Card's File menu.
- *MacMusic Utilities*: Chapter 1's journey ends with MacMusic, so it
  changes only where it quotes the start-up time.

## Order

1. Part 0 measured and reported; anything it changes brought back to
   the owner.
2. Part A (small: build, tests, your check, commit).
3. Part B in steps: the cache's format and host tests; reading it at
   start-up; writing it; the progress bar (if wanted). Your check in
   Mini vMac, and on the Plus, after each.
4. Part C in steps: the menu and dialogs; the layout and host tests;
   printing on your printers.
5. The documentation, the manual checklist on the real Plus, the version
   (`project(MacMusic VERSION 1.1)`), the release disk, tag `v1.1`, and
   the environment archive if the toolchain changed.

Each step: build, both test scripts, your check, then commit only when
you say.

## Where it stands (2026-10-11, put aside for a few days)

Done and committed, each checked by the owner in Mini vMac:

| Step | Commit |
|---|---|
| Part 0, the measurements | `600df6c`, `7282c4b` |
| Part A, "New library in:" | `2ed64cf` |
| B1, the merge sort (N11-23) | `d69799c` |
| B2, the cache, groups noted as built | `66a3241` |
| B3, "Loading the library…" | `da1877b` |
| C1, the print layout | `84366ec` |
| C2, printing (untested on paper) | `2b60185` |
| The documentation, and the File menu fix | `2d1694f` |

Still to do, in order (Part D added 2026-10-12, before these):

0. **Part D** (the owner, 2026-10-13: N11-32's start of about 13 s is
   acceptable; no more chasing seconds). Done: N11-32 (`bcca2ca`);
   N11-33 tried and reverted (`ea550e8`, `65a0d83`). In order:
   N11-28 and N11-29 (built); N11-30 (`-O2`); N11-25 and N11-26, the
   import run, in steps; N11-31 and the User's Guide; then a new test
   image for the Plus.

Still to do, in order:

1. **The owner's check on the real Plus** with the test image
   `~/MacPlusProjects/releases/MacMusic-1.1-test-2d1694f/HD30_512 MacMusic 1.1 Test.hda`
   (SCSI ID 3; the steps are on the disk, in About This Test): print each
   list on the ImageWriter; Page Setup; Command-period; the music pausing;
   "Loading the library…" and the start-up time with the 307-song Scratch
   library. The manual checklist's section 10 covers it all.
2. Anything the printed pages or the Plus show: fonts, columns, header
   and footer are PROVISIONAL (N11-12, N11-13). Correct the guides' times
   if the Plus differs from Mini vMac's (12 s, then 7.5 s, for 500 songs).
3. The rest of the manual checklist on the Plus, and a 1 MB check
   (10.17, 10.18).
4. The version: `project(MacMusic VERSION 1.1)` in `CMakeLists.txt`, the
   About box and vers resources follow; the guides' "1.1" is already
   written.
5. `tools/make-release.sh` (a clean tree, both tests): the release
   folder `MacMusic-1.1`, its floppy with the 1.1 Read Me; then tag
   `v1.1`, and the CHANGELOG's "1.1 (in progress)" given its date.
6. The User's Guide PDF (`tools/manual/make-book.py`) for the release;
   remake the environment archive only if the toolchain changed (it
   didn't: the 1 MB Mini vMac and the British 6.0.7 disks are in the
   scratchpad only, and could be added to the archive if wanted).
