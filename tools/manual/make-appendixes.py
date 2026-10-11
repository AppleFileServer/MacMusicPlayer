#!/usr/bin/env python3
"""Writes docs/manual/appendixes.html, the appendixes of the MacMusic
User's Guide (Phase 6; outline.md):
  A  Messages: every message MacMusic shows, quoted from the code
     (src/*.c, resources/MacMusic.r), with what it means and what to do;
  B  About Sound Files;
  C  Keyboard and Click Wheel Summary;
  D  Troubleshooting.

Build it with tools/manual/build-pdf.sh docs/manual/appendixes.html <pdf>."""
import os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from book import img, draft_title, page, anchor

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "..", "docs", "manual", "appendixes.html")
SCALE = 0.55            # a dialog box's points per Mac pixel


def appendix(letter, name, body, first=False):
    return ('<div class="chapter%s"><div class="appendix-title" id="appendix-%s" '
            'data-foot="Appendix %s: %s">'
            '<p class="ch">Appendix %s</p><h1 class="name">%s</h1></div>'
            '<div class="flow">%s</div></div>\n'
            % ("" if first else " page-break", letter.lower(), letter, name, letter, name, body))


def sub(title):
    return '<h3 class="ref-head loose" id="a-%s">%s</h3>' % (anchor(title), title)


def msg(text, meaning, figure=None):
    """A message, quoted, and what it means; with its picture, if any
    (a figure name and its width in Mac pixels)."""
    pic = ""
    if figure:
        name, width = figure
        pic = '<figure class="dialog">%s</figure>' % img(name, round(width * SCALE))
    return ('<div class="msg"><p class="msg-text">%s</p>%s<p>%s</p></div>'
            % (text, pic, meaning))


def q(s):
    return "&ldquo;%s&rdquo;" % s


def boxes(*items):
    return '<ul class="boxes">%s</ul>' % "".join("<li>%s</li>" % i for i in items)


def table(head, *rows, cls="ref"):
    h = "".join("<th>%s</th>" % c for c in head)
    r = "".join("<tr>%s</tr>" % "".join("<td>%s</td>" % c for c in row) for row in rows)
    return '<table class="%s"><thead><tr>%s</tr></thead><tbody>%s</tbody></table>' % (cls, h, r)


def trouble(problem, *paras):
    return ('<div class="entry"><div class="margin"><h4 class="cmd" id="t-%s">%s</h4></div>%s</div>'
            % (anchor(problem), problem, "".join("<p>%s</p>" % p for p in paras)))


def key(*names):
    return "".join('<kbd>%s</kbd>' % n for n in names)


# ---- Appendix A ---------------------------------------------------------

