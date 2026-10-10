# Future releases: the list (for later review)

Written 2026-10-10 from the owner's description, **for review, not yet
planned or decided.** It lists what's to be released, and notes what
each will need decided or checked; nothing here changes the work in hand
(MacMusic 1.1, `release-1.1.md`).

## What a "MacMusic system" is

One boxed set, everything a user needs:

1. **MacMusic**, the program for the Macintosh Plus;
2. the **MacMusic Converter Utility**, the **MacMusic Transfer Utility**
   and the **Macintosh Backup Utility**, for a modern Mac;
3. the ***MacMusic User's Guide***;
4. ***MacMusic Utilities***, the utilities' manual;
5. a **quick-start guide** (today the *MacMusic Quick Reference Card*);
6. from 1.1 on, an **update card** for each new version (below).

## The owner's own systems: "Personal"

Not for public release: they contain the owner's own assets and others'
artwork (Apple's cover photograph and rainbow logo, the Macintosh
wallpaper scenes in the chapter openers, screenshots of the owner's
Mac), and are in British English throughout.

| Name | What it is |
|---|---|
| **B-1.0 Personal** | The whole system as released at 1.0 (2026-10-08 to 10), renamed from "1.0". |
| **B-1.1 Personal** | The same for 1.1, plus the **v1.1 Update Card**: what's changed in 1.1, written for the user, to go in the box. |

To decide or check:
- **Renaming 1.0 to "B-1.0 Personal":** what changes, and what doesn't.
  Today "1.0" is in MacMusic's version resources and About box, the
  release files' names (`MacMusic-1.0.bin` ...), the git tag `v1.0`, the
  utilities' releases, the manuals' title pages and the Read Me. Changing
  the program's own version string means a new build, not the released
  one; the tag can stay as it is with the new name added (`B-1.0-personal`),
  so the history isn't rewritten. Possibly the name goes on the box, the
  manuals and the release folders only, and the program keeps "1.0".
- **The update card:** its size (the Quick Reference Card's 7.5 x 9 in,
  or smaller), its style (Apple's update cards and "What's New" sheets
  of the time), and whether each later version gets one.
- **What's in the box:** the USB drive's contents (UB8 in
  `manual-utilities.md`), the floppy, and the printed pieces.

## The public systems (from 1.1)

A plain MacMusic system, **stripped of every owned asset and anything
that might raise a copyright or trademark issue**, for public release.

### Languages and names

Following Apple's own naming of the time (the letter before the version,
as System 6's "B-6.0.7"):

| Name | Language |
|---|---|
| **1.1** | American English (American spelling) |
| **B-1.1** | British English |
| **F-1.1** | French |
| **D-1.1** | German |
| **T-1.1** | Italian |

and others Apple localised for in 1986 if wanted: Spanish (E), Dutch (N),
Swedish (S), Danish (DK), Norwegian (H), Finnish (K), Japanese (J,
KanjiTalk), International English (Z).

### Machines

| Target | Memory | System | Disks |
|---|---|---|---|
| **The main release:** Macintosh Plus | 4 MB, and 1 MB | System 6.0.7 | SCSI (a hard disk, or a BlueSCSI) |
| Macintosh 512Ke | 512K | an earlier System (to choose) | no SCSI: floppies, or a Hard Disk 20 |
| Macintosh 512K | 512K | an earlier System, MFS (no HFS in its 64K ROM without the HD20 file) | no SCSI |
| Macintosh 128K (maybe) | 128K | System 1-3 era | no SCSI, 400K floppies |

### To decide or check, for the public systems

**What has to come out, or be replaced:**
- Apple's artwork and photographs (the cover, the rainbow logo, the
  opener scenes), already kept apart in the private assets: the plain
  edition's stand-ins exist for the manuals; they'd need to be the
  finished artwork.
- **The iPod.** MacMusic is a faithful copy of the 4th-generation iPod's
  look and behaviour; "iPod" is Apple's trademark, and the look may be
  protected too (trade dress). The public system would need its own
  look, or legal advice; the name "MacMusic" is to be checked as well.
- Apple's System software, ROMs and the BlueSCSI disk template (which
  holds Apple's hard disk driver) can't be distributed; the user would
  bring their own.
- Screenshots in the manuals that show the owner's own files and folders.
- The Converter's YouTube feature: its legal position for a public
  release, in each country.
- The bundled programs' licences (FFmpeg and hfsutils are GPL, so their
  source must be offered; yt-dlp, Deno): already noted in the manuals'
  Appendix D, to be met in full for distribution.
- The legal statement and licence (`docs/legal/legal-statement.md`), to
  be written for public release (already on the roadmap).

**What localising means:**
- MacMusic's every string, alert, menu and the Read Me (most are in
  resources or `legal.r`, but some are in the C source); the manuals,
  the card and the update card; the utilities' windows (on the modern
  Mac); dates, numbers and sorting ("The" ignored is English-only).
- The fonts the Plus has for each language (all the 1986 European
  languages use the Mac's Roman character set; Japanese needs KanjiTalk).
- American spelling for "1.1": the project is written in British English
  now.

**What the earlier Macs mean** (feasibility first):
- MacMusic plays from a 148K sound ring with the library in memory, and
  uses the Sound Manager as System 6 has it (I9): a 512K Mac leaves far
  less, and earlier Systems have an older Sound Driver. A 128K Mac is
  very probably out of reach.
- Without SCSI, songs live on floppies: a MacMusic WAV is about 1.3 MB a
  minute, so an 800K floppy holds about 37 seconds. A no-SCSI version
  would need a Hard Disk 20 (on the 512Ke, with its file), or a lower
  sample rate (11,025 Hz halves the size), or both; that's a different
  song format (a 2.0-level change under Part E's rules).
- The Transfer Utility makes BlueSCSI disk images; no-SCSI Macs would
  need floppy images instead (or Hard Disk 20 images).
- Each target needs its own testing set-up (a Mini vMac built for each
  model and memory size, with its System).

## Order (to agree later)

Nothing here is scheduled. MacMusic 1.1 comes first (`release-1.1.md`);
then the owner's review of the outstanding issues; then this list.
