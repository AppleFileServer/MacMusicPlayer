# Plan: the MacMusic manual, in the style of Apple's early Macintosh manuals

Status: approved (2026-10-08), decisions MM1-MM8 as recommended.
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
- **MM2:** written in the period's voice, describing the real product:
  its requirements (System 6.0.7, WAV files) stated plainly, even where
  they post-date 1987.
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