A = "".join([
    "<p>This appendix lists every message MacMusic can show, in the order "
    "you&rsquo;re likely to meet them, with what each one means and what to "
    "do. Messages that are questions have buttons; the one with the bold "
    "outline is chosen if you press Return.</p>",
    "<p>Some messages end with a number in brackets, such as "
    "&ldquo;(error &minus;34).&rdquo; It&rsquo;s the Macintosh&rsquo;s own "
    "code for what went wrong. The commonest are:</p>",
    table(("Number", "Means"),
          ("&minus;34", "the disk is full"),
          ("&minus;35", "the disk isn&rsquo;t there"),
          ("&minus;39", "a file ended sooner than it should"),
          ("&minus;43", "a file couldn&rsquo;t be found"),
          ("&minus;44, &minus;46", "the disk is locked"),
          ("&minus;48", "the name is already in use"),
          ("&minus;49", "the file is already open"),
          ("&minus;108", "there isn&rsquo;t enough memory"),
          ("&minus;120", "a folder couldn&rsquo;t be found"), cls="ref narrow"),

    sub("Starting Up and Choosing a Library"),
    msg(q("MacMusic keeps its songs in a folder called &lsquo;MacMusic "
          "Library.&rsquo; Find an existing one, or create a new one."),
        "MacMusic has no library yet. Click Find Existing to open one "
        "you have, Create New to make one, or Not Now to carry on without "
        "one; you can choose a library later with Choose Library in the "
        "File menu.", ("c1-first-time", 357)),
    msg(q("The MacMusic Library folder could not be found. Its disk may not "
          "be inserted, or it may have moved. Find it, or create a new one."),
        "The library you used last time isn&rsquo;t where it was. Insert its "
        "floppy disk, or switch on its hard disk, and click Find Existing; "
        "or click Create New, or Not Now."),
    msg(q("That MacMusic Library can&rsquo;t be found. Its disk may not be "
          "inserted, or it may have moved."),
        "You chose a library from the File menu that isn&rsquo;t there. "
        "Insert its disk or switch it on, and choose it again; if it has "
        "moved, use Choose Library."),
    msg(q("Use an existing MacMusic Library, or create a new one? To use an "
          "existing library, open its &lsquo;MacMusic Library Info&rsquo; "
          "file or one of its songs."),
        "You chose Choose Library. Click Use Existing Library or Create New "
        "Library, or Cancel.", ("c2-use-existing", 448)),
    msg(q("That file isn&rsquo;t inside a MacMusic Library. Open the "
          "&lsquo;MacMusic Library Info&rsquo; file, or a song, in the "
          "library&rsquo;s folder."),
        "The file you opened isn&rsquo;t part of a library. Open the "
        "library&rsquo;s folder and choose its MacMusic Library Info "
        "file."),
    msg(q("This folder is already a MacMusic Library. Use it, instead of "
          "creating a new library inside it?"),
        "You chose to create a library inside an existing one. Click Use It "
        "to use the existing library, or Cancel to choose another "
        "place."),
    msg(q("A file with that name is already there. Choose another name or "
          "place."),
        "There&rsquo;s already a file with the name you gave the new "
        "library. Type another name, or choose another folder."),
    msg(q("The library location could not be remembered."),
        "MacMusic couldn&rsquo;t save its preferences, perhaps because the "
        "startup disk is locked or full. The library is open; MacMusic will "
        "ask for it again next time."),
    msg(q("There is not enough memory to read the MacMusic Library."),
        "Quit other applications and try again, or give MacMusic more memory "
        "(see &ldquo;MacMusic and Your Other Applications&rdquo; in Chapter "
        "2)."),
    msg(q("1 file in the MacMusic Library was not recognised and was "
          "skipped.") + " (or " + q("<i>n</i> files &hellip; were") + ")",
        "The library&rsquo;s folder holds files that aren&rsquo;t MacMusic "
        "songs, or a song with the same number as another (copied in from "
        "another library, say). Take out any files you put there with the "
        "Finder. Library Info shows how many files couldn&rsquo;t be "
        "read."),
    msg(q("<i>n</i> songs weren&rsquo;t loaded: the MacMusic Library holds at "
          "most 500 songs. Export songs to make room."),
        "The library&rsquo;s folder holds more than 500 songs, so some were "
        "left out. Export songs you don&rsquo;t want, then choose Rescan "
        "Library."),
    msg(q("<i>n</i> playlists weren&rsquo;t loaded: MacMusic can have up to "
          "10 playlists."),
        "The Playlists folder holds more than ten playlists. Delete one you "
        "don&rsquo;t want, then choose Rescan Library."),

    sub("Importing Songs"),
    msg(q("Importing from another disk will take some time. Do you still "
          "want to import?"),
        "The file is on a floppy disk or another hard disk, so it has to be "
        "copied. Click Import, or Cancel."),
    msg(q("The file will be moved into the MacMusic Library. Choose Just "
          "Copy to leave the original where it is (copying takes longer)."),
        "Click OK to move the file into your library (from another disk, "
        "MacMusic copies it and then removes the original), or Just Copy to "
        "keep the original where it is.", ("c2-import-move", 357)),
    msg(q("This file is locked, so it can&rsquo;t be moved. Copy it into the "
          "MacMusic Library instead?") + " and "
        + q("This file is in use or its disk is locked, so it can&rsquo;t be "
            "moved. Copy it into the MacMusic Library instead?"),
        "Click Copy to import a copy, leaving the original as it is, or "
        "Cancel."),
    msg(q("This WAV file&rsquo;s audio is already in the MacMusic Library as "
          "&lsquo;<i>title</i>&rsquo;. Import it again?"),
        "The song is already in your library, perhaps under another name. "
        "Click Cancel, or Import Anyway to have it twice.", ("c2-duplicate", 350)),
    msg(q("This is not a WAV file."),
        "MacMusic can import only WAV sound files. Click OK.", ("c2-not-wav", 336)),
    msg(q("Only mono, 22050 Hz, 8-bit PCM WAV files can be imported. This "
          "file is stereo, 44100 Hz, 16-bit."),
        "The file is a WAV file of another kind; the message says what it "
        "is. It has to be made into the kind MacMusic plays: see Appendix B, "
        "and <i>MacMusic Utilities</i>."),
    msg(q("This WAV file is not plain PCM audio (it may be compressed or "
          "floating-point). Only uncompressed PCM WAV files can be "
          "imported.") + " and " + q("This WAV file uses the extensible "
          "format, which can&rsquo;t be imported. Save it as plain PCM."),
        "The file holds its sound in a form MacMusic can&rsquo;t play. It has "
        "to be prepared again; see <i>MacMusic Utilities</i>."),
    msg("Messages saying the WAV file is damaged or incomplete, has no "
        "format information or audio data, or that its format information "
        "is inconsistent",
        "The file is damaged, or wasn&rsquo;t finished when it was made. Make "
        "a new copy from the original."),
    msg(q("The file could not be opened.") + ", " + q("The file could not be "
        "found.") + " and " + q("The WAV file could not be read."),
        "The file was removed, or its disk ejected, or the disk is damaged. "
        "Check the disk, and try again."),
    msg(q("This file is already inside the MacMusic Library folder."),
        "It&rsquo;s already one of your songs. Choose a file from outside the "
        "library."),
    msg(q("The MacMusic Library is full (500 songs). Export a song to make "
          "room."),
        "Export a song you no longer want, or start another library."),
    msg(q("There is not enough memory to import a song."),
        "Quit other applications, or give MacMusic more memory."),
    msg(q("There is not enough space on the MacMusic Library&rsquo;s disk for "
          "this song."),
        "Make room on the disk by throwing away files you don&rsquo;t need, "
        "or by exporting songs; then import the song again."),
    msg(q("The song could not be imported."),
        "Something went wrong while the song was copied or moved; the number "
        "in brackets says what. The song isn&rsquo;t in your library. Check "
        "the disks, and try again."),
    msg(q("The song was imported, but the original file couldn&rsquo;t be "
          "removed from its disk, so it was left in place."),
        "The song is in your library. Its original is still on the other "
        "disk, probably because the disk is locked; throw it away yourself "
        "if you like."),
    msg(q("Please enter a title.") + " and " + q("Track must be a whole number "
        "from 0 to 9999."),
        "In the song&rsquo;s details, type a title (every song needs one), and "
        "a track number of 9,999 or less, or leave the track empty."),
    msg(q("The song details window could not be opened."),
        "There isn&rsquo;t enough memory. Quit other applications, and try "
        "again."),

    sub("Changing and Exporting Songs"),
    msg(q("Choose a folder outside the MacMusic Library."),
        "An exported song can&rsquo;t be saved inside the library. Choose "
        "another folder."),
    msg(q("That folder could not be found.") + " and " + q("There is not "
        "enough space on that disk."),
        "Choose another folder or disk, or make room, and export again."),
    msg(q("The song could not be exported."),
        "The song is still in your library. Check the disk you chose, and "
        "try again."),
    msg(q("The song was exported, but its file couldn&rsquo;t be removed from "
          "the MacMusic Library."),
        "The exported file was saved, but the song is still in your "
        "library. Check that the library&rsquo;s disk isn&rsquo;t locked, and "
        "choose Rescan Library."),
    msg(q("The song was exported, but it couldn&rsquo;t be given the name you "
          "chose, so it kept its MacMusic Library name."),
        "The file was saved under its number instead. Rename it in the "
        "Finder."),
    msg(q("The song&rsquo;s file could not be read."),
        "The song&rsquo;s file is damaged, or its disk has gone. Check the "
        "disk; you may need to import the song again from the original."),
    msg(q("The song&rsquo;s details couldn&rsquo;t be saved."),
        "The details weren&rsquo;t changed, perhaps because the disk is "
        "locked or full."),
    msg(q("The song&rsquo;s details were saved, but its title couldn&rsquo;t "
          "be updated in every playlist."),
        "The song is changed. Playlists list songs by number, so they still "
        "work; only the title written in a playlist file for people to read "
        "is out of date."),

    sub("Playlists"),
    msg(q("MacMusic can have up to 10 playlists."),
        "You already have ten. Delete one you no longer want first.",
        ("c1-ten-limit", 336)),
    msg(q("Please enter a name for the playlist.") + ", " + q("Playlist names "
        "can be at most 31 characters long.") + " and " + q("Playlist names "
        "can&rsquo;t contain a colon (:)."),
        "Type a name of 1 to 31 characters, without a colon."),
    msg(q("There is already a playlist with that name.") + " and " + q("There "
        "is already a file with that name in the Playlists folder."),
        "Each playlist needs a name of its own. Type another."),
    msg(q("&lsquo;<i>title</i>&rsquo; is already in the playlist "
          "&lsquo;<i>name</i>&rsquo;. Add it again?"),
        "Click Add Again to have the song on the playlist twice, or "
        "Cancel.", ("c2-add-again", 336)),
    msg(q("Delete the playlist &lsquo;<i>name</i>&rsquo;? The songs in it "
          "stay in the library."),
        "Click Delete to delete the playlist, or Cancel to keep it.",
        ("c2-delete-alert", 336)),
    msg(q("The playlist could not be created.") + ", " + q("&hellip; renamed.")
        + ", " + q("&hellip; deleted.") + ", " + q("The song could not be added "
        "to the playlist.") + " and " + q("The song could not be removed from "
        "the playlist."),
        "The playlist&rsquo;s file couldn&rsquo;t be changed, perhaps because "
        "the disk is locked or full. Nothing was changed."),

    sub("Playing"),
    msg(q("Sound playback isn&rsquo;t available; songs will play silently."),
        "MacMusic can&rsquo;t use the Macintosh&rsquo;s sound. Songs still "
        "&ldquo;play,&rdquo; with their times shown, but you won&rsquo;t hear "
        "them. MacMusic is made for the Macintosh Plus."),
    msg(q("A song&rsquo;s file couldn&rsquo;t be read, so it was skipped."),
        "A song&rsquo;s file is damaged, or its disk has gone. MacMusic went "
        "on to the next song. Choose Rescan Library; if the song still "
        "won&rsquo;t play, export it and import it again from the "
        "original."),
    msg(q("The song&rsquo;s rating couldn&rsquo;t be saved."),
        "The rating wasn&rsquo;t changed, perhaps because the library&rsquo;s "
        "disk is locked or full."),

    sub("Printing"),
    msg(q("No printer is chosen. Choose one in the Chooser, from the Apple "
          "menu."),
        "Open the Chooser, click your printer&rsquo;s icon (and, for an "
        "ImageWriter, the printer port), and close it; then print again."),
    msg(q("The library couldn&rsquo;t be printed. Check that the printer is "
          "switched on, and chosen in the Chooser."),
        "The printer didn&rsquo;t answer, or something went wrong while "
        "printing; the number after the message says what. Check the "
        "printer, its cable and its paper, and try again."),
    msg(q("There isn&rsquo;t enough memory to print. Quit other programs, or "
          "give MacMusic more memory in Get Info."),
        "Printing needs a little memory of its own. Under MultiFinder, quit "
        "an application you aren&rsquo;t using, or give MacMusic more memory "
        "(see Chapter 2)."),
    msg(q("The Print Library window could not be opened."),
        "MacMusic is short of memory. Quit other applications, and try "
        "again."),
])

