#!/usr/bin/env python3
"""Writes docs/utilities-manual/update-card-1.2.html, the *MacMusic 1.2
Update* card (docs/plans/release-1.2.md, step E3; MD10 of
docs/plans/manual-1.1-part-d.md: each version with changes a user would
notice gets its own card): what's different in MacMusic 1.2, for someone
who knows 1.1, to go in the box with it. Made as 1.1's card is
(make-update-card.py, which stays as it was, for 1.1). One
leaf, two pages the size of the manuals' (7.5 x 9 in), in the *Quick
Reference Card*'s style (make-card.py), after Apple's update sheets of
the time. Nothing here that the *MacMusic User's Guide* doesn't say; each
item gives the guide's page, from its last build (build-manual/
anchors.json: build the guide first, tools/manual/make-book.py).

The own-use edition (MD8) has the rainbow logo when the private assets
hold it (MANUAL_ASSETS, as the guide's); otherwise MacMusic's icon.

Build it with tools/utilities-manual/build-pdf.sh
docs/utilities-manual/update-card-1.2.html <pdf>."""
import importlib.util, json, os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, HERE)
from ub import page

spec = importlib.util.spec_from_file_location("make_card", os.path.join(HERE, "make-card.py"))
qrc = importlib.util.module_from_spec(spec)
spec.loader.exec_module(qrc)
card, h2, rows, k = qrc.card, qrc.h2, qrc.rows, qrc.k

OUT = os.path.join(ROOT, "docs", "utilities-manual", "update-card-1.2.html")
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
    LOGO + "MacMusic 1.2 Update",
    "What&rsquo;s new since MacMusic 1.1",
    '<p class="intro">MacMusic 1.2 plays songs at the Macintosh Plus&rsquo;s own rate, '
    "lists an artist&rsquo;s or a genre&rsquo;s songs album by album, and makes the song "
    "details window easier to click. Everything else works as before.</p>",

    h2("Songs at their true speed"),
    rows(("22,254 Hz songs", "MacMusic now plays WAV files made at 22,254 samples a second, "
          "the Plus&rsquo;s own rate, as well as 22,050. They play at their true speed and "
          "pitch. " + guide("a-the-files-macmusic-plays")),
         ("The MacMusic Converter 1.1", "Makes its MacMusic WAVs at 22,254, and brings "
          "every song to the same loudness (<i>MacMusic Utilities</i>, Chapter 2)."),
         ("Songs at 22,050", "Play as before, about 0.9% fast; the times shown are right. "
          "To have one at its true speed, convert it again and import it. "
          + guide("t-songs-play-a-little-fast")),
         ("Importing", "Import Song takes either rate. A WAV of any other kind is refused, "
          "with a message saying what&rsquo;s needed. " + guide("to-import-a-song"))),

    h2("All, album by album"),
    rows(("All in an Albums list", "Gives the songs album by album, each album in track "
          "order, as you&rsquo;d play the records. That&rsquo;s All in Albums, and All in "
          "an artist&rsquo;s or a genre&rsquo;s albums. " + guide("to-use-all")),
         ("Songs", "Music &gt; Songs still lists every song by title.")),

    h2("Smaller changes"),
    rows(("The song details window", "A click anywhere inside a box&rsquo;s frame now "
          "selects that box, even right at its edge. Before, a click near the frame "
          "missed, and the typing stayed in the last box. "
          + guide("to-change-a-songs-details")),
         ("Edit Song and ratings", "Keep each song&rsquo;s rate when they save its "
          "details.")),
    first=True,
)

PAGE_2 = card(
    "Moving up from MacMusic 1.1",
    "Your songs, playlists and settings come with you",
    h2("Installing MacMusic 1.2"),
    rows(("To install it", "Quit MacMusic. Drag the new MacMusic from its disk to where the "
          "old one is on your hard disk, and click OK when the Finder asks to replace it."),
         ("Your libraries", "Stay as they are: songs, playlists, ratings and the libraries "
          "MacMusic remembers. So do your settings."),
         ("The first start", "As quick as before: MacMusic 1.2 uses 1.1&rsquo;s note of your "
          "songs. " + guide("r-the-macmusic-library")),
         ("Memory", "As 1.1: 640K under MultiFinder, at least 512K.")),

    h2("Going back to 1.1 or 1.0"),
    rows(("Songs at 22,254 Hz", "MacMusic 1.0 and 1.1 can&rsquo;t play them: they leave them "
          "out, saying some files weren&rsquo;t recognised. The files aren&rsquo;t changed, "
          "and the songs are back when 1.2 opens the library again. "
          + guide("t-an-earlier-macmusic-leaves-songs-out")),
         ("The first start in 1.1", "Reads every song file once, as 1.1 doesn&rsquo;t "
          "know 1.2&rsquo;s note of your songs."),
         ("Songs at 22,050 Hz", "Are the same in every version, so they can go back and "
          "forth.")),

    h2("What hasn&rsquo;t changed"),
    rows(("The iPod", "Its screens, the click wheel, Now Playing, Shuffle and Repeat, the "
          "Clock."),
         ("The library", "500 songs and 10 playlists at most; printing, as in 1.1."),
         ("The utilities", "The Converter 1.1 makes songs for 1.2. The Transfer Utility 1.0 "
          "takes only 22,050 Hz songs for now.")),

    '<p class="foot">Everything in MacMusic 1.2 is described in the <i>MacMusic User&rsquo;s '
    "Guide</i>, third edition (November 2026), and in brief in the MacMusic Read Me on its "
    "disk. &copy; 2026 Jack McAteer. Apple, Macintosh and Mac are trademarks of Apple Inc. "
    "For the owner&rsquo;s own use.</p>",
)

if __name__ == "__main__":
    open(OUT, "w").write(page("MacMusic 1.2 Update", STYLE + PAGE_1 + PAGE_2))
    print("wrote", os.path.normpath(OUT), "(%s)" % ("own use" if OWN_USE else "plain"))
