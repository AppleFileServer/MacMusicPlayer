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

## Screenshots still wanted

For the Converter (when convenient): the Licences window; a refused
file and a refused video, with their messages; Show in Finder's result;
cancelling a conversion; the question when quitting mid-conversion; the
MP3 re-encode question (Re-encode / WAV Only). For the Transfer
Utility: all of it (the owner will take them). For the Backup Utility:
after it exists.
