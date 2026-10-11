#!/usr/bin/env python3
"""Writes docs/manual/chapter3.html, Chapter 3 of the MacMusic User's
Guide, "MacMusic Reference" (Phase 6; outline.md; style guide §10):
MacMusic's elements, its screens and controls, every menu and command,
the settings, the specifications and the preset values. The pages use
the margin column, as Chapter 1 does: section heads and command names in
the margin, the text beside them.

Build it with tools/manual/build-pdf.sh docs/manual/chapter3.html <pdf>."""
import os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from book import img, draft_title, opener, page, anchor

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "..", "docs", "manual", "chapter3.html")

SECTIONS = []           # (title, anchor), for the contents


def section(title):
    a = anchor(title)
    SECTIONS.append((title, a))
    return '<div class="margin"><h2 class="margin-head" id="%s">%s</h2></div>' % (a, title)


def sub(title):
    """A red-square subheading in the text column (§10)."""
    return '<h3 class="ref-head" id="r-%s">%s</h3>' % (anchor(title), title)


def cmd(name, *paras):
    """A command: its name in the margin, what it does beside it."""
    return ('<div class="entry"><div class="margin"><h4 class="cmd" id="c-%s">%s</h4></div>'
            % (anchor(name), name)
            + "".join("<p>%s</p>" % p for p in paras) + "</div>")


def boxes(*items):
    return '<ul class="boxes">%s</ul>' % "".join("<li>%s</li>" % i for i in items)


def keep(*parts):
    """Parts kept together on one page."""
    return '<div class="keep">%s</div>' % "".join(parts)


def table(head, *rows, cls="ref"):
    h = "".join("<th>%s</th>" % c for c in head)
    r = "".join("<tr>%s</tr>" % "".join("<td>%s</td>" % c for c in row) for row in rows)
    return '<table class="%s"><thead><tr>%s</tr></thead><tbody>%s</tbody></table>' % (cls, h, r)


def states(*figs):
    """Small screens in a grid, each with a caption below it."""
    return ('<figure class="states">%s</figure>'
            % "".join('<div>%s<span>%s</span></div>' % (img(n, 100), c) for n, c in figs))


