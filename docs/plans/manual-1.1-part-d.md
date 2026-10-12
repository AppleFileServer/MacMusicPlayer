# The User's Guide for 1.1's Part D, and the 1.1 Update Card: plan

Status: **approved and done, 2026-10-13** (the owner approved the
documents the same day). Decisions are
numbered (MD1 and so on); **(R)** marks a recommendation.

The *MacMusic User's Guide*, second edition (`tools/manual/`, built to
`build-manual/MacMusic-Users-Guide.pdf`), was brought up to 1.1 in
`2d1694f`: printing, the cache, 1 MB, the clock battery. Part D came
after it (`docs/plans/release-1.1.md`), so the book doesn't yet show:

- importing one song after another, with "Import another song?"
  (N11-25, N11-26);
- "Loading the library…" at every start, not only when files are read
  (N11-29);
- the real Plus's start-up time, about 13 s for 300 songs, instead of
  Mini vMac's 12 s and "a few seconds" (N11-31).

MacMusic in front at start-up (N11-28) and the optimised build (N11-30)
change nothing a reader does, so they aren't mentioned. The plain text
guide and the Read Me already have all of Part D (released with 1.1).

## What changes

### The *MacMusic User's Guide*

1. **Chapter 1, Lesson 7 (importing Song.wav):** after Import, the new
   question. A step: "MacMusic asks whether to import another song.
   Click Done." One sentence on what Import Another would do, pointing to
   Chapter 2.
2. **Chapter 2, "Adding Songs":**
   - "To Import a Song": the question as the last step;
   - **a new task, "To Import Several Songs":** Import Another after each
     song, then Done; each song is in the library as soon as it's
     imported, so Done (or Cancel in the Open dialog) keeps them all;
     Cancel in a song's details skips just that song; the library is
     read once, at the end, with "Loading the library…";
   - "If the Song Is Already There": Cancel skips that song, and the
     question follows; MacMusic also knows a song imported earlier in the
     same run.
3. **Chapter 2, "To Rescan a Library":** "About 12 seconds for 500 songs"
   is Mini vMac's figure (MD2).
4. **Chapter 3, the File menu:** Import Song… "asks for WAV files one
   after another and imports them".
5. **Appendix A, the messages:** "Import another song?" (Import Another,
   Done), with its picture; the library-full message, now also shown in
   the middle of a run, ending it.
6. **Appendix D, "Starting up takes a while":** the Plus's times, and
   "Loading the library…" at every start.
7. **The index and glossary:** "importing several songs", "Import
   another song?", "Done".

### The *MacMusic Quick Reference Card* (`tools/utilities-manual/make-card.py`)

8. "Adding songs from the Transfer disk": a row for the question, Import
   Another for the next song, Done to finish.

### *MacMusic Utilities*

9. Checked for anything Part D changes. The 1.1 plan expected only the
   start-up time; a search of its Chapter 1 finds no time quoted, so
   probably nothing. I'll confirm page by page where it shows an import.

### The *MacMusic 1.1 Update Card* (new, a separate document)

10. **What it is:** a card that goes in the box with 1.1, telling someone
    who knows 1.0 what's different, as `docs/plans/future-releases.md`
    foresaw ("the v1.1 Update Card: what's changed in 1.1, written for
    the user"). Nothing on it that the User's Guide doesn't say; each
    item ends with where the guide tells more (chapter and page, from
    the book's page numbers).
11. **What it says** (in the book's words, British English):
    - **What's new in 1.1:**
      - *Starting faster:* MacMusic keeps a note of your songs, so a
        start takes about 13 seconds for 300 songs on a Macintosh Plus;
        "Loading the library…" while it reads. The first start with a
        library, and Rescan Library, read every song and take longer.
      - *Printing your library:* Print Library… prints your songs, your
        albums (with their songs or not) and your artists (with their
        albums or not), to an ImageWriter, a LaserWriter or a printer on
        AppleTalk; Page Setup…; A4 to begin with.
      - *Importing several songs:* "Import another song?" after each.
      - *Small things:* MacMusic comes to the front at once under
        MultiFinder; the Create New Library prompt is shown whole.
        (Running on a 1 MB Plus is as 1.0: its memory request is
        unchanged, so it isn't on the card.)
    - **Moving up from 1.0:** replace MacMusic with the new one; your
      libraries, songs, playlists and settings are kept as they are. The
      first start reads every song once, to make its note ("MacMusic
      Cache", an invisible folder in the library). A library used by 1.1
      still opens in 1.0.
    - **Where to read more:** the *MacMusic User's Guide*, second
      edition.

## Decisions

- **MD1. The picture of "Import another song?"** (Appendix A, and perhaps
  Chapter 2). It needs a screenshot of Mini vMac, which I take only when
  you ask. **(R)** Yes: one screenshot of the alert, taken the way the
  book's other dialogs were (Mini vMac at 1x, the crop and scale in
  `figures.md`). If not, Appendix A gives the message without a picture.
- **MD2. The Rescan Library time.** The Plus's time for reading every
  file hasn't been measured. **(R)** You time one Rescan Library on the
  Plus with the Scratch library (manual checklist 10.1), and the book
  quotes it; until then, "This takes longer than starting up", with no
  figure.
- **MD3. The edition.** **(R)** It stays the *second edition, October
  2026*: no copy of the second edition has gone out, and it's 1.1's
  book. The imprint's date is unchanged.
- **MD4. The PDFs in the release folder.** **(R)** Once you've reviewed
  them, the own-use User's Guide and the plain edition go into
  `~/MacPlusProjects/releases/MacMusic-1.1/`, added to `SHA256SUMS`.
  The release itself (the app, the disks, the tag) doesn't change.
- **MD5. Both editions** are rebuilt: the own-use edition (the private
  cover and logo) and the plain one, from the same pages.

## The Update Card's decisions

- **MD6. Its size and look.** **(R)** The *Quick Reference Card*'s: one
  leaf of the manuals' size (7.5 x 9 in), two pages, the red rule and red
  squares, so the two cards match in the box; titled *MacMusic 1.1
  Update*, after Apple's update sheets of the time. (Smaller, a single
  page, is possible; then it can say less.)
- **MD7. Where it's made.** **(R)** Beside the card:
  `tools/utilities-manual/make-update-card.py` writing
  `docs/utilities-manual/update-card.html`, built with the card's
  `build-pdf.sh`, sharing its styles.
- **MD8. Its editions.** **(R)** As the book (MD5): an own-use edition
  with the rainbow logo, and a plain one; the text the same.
- **MD9. Printing untried on paper.** **(R)** Not on the card: it's
  written for the user, and the gap is recorded in
  `docs/testing/runs/1.1.md`; once you've printed, the card stands.
- **MD10. Later versions.** **(R)** Each later version with changes a
  user would notice gets its own card (1.2 ...); a 1.1.x of fixes only,
  none.

## Order

1. The text: items 1-8 (and 9 if needed), in the generators
   (`tools/manual/make-*.py`, `make-card.py`).
2. The Update Card (items 10-11, MD6-MD10), once the book's pages are
   settled, so its page references are right.
3. The screenshot (MD1), if wanted, and the figure list.
4. The books and cards built (`make-book.py`, the cards, both
   editions); I check the page breaks, the contents and index page
   numbers, and that nothing else moved.
5. Your review of the PDFs; corrections.
6. Commit; the PDFs, the Update Card's included, into the release folder
   (MD4).
