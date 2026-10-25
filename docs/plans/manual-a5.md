# An A5 edition of the MacMusic User's Guide: plan

Status: **not pursued** (the owner, 2026-10-25). The original 7½ x 9 in
manuals stay the standard. Both ways were proofed first, (a) reduced
(92 pages) and (b) re-laid (116 pages), in the scratchpad, without
changing any existing file; the release's files were checked unchanged
against its `SHA256SUMS`. The plan is kept in case A5 is wanted later.
Decisions are numbered (A5-1 and so on); **(R)** marks a recommendation.

How the proofs were made, for a later start: `make-book.py` loaded as a
module and its `book()` and `feet()` called with an extra stylesheet;
(a) printed by a copy of `print-pdf.py` that, after Paged.js's layout,
sets an A5 `@page` and `zoom: 0.777` on each page; (b) by a stylesheet
after `manual.css` (A5 page, 84 pt margin column, 11.5 pt text, task
columns as two inline-flex columns 525 pt high, pictures and labelled
SVGs capped at the width), laid out twice by `print-pdf.py` unchanged.
What (b) still needed: task text touching the column rules, many task
pages two-thirds full, the labels (then 6.7 pt) out of line with the
margin column.

The owner's condition: **no file that already exists is changed.** The
A5 edition is made only by new files, which read the existing ones and
never write over them, and it's built into a folder of its own.

## Where the guide is now

- *MacMusic User's Guide*, second edition: 92 pages of 7½ x 9 in
  (540 x 648 pt), in an own-use and a plain edition. It's made by
  `tools/manual/make-book.py` (with `build-pdf.sh`, `print-pdf.py`,
  `docs/manual/manual.css`): Paged.js in headless Chrome, laid out
  twice so the contents and the index get their page numbers.
- A5 is 148 x 210 mm (419.5 x 595.3 pt): **22% narrower but only 8%
  shorter** than the guide's page, a taller, thinner shape (0.71 against
  0.83).