BODY = "".join([
    section("Elements of MacMusic"),
    "<p>MacMusic works with three things: a library, the songs in it, "
    "and playlists. This section describes each, and its limits.</p>",

    sub("The MacMusic Library"),
    '<figure class="beside">%s</figure>' % img("c1-library", 150),
    "<p>A <b>MacMusic Library</b> is a folder, on any disk, that holds "
    "your songs and playlists. MacMusic uses one library at a time; you "
    "can have as many as you like, and switch between them with the "
    "File menu. Inside the folder are:</p>",
    boxes("<b>MacMusic Library Info,</b> a small file that marks the "
          "folder as a library. To open a library, you open this file.",
          "<b>Playlists,</b> a folder holding a text file for each "
          "playlist.",
          "a <b>song file</b> for each song, named by its number."),
    "<p>Each song is given a number of its own when it&rsquo;s imported. "
    "A number is never used twice in the same library, even after its "
    "song has been exported.</p>",
    "<p>MacMusic reads the whole library each time it starts, and when "
    "you choose Rescan Library. For a library of 500 songs, that takes "
    "between a minute and a half and two minutes.</p>",
    '<p class="grey">Only MacMusic should change a library&rsquo;s files. A '
    "song file copied in from another library may have the same number "
    "as one already there; two songs with the same number can&rsquo;t "
    "both be used, and MacMusic tells you which one it has left out.</p>",
    "<p>MacMusic keeps its preferences (the library in use, the "
    "libraries you&rsquo;ve used before, the volume and the settings) in a "
    "file called MacMusic Prefs, in the System Folder.</p>",

    sub("Songs"),
    "<p>A song file is the WAV file you imported, unchanged, with "
    "MacMusic&rsquo;s details for it added at the end. That&rsquo;s why "
    "importing can simply move a file into the library, and why "
    "exporting gives back exactly the file you imported. Each song "
    "has:</p>",
    boxes("a title, which every song must have;",
          "an artist, an album, a genre and a composer, any of which can "
          "be left empty;",
          "a track number, from 1 to 9,999, or none;",
          "the Audiobook box, checked or not;",
          "a rating: none, or one to five stars."),
    "<p>The title and the other details can be up to 63 characters "
    "each. An <b>audiobook</b> is a song with its Audiobook box checked: "
    "it&rsquo;s listed only under Audiobooks, and never shuffled.</p>",

    sub("Playlists"),
    "<p>A playlist is a text file in the library&rsquo;s Playlists folder, "
    "named after the playlist. Each line lists one song, by its number "
    "and, for anyone reading the file, its title; MacMusic reads only "
    "the number. A song can be on a playlist more than once. Exporting a "
    "song takes it off every playlist.</p>",
    boxes("A library can have as many as 10 playlists.",
          "A playlist&rsquo;s name can be up to 31 characters, and "
          "can&rsquo;t contain a colon (:).",
          "MacMusic reads as many as 1,000 songs from one playlist."),

    section("MacMusic&rsquo;s Screens"),
    "<p>MacMusic&rsquo;s screen shows one list at a time. A <b>&gt;</b> at "
    "the right of an item means it leads to another list; a scroll bar "
    "at the right means the list is longer than the screen.</p>",

    sub("The Main Menu"),
    '<figure class="beside">%s</figure>' % img("c1-wheel-1", 100),
    boxes("<b>Music</b> leads to everything in your library.",
          "<b>Extras</b> holds the Clock.",
          "<b>Settings</b> holds Shuffle, Repeat and the others; see "
          "&ldquo;Settings in Detail.&rdquo;",
          "<b>Shuffle Songs</b> plays every song but the audiobooks, in a "
          "random order.",
          "<b>Backlight</b> turns the screen light on dark, and back.",
          "<b>Now Playing</b> appears at the end of the menu while a song "
          "is playing or paused."),

    sub("Music"),
    '<figure class="beside">%s</figure>' % img("c1-music", 100),
    table(("Item", "Leads to"),
          ("Playlists", "your playlists, then a playlist&rsquo;s songs, in "
           "their order"),
          ("Artists", "All and the artists, then albums, then songs"),
          ("Albums", "All and the albums, then songs, in track order"),
          ("Songs", "every song, by title (a leading &ldquo;The&rdquo; "
           "ignored)"),
          ("Genres", "All and the genres, then artists, albums and songs"),
          ("Composers", "the composers, then their songs"),
          ("Audiobooks", "the audiobooks"), cls="ref narrow"),

    sub("Extras"),
    "<p>Extras holds the <b>Clock</b>, which shows the time and date from "
    "the Macintosh&rsquo;s own clock, kept up to the minute.</p>",

    sub("Now Playing"),
    "<p>Now Playing shows the song that&rsquo;s playing: where it is in "
    "the list, its title, artist and album, a bar showing how much has "
    "played, and the time played and the time left. At the top left, an "
    "icon shows whether it&rsquo;s playing, paused, or moving fast; at the "
    "top right, the Shuffle and Repeat icons appear when they&rsquo;re on "
    "(the Repeat icon is the same for One and All).</p>",
    states(("c2-now-playing", "Playing"), ("c2-volume", "The volume bar"),
           ("c2-scrubber", "The scrubber"), ("c2-rating", "The rating"),
           ("c2-paused", "Paused"), ("c2-ff", "Fast-forward")),
    "<p>On Now Playing, turning the click wheel changes the volume, and "
    "the volume bar shows for two seconds. Clicking the centre button "
    "steps through the scrubber, then the rating, then back to the "
    "progress bar; the scrubber and the rating go away by themselves "
    "after five seconds.</p>",

    keep(section("The Controls"),
    "<p>You can work the click wheel with the mouse, or use the keyboard "
    "instead:</p>",
    table(("To", "With the mouse", "With the keyboard"),
          ("Move up or down a list; change the volume",
           "Drag round the click wheel", "Up Arrow, Down Arrow"),
          ("Choose (Select)", "Click the centre button", "Return or Enter"),
          ("Go back (MENU)", "Click MENU", "Delete"),
          ("Play or pause", "Click the bottom of the wheel", "Space bar"),
          ("Next or previous song", "Click the right or left of the wheel",
           "Right Arrow, Left Arrow"),
          ("Fast-forward or rewind", "Hold down the right or left of the "
           "wheel", "Option-Right Arrow, Option-Left Arrow, held down"),
          cls="ref controls")),
    "<p>On Now Playing, the Down Arrow key makes the music louder and the "
    "Up Arrow key softer, a step at a time. A hold starts after half a "
    "second without moving. Holds work only on Now Playing; elsewhere, "
    "holding a button down is the same as clicking it.</p>",

    section("Apple Menu"),
    "<p>The Apple menu holds About MacMusic and your desk accessories.</p>",
    cmd("About MacMusic&hellip;",
        "Shows MacMusic&rsquo;s version and copyright. Click OK to put it "
        "away."),
    '<figure class="dialog">%s</figure>' % img("c3-about", 237),
    cmd("Desk accessories",
        "The music plays on while you use any desk accessory. Two are "
        "particularly useful with MacMusic:"),
    boxes("<b>Alarm Clock</b> sets the Macintosh&rsquo;s clock, which "
          "MacMusic&rsquo;s Clock shows.",
          "<b>Control Panel</b> sets the speaker volume, which is "
          "MacMusic&rsquo;s volume too. When MacMusic starts, it sets the "
          "volume to the level you last chose in MacMusic."),

    section("File Menu"),
    "<p>The File menu&rsquo;s commands look after your library: what goes "
    "into it, what comes out, and which library you&rsquo;re using.</p>",
    '<figure class="beside">%s</figure>' % img("c2-export-menu", 170),
    cmd("Import Song&hellip;",
        "Asks for a WAV file and imports it into the library; see "
        "&ldquo;Adding Songs&rdquo; in Chapter 2. Dimmed when no library is "
        "open."),
    cmd("Export Song&hellip;",
        "Gives the highlighted song back as its original WAV file, and "
        "removes it from the library and from every playlist. Dimmed "
        "unless a song is highlighted."),
    cmd("Choose Library&hellip;",
        "Asks whether to use an existing library or create a new one, "
        "and switches to it."),
    cmd("Rescan Library",
        "Reads every song file in the library again, and the library&rsquo;s "
        "note of them is made afresh. Dimmed when no library is open."),
    cmd("Library Info&hellip;",
        "Shows how many songs the library holds (out of 500) and how many "
        "playlists, how much room the songs take, how much is free on its "
        "disk, and how many files couldn&rsquo;t be read."),
    '<figure class="dialog">%s</figure>' % img("c2-library-info", 193),
    cmd("Page Setup&hellip;",
        "The paper, and which way up, for the printer chosen in the "
        "Chooser. MacMusic remembers it. The dialog is the printer&rsquo;s "
        "own: the ImageWriter&rsquo;s is shown."),
    '<figure class="dialog">%s</figure>' % img("c2-page-setup", 300),
    cmd("Print Library&hellip; &#8984;P",
        "Asks which lists to print (Songs; Albums, with each album&rsquo;s "
        "songs or not; Artists, with each artist&rsquo;s albums or not), "
        "then shows the printer&rsquo;s Print dialog. The first time, Page "
        "Setup comes first. The music pauses while it prints. Dimmed when "
        "the library has no songs."),
    cmd("Libraries used before",
        "Up to five, with the one in use checked. Choose one to switch to "
        "it. A library whose disk isn&rsquo;t there is dimmed."),
    cmd("Quit &#8984;Q",
        "Stops the music and quits MacMusic. MacMusic remembers the "
        "library, the volume and the settings for next time."),

    section("Edit Menu"),
    "<p>The Edit menu is where you change a song&rsquo;s details.</p>",
    '<figure class="beside">%s</figure>' % img("c2-edit-menu", 85),
    cmd("Undo, Cut, Copy, Paste, Clear",
        "MacMusic has nothing to cut or paste, so these are dimmed."),
    cmd("Edit Song&hellip;",
        "Shows the details of the highlighted song, or of the song on Now "
        "Playing, for you to change. Save keeps you on the same screen. "
        "Dimmed unless a song is highlighted or playing."),

    section("Playlists Menu"),
    "<p>Everything to do with playlists is in the Playlists menu. Its "
    "commands act on the playlist or song highlighted on MacMusic&rsquo;s "
    "screen.</p>",
    '<figure class="beside">%s</figure>' % img("c1-add-to", 129),
    cmd("New Playlist&hellip;",
        "Asks for a name (&ldquo;Untitled Playlist&rdquo; is suggested) and "
        "makes an empty playlist. If you already have ten, MacMusic says "
        "so instead."),
    cmd("Rename &ldquo;<i>name</i>&rdquo;&hellip;",
        "Renames the playlist highlighted in the Playlists list, or the "
        "one you&rsquo;re in. Otherwise it reads Rename Playlist, and is "
        "dimmed."),
    cmd("Delete &ldquo;<i>name</i>&rdquo;&hellip;",
        "Deletes that playlist, after asking you; Cancel is the default. "
        "Its songs stay in the library. Otherwise it reads Delete "
        "Playlist, and is dimmed."),
    cmd("Remove from Playlist",
        "Takes the highlighted song off the playlist you&rsquo;re in. The "
        "song stays in the library. Dimmed elsewhere."),
    cmd("Add to &ldquo;<i>name</i>&rdquo;",
        "One for each playlist. Adds the highlighted song at the end of "
        "that playlist, asking first if it&rsquo;s already there. Dimmed "
        "unless a song is highlighted."),

    section("Settings in Detail"),
    '<figure class="beside">%s</figure>' % img("c2-settings", 100),
    "<p>Choose a setting and click the centre button to show its next "
    "value. MacMusic remembers the settings.</p>",
    table(("Setting", "Values", "Preset"),
          ("Shuffle", "Off; Songs (a list in random order, the chosen song "
           "first); Albums (whole albums in random order, each in track "
           "order)", "Off"),
          ("Repeat", "Off; One (the same song again); All (back to the "
           "start of the list, reshuffled each time if Shuffle is on)", "Off"),
          cls="ref narrow"),
    "<p>A change to Shuffle applies to the next list you play. A change to "
    "Repeat may wait until the song already lined up has started.</p>",
    cmd("About", "Songs (how many, out of 500), Available (the free space "
        "on the library&rsquo;s disk) and Version."),
    cmd("Legal", "The legal statement, in full. Turn the click wheel to "
        "read it."),
    '<figure class="beside">%s</figure>' % img("c2-legal", 100),
    cmd("Reset All Settings", "Cancel or Reset. Reset puts Shuffle and "
        "Repeat back to Off; the volume is left as it is."),

    section("Specifications"),
    table(("", ""),
          ("Computer", "Macintosh Plus, System 6.0.7, with or without "
           "MultiFinder"),
          ("Memory", "640K suggested; 512K at least. With 1 MB, under "
           "MultiFinder, 500 songs with ordinary details; under the Finder "
           "alone, any library"),
          ("Songs in a library", "500, audiobooks included"),
          ("Playlists", "10"),
          ("Songs read from one playlist", "1,000"),
          ("Song details", "63 characters each"),
          ("Playlist names", "31 characters, no colon"),
          ("Track numbers", "1 to 9,999, or none"),
          ("Sound files", "WAV: mono, 22,050 samples a second, 8 bits"),
          ("Sound output", "mono, through the built-in speaker or the sound "
           "output port"),
          ("Playing speed", "about 0.9% fast, as the Macintosh Plus plays "
           "22,254 samples a second; the times shown are right"),
          ("Volume", "8 levels; the softest isn&rsquo;t silent"),
          ("Fast-forward and rewind", "4 times normal speed, then 16 times "
           "after 2 seconds"),
          ("A hold", "half a second"),
          ("The volume bar", "2 seconds"),
          ("The scrubber and the rating", "5 seconds"),
          ("Previous starts the song again", "after 3 seconds of it"),
          ("Libraries remembered", "5"),
          ("Reading a library", "about 12 seconds for 500 songs the first "
           "time and at Rescan Library; a few seconds after that"),
          ("Printing", "the printer chosen in the Chooser: ImageWriter, "
           "LaserWriter, on the printer port or AppleTalk"),
          ("Room for music", "about 1.3 MB a minute"),
          cls="ref spec"),

    keep(section("Preset Values"),
    "<p>What MacMusic starts with, the first time and after Reset All "
    "Settings:</p>",
    table(("", ""),
          ("Shuffle", "Off"),
          ("Repeat", "Off"),
          ("Backlight", "Off, each time MacMusic starts"),
          ("Volume", "as last set in MacMusic"),
          ("Library", "the one last used"),
          ("A new library&rsquo;s name", "MacMusic Library"),
          ("A new playlist&rsquo;s name", "Untitled Playlist"),
          ("A new song&rsquo;s title", "the file&rsquo;s name, unless the "
           "file gives one"),
          cls="ref spec")),
])
# Each section head stays on the page with its first paragraph.
BODY = re.sub(r'(<div class="margin"><h2 class="margin-head"[^>]*>.*?</h2></div>)(\s*<p>.*?</p>)',
              r'<div class="keep">\1\2</div>', BODY, flags=re.S)

CONTENTS = "\n".join('<li class="part"><a href="#%s">%s</a></li>' % (a, t)
                     for t, a in SECTIONS)

DRAFT = draft_title("Chapter 3: MacMusic Reference",
                    "A draft for review, 8 October 2026. In the finished book this "
                    "chapter follows Chapter 2, and its pages are numbered on from "
                    "there.")
PART = (opener(3, "MacMusic Reference", "c3-opener")
        + '<div class="chapter chapter-contents"><div class="flow">'
          '<div class="margin"><h2 class="contents-head">Contents</h2></div>'
          '<ul class="toc">%s</ul></div></div>\n' % CONTENTS
        + '<div class="chapter page-break"><div class="flow">%s</div></div>' % BODY)


# The chapter for the book (make-book.py), and on its own for review.
if __name__ == "__main__":
    open(OUT, "w").write(page("MacMusic User&rsquo;s Guide: Chapter 3", DRAFT + PART))
    print("wrote", os.path.normpath(OUT))