# ---- Appendix B ---------------------------------------------------------

B = "".join([
    sub("The Files MacMusic Plays"),
    "<p>MacMusic plays <b>WAV</b> sound files of one kind:</p>",
    boxes("<b>mono:</b> one channel of sound;",
          "<b>22,050 samples a second;</b>",
          "<b>8 bits</b> to each sample, uncompressed (&ldquo;PCM&rdquo;)."),
    "<p>That&rsquo;s the Macintosh Plus&rsquo;s own sound. Its sound "
    "hardware plays 8-bit samples, about 22,000 a second, through one "
    "channel, to the built-in speaker or the sound output port. A file "
    "made for it plays just as it was made, with nothing lost along the "
    "way, and the Macintosh has time left over for everything else.</p>",
    "<p>Because the Macintosh has one sound channel, MacMusic&rsquo;s sound "
    "is mono, through the speaker and the sound output port alike. A "
    "stereo recording is mixed into one channel when its file is "
    "made.</p>",
    "<p>The Macintosh Plus actually plays 22,254 samples a second, so songs "
    "play about 0.9% faster than they were recorded: a four-minute song "
    "ends two seconds early. Few ears can tell, and the times MacMusic "
    "shows are right.</p>",

    sub("How Much Room Music Takes"),
    "<p>At 22,050 bytes a second, music takes about 1.3 MB of disk space "
    "for each minute. So:</p>",
    table(("On", "There&rsquo;s room for about"),
          ("An 800K floppy disk", "30 seconds, beside MacMusic itself"),
          ("A 20 MB hard disk, such as the Apple Hard Disk 20SC",
           "15 minutes, or four or five songs"),
          ("An 80 MB hard disk", "an hour"),
          ("A 2 GB hard disk, the largest System 6 can use", "25 hours"),
          cls="ref room"),
    "<p>A full library of 500 songs, at four minutes a song, would need about "
    "2.6 GB, more than one disk can hold; you can keep a library on each of "
    "several disks, and switch between them with the File menu.</p>",
    "<p>Settings, About, and Library Info in the File menu both show how much "
    "room is left on the library&rsquo;s disk.</p>",

    sub("Where Songs Come From"),
    "<p>MacMusic plays the files; it doesn&rsquo;t make them. Your music has "
    "to be prepared as MacMusic&rsquo;s kind of WAV file, and brought to "
    "the Macintosh, before you can import it. <i>MacMusic Utilities</i>, "
    "a separate manual, explains how: see &ldquo;Converting Songs.&rdquo;</p>",
    "<p>If a file isn&rsquo;t the right kind, MacMusic says so when you import "
    "it, and says what the file is. The messages are listed in Appendix "
    "A, under &ldquo;Importing Songs.&rdquo;</p>",
])

