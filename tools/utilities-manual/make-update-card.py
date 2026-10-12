#!/usr/bin/env python3
"""Writes docs/utilities-manual/update-card.html, the *MacMusic 1.1 Update*
card (docs/plans/manual-1.1-part-d.md, MD6-MD10): what's different in
MacMusic 1.1, for someone who knows 1.0, to go in the box with it. One
leaf, two pages the size of the manuals' (7.5 x 9 in), in the *Quick
Reference Card*'s style (make-card.py), after Apple's update sheets of
the time. Nothing here that the *MacMusic User's Guide* doesn't say; each
item gives the guide's page, from its last build (build-manual/
anchors.json: build the guide first, tools/manual/make-book.py).

The own-use edition (MD8) has the rainbow logo when the private assets
hold it (MANUAL_ASSETS, as the guide's); otherwise MacMusic's icon.

Build it with tools/utilities-manual/build-pdf.sh
docs/utilities-manual/update-card.html <pdf>."""
import importlib.util, json, os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, HERE)
from ub import page

spec = importlib.util.spec_from_file_location("make_card", os.path.join(HERE, "make-card.py"))
qrc = importlib.util.module_from_spec(spec)
spec.loader.exec_module(qrc)
card, h2, rows, k = qrc.card, qrc.h2, qrc.rows, qrc.k

OUT = os.path.join(ROOT, "docs", "utilities-manual", "update-card.html")
ANCHORS = os.path.join(ROOT, "build-manual", "anchors.json")
PRIVATE = os.environ.get("MANUAL_ASSETS",
                         os.path.expanduser("~/MacPlusProjects/manual-assets-private"))
OWN_USE = os.path.exists(os.path.join(PRIVATE, "rainbow-logo.png"))

AT = json.load(open(ANCHORS)) if os.path.exists(ANCHORS) else {}


def guide(ident):
    """"Guide, page n": the page the User's Guide prints for an anchor
    (its page_number: Chapter 1 starts on page 2)."""
    if ident not in AT or "chapter-1" not in AT:
        sys.exit("update card: no page for %s; build the User's Guide first" % ident)
    return "<i>Guide</i>, page %d." % (AT[ident] - AT["chapter-1"] + 2)


STYLE = qrc.STYLE + """<style>
.card .title .logo { height: 17pt; width: auto; vertical-align: -2pt; margin-right: 5pt; }
.card .intro { font: 9.5pt/12pt var(--serif); margin: 4pt 0 2pt 0; }
</style>
"""

LOGO = ('<img class="logo" src="private/rainbow-logo.png">' if OWN_USE
        else '<img class="logo" src="figures/mm-icon.png">')   # build-pdf.sh copies it

PAGE_1 = card(
    LOGO + "MacMusic 1.1 Update",
    "What&rsquo;s new since MacMusic 1.0",
    '<p class="intro">MacMusic 1.1 starts more quickly, prints your library, and imports '
    "several songs one after another. Everything else works as before.</p>",

    h2("Starting faster"),
    rows(("A note of your songs", "MacMusic keeps a note of each song in your library, so it "
          "needn&rsquo;t open every song file each time it starts. On a Macintosh Plus a library "
          "of 300 songs starts in about 13 seconds. " + guide("r-the-macmusic-library")),
         ("&ldquo;Loading the library&hellip;&rdquo;", "Shown under the Apple logo while "
          "MacMusic reads your library, each time it starts and after you import songs. "
          + guide("t-starting-up-takes-a-while")),
         ("The first start, and Rescan Library", "Read every song file, and take longer. "
          "MacMusic reads a song again by itself whenever its file has changed. "
          + guide("to-rescan-a-library"))),

    h2("Printing your library"),
    rows(("Print Library&hellip; (" + k("&#8984;", "P") + ")", "Prints a list of your songs, "
          "your albums (with their songs, if you like) or your artists (with their albums), "
          "with the library&rsquo;s name and the date at the top of each page. "
          + guide("to-print-a-list-of-your-library")),
         ("Page Setup&hellip;", "Chooses the paper: A4 to begin with, or US Letter. The "
          "first print asks for it. " + guide("to-choose-the-paper")),
         ("Printers", "Whichever the Chooser has: an ImageWriter or a LaserWriter, on the "
          "printer port or AppleTalk. The music pauses while it prints; " + k("&#8984;") + " "
          + k(".") + " stops. " + guide("printing-your-library"))),

    h2("Importing several songs"),
    rows(("&ldquo;Import another song?&rdquo;", "Asked after each song you import. Click "
          "Import Another to choose the next, or Done when you&rsquo;ve finished. "
          + guide("to-import-several-songs")),
         ("Each song", "Is in your library as soon as it&rsquo;s imported; the library is "
          "read once, at the end. Cancel in a song&rsquo;s details skips just that song.")),

    h2("Smaller changes"),
    rows(("Under MultiFinder", "MacMusic&rsquo;s window comes to the front as soon as it "
          "starts, instead of staying behind the Finder&rsquo;s while the library loads."),
         ("Create New Library", "The Save dialog box&rsquo;s prompt, &ldquo;New library "
          "in:&rdquo;, is shown whole.")),
    first=True,
)

PAGE_2 = card(
    "Moving up from MacMusic 1.0",
    "Your songs, playlists and settings come with you",
    h2("Installing MacMusic 1.1"),
    rows(("To install it", "Quit MacMusic. Drag the new MacMusic from its disk to where the "
          "old one is on your hard disk, and click OK when the Finder asks to replace it."),
         ("Your libraries", "Stay as they are: songs, playlists, ratings and the libraries "
          "MacMusic remembers. So do your settings."),
         ("The first start", "MacMusic reads every song in the library once, to make its "
          "note: an invisible folder called MacMusic Cache, inside the library. After that "
          "it starts more quickly. " + guide("r-the-macmusic-library")),
         ("Memory", "As 1.0: 640K under MultiFinder, at least 512K.")),

    h2("Going back to 1.0"),
    rows(("A library used by 1.1", "Opens in MacMusic 1.0 as before; 1.0 doesn&rsquo;t "
          "notice the MacMusic Cache folder."),
         ("Your song files", "Are the same in both versions, so songs can go back and forth "
          "between them.")),

    h2("What hasn&rsquo;t changed"),
    rows(("The iPod", "Its screens, the click wheel, Now Playing, Shuffle and Repeat, the "
          "Clock."),
         ("The library", "500 songs and 10 playlists at most; MacMusic WAV files, mono, "
          "22,050 Hz, 8-bit."),
         ("The utilities", "The MacMusic Converter, Transfer and Backup Utilities work with "
          "1.1 as with 1.0.")),

    '<p class="foot">Everything in MacMusic 1.1 is described in the <i>MacMusic User&rsquo;s '
    "Guide</i>, second edition (October 2026), and in brief in the MacMusic Read Me on its "
    "disk. &copy; 2026 Jack McAteer. Apple, Macintosh and Mac are trademarks of Apple Inc. "
    "For the owner&rsquo;s own use.</p>",
)

if __name__ == "__main__":
    open(OUT, "w").write(page("MacMusic 1.1 Update", STYLE + PAGE_1 + PAGE_2))
    print("wrote", os.path.normpath(OUT), "(%s)" % ("own use" if OWN_USE else "plain"))
