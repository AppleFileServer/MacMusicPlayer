# *MacMusic Utilities*: outline

Phase 1 of `docs/plans/manual-utilities.md`. The book's chapters and
sections (UB5), what each covers, its figures (codes below), and the
page estimates for a 7½ × 9 in page, set as the User's Guide is. For
comparison, the User's Guide's chapters run 20-26 pages.

Facts come from the programs themselves: the Converter's 1.0 source and
`Documentation/SPECIFICATION.md`, the Transfer Utility's, and the
screenshots. CHECK marks what isn't yet seen on screen.

## Front matter (about 6 pages)

Cover, copyright page, title page, contents (as the User's Guide's,
§12). **About This Manual:** what the three utilities are for, in a
paragraph; what's on the USB drive (UB8); what you need (a Mac with
Apple silicon and macOS 26; the Macintosh Plus with System 6.0.7 and a
BlueSCSI; MacMusic and the *MacMusic User's Guide*); what you should
know already (using a Mac; the Plus as the User's Guide teaches it);
how to use the book; conventions (your Mac and the Macintosh Plus).

## Chapter 1: Getting Started (about 12 pages)

- Opener spread; contents.
- **Introduction: From Your Music to MacMusic.** The journey of a song
  in one picture: a sound file or a YouTube video → the Converter → a
  MacMusic WAV → the Transfer Utility → a disk image → the BlueSCSI's
  SD card → the Macintosh Plus → MacMusic's Import Song. And keeping it
  safe: the Backup Utility backs up the SD card on your Mac (CHECK: after
  it's designed).
- **What's on the USB Drive** (UB8, PROVISIONAL).
- **Installing the Utilities:** unzipping, dragging to Applications,
  opening the first time (Control-click, Open). Figures CV01-CV05, CV35.
- **Lesson 1: A Song's Journey,** in brief, pointing to the chapters
  that teach each part.
- **What Now?**

## Chapter 2: Converting Songs (about 25 pages)

The MacMusic Converter Utility. The title is fixed by the User's Guide.

- Opener spread; contents (3 pages).
- **Introduction** (2): what it makes (MacMusic WAV: mono, 22,050 Hz,
  8-bit; MP3: 192 kbps; or both); where songs come from (a file, or a
  YouTube video you own or may save); what it never does (change the
  source, leave anything half-made).
- **Lesson 1: Converting a Song From a File** (3): Choose File, the
  file's details, Make, Title, File name, Saves as, Convert, the result,
  Show in Finder. CV06, CV08-CV10, CV12, CV28.
- **Lesson 2: Converting a YouTube Video** (3): the address, Look Up,
  the title and length, Both, the stages as it works, the result.
  CV07, CV16-CV21, CV24-CV27.
- **Task pages** (about 7):
  - Choosing a Source: To Convert a Sound File; To Drop a File on the
    Window; To Convert a YouTube Video; If a Video Can't Be Used.
  - Choosing What to Make: To Make a MacMusic WAV, an MP3 or Both; If
    the Source Is Already an MP3 (Re-encode, WAV Only). CV11, CV13, CV14.
  - Naming: To Set the Title MacMusic Shows; To Set the File Name.
  - The Sound: To Normalise (or Not); To Trim Silence at the Ends; To
    Change the Normalise Level.
  - Saving: To Choose Where Files Are Saved; If a File Already Exists
    (Keep Both, Replace). CV15, CV22, CV23.
  - While It Works: To Cancel; Quitting While It Works (CHECK).
  - Afterwards: To Find the Files; To Copy Them to the Macintosh (see
    Chapter 3).
- **Converter Reference** (about 8):
  - The window, with callouts: Local File and YouTube; Source or YouTube
    Video; Output; Sound; Save To; Advanced; Results; Convert and the
    progress bar. CV06, CV07, CV15, CV28.
  - What each stage does: retrieving, analysing the sound, making the
    WAV or MP3, checking it.
  - Advanced: Normalise to; the fixed formats; FFmpeg, yt-dlp and Deno;
    Version; Licences (CHECK: the Licences window).
  - The menus: the application menu, Edit, View, Window, Help.
    CV29-CV34; the About box, CV30.
  - Limits: names of 31 characters at most; one video at a time, no
    playlists; no sign-in; peak normalising, not loudness levelling.

## Chapter 3: Moving Songs to the Macintosh (about 21 pages)

The MacMusic Transfer Utility. Waits for its screenshots and the Plus
check (UB7). Figures TU-codes, to be assigned.

- Opener spread; contents (3).
- **Introduction** (2): a BlueSCSI and its SD card; a disk image is a
  whole hard disk in one file; SCSI IDs (one image each; 7 is the Mac;
  0 is usually the startup disk); what the Transfer Utility makes (a disk
  named Transfer with a folder WAV Files); it never writes to the card.
- **Lesson 1: Putting Songs on a Disk for the Plus** (4): adding WAVs,
  the checks and names, the SCSI ID, Build Transfer Image, the result;
  copying the image to the SD card; on the Plus: the disk, Import Song
  (Macintosh Plus screens from Mini vMac).
- **Task pages** (about 6): To Add Files or a Folder; To Remove Files;
  If a File Is Refused; If a Name Is Changed; To Choose the SCSI ID; To
  Name the Image; To Choose Where It's Saved; If an Image Already Exists
  (Keep Both, Replace); To Cancel; To Copy the Image to the SD Card; To
  Use the Disk on the Macintosh Plus; To Add More Songs Later (a new
  image); To Use Another Disk Template.
- **Transfer Reference** (about 6): the window, with callouts; the
  table's columns; the size and how it's worked out; Advanced (the
  template); what the checks check; the summary; names on the Mac
  (31 characters, no colons, clashes ignoring case); limits (2 GB, one
  folder level).

## Chapter 4: Backing Up Your Macintosh (about 15-20 pages, CHECK)

The Macintosh Backup Utility, written after it exists (UB6), in the same
shape: introduction, a lesson, task pages, reference. Its purpose: it
backs up the BlueSCSI's SD cards on your Mac and restores a disk image
to a card if data is lost on the Macintosh Plus, rather as Time Machine
does for a modern Mac. Likely sections (CHECK, from its design): why
back up; backing up a card; the backups kept, and when; restoring a
disk image to a card; checking a backup.

## Appendixes (about 18 pages)

- **A, Messages** (about 8): every message each program shows, what it
  means, what to do; the Converter's (about 40) and the Transfer
  Utility's (about 27), grouped by what you were doing.
- **B, About the Files** (about 4): MacMusic's WAV format and why
  (with the User's Guide's Appendix B); MP3s; disk images and BlueSCSI's
  file names (`HD60_512 Transfer.hda`).
- **C, Troubleshooting** (about 3): a song won't import on the Plus;
  the disk doesn't appear on the Plus; YouTube stops working (yt-dlp
  needs updating); macOS won't open an app.
- **D, Specifications and Licences** (about 3): what each program needs
  and makes; the bundled software (FFmpeg 9.0.2, yt-dlp 2026.08.19,
  Deno 2.9.7, hfsutils 3.2.6) and their licences; where the source is on
  the USB drive.

## Back matter (about 8 pages)

Glossary (about 3), index (about 3), inside back and back covers.

## Size

| Part | Pages |
|---|---|
| Front matter | 6 |
| Chapter 1, Getting Started | 12 |
| Chapter 2, Converting Songs | 25 |
| Chapter 3, Moving Songs to the Macintosh | 21 |
| Chapter 4, Backing Up Your Macintosh | 15-20 |
| Appendixes | 18 |
| Back matter | 8 |
| **The book** | **about 105-110** |

Each program's chapter (21-25 pages) is the size of a User's Guide
chapter (20-26), so one chapter per program fits (UB5). A part per
program, each with Learning, Using and Reference chapters, would make
three chapters of 6-10 pages each, with an opener spread and a
contents page apiece: about 12 pages more, for little gain. To be
reviewed again when Chapter 2 is drafted (Phase 3) and Chapter 4's
program is known.

## Figures

### The Converter (CV: the owner's screenshots, 2026-10-09)

Originals in `~/MacPlusProjects/manual-assets-private/utilities-screenshots/converter/`,
2816 × 1762 (the whole screen); each figure is a crop to the window.

| Code | What it shows | Used in |
|---|---|---|
| CV01 | the zip in a Finder window | 1, Installing |
| CV02 | Archive Utility expanding it | 1, Installing |
| CV03 | the app beside the zip | 1, Installing |
| CV04 | dragging it to Applications | 1, Installing |
| CV05 | the app in the Applications folder | 1, Installing |
| CV06 | the window, Local File, nothing chosen | 2, Lesson 1; Reference |
| CV07 | the window, YouTube, empty | 2, Lesson 2; Reference |
| CV08-CV09 | the Open dialog, finding the Music folder | 2, Lesson 1 |
| CV10 | choosing "Good Luck, Babe!.mp3" | 2, Lesson 1 |
| CV11 | an MP3 source with MP3 chosen: the re-encode note | 2, tasks |
| CV12 | MacMusic WAV chosen: Title, File name, Saves as | 2, Lesson 1 |
| CV13 | MP3 chosen | 2, tasks |
| CV14 | Both chosen: two Saves as names | 2, tasks |
| CV15 | Save To and Advanced, opened | 2, tasks; Reference |
| CV16 | Look Up: "Looking it up…" | 2, Lesson 2 |
| CV17 | the video found: title, channel, length | 2, Lesson 2 |
| CV18 | the title shortened to "Chill Dreamy" | 2, Lesson 2 |
| CV19-CV21 | the progress: retrieving, making the MP3 | 2, Lesson 2 |
| CV22 | Saves as with the "already exists" marks | 2, tasks |
| CV23 | the "already exists" alert: Keep Both, Replace, Cancel | 2, tasks |
| CV24-CV27 | the progress: retrieving, analysing the sound, making the MP3 | 2, Lesson 2; Reference |
| CV28 | Results: Chill Dreamy.wav, Show in Finder; Advanced | 2, Lesson 1; Reference |
| CV29, CV31, CV32 | the application menu | 2, Reference |
| CV30 | the About box | 2, Reference |
| CV33 | the View menu | 2, Reference |
| CV34 | the Window menu | 2, Reference |
| CV35-CV36 | the app's icon among the Applications | 1, Installing |

Still wanted: listed in the plan.

### The Transfer Utility (TU), the Backup Utility (BU), the Plus (MP)

To come. Macintosh Plus screens (the Transfer disk, its WAV Files folder,
Import Song) are made in Mini vMac with the User's Guide's figure tools.