# ---- Appendix C ---------------------------------------------------------

C = "".join([
    "<p>Everything you do in the MacMusic window can be done with the mouse "
    "on the click wheel, or with the keyboard. The keys work while the "
    "MacMusic window is in front. A hold starts after half a second; "
    "fast-forward and rewind run at four times normal speed, then sixteen "
    "times after two seconds.</p>",
    table(("To", "On the click wheel", "On the keyboard"),
          ("Move the highlight up or down a list",
           "Drag round the wheel, anticlockwise or clockwise", key("&uarr;") + " " + key("&darr;")),
          ("Choose the highlighted item", "Click the centre button",
           key("Return") + " or " + key("Enter")),
          ("Go back one screen", "Click MENU", key("Delete")),
          ("Play the highlighted item; play or pause on Now Playing",
           "Click the bottom of the wheel", key("Space bar")),
          ("Go to the next song", "Click the right of the wheel", key("&rarr;")),
          ("Go to the song before, or start the song again",
           "Click the left of the wheel", key("&larr;")),
          ("Fast-forward (on Now Playing)", "Hold down the right of the wheel",
           key("Option") + " " + key("&rarr;") + ", held"),
          ("Rewind (on Now Playing)", "Hold down the left of the wheel",
           key("Option") + " " + key("&larr;") + ", held"),
          ("Change the volume (on Now Playing)",
           "Turn the wheel: clockwise for louder", key("&darr;") + " louder, "
           + key("&uarr;") + " softer"),
          ("Use the scrubber, or rate a song (on Now Playing)",
           "Click the centre button, then turn the wheel",
           key("Return") + ", then " + key("&uarr;") + " " + key("&darr;")),
          ("Quit MacMusic", "", key("&#8984;") + " " + key("Q")),
          cls="ref controls"),
])

