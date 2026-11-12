#!/usr/bin/env python3
"""Builds the whole MacMusic User's Guide (Phase 7; outline.md, style
guide §5, §11, §12): the front matter, Chapters 1 to 3 and the
appendixes (from make-chapter1.py ... make-appendixes.py), the glossary,
the index and the back matter, as docs/manual/book.html, printed to
build-manual/MacMusic-Users-Guide.pdf.

The contents and the index give page numbers, so the book is laid out
twice: the first time to find which page each part, task and command
fell on (print-pdf.py's anchors), the second with the numbers filled in.

The own-use edition (MM1a) uses the private cover and the rainbow logo
when ~/MacPlusProjects/manual-assets-private (MANUAL_ASSETS) holds them
(make-cover.py makes the cover); otherwise the plain edition is built,
with MacMusic's own icon.

Usage: make-book.py"""
import importlib.util, json, os, re, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from book import BOOK, img, page

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
OUT = os.path.join(ROOT, "docs", "manual", "book.html")
ANCHORS = os.path.join(ROOT, "build-manual", "anchors.json")
PRIVATE = os.environ.get("MANUAL_ASSETS",
                         os.path.expanduser("~/MacPlusProjects/manual-assets-private"))
OWN_USE = all(os.path.exists(os.path.join(PRIVATE, f))
              for f in ("cover-macmusic.jpg", "rainbow-logo.png"))
PDF = os.path.join(ROOT, "build-manual", "MacMusic-Users-Guide%s.pdf" % ("" if OWN_USE else "-plain"))
EDITION = "MacMusic 1.2 &middot; Third edition, November 2026"


