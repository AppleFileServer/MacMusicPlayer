# Plan: *MacMusic Utilities*, the companion manual

The second book in the box, beside the *MacMusic User's Guide*
(`docs/plans/manual-1987.md`). That book's MU2 promised it: converting
songs to MacMusic's sound files on a modern Mac, moving them to the
Macintosh Plus, BlueSCSI and the project's own programs. It covers
three programs, delivered on an external USB drive in the box:

- the **MacMusic Converter Utility** (1.0, `~/MacPlusProjects/MacMusicConverter`);
- the **MacMusic Transfer Utility** (1.0 candidate, `~/MacPlusProjects/MacMusicTransfer`);
- the **Macintosh Backup Utility** (not yet developed).

Decisions UB1-UB10 were approved by the owner on 2026-10-09.

## What the User's Guide already fixes

- The title, *MacMusic Utilities*, in italics wherever it's named (MU2, MU5).
- A chapter called **"Converting Songs"**: Appendix B of the User's
  Guide sends readers to it by name.
- Its job, as the User's Guide's About This Manual puts it: "explains how
  to make your music into MacMusic's sound files and bring them to your
  Macintosh."

## Decisions

- **UB1, for the owner only.** The book, the USB drive and everything on
  it are for the owner's own use, like the User's Guide's own-use
  edition. So the Transfer Utility may carry Apple's driver, the drive
  may hold Song.wav (MU3), and the screenshots may show the owner's
  folders and the YouTube video used. Nothing is published.
- **UB2, the world and the voice.** The User's Guide's MU1 (the Plus
  only) doesn't apply: this book's reader works on an Apple silicon Mac
  with macOS 26, and on the Macintosh Plus with System 6.0.7 and a
  BlueSCSI where the songs end up. The voice is the User's Guide's
  plain Apple voice; British spelling; never "iPod"; no pretending to be
  1987: modern terms as macOS uses them (Finder, Applications folder,
  the SD card).
- **UB3, one series.** The same page size, grid, type, red, opener
  spreads, task pages, glossary and index as the User's Guide, built
  with the same tools (`tools/manual/book.py`, `docs/manual/manual.css`,
  Paged.js and `print-pdf.py`). Differences are listed in
  `docs/utilities-manual/style-addendum.md`.
- **UB4, the figures.** The owner's screenshots, cropped to the app's
  window (no desktop, Dock or Terminal), printed in greyscale so the
  book stays black and red; callouts in red in the margin, as the
  User's Guide's. The originals are kept in
  `~/MacPlusProjects/manual-assets-private/utilities-screenshots/`
  (`cvNN` for the Converter), never in the repository; a script crops
  them by recorded rectangles into `docs/utilities-manual/figures/png`.
