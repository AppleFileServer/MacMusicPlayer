# Plan: the MacMusic manual, in the style of Apple's early Macintosh manuals

The book's title is ***MacMusic User's Guide*** (MM9).

Status: approved (2026-10-08), decisions MM1-MM8 as recommended.
**Status (2026-10-08):** Phases 1-6 done, and Phase 7 in draft: the
whole book builds (`tools/manual/make-book.py`, 88 pages) with its front
matter, three chapters, four appendixes, glossary, index and back
matter. Next: the owner's review of the front and back matter, then
Phase 8 (proofing), and the clean-up list in
`docs/manual/review-issues.md`.

Phase 1 done: `docs/manual/style-guide.md` (SG1-SG8) and the figure plan
`docs/manual/figures.md` (F1-F6), from the owner's screenshots, which
replace MM5's test-only build for all but one picture. Phase 3 done:
`docs/manual/outline.md` (O1-O6). Phase 4 next. The manual never calls
MacMusic an "iPod" (style guide §4).

## Aim

A printed-style user's guide (PDF) for MacMusic 1.0 that reads, looks
and is organised like the manuals Apple shipped with its own Macintosh
applications in the mid-1980s: the voice, the page design, the hands-on
tutorial, the menu reference, the glossary and the index. Its content
stays exactly true to the real program: every command, message and
limit is checked against the code, as `docs/user-guide.md` was.

## The references

Kept outside the repository (they are Apple's copyright; the style
guide describes them, and nothing of theirs is copied into the project):

- **Manuals** (`~/MacPlus/Manuals/`), only these four, by the owner's
  choice:
  - *MacWrite* (1985), 176 pages;
  - *MacPaint* (1984), 36 pages;
  - *MacProject* (1984), 142 pages;
  - *MacDraw* (undated, about 1984-85), 123 pages.

  The folder's other manuals (HyperCard, the Macintosh owner's guides,
  System, Utilities, MultiFinder) are not used.
- **Packaging** (`~/MacPlus/Packaging/`): photographs of the MacDraw,
  MacProject, MacPascal and MacTerminal packages, for the cover, the
  title page and "what's in the package".

So the model is Apple's 1984-85 application manuals, the series the
project's 1987 feel will be built on.

## What the style study is expected to find (to be confirmed in Phase 1)

From general knowledge, not yet checked against the references:
- **Voice:** second person, friendly, plain, short sentences; Apple's
  vocabulary (choose a command, click, drag, select, pointer, dialog box,
  alert box, icon, window).
- **Structure:** a "learn it" half (getting started, a hands-on tour with
  numbered steps, everyday tasks) and a "look it up" half (a menu-by-menu
  reference, messages, appendices, glossary, index).
- **Page design:** a condensed Garamond text face; headings and notes in
  a wide margin; 1-bit screen images with callouts; line illustrations;
  "Important" and "Note" asides; running heads; pages numbered within
  chapters.
- **Front and back matter:** cover, contents, preface (about this
  manual, what you need, conventions), appendices, glossary, index, a
  reader comment card.

## Phases

1. **Style study:** read the four manuals and the packaging; write
   `docs/manual/style-guide.md`: voice with sample sentences, vocabulary,
   chapter and section structure, the page grid and type sizes, figures
   and callouts, asides, running heads and numbering, front and back
   matter, the cover. Each rule cites where it comes from (manual and
   page). For the owner's approval.
2. **Framing:** MM1 and MM2 as decided below, applied to the style guide.
3. **Outline:** chapters and sections, each mapped to the features and
   messages it covers.
4. **Production:** page size, fonts, source format and the PDF build
   (MM3, MM4); a two-page sample spread for approval before the writing.
5. **Screen images** (MM5): every MacMusic screen, dialog, alert and menu the
   manual shows, as exact 1-bit images.
6. **Writing,** chapter by chapter, each reviewed; every fact checked
   against the code and the user guide.
7. **Glossary, index and cross-references.**
8. **Proofing:** a full read-through, every command, message and figure
   checked against MacMusic 1.0, a test print of a few pages, the final
   PDF.