def part(name):
    """A chapter's or the appendixes' body, from its generator."""
    path = os.path.join(HERE, "make-%s.py" % name)
    spec = importlib.util.spec_from_file_location(name.replace("-", "_"), path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module.PART


def logo(width):
    if OWN_USE:     # the rainbow logo is 314 x 295 pixels
        return ('<img class="logo" src="private/rainbow-logo.png" style="width:%spt; height:%.1fpt">'
                % (width, width * 295 / 314))
    return img("mm-icon", width, "logo")


def boxes(*items):
    return '<ul class="boxes">%s</ul>' % "".join("<li>%s</li>" % i for i in items)


def margin_head(title, ident):
    return '<div class="margin"><h2 class="margin-head" id="%s">%s</h2></div>' % (ident, title)


# ---- Front matter ---------------------------------------------------------

def cover():
    if OWN_USE:
        return '<div class="cover"><img src="private/cover-macmusic.jpg"></div>\n'
    return ('<div class="cover plain"><div class="cover-logo">%s</div>'
            '<p class="cover-title">%s</p>%s</div>\n'
            % (img("mm-icon", 40), BOOK, img("c0-cover-screen", 400)))


COPYRIGHT = """
<div class="copyright"><div class="copy-columns">
 <div class="copy-left">
  <p class="copy-head">Copyright</p>
  <p>This manual and the software it describes are copyrighted, with all
  rights reserved, except where otherwise stated. Copyright in MacMusic&rsquo;s
  software, documentation, artwork, application resources and file formats
  is reserved by the copyright holder.</p>
  <p>&copy; 2026 Jack McAteer.</p>
  <p>MacMusic is an independent project, and is not affiliated with,
  endorsed by, sponsored by or produced by Apple Inc.</p>
  <p>Apple, Macintosh, Mac and iPod are trademarks of Apple Inc. Other
  names, logos and trademarks belong to their respective owners.</p>
  <p>Program and manual by Jack McAteer.<br>
  Tested on a Macintosh Plus, with &ldquo;Chariots of Fire&rdquo; played
  rather more times than Vangelis intended.</p>
  <p>%s</p>
 </div>
 <div class="copy-right">
  <p class="copy-head">Disclaimer</p>
  <p>MacMusic is provided under the terms of its accompanying licence, and
  without any representation that it is suitable for a particular
  purpose.</p>
  <p>Because MacMusic is intended to run on vintage hardware and software,
  you should keep backup copies of your music and other important
  data.</p>
  <p class="caps">The developer accepts no responsibility for loss or
  corruption of user data, music files, library files, playlists, or other
  material arising from the use of the software, except to the extent that
  such liability cannot lawfully be excluded or limited.</p>
  <p>MacMusic doesn&rsquo;t provide or distribute music. You&rsquo;re
  responsible for having the right to copy, convert, store and play the
  music you use with it. The full legal statement is at the back of this
  manual, and under Settings, Legal.</p>
 </div>
</div></div>
""" % EDITION


def title_page():
    return ('<div class="title-page" id="title-page">'
            '<div class="title-logo">%s</div><p class="title-name">%s</p></div>\n'
            % (logo(36), BOOK))


ABOUT = "".join([
    '<div class="front"><div class="flow">',
    margin_head("About This Manual", "about"),
    "<p>MacMusic turns your Macintosh Plus into a music player. It keeps "
    "your songs in a library on your hard disk, plays them through the "
    "Macintosh&rsquo;s own speaker, and lets you find and play them with "
    "a click wheel and a small screen, as you would a pocket music "
    "player. This manual shows you how.</p>",

    '<div class="margin"><h3 class="margin-sub">What you need</h3></div>',
    "<p>To use MacMusic, you need:</p>",
    boxes("a Macintosh Plus with System 6.0.7;",
          "an Apple Hard Disk 20SC or other SCSI hard disk, for MacMusic "
          "and your library;",
          "under MultiFinder, 640K of memory for MacMusic (it works in "
          "512K);",
          "your music, as MacMusic&rsquo;s kind of sound file (see below)."),

    '<div class="margin"><h3 class="margin-sub">Other MacMusic manuals</h3></div>',
    "<p><i>MacMusic Utilities</i>, a separate manual, explains how to make "
    "your music into MacMusic&rsquo;s sound files and bring them to your "
    "Macintosh. This manual names it wherever you&rsquo;ll need it.</p>",

    '<div class="margin"><h3 class="margin-sub">What you should know</h3></div>',
    "<p>This manual assumes you know the basics of using your Macintosh: "
    "pointing, clicking and dragging with the mouse, choosing commands from "
    "menus, working with windows, and using disks. If you don&rsquo;t, "
    "start with the <i>Macintosh Owner&rsquo;s Guide</i> and its guided "
    "tour.</p>",

    '<div class="margin"><h3 class="margin-sub">How to use this manual</h3></div>',
    "<p>There are three ways in:</p>",
    boxes("<b>Learn it:</b> Chapter 1, &ldquo;Learning MacMusic,&rdquo; takes "
          "you from installing MacMusic to making your first playlist, in ten "
          "short lessons.",
          "<b>Look it up:</b> Chapter 2, &ldquo;Using MacMusic,&rdquo; shows "
          "how to do each task, one to a column. Chapter 3, &ldquo;MacMusic "
          "Reference,&rdquo; describes every screen, menu and command, and the "
          "appendixes list MacMusic&rsquo;s messages and what to do when "
          "something goes wrong.",
          "<b>Just try it:</b> nothing you do on MacMusic&rsquo;s screen can "
          "harm your songs, and MENU always takes you back."),
    "<p>The glossary explains the words this manual uses, and the index "
    "finds any subject.</p>",

    '<div class="margin"><h3 class="margin-sub">If you&rsquo;ve used a pocket music player</h3></div>',
    "<p>Much will feel familiar: the click wheel, the menus, Now Playing, "
    "playlists, Shuffle and Repeat all work as you&rsquo;d expect. A few "
    "things are different:</p>",
    boxes("You turn the click wheel with the mouse, not your thumb, and the "
          "keyboard works too.",
          "Your library is a folder on your hard disk. You add songs by "
          "importing them, with the File menu.",
          "The music comes from the Macintosh&rsquo;s own sound: mono, "
          "through its speaker or its sound output port.",
          "There&rsquo;s no battery to run down; the battery on the screen is "
          "just for show."),

    '<div class="margin"><h3 class="margin-sub">Conventions</h3></div>',
    '<p class="step">A step tells you what to do.</p>',
    "<p>The plain text after it tells you what happens.</p>",
    '<p class="grey">Grey text adds a remark you can skip.</p>',
    '<div class="type-this">Text to type</div>',
    "<p>Commands are named with their menus: &ldquo;Choose Import Song from "
    "the File menu.&rdquo; Keys are named as they&rsquo;re marked: the Return "
    "key, the Option key.</p>",
    "</div></div>\n",
])


# ---- The contents ----------------------------------------------------------

def book_contents(ch2_topics, ch3_sections):
    rows = [("about", "About This Manual", "part")]
    rows += [("chapter-1", "Chapter 1: Learning MacMusic", "ch"),
             ("intro", "Introduction: What Is MacMusic?", "sec"),
             ("lessons", "Lessons", "sec"), ("whatnow", "What Now?", "sec")]
    rows += [("chapter-2", "Chapter 2: Using MacMusic", "ch")]
    rows += [(a, t, "sec") for a, t in ch2_topics]
    rows += [("chapter-3", "Chapter 3: MacMusic Reference", "ch")]
    rows += [(a, t, "sec") for t, a in ch3_sections]
    rows += [("appendix-a", "Appendix A: Messages", "part"),
             ("appendix-b", "Appendix B: About Sound Files", "part"),
             ("appendix-c", "Appendix C: Keyboard and Click Wheel Summary", "part"),
             ("appendix-d", "Appendix D: Troubleshooting", "part"),
             ("glossary", "Glossary", "part"), ("index", "Index", "part"),
             ("legal", "Legal Statement", "part"),
             ("comments", "Tell Us What You Think", "part")]
    # (Row classes are prefixed: a class "chapter" would start a page.)
    items = "".join('<li class="toc-%s"><span class="t">%s</span><span class="pg">%s</span></li>'
                    % (cls, t, "{{%s}}" % a) for a, t, cls in rows)
    return ('<div class="front"><div class="flow">'
            '<div class="margin"><h2 class="contents-head" id="book-contents">Contents</h2></div>'
            '<ul class="toc book-toc">%s</ul></div></div>\n' % items)


# ---- Back matter ------------------------------------------------------------

GLOSSARY = [
    ("alert box", "A box that appears on the screen to give a warning or a "
     "message, with a button, such as OK, to put it away."),
    ("audiobook", "A song marked as an audiobook in its details. Audiobooks "
     "are listed only under Audiobooks, and never shuffled."),
    ("backlight", "In MacMusic, a novelty that turns MacMusic&rsquo;s screen "
     "light on dark."),
    ("centre button", "The round button in the middle of the click wheel. "
     "Clicking it chooses the highlighted item."),
    ("choose", "To pick a command from a menu, or an item on MacMusic&rsquo;s "
     "screen."),
    ("click", "To position the pointer on something, and press and quickly "
     "release the mouse button."),
    ("click wheel", "The wheel below MacMusic&rsquo;s screen. You turn it by "
     "dragging round it, and click its parts to choose, go back, play and "
     "skip."),
    ("Chooser", "The desk accessory, in the Apple menu, where you pick the "
     "printer: an ImageWriter, say, and the port it&rsquo;s connected to."),
    ("Clock", "In Extras: the time and date from the Macintosh&rsquo;s own "
     "clock."),
    ("close box", "The small box at the left of a window&rsquo;s title bar. "
     "Clicking MacMusic&rsquo;s close box quits MacMusic."),
    ("desk accessory", "A small application, such as the Calculator or the "
     "Alarm Clock, chosen from the Apple menu."),
    ("dialog box", "A box in which MacMusic asks for information or a "
     "choice, such as a song&rsquo;s details."),
    ("dimmed", "Shown in grey: a dimmed command or button can&rsquo;t be used "
     "just now."),
    ("drag", "To position the pointer, hold down the mouse button, move the "
     "mouse, and release the button."),
    ("export", "To give a song back from your library as a WAV file, which "
     "removes it from the library."),
    ("Finder", "The application that shows your disks, folders and files, and "
     "lets you start applications."),
    ("floppy disk", "A 3&frac12;-inch disk, such as the MacMusic disk, that "
     "holds 800K."),
    ("hard disk", "A disk that holds far more than a floppy disk, inside or "
     "attached to the Macintosh: the place for MacMusic and your library."),
    ("highlight", "To make an item stand out, white on black, to show that "
     "it&rsquo;s the one a click will choose."),
    ("hold", "To keep a click wheel button pressed for half a second or more. "
     "On Now Playing, holding Next or Previous fast-forwards or rewinds."),
    ("import", "To put a song into your library."),
    ("library marker", "The MacMusic Library Info file, which marks a folder as "
     "a MacMusic Library."),
    ("MacMusic Library", "The folder where MacMusic keeps your songs and "
     "playlists."),
    ("MacMusic window", "MacMusic&rsquo;s window, with its screen and its "
     "click wheel."),
    ("main menu", "The first list on MacMusic&rsquo;s screen: Music, Extras, "
     "Settings, Shuffle Songs and Backlight."),
    ("MENU", "The top of the click wheel. Clicking it goes back one screen."),
    ("menu bar", "The strip across the top of the Macintosh screen holding the "
     "Apple, File, Edit and Playlists menus."),
    ("MultiFinder", "The part of System 6 that lets several applications be "
     "open at once; MacMusic plays on behind them."),
    ("Now Playing", "The screen showing the song that&rsquo;s playing."),
    ("Page Setup", "The command, in the File menu, that sets the paper and "
     "which way up it goes, for the printer chosen in the Chooser."),
    ("playlist", "A list of songs from your library, in the order you choose."),
    ("pointer", "The arrow on the screen that moves as you move the mouse."),
    ("rating", "From one to five stars, given to a song on Now Playing."),
    ("Repeat", "A setting that plays one song, or a whole list, again and "
     "again."),
    ("rescan", "To have MacMusic read its library&rsquo;s folder again."),
    ("SCSI", "Small Computer System Interface: the port on the back of the "
     "Macintosh Plus that a hard disk connects to."),
    ("scroll bar", "A bar at the side of a list that shows there&rsquo;s more "
     "than fits on the screen."),
    ("scrubber", "A diamond on Now Playing&rsquo;s progress bar, moved with the "
     "click wheel to go to any place in a song."),
    ("Shuffle", "A setting that plays songs, or albums, in a random order."),
    ("song", "One piece of music in your library, with its details: title, "
     "artist, album and so on."),
    ("song file", "The file holding a song in your library: its WAV file, with "
     "MacMusic&rsquo;s details added."),
    ("sound output port", "The socket on the back of the Macintosh for "
     "headphones, a speaker or a tape recorder."),
    ("startup disk", "The disk holding the System Folder the Macintosh started "
     "from."),
    ("track number", "A song&rsquo;s place on its album; an album&rsquo;s songs "
     "are listed in track order."),
    ("WAV file", "A kind of sound file. MacMusic plays WAV files that are mono, "
     "with 22,254 or 22,050 8-bit samples a second."),
    ("window", "An area of the screen with a title bar, that shows a "
     "document, a disk&rsquo;s contents or, in MacMusic&rsquo;s case, the "
     "player."),
]


def glossary():
    entries = "".join('<div class="gloss"><p class="term">%s</p><p>%s</p></div>' % e
                      for e in GLOSSARY)
    return ('<div class="chapter page-break"><div class="appendix-title" id="glossary" '
            'data-foot="Glossary"><h1 class="name">Glossary</h1></div>'
            '<div class="flow">%s</div></div>\n' % entries)


# The index: each subject, and the ids it points to (its pages come from
# the first layout). A "see" entry points to another subject instead.
INDEX = [
    ("About MacMusic (command)", ["c-about-macmusic"]),
    ("About (Settings)", ["to-see-about", "c-about"]),
    ("Add Again", ["to-add-a-song-again"]),
    ("Add to (command)", ["l9", "to-add-a-song", "c-add-to-name"]),
    ("Alarm Clock", ["c-desk-accessories", "to-see-the-clock"]),
    ("alert boxes", "messages"),
    ("albums", ["to-find-an-album", "r-music"]),
    ("&ldquo;All&rdquo;", ["to-use-all"]),
    ("Apple menu", ["apple-menu"]),
    ("arrow keys", ["the-controls", "appendix-c"]),
    ("artists", ["to-find-an-artists-songs", "r-music"]),
    ("audiobooks", ["to-find-an-audiobook", "to-mark-a-song-as-an-audiobook", "r-songs"]),
    ("Backlight", ["to-turn-the-backlight-on-or-off"]),
    ("backup copy", ["l1"]),
    ("Calculator", "desk accessories"),
    ("centre button", ["l5", "the-controls"]),
    ("Choose Library", ["to-create-another-library", "to-open-an-existing-library",
                        "c-choose-library"]),
    ("click wheel", ["l5", "l6", "the-controls", "appendix-c"]),
    ("Clock", ["to-see-the-clock", "r-extras"]),
    ("close box", ["l5", "l10"]),
    ("composers", ["to-find-songs-by-genre-or-composer"]),
    ("Control Panel", ["c-desk-accessories"]),
    ("copying a song", ["to-keep-the-original", "to-import-from-a-floppy-disk-or-another-hard-disk"]),
    ("Delete Playlist", ["to-delete-a-playlist", "c-delete-name"]),
    ("desk accessories", ["to-use-desk-accessories", "c-desk-accessories"]),
    ("Edit menu", ["edit-menu"]),
    ("Edit Song", ["to-change-a-songs-details", "c-edit-song"]),
    ("error numbers", ["appendix-a"]),
    ("Export Song", ["to-export-remove-a-song", "c-export-song"]),
    ("Extras", ["r-extras"]),
    ("fast-forward", ["to-fast-forward-or-rewind"]),
    ("File menu", ["file-menu"]),
    ("floppy disks", ["l1", "to-import-from-a-floppy-disk-or-another-hard-disk", "a-how-much-room-music-takes"]),
    ("genres", ["to-find-songs-by-genre-or-composer"]),
    ("Get Info", ["to-change-macmusics-memory-size"]),
    ("hard disk", ["l2", "a-how-much-room-music-takes"]),
    ("headphones", ["to-listen-through-headphones"]),
    ("holds", ["the-controls"]),
    ("Import Song", ["l7", "to-import-a-song", "c-import-song"]),
    ("Import Another", ["to-import-several-songs", "appendix-a"]),
    ("importing several songs", ["to-import-several-songs", "c-import-song"]),
    ("installing MacMusic", ["l2"]),
    ("Just Copy", ["l7", "to-keep-the-original"]),
    ("keyboard", ["the-controls", "appendix-c"]),
    ("Legal", ["c-legal", "legal"]),
    ("library", "MacMusic Library"),
    ("Library Info", ["to-see-library-info", "c-library-info"]),
    ("MacMusic Library", ["intro", "l4", "r-the-macmusic-library", "using-more-than-one-library"]),
    ("MacMusic Library Info (file)", ["l4", "r-the-macmusic-library"]),
    ("MacMusic Prefs", ["r-the-macmusic-library"]),
    ("<i>MacMusic Utilities</i>", ["about", "l7", "a-where-songs-come-from"]),
    ("main menu", ["l6", "r-the-main-menu"]),
    ("memory", ["about", "to-see-how-much-memory-macmusic-uses", "to-change-macmusics-memory-size"]),
    ("MENU", ["l6", "to-go-back"]),
    ("messages", ["appendix-a"]),
    ("mono", ["headphones-speakers-and-tape", "a-the-files-macmusic-plays"]),
    ("MultiFinder", ["to-keep-playing-while-you-work", "to-see-how-much-memory-macmusic-uses"]),
    ("New Playlist", ["l9", "to-make-a-playlist", "c-new-playlist"]),
    ("Next", ["to-skip-to-the-next-song"]),
    ("Now Playing", ["l8", "what-now-playing-shows", "r-now-playing"]),
    ("pausing", ["l8", "to-pause-and-resume"]),
    ("playlists", ["intro", "l9", "working-with-playlists", "r-playlists"]),
    ("Playlists menu", ["playlists-menu"]),
    ("Previous", ["to-go-back-a-song-or-start-it-again"]),
    ("quitting", ["l10", "c-quit-q"]),
    ("rating a song", ["to-rate-a-song"]),
    ("recording onto tape", ["to-record-onto-tape"]),
    ("Remove from Playlist", ["to-remove-a-song-from-a-playlist", "c-remove-from-playlist"]),
    ("Rename Playlist", ["to-rename-a-playlist", "c-rename-name"]),
    ("Repeat", ["to-change-shuffle-or-repeat", "settings-in-detail"]),
    ("Rescan Library", ["to-rescan-a-library", "c-rescan-library"]),
    ("Page Setup", ["to-choose-the-paper", "c-page-setup"]),
    ("Print Library", ["to-print-a-list-of-your-library", "c-print-library-p"]),
    ("printing", ["printing-your-library", "a-printing", "t-nothing-prints"]),
    ("stopping printing", ["to-print-a-list-of-your-library"]),
    ("Chooser", ["printing-your-library", "a-printing"]),
    ("ImageWriter", "printing"),
    ("LaserWriter", "printing"),
    ("MacMusic Cache (folder)", ["r-the-macmusic-library", "t-song-details-look-wrong-after-changing-files-in-the-finder"]),
    ("Loading the library", ["l7", "to-rescan-a-library", "t-starting-up-takes-a-while"]),
    ("clock battery", ["t-the-macintosh-forgets-the-time"]),
    ("Reset All Settings", ["to-reset-all-settings", "c-reset-all-settings"]),
    ("rewind", ["to-fast-forward-or-rewind"]),
    ("scroll bar", ["to-scroll-a-list"]),
    ("scrubber", ["to-use-the-scrubber"]),
    ("Settings", ["settings", "settings-in-detail"]),
    ("Shuffle", ["shuffling", "settings-in-detail"]),
    ("Shuffle Songs", ["to-shuffle-every-song"]),
    ("Song.wav", ["l2", "l7"]),
    ("songs", ["r-songs", "to-find-a-song-by-its-title"]),
    ("sound files", ["appendix-b"]),
    ("sound output port", ["headphones-speakers-and-tape", "t-no-sound"]),
    ("speaker", ["to-play-through-a-speaker-or-stereo", "t-no-sound"]),
    ("specifications", ["specifications"]),
    ("starting MacMusic", ["l3"]),
    ("tape recorder", "recording onto tape"),
    ("troubleshooting", ["appendix-d"]),
    ("volume", ["l8", "to-change-the-volume", "headphones-speakers-and-tape"]),
    ("WAV files", ["to-import-a-song", "appendix-b"]),
]


def pages_text(ids, at):
    """The pages for some ids: sorted, each once, runs joined (12&ndash;13)."""
    nums = sorted({int(page_number(at[i], at)) for i in ids if i in at
                   and page_number(at[i], at).isdigit()})
    out, i = [], 0
    while i < len(nums):
        j = i
        while j + 1 < len(nums) and nums[j + 1] == nums[j] + 1:
            j += 1
        out.append(str(nums[i]) if i == j else "%d&ndash;%d" % (nums[i], nums[j]))
        i = j + 1
    return ", ".join(out) or "00"


def index(at):
    groups = {}
    for term, refs in INDEX:
        key = re.sub(r"<[^>]+>|&[a-z]+;", "", term)[0].upper()
        groups.setdefault(key, []).append((term, refs))
    html = []
    for letter in "ABCDEFGHIJKLMNOPQRSTUVWXYZ":
        html.append('<h3 class="index-letter">%s</h3>' % letter)
        for term, refs in sorted(groups.get(letter, []),
                                 key=lambda e: re.sub(r"<[^>]+>|&[a-z]+;", "", e[0]).lower()):
            if isinstance(refs, str):
                html.append('<p class="ix">%s. <i>See</i> %s</p>' % (term, refs))
            else:
                html.append('<p class="ix">%s, %s</p>' % (term, pages_text(refs, at)))
    return ('<div class="chapter page-break"><div class="appendix-title" id="index" '
            'data-foot="Index"><h1 class="name">Index</h1></div>'
            '<div class="index-columns">%s</div></div>\n' % "".join(html))


def legal():
    text = open(os.path.join(ROOT, "docs", "legal", "legal-statement.md")).read()
    out = []
    for block in text.strip().split("\n\n"):
        block = re.sub(r"\*\*(.+?)\*\*", r"<b>\1</b>", block.strip())
        block = block.replace("'", "&rsquo;")
        if block.startswith("## "):
            continue                    # the page has its own title
        elif block.startswith("### "):
            out.append('<h3 class="ref-head">%s</h3>' % block[4:])
        elif block == "---":
            continue
        else:
            out.append("<p>%s</p>" % block)
    return ('<div class="chapter page-break"><div class="appendix-title" id="legal" '
            'data-foot="Legal Statement"><h1 class="name">Legal Statement</h1></div>'
            '<div class="flow legal">%s</div></div>\n' % "".join(out))


COMMENTS = """
<div class="chapter page-break"><div class="appendix-title" id="comments"
 data-foot="Tell Us What You Think"><h1 class="name">Tell Us What You Think</h1></div>
<div class="flow">
 <p>This manual is meant to help you get the most from MacMusic. Your
 comments, and the problems you find, make the next edition better.</p>
 <div class="form">
  <p>Did MacMusic do what you expected?
   <span class="tick">&#9744; Always</span><span class="tick">&#9744; Usually</span><span class="tick">&#9744; Rarely</span></p>
  <p>Which parts of the manual did you use?
   <span class="tick">&#9744; Learning</span><span class="tick">&#9744; Using</span><span class="tick">&#9744; Reference</span><span class="tick">&#9744; Appendixes</span></p>
  <p>Was there anything you couldn&rsquo;t find?</p>
  <div class="lines"></div>
  <p>Did you find any mistakes? (Please give the page.)</p>
  <div class="lines"></div>
  <p>What would you like MacMusic to do next?</p>
  <div class="lines"></div>
 </div>
 <p class="grey">Please pass your comments to the MacMusic project, with
 your name if you&rsquo;d like a reply. Thank you.</p>
</div></div>
"""


def back_cover():
    return ('<div class="inside-back"></div>\n'
            '<div class="back-cover"><p class="publisher">MacMusic</p>'
            '<div class="back-logo">%s</div></div>\n' % logo(30))


# ---- The book ---------------------------------------------------------------

def book(at):
    ch1, ch2, ch3, app = part("chapter1"), part("chapter2"), part("chapter3"), part("appendixes")
    # The chapters' own ids, so the contents can list Chapter 2's topics and
    # Chapter 3's sections.
    topics = re.findall(r'<h2 class="topic" id="([^"]+)">(.*?)</h2>', ch2)
    spec = importlib.util.spec_from_file_location("c3", os.path.join(HERE, "make-chapter3.py"))
    c3 = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(c3)
    # Each chapter's first page, its opener, for the contents.
    ch1 = ch1.replace('<div class="opener opener-left">',
                      '<div class="opener opener-left" id="chapter-1">', 1)
    ch2 = ch2.replace('<div class="opener opener-left">',
                      '<div class="opener opener-left" id="chapter-2">', 1)
    ch3 = ch3.replace('<div class="opener opener-left">',
                      '<div class="opener opener-left" id="chapter-3">', 1)
    # The chapters' own contents pages: page numbers from the layout too.
    def own_contents(text):
        return re.sub(r'<ul class="toc">.*?</ul>', lambda m: re.sub(
            r'<a href="#([^"]+)">(.*?)</a>',
            r'<span class="toc-t">\2</span><span class="toc-pg">{{\1}}</span>', m.group(0)),
            text, flags=re.S)
    ch1, ch2, ch3 = own_contents(ch1), own_contents(ch2), own_contents(ch3)
    body = (cover() + COPYRIGHT + title_page()
            + book_contents(topics, c3.SECTIONS) + ABOUT
            + ch1 + ch2 + ch3 + app
            + glossary() + index(at) + legal() + COMMENTS + back_cover())

    # Page numbers: roman from the title page, arabic from Chapter 1's
    # opener (page 2, as the chapter drafts are numbered).
    body = re.sub(r"\{\{([a-z0-9-]+)\}\}",
                  lambda m: page_number(at[m.group(1)], at) if m.group(1) in at else "00", body)
    return body


FOOT_NAMES = [("chapter-1", "Chapter 1: Learning MacMusic"),
              ("chapter-2", "Chapter 2: Using MacMusic"),
              ("chapter-3", "Chapter 3: MacMusic Reference"),
              ("appendix-a", "Appendix A: Messages"),
              ("appendix-b", "Appendix B: About Sound Files"),
              ("appendix-c", "Appendix C: Keyboard and Click Wheel Summary"),
              ("appendix-d", "Appendix D: Troubleshooting"),
              ("glossary", "Glossary"), ("index", "Index"),
              ("legal", "Legal Statement"), ("comments", "Tell Us What You Think")]


def page_number(p, at):
    """The number printed on the p'th page: roman from the title page,
    arabic from Chapter 1's opener, which is page 2 (§6.5)."""
    if p >= at.get("chapter-1", 10 ** 6):
        return str(p - at["chapter-1"] + 2)
    return roman(p - at.get("title-page", 1) + 1)


def feet(at):
    """Each page's number, written into its running foot's number box
    (Paged.js's own page counter doesn't follow resets in its margin
    boxes)."""
    if not at:
        return ""
    # Where each part starts, and the name its left-hand feet carry.
    parts = sorted((at[i], name) for i, name in FOOT_NAMES if i in at)
    rules = []
    for p in range(at.get("title-page", 1), 400):
        n = page_number(p, at)
        name = ([nm for start, nm in parts if start <= p] or [""])[-1]
        left = n + ("\\2003\\2003 " + name if name and p >= at.get("chapter-1", 10 ** 6) else "")
        for box, text in (("bottom-left", left), ("bottom-right", n)):
            rules.append('.pagedjs_page[data-page-number="%d"] .pagedjs_margin-%s '
                         '> .pagedjs_margin-content::after { content: "%s" !important; }'
                         % (p, box, text))
    return "<style>%s</style>\n" % "\n".join(rules)


def roman(n):
    out = ""
    for value, letters in ((10, "x"), (9, "ix"), (5, "v"), (4, "iv"), (1, "i")):
        while n >= value:
            out += letters
            n -= value
    return out


def build(at, anchors=None):
    html = page(BOOK, book(at)).replace("</head>", feet(at) + "</head>", 1)
    open(OUT, "w").write(html)
    cmd = [os.path.join(HERE, "build-pdf.sh"), OUT, PDF] + ([anchors] if anchors else [])
    subprocess.run(cmd, check=True, cwd=ROOT)


if __name__ == "__main__":
    build({}, ANCHORS)                      # the first layout: where everything fell
    at = json.load(open(ANCHORS))
    for ident in [i for term, refs in INDEX if not isinstance(refs, str) for i in refs]:
        if ident not in at:
            print("index: no such id:", ident)
    build(at)                               # the second, with the page numbers
    print("edition:", "own use (MM1a)" if OWN_USE else "plain")
