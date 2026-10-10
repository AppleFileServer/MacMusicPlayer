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

### Not in 1.1 (for printing)

Printing from the Now Playing screen, cover pages, PostScript tricks,
and choosing fonts or columns.

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