## A proposed outline (to be reshaped by the style study)

- Cover and contents
- Preface: About this manual (what MacMusic is, what you need, how to
  use the manual, conventions)
- 1 Getting Started: what's in the package, making a backup copy,
  installing on a hard disk, starting MacMusic, creating your MacMusic
  Library
- 2 A Tour of MacMusic: a hands-on tutorial (import a song, play it, use
  the click wheel, make a playlist)
- 3 Playing Music: the click wheel and buttons, the menus, Now Playing
  and its modes, fast-forward, Shuffle and Repeat, Settings, the Clock
- 4 Managing Your Music Library: importing, editing, exporting and
  removing songs, playlists, libraries, Library Info, rescanning
- 5 MacMusic and Your Other Applications: MultiFinder, desk accessories,
  playing in the background
- 6 Menu Reference: every command in every menu
- Appendix A Messages: every alert, what it means, what to do
- Appendix B Preparing Sound Files: the format, converting, the limits
- Appendix C Keyboard and Click Wheel Summary
- Appendix D Troubleshooting
- Glossary, Index
- Back matter: the legal statement, a reader comment page

## Decisions (2026-10-08)

- **MM1:** Apple's style, MacMusic's name. The publisher is MacMusic
  (© 2026 Jack McAteer), with trademark acknowledgements and the legal
  statement; no Apple name as publisher, no Apple logo, no Apple
  copyright line. The manual imitates a style, not an Apple publication.
- **MM2 (narrowed 2026-10-08, MU1):** written in the period's voice,
  describing the real product as it runs on the Macintosh Plus: its
  requirements (System 6.0.7, its sound-file format) stated plainly,
  even where they post-date 1987, but nothing outside the Plus.
- **MM3:** HTML with print styles, built to PDF (a headless browser if
  available here, else LaTeX), at the references' page size; sources in
  `docs/manual/`, the PDF with the release files.
- **MM4:** freely licensed lookalike fonts (a condensed Garamond-style
  face; a Chicago-style face for screen text), chosen after Phase 1.
- **MM5:** screen images made by a test-only build of MacMusic that draws
  each screen, dialog and alert offscreen and saves it, run under
  LaunchAPPL (no screen capture or GUI control); the owner captures
  anything that can't reach (menus pulled down, the Finder).
