# *MacMusic Utilities*: style addendum

The User's Guide's style guide (`docs/manual/style-guide.md`) applies
in full, except as listed here (UB2-UB4, `docs/plans/manual-utilities.md`).

## The world (replaces the style guide's MU1 rules)

- The reader uses two computers: **a Mac** (Apple silicon, macOS 26),
  where the utilities run, and **the Macintosh Plus** (System 6.0.7,
  a BlueSCSI), where MacMusic plays. The book always says which: "your
  Mac" for the modern one, "the Macintosh Plus" (or "the Plus" after
  the first mention in a section) for the old one.
- Modern words as macOS uses them: the Finder, the Applications folder,
  a window's sidebar, Control-click, the SD card, the USB drive. MP3s,
  YouTube and BlueSCSI are named freely.
- The programs are named in full the first time in a chapter (the
  MacMusic Converter Utility), then "the Converter", "the Transfer
  Utility", "the Backup Utility".
- The User's Guide is named in italics: *MacMusic User's Guide*, with a
  chapter where useful.
- Still never "iPod"; British spelling; the plain Apple voice.

## Rights (the Converter's own rule)

Wherever YouTube is described, the book repeats the program's own rule:
only videos you own or have permission to save.

## Figures

- Screens are the owner's macOS screenshots, cropped to the window (or
  the dialog or menu), converted to greyscale, printed at a size where
  the window's text is readable (the window about 4 inches wide).
- **Rounded corners** (the owner's note, 2026-10-09): macOS windows
  have rounded corners, so a crop to the window's rectangle leaves the
  desktop showing in its four corners. Each crop is cut to the window's
  frame without its shadow, and the corners outside the rounded frame
  are painted the page's white, so the window prints with its own
  rounded shape and nothing of the desktop. The same for dialogs, alerts
  and menus (a menu's corners are rounded too). The crop script checks
  each corner for desktop colours left behind.
  As made (Phase 2): macOS's corners are smooth curves, not circular
  arcs, so the mask follows their measured shape
  (`tools/utilities-manual/make-figures.py`, CORNER_36), and each figure
  gets a grey outline 2 pixels wide just inside its frame: a white
  window needs an edge on a white page. A menu keeps its name's
  highlight in the menu bar above it. Menus are translucent in macOS 26,
  so a faint blur of what was behind them shows through, as on screen.
- No torn edges for crops of modern windows: a partial window is cut
  square, with a thin grey rule, since macOS windows have no 1-bit
  pattern to tear.
- Callouts as the User's Guide's (style guide §8.2), red leaders.
- Screens of the Macintosh Plus (the disk mounted, Import Song) come
  from Mini vMac at 1x, as the User's Guide's, through its existing
  figure tools.

## Structure within a program's chapter

Opener spread; the chapter's contents; Introduction; a lesson or two
(numbered within the chapter, as Chapter 1's of the User's Guide); task
pages (style guide §9); the reference section (style guide §10): the
window's parts with callouts, each control, the menus, Advanced.