# ---- Appendix D ---------------------------------------------------------

D = "".join([
    "<p>If something isn&rsquo;t working as you expect, look for it here. "
    "For a message MacMusic shows, see Appendix A.</p>",
    trouble("No sound",
            "Go to Now Playing and turn the click wheel clockwise: the volume "
            "may be turned down.",
            "Check the sound output port on the back of the Macintosh. While "
            "anything is plugged into it, the built-in speaker is silent; "
            "if headphones or a speaker are connected, check them, and their "
            "own volume.",
            "If MacMusic said &ldquo;Sound playback isn&rsquo;t available,&rdquo; "
            "it can&rsquo;t use this Macintosh&rsquo;s sound; MacMusic is made "
            "for the Macintosh Plus."),
    trouble("MacMusic can&rsquo;t find your library",
            "Insert the library&rsquo;s floppy disk, or make sure its hard disk "
            "is connected and switched on. Switch an external hard disk on "
            "before you switch on the Macintosh, so the Macintosh finds it "
            "as it starts up.",
            "Then click Find Existing and open the library&rsquo;s MacMusic "
            "Library Info file, or choose the library from the File menu."),
    trouble("A file won&rsquo;t import",
            "MacMusic says why; see Appendix A, &ldquo;Importing Songs.&rdquo; "
            "Most often the file isn&rsquo;t MacMusic&rsquo;s kind of WAV file. "
            "<i>MacMusic Utilities</i> explains how to prepare one."),
    trouble("The music pauses while another application works hard",
            "MacMusic plays in the background, but if another application "
            "keeps the Macintosh completely busy for more than about six "
            "seconds (copying many files, say, or initialising a disk), the "
            "sound can run out. You may hear a moment repeat, then a pause; "
            "the music carries on by itself when the Macintosh is free."),
    trouble("Not enough memory under MultiFinder",
            "MacMusic asks for 640K, and needs at least 512K. Quit applications "
            "you aren&rsquo;t using, or give MacMusic less memory with Get Info "
            "(see Chapter 2)."),
    trouble("Starting up takes a while",
            "The first time MacMusic opens a library, and when you choose "
            "Rescan Library, it reads every song file: about 12 seconds for "
            "500 songs, while the screen says Loading the library. After "
            "that it reads only songs that are new or have changed, and starts "
            "in a few seconds."),
    trouble("Song details look wrong after changing files in the Finder",
            "MacMusic keeps a note of your songs, and reads a song again when "
            "its file changes. If a song was changed by another application "
            "and still shows its old details, choose Rescan Library, which "
            "reads every song."),
    trouble("The Macintosh forgets the time",
            "If the date and time go back each time you switch on, the "
            "Macintosh&rsquo;s clock battery is flat, and needs replacing. "
            "MacMusic copes with a clock that has stopped; after songs have "
            "been changed outside MacMusic, choose Rescan Library."),
    trouble("Nothing prints",
            "Check that the printer is switched on, has paper, and is chosen "
            "in the Chooser; see Appendix A, &ldquo;Printing.&rdquo; If MacMusic "
            "says there isn&rsquo;t enough memory, quit other applications, or "
            "give MacMusic more memory with Get Info (Chapter 2). Under "
            "MultiFinder with background printing on, a LaserWriter&rsquo;s "
            "pages are printed by PrintMonitor once MacMusic has finished: "
            "leave the Macintosh switched on until they are."),
    trouble("Songs play a little fast",
            "By about 0.9%, because the Macintosh Plus plays its sound a little "
            "faster than the files were made for. The times shown are right. "
            "See Appendix B."),
    trouble("The softest volume isn&rsquo;t silent",
            "It&rsquo;s the Macintosh Plus&rsquo;s lowest sound level. To "
            "silence the music, pause it."),
    trouble("&ldquo;A song&rsquo;s file couldn&rsquo;t be read&rdquo;",
            "The song&rsquo;s file is damaged, or its disk has gone. Choose "
            "Rescan Library. If the song still won&rsquo;t play, export it and "
            "import it again from the original file."),
    trouble("A song is missing from the lists",
            "An audiobook is listed only under Audiobooks; check the song&rsquo;s "
            "Audiobook box with Edit Song. If the song was copied into the "
            "library folder with the Finder, MacMusic may have skipped it: "
            "import songs with Import Song instead."),
])

# A subheading stays on the page with the entry that follows it.
A = re.sub(r'(<h3 class="ref-head loose">.*?</h3>)(<div class="msg">.*?</div>)',
           r'<div class="keep">\1\2</div>', A, flags=re.S)

DRAFT = draft_title("Appendixes A to D",
                    "A draft for review, 8 October 2026. In the finished book the "
                    "appendixes follow Chapter 3, and their pages are numbered on "
                    "from there.")
PART = (appendix("A", "Messages", A, first=False)
        + appendix("B", "About Sound Files", B)
        + appendix("C", "Keyboard and Click Wheel Summary", C)
        + appendix("D", "Troubleshooting", D))


# The chapter for the book (make-book.py), and on its own for review.
if __name__ == "__main__":
    open(OUT, "w").write(page("MacMusic User&rsquo;s Guide: Appendixes", DRAFT + PART))
    print("wrote", os.path.normpath(OUT))