- **MM6:** a few period-style line drawings (the click wheel, the
  window's parts).
- **MM7:** the full manual (about 60-80 pages); a one-page
  quick-reference card last, if wanted.
- **MM8:** the owner approves the style guide, the sample spread, the
  outline, and each chapter.

## The own-use edition (2026-10-08)

- **MM1a, small Apple assets, for the owner's own copy only:** the
  manual stays our own work, but a few small assets from Apple's
  manuals give it the period polish: the cover's hand, mouse and cable
  (from MacDraw's cover, the sharpest scan of the four), and the rainbow
  logo. They are kept outside the repository, in
  `~/MacPlusProjects/manual-assets-private/` (with a README), and the
  build uses them only when that folder is there, so a plain edition
  (MacMusic's own icon, a drawn cover) can always be made. MM1's rule
  otherwise stands: no Apple name as publisher, no Apple copyright
  line; the copyright page is MacMusic's.
- **The cover's binding holes** are painted out (the owner's choice,
  2026-10-08): the cover shows a plain left edge, not the wire binding.
- **The cover (front matter, last to be made, O6):** MacDraw's
  composition, which fits the Mac Plus screen almost exactly: the
  rainbow logo top left, "MacMusic" in Garamond, MacMusic's Now Playing
  screen (S10) tinted pale blue where MacDraw's screen is, and the
  cable curving down to the hand on the mouse.

## The Macintosh Plus only, and *MacMusic Utilities* (2026-10-08)

- **MU1, one world:** the manual covers only MacMusic and what runs on
  the Macintosh Plus: System 6, the Finder, MultiFinder, desk
  accessories, floppy disks and an original SCSI hard disk. Nothing
  about modern computers, MP3s, converting songs, BlueSCSI or the
  project's own tools. The one stretch allowed is the hard disk's
  size: volumes up to System 6's 2 GB limit.
- **MU2, the companion book:** *MacMusic Utilities*, a separate, later
  manual, covers converting songs to MacMusic's sound files on a modern
  machine, moving files to the Mac, BlueSCSI and the project's tools.
  This manual names it wherever a reader would ask where songs come
  from. Not planned here.
- **MU3, the tutorial song:** "Song.wav" on the 800K MacMusic disk, a
  clip (30 seconds at most, to fit beside MacMusic and the Read Me) of
  "Chariots of Fire" by Vangelis; the manual may name it. *For the
  release files:* `tools/make-release.sh` will need to put Song.wav on
  the floppy, from the owner's clip. A commercial recording: such a
  floppy is for the owner's own use and isn't to be shared (the legal
  statement says MacMusic doesn't distribute copyrighted music).
- **MU4, the hard disk:** named as Apple's: "an Apple Hard Disk 20SC or
  other SCSI hard disk".
- **MU5, cross-references (provisional):** the companion book's title in
  italics, with a chapter name where useful: see "Converting Songs" in
  *MacMusic Utilities*.

## The title, and the sound output (2026-10-08)

- **MM9, the title:** the book is the *MacMusic User's Guide*, as
  Apple named the Macintosh's own guides. It is named so on the cover,
  the title page, the spine flap and the copyright page, and in
  *MacMusic Utilities*' references to it.
- **MU6, the sound output port:** the manual says that the
  Macintosh's sound output port, on the back of the computer, takes
  headphones, an external speaker (an amplified one) or a tape
  recorder, so MacMusic can be heard through them or recorded. The
  sound is mono only: the Macintosh has one sound channel, so MacMusic
  plays in mono too (stereo songs are mixed to mono when they're
  prepared, as *MacMusic Utilities* describes). Details of the port and
  what to plug into it: the *Macintosh Owner's Guide*. Where it goes:
  - Chapter 2, a task "To Listen Through Headphones or a Speaker" (and
    to record onto tape) on the Volume page;
  - Appendix B, "About Sound Files": mono, and why;
  - Appendix D, "No sound": check whether something is plugged into
    the sound output port;
  - Chapter 3's specifications: "Sound output: mono, through the
    built-in speaker or the sound output port".
- **MU6a, setting the level safely (2026-10-08):** connecting
  something to the port is explained as a short procedure, so that
  nothing is overloaded and nobody's ears get a surprise. MacMusic's
  volume *is* the Macintosh's own sound level (the Control Panel's
  speaker volume: the code sets the same hardware setting, eight steps),
  and MacMusic sets it again to the level it remembers each time it
  starts, so the manual says to turn it down in MacMusic, not only in
  the Control Panel.
  - **Headphones:** turn MacMusic's volume all the way down, plug in,
    put them on, start a song, then turn the click wheel up a step at a
    time.
  - **An amplified speaker or a stereo:** turn the speaker's (or
    amplifier's) volume down, and MacMusic's down, before connecting;
    connect to a line or auxiliary input; play a song; raise the
    speaker's volume to a comfortable level first, then set MacMusic's
    to suit. A higher MacMusic volume with a lower speaker volume
    usually sounds cleanest.
  - **A tape recorder:** connect to its line or auxiliary input, not
    the microphone input (which expects a much smaller signal and would
    distort); set the recorder to record-pause; play the song and set
    MacMusic's volume, then the recorder's recording level, so the
    level meters stay just short of the red at the loudest passages;
    then go back to the start of the song (Previous), start the
    recorder, and play. MacMusic's volume has only eight steps, so the
    fine adjustment is made on the recorder.
  - **Confirmed by the owner (2026-10-08):** plugging into the port
    silences the built-in speaker; the cable needs a mono 3.5 mm
    miniplug, with an adaptor for equipment with stereo inputs, so the
    sound reaches both channels. The *Macintosh Owner's Guide* is named
    for more about the port.