- **UB5, the structure:** front matter; Chapter 1, Getting Started (the
  USB drive, installing, and one song's whole journey as lessons);
  one chapter per program, each with an introduction, a short lesson,
  task pages and a reference section (Chapter 2, Converting Songs;
  Chapter 3, Moving Songs to the Macintosh; Chapter 4, Backing Up Your
  Macintosh); Appendixes A (messages), B (the files), C (troubleshooting),
  D (specifications, bundled software, licences); glossary, index,
  covers. **To be reviewed against the page estimates in the outline:**
  if a program's chapter is too big, the book is divided into a part
  per program, each with its own chapters (the owner's alternative,
  2026-10-09).
- **UB6, the Backup Utility** gets its own brief and plan as a program
  project. Its chapter is written after it exists; the book isn't final
  until all three chapters are. Its purpose (the owner, 2026-10-09): it
  takes the BlueSCSI's SD cards and backs them up on the modern Mac, so
  that if data is lost on the Macintosh Plus, a disk image can be
  restored and put back on the card. It works rather as Time Machine
  does for a modern Mac, but for the Plus.
- **UB7, accuracy.** Every statement is checked against the released
  program and the screenshots; anything not seen is flagged in the draft
  (CHECK), not guessed. The Transfer chapter waits for its screenshots
  and the owner's Plus check of the Transfer Utility.
- **UB8, the USB drive (PROVISIONAL):** the three apps (zipped), both
  manuals as PDFs, MacMusic's floppy-disk image for the Plus, the
  source and licences, and a Read Me. Settled when the drive is made.
- **UB9, the files.** This plan; `docs/utilities-manual/` for the book
  (outline, style addendum, figure list, pages); `tools/utilities-manual/`
  for its scripts; the decisions in `docs/decisions.md`.
- **UB10, phases,** each a draft PDF for review, committed only with
  the owner's approval:
  1. this plan, the style addendum and the outline with its figure list;
  2. the Converter's figures (crops and callouts);
  3. Chapter 2, Converting Songs;
  4. Chapter 3, Moving Songs to the Macintosh (after its screenshots);
  5. Chapter 1, Getting Started (it needs both programs);
  6. Appendixes A-D for the Converter and the Transfer Utility;
  7. Chapter 4 (after the Backup Utility exists);
  8. front and back matter, glossary, index, cover; the whole book.

## Since approval

- **The task pages** (Phase 3): in the wide text column, a modern window
  being unreadable at a quarter-page column (style addendum; DECISION,
  for the owner's review).
- **The openers** (the owner, 2026-10-09): scenes from macOS's Macintosh
  wallpaper in flat pale-grey pixels, one per chapter, kept in the
  private assets (style addendum).

- **The cover** (UC1-UC5, 2026-10-09): the User's Guide's MacDraw
  composition, titled *MacMusic Utilities*, its screen the owner's Mini
  vMac shot of a MacMusic Utilities disk with the three utilities'
  1-bit icons (stand-in programs, props only). Made by
  `tools/utilities-manual/make-cover.py` into the private assets; the
  tinted screen (`ub-cover-screen`) is in the repository for the plain
  edition, as the User's Guide's is.

- **Chapter 4** (Phase 7, drafted 2026-10-10, before Chapters 3 and 1,
  as the Backup Utility was finished first): written against the Backup
  Utility 1.0; its figures from the owner's screenshots of earlier builds
  (`bu01`-`bu14`, kept privately), to be replaced from screenshots of
  1.0. The window is twice the Converter's, so strips of it (the status)
  print across the whole page to be readable.

- **Chapter 3** (Phase 4, drafted 2026-10-10): the Transfer Utility 1.0,
  its figures from the owner's screenshots of 2026-10-10 (`n01`-`n07`,
  privately; `tuNN`), taken just before its last small changes (sizes the
  Finder's way, the file list's columns, Keep Building), to be replaced.
  Chapter 4's opener changed to MacPaint's quit dialog, as its first
  scene cropped too like Chapter 3's.

- **Chapter 1** (Phase 5, drafted 2026-10-10): a song's journey in one
  picture (drawn: the programs in red-ruled boxes, the Backup Utility
  apart in a dashed one), what's on the USB drive (PROVISIONAL, UB8),
  installing (the Converter's install screenshots, the Finder window
  cropped to its sidebar and first rows), a tour of the journey pointing
  to each chapter, and What Now?

- **Captures of 1.0** (2026-10-10, taken by Claude with the owner's
  leave, kept privately in `utilities-screenshots/taken-2026-10-10/`):
  each window captured alone, so its frame is the whole image; sheets
  and alerts cut out of them. They replace the Transfer Utility's
  provisional figures (`tu01` and its parts, and `tu-...`), add the
  Converter's missing questions and results (`cv-...`), the Plus's
  WAV Files folder from Mini vMac (`plus-wav-files`), and what the
  Backup Utility shows without a card (`bu-...`: the Restore sheet and
  its conflict question, Verify's result, the snapshot menu, Delete,
  Settings). A folder chosen as the card shows the Mac's own disk in the
  card details, so those screens are used only in parts.
- **The Backup Utility in the appendixes** (2026-10-10): its messages,
  its backup folder, troubleshooting and its specification, from its
  1.0 source.
- **The whole book** (Phase 8, 2026-10-10): `make-book.py`, as the
  User's Guide's, with each chapter's ids prefixed (`c1-` ... `c4-`),
  the full glossary (with the Backup Utility's words) and the index;
  110 pages.

- **The Quick Reference Card** (2026-10-10, the owner's request):
  `make-card.py`, four pages the manuals' size, after Apple's Macintosh
  Quick Reference Card (1988, 030-3180-B): a song's journey and the three
  utilities, then MacMusic's controls, screens, menus, importing from the
  Transfer disk, playlists and limits, all from the two manuals.

## Screenshots still wanted

For the Backup Utility, with a real card in the Mac: the window with
the card connected ("Registered as", Eject), the registration sheet,
the card ready to back up and the Back Up sheet (to replace `bu13`
and the card details), and Restore to a New Card with only the new card
listed. The Plus's own screens again after the owner's Plus check, if
the real Plus shows them differently from Mini vMac (its WAV Files
folder in icon view). The Converter's Edit menu (optional).