- What the page's shape is built into:
  - the margin column (118 pt), the gutter and the margins, so a text
    column of 308 pt;
  - 20 task pages, each four columns in a fixed 556 pt high grid;
  - figures up to 448 pt wide (the full text area), most 240 pt;
  - the cover and the chapter openers, which bleed off every edge;
  - layout fitted by hand (for example the two tasks merged in 1.1 so a
    page didn't overflow).

## The decisions

- **A5-1. Which documents.** **(R)** The *User's Guide* only, both
  editions. The *Quick Reference Card*, the *1.1 Update Card* and
  *MacMusic Utilities* can follow the same way later if wanted.

- **A5-2. How the pages become A5.** The main choice.
  - **(a) Reduced: the same pages, at 78%.** Every page is laid out
    exactly as now and printed onto A5 at 77.7%, the scale that fits the
    width (540 pt to 419.5 pt). The result:
    - the same 92 pages, page breaks, contents and index;
    - the Update Card's page references stay right;
    - body text 10.1 pt instead of 13, as in a paperback; the task
      columns 8.5 pt, small but readable; figures at 78% (a Mac pixel
      0.68 pt);
    - 92 pt of A5's height left over on each page (A5-3).
    - Quick to make and little to go wrong: no page needs checking for
      breaks.
  - **(b) Re-laid: a true A5 design.** A new stylesheet, loaded after
    `manual.css`, overrides it:
    - the A5 page and margins; the margin column narrowed to about 80 pt;
      body text about 11.5 pt;
    - the task pages in two columns instead of four, so each becomes
      two pages;
    - figures capped at the text width; the openers and cover cropped to
      A5.
    - Paged.js lays the book out afresh, with new contents and index
      numbers: probably 120-140 pages.
    - Every page then needs checking for bad breaks. Since the
      generators can't change, a break is fixed only through the new
      stylesheet or by rewriting the generated HTML in the new build
      script, which is fragile. Some of the hand fitting won't survive.
    - The Update Card's "Guide, page n" then fits only the 7½ x 9 in
      guide.

  **(R) (a).** For own use it gives a good A5 book for a fraction of
  the work, with nothing to go wrong in the layout. **Step 1 of the
  Order is a proof:** a few sample pages (a lesson, a task page, a
  reference page, an appendix message) printed on paper. If the 8.5 pt
  task text is too small in your hands, we go to (b), or to (a) with the
  task pages alone re-laid (an extra decision then).

- **A5-3. With (a), where the page sits on the A5 sheet.** The reduced
  page is 419.5 x 503.5 pt, leaving 92 pt of height.
  - **(R)** Text pages: at the top, with the 92 pt added below, so the
    running feet sit above a deeper bottom margin and the text block
    stays where the eye expects it. (Centring is the alternative.)
  - **(R)** The cover, the chapter openers and the back cover fill the
    whole A5 page. They are pictures with `object-fit: cover`, so they
    can be laid out at the A5 shape (766 pt high before reduction) and
    cropped at the sides. The own-use cover's crop is checked, so its
    title isn't cut. If it is, the cover gets a white band instead.

- **A5-4. How it's built** (new files only):
  - `tools/manual-a5/make-book-a5.py` loads `tools/manual/make-book.py`
    as a module, as make-book loads its chapters, and points its output
    names (`OUT`, `ANCHORS`, `PDF`) and its PDF step at the A5 build
    folder. The book's text and order come from the existing generators
    unchanged, so the A5 edition always matches the 7½ x 9 in one.
  - `tools/manual-a5/build-pdf-a5.sh`: `build-pdf.sh`'s copying, into
    the A5 folder.
  - `tools/manual-a5/print-pdf-a5.py`: `print-pdf.py`'s printing, with
    the paper set to A5 and the 77.7% scale (Chrome's own print
    settings), and the page boxes made A5's shape so no page drifts
    across a sheet. Nothing new to install: Chrome does the reduction.
  - `docs/manual/a5.css` (A5-3's placing; with (b), the whole A5
    design).
  - Both editions as now: the own-use one when the private assets are
    there, the plain one with `MANUAL_ASSETS=/nonexistent`.

- **A5-5. Where it's built.** **(R)** `build-manual/a5/`, inside the
  folder `.gitignore` already ignores (a folder of its own would mean
  changing `.gitignore`). It never touches `docs/manual/book.html`,
  `build-manual/anchors.json` or the existing PDFs, so the 7½ x 9 in
  guide and the Update Card's page numbers are untouched.
  The PDFs: `MacMusic-Users-Guide-A5.pdf` and
  `MacMusic-Users-Guide-A5-plain.pdf`.

- **A5-6. The release folder.** Adding them to
  `releases/MacMusic-1.1/` would change its `SHA256SUMS`.
  **(R)** A new folder, `releases/MacMusic-1.1-A5/`, with the two PDFs
  and a `SHA256SUMS` of its own.

- **A5-7. Printing.** **(R)** Plain A5 pages, for A5 paper or a print
  shop. A booklet for A4 paper (two A5 pages to each side, imposed to
  fold and staple) is possible later; at 92 pages it's 23 sheets, which
  folds but needs a long-reach stapler.

- **A5-8. The documents that describe the build.** `README.md`,
  `RESTART.md`, `CHANGELOG.md`, `docs/environment.md` and the manual's
  notes all exist, so under the condition they stay as they are. This
  plan, kept in `docs/plans/`, records how the A5 edition is made.
  **(R)** Once it's done, you may want to lift the condition for one
  line in `RESTART.md` pointing here; otherwise the A5 edition is found
  only through this plan.

## Order

1. The proof (A5-2): the scripts of A5-4 with A5-3's placing; a few
   sample pages as A5 for you to print and judge. Your decision on (a)
   or (b).
2. The whole book in both editions (with (b): the pages checked one by
   one, and the breaks fixed).
3. I check every page against the 7½ x 9 in guide: nothing cut off, the
   feet and numbers in place, the cover and openers filling the page.
4. Your review of the PDFs; corrections.
5. With your approval: commit the new files, and make the release folder
   (A5-6).
