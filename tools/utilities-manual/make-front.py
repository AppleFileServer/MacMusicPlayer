#!/usr/bin/env python3
"""Writes docs/utilities-manual/front.html, *MacMusic Utilities*' front
matter and a first glossary (Phase 8, drafted early at the owner's
request, 2026-10-09; outline.md): the title page, the copyright page,
About This Manual, and the glossary of the words the Converter and
Transfer Utility chapters use. The cover is made last (as the User's
Guide's), and the index once every chapter is written.

What's provisional: the USB drive's contents (UB8) are marked CHECK.

Build it with tools/utilities-manual/build-pdf.sh docs/utilities-manual/front.html <pdf>."""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from ub import BOOK, draft_title, page

OUT = os.path.join(HERE, "..", "..", "docs", "utilities-manual", "front.html")
PRIVATE = os.environ.get("MANUAL_ASSETS", os.path.expanduser("~/MacPlusProjects/manual-assets-private"))
OWN_USE = os.path.exists(os.path.join(PRIVATE, "rainbow-logo.png"))
EDITION = "<i>MacMusic Utilities</i> &middot; First edition, October 2026"


def logo(width):
    """The own edition's rainbow logo (the User's Guide's, private), or
    MacMusic's icon."""
    if OWN_USE:     # 314 x 295 pixels
        return ('<img class="logo" src="private/rainbow-logo.png" style="width:%spt; height:%.1fpt">'
                % (width, width * 295 / 314))
    return '<img class="logo" src="figures/mm-icon.png" style="width:%spt; height:%spt">' % (width, width)


def boxes(*items):
    return '<ul class="boxes">%s</ul>' % "".join("<li>%s</li>" % i for i in items)


def margin_head(title, ident):
    return '<div class="margin"><h2 class="margin-head" id="%s">%s</h2></div>' % (ident, title)


def margin_sub(title):
    return '<div class="margin"><h3 class="margin-sub">%s</h3></div>' % title


COPYRIGHT = """
<div class="copyright"><div class="copy-columns">
 <div class="copy-left">
  <p class="copy-head">Copyright</p>
  <p>This manual and the programs it describes are copyrighted, with all
  rights reserved, except where otherwise stated.</p>
  <p>&copy; 2026 Jack McAteer.</p>
  <p>The MacMusic utilities are an independent project, and are not
  affiliated with, endorsed by, sponsored by or produced by Apple Inc. or
  Google LLC.</p>
  <p>Apple, Macintosh, Mac and macOS are trademarks of Apple Inc. YouTube
  is a trademark of Google LLC. Other names, logos and trademarks belong
  to their respective owners.</p>
  <p>Programs and manual by Jack McAteer.</p>
  <p>%s</p>
 </div>
 <div class="copy-right">
  <p class="copy-head">For your own use</p>
  <p>This manual, the USB drive and the programs on it are for their
  owner&rsquo;s own use. They include software and artwork belonging to
  others, Apple&rsquo;s among them, and aren&rsquo;t for passing on.</p>
  <p>The programs inside the utilities (FFmpeg, yt-dlp, Deno and
  hfsutils) are each under their own licence: see Appendix D.</p>
  <p class="copy-head">Disclaimer</p>
  <p>The utilities are provided without any representation that they are
  suitable for a particular purpose. Keep backup copies of your music and
  of your Macintosh&rsquo;s disks.</p>
  <p class="caps">The developer accepts no responsibility for loss or
  corruption of data, music files, disk images or other material arising
  from the use of the programs, except to the extent that such liability
  cannot lawfully be excluded or limited.</p>
  <p>The utilities don&rsquo;t provide or distribute music. You&rsquo;re
  responsible for having the right to copy, convert, store and play the
  music you use with them. Save from YouTube only videos you own or have
  permission to save.</p>
 </div>
</div></div>
""" % EDITION


def title_page():
    return ('<div class="title-page" id="title-page">'
            '<div class="title-logo">%s</div><p class="title-name"><i>%s</i></p></div>\n'
            % (logo(36), BOOK))


ABOUT = "".join([
    '<div class="front"><div class="flow">',
    margin_head("About This Manual", "about"),
    "<p>MacMusic plays music on a Macintosh Plus. The MacMusic utilities are "
    "three programs for your Mac, a modern Mac, that look after that music: "
    "they make your songs into MacMusic&rsquo;s sound files, take them to the "
    "Plus, and keep a safe copy of what&rsquo;s there. This manual shows you "
    "how.</p>",
    boxes("The <b>MacMusic Converter Utility</b> makes a song, from a sound "
          "file or a YouTube video, into a MacMusic WAV file, and an MP3 if you "
          "like.",
          "The <b>MacMusic Transfer Utility</b> puts MacMusic WAV files on a "
          "disk for the Plus: a disk image that the Plus&rsquo;s BlueSCSI reads "
          "from its SD card.",
          "The <b>Macintosh Backup Utility</b> keeps copies of the BlueSCSI&rsquo;s "
          "SD cards on your Mac, rather as Time Machine does for your Mac, so a "
          "disk can be put back if something on the Plus is lost."),

    margin_sub("What&rsquo;s on the USB drive"),
    "<p>The USB drive in the box holds:</p>",
    boxes("the three utilities;",
          "this manual and the <i>MacMusic User&rsquo;s Guide</i>, to read on "
          "your Mac;",
          "the MacMusic disk, for the Macintosh Plus;",
          "the utilities&rsquo; source code and licences;",
          "a Read Me."),
    '<p class="grey">CHECK: the drive&rsquo;s contents are settled when it&rsquo;s made (UB8).</p>',

    margin_sub("What you need"),
    boxes("a Mac with Apple silicon and macOS 26;",
          "a Macintosh Plus with System 6.0.7 and MacMusic 1.2 or later;",
          "a BlueSCSI in the Plus, and a way to read its SD card on your Mac;",
          "for YouTube, an internet connection."),

    margin_sub("Other MacMusic manuals"),
    "<p>The <i>MacMusic User&rsquo;s Guide</i> explains MacMusic itself: "
    "importing the songs this manual helps you make, and playing them. This "
    "manual names it wherever you&rsquo;ll need it.</p>",

    margin_sub("What you should know"),
    "<p>This manual assumes you know your Mac: opening programs, using the "
    "Finder and its folders, and copying files. It also assumes you know the "
    "Macintosh Plus, as the <i>MacMusic User&rsquo;s Guide</i> and the "
    "<i>Macintosh Owner&rsquo;s Guide</i> teach it.</p>",

    margin_sub("Two computers"),
    "<p>The utilities run on one computer, and their work ends up on another. "
    "This manual always says which: <b>your Mac</b> is the modern one; "
    "<b>the Macintosh Plus</b>, or the Plus, is the one that plays MacMusic.</p>",

    margin_sub("How to use this manual"),
    boxes("<b>Start here:</b> Chapter 1, &ldquo;Getting Started,&rdquo; explains "
          "what&rsquo;s on the USB drive, installs the utilities, and follows one "
          "song from your Mac to the Plus.",
          "<b>Each program:</b> Chapters 2 to 4 take one utility each: "
          "&ldquo;Converting Songs,&rdquo; &ldquo;Moving Songs to the "
          "Macintosh&rdquo; and &ldquo;Backing Up Your Macintosh.&rdquo; Each "
          "has a lesson or two, the tasks it does one at a time, and a reference "
          "to every part of its window.",
          "<b>When something goes wrong:</b> the appendixes list every message "
          "and what to do, and describe the files the utilities make."),
    "<p>The glossary explains the words this manual uses, and the index "
    "finds any subject.</p>",

    margin_sub("Conventions"),
    '<p class="step">A step tells you what to do.</p>',
    "<p>The plain text after it tells you what happens.</p>",
    '<p class="grey">Grey text adds a remark you can skip.</p>',
    "<p>Commands are named with their menus: &ldquo;Choose Paste from the "
    "Edit menu.&rdquo; Buttons and boxes are named as they appear in the "
    "window: Convert, Look Up, File name.</p>",
    "</div></div>\n",
])

GLOSSARY = [
    ("Apple silicon", "The kind of processor in Macs made since 2020. The "
     "utilities need one."),
    ("BlueSCSI", "A small board that plugs into the Macintosh Plus&rsquo;s SCSI "
     "port and acts as one or more hard disks, reading disk images from an SD "
     "card."),
    ("backup folder", "The folder on your Mac where the Backup Utility keeps "
     "its snapshots: Mac Plus Backups, in your home folder, unless you choose "
     "another."),
    ("check-sum", "A number worked out from every byte of a file, so that a "
     "copy can be checked against it: one byte different, and the number is "
     "different. The Backup Utility uses SHA-256."),
    ("clean shutdown", "A disk put away properly, as Shut Down does. A disk "
     "that lost power while the Plus was using it wasn&rsquo;t shut down "
     "cleanly, and may be damaged."),
    ("dBFS", "Decibels below full scale: how loud a sound is compared with the "
     "loudest a file can hold. 0 dBFS is the most; &minus;1.0 dBFS is just "
     "below it."),
    ("disk image", "A whole disk kept in one file. The Transfer Utility makes "
     "them; a BlueSCSI reads them."),
    ("disk template", "The start of a BlueSCSI disk image (its partition map "
     "and Apple&rsquo;s hard disk driver), which the Transfer Utility copies at "
     "the start of every disk it makes."),
    ("eject", "To tell your Mac you&rsquo;ve finished with a disk or card, so "
     "it can be taken out safely."),
    ("exFAT", "The way of keeping files that SD cards for a BlueSCSI use, "
     "which your Mac reads and writes."),
    ("Finder", "The program that shows your files, folders and disks, on your "
     "Mac and on the Macintosh Plus alike."),
    ("HFS", "The Macintosh&rsquo;s own way of keeping files on a disk, which "
     "System 6 uses. A Transfer Utility disk is an HFS disk."),
    ("import", "In MacMusic, to add a song to your library, with Import Song "
     "from the File menu (<i>MacMusic User&rsquo;s Guide</i>)."),
    ("incomplete job", "A backup that didn&rsquo;t finish, or wasn&rsquo;t "
     "verified. It holds no snapshot, and can be cleared."),
    ("kbps", "Thousands of bits a second: how much an MP3 uses for each second "
     "of sound. The Converter&rsquo;s MP3s are 192 kbps."),
    ("LUFS", "How loud music sounds to the ear. The Converter brings every "
     "song to &minus;16 LUFS."),
    ("MacMusic WAV", "The one kind of sound file MacMusic plays: a WAV file, "
     "mono, with 22,254 samples a second, 8 bits each. The Converter makes "
     "them."),
    ("manifest", "A snapshot&rsquo;s record of every file in it, with its "
     "size and check-sum."),
    ("Mini vMac", "A program that makes your Mac act as a Macintosh Plus. "
     "Some of this manual&rsquo;s pictures of the Plus come from it."),
    ("mono", "Sound on one channel. MacMusic plays in mono, since the Macintosh "
     "has one sound channel; a stereo song is mixed into one."),
    ("MP3", "A kind of sound file that takes much less room than a WAV file, "
     "for listening on your Mac or elsewhere. MacMusic doesn&rsquo;t play "
     "them."),
    ("peak", "A song&rsquo;s loudest moment."),
    ("register", "In the Backup Utility, to give a card a name, so it&rsquo;s "
     "recognised whenever it&rsquo;s inserted and its backups are kept "
     "together."),
    ("restore", "To copy files back out of a snapshot."),
    ("re-encode", "To make an MP3 from a song that&rsquo;s already an MP3 (or "
     "another compressed kind), which loses a little quality."),
    ("Results", "The part of the Converter&rsquo;s window that shows what a "
     "conversion made."),
    ("sample rate", "How many times a second a sound is measured: 22,254 for a "
     "MacMusic WAV, 44,100 for a CD."),
    ("Saves as", "In the Converter, the names the files will have, shown before "
     "you convert."),
    ("SCSI ID", "A number, 0 to 7, that tells apart the devices on the Macintosh "
     "Plus&rsquo;s SCSI port. 7 is the Macintosh itself; a startup disk is "
     "usually 0. Each disk image on a BlueSCSI&rsquo;s card has one."),
    ("SD card", "The small memory card a BlueSCSI keeps its disk images on. You "
     "copy disk images to it on your Mac."),
    ("snapshot", "A complete copy of everything on a card at one moment, made "
     "by the Backup Utility and checked file by file."),
    ("thinning", "Removing older snapshots after a backup, so that the "
     "backups don&rsquo;t fill your Mac: one a day for a week, one a week for "
     "a month, one a month for a year."),
    ("title", "A song&rsquo;s name as MacMusic shows it. The Converter puts it "
     "inside the WAV file."),
    ("Transfer disk", "The disk the Transfer Utility makes, called Transfer, "
     "with your songs in its WAV Files folder."),
    ("trim silence", "In the Converter, to cut long silence at the start or end "
     "of a song down to half a second."),
    ("verify", "To check every file of a snapshot against its manifest."),
    ("WAV file", "A common kind of sound file. MacMusic plays one kind of WAV "
     "file only: see MacMusic WAV."),
    ("YouTube address", "A video&rsquo;s web address, from your browser&rsquo;s "
     "address bar, such as https://www.youtube.com/watch?v=&hellip;."),
]


def glossary():
    entries = "".join('<div class="gloss"><p class="term">%s</p><p>%s</p></div>' % e
                      for e in sorted(GLOSSARY, key=lambda e: e[0].lower()))
    return ('<div class="chapter page-break"><div class="appendix-title" id="glossary" '
            'data-foot="Glossary"><h1 class="name">Glossary</h1></div>'
            '<div class="flow">%s</div></div>\n' % entries)


FRONT = COPYRIGHT + title_page() + ABOUT      # as the User's Guide: the copyright page inside the front cover
PART = FRONT                       # for the whole book (make-book.py, Phase 8)
BACK = glossary()

DRAFT = draft_title("Front matter and glossary",
                    "A draft for review, 10 October 2026: the title page, the "
                    "copyright page and About This Manual, with a first glossary "
                    "(which goes at the back of the book). The cover and the index "
                    "come last. CHECK marks what&rsquo;s provisional.")

if __name__ == "__main__":
    open(OUT, "w").write(page("MacMusic Utilities: Front Matter", DRAFT + FRONT + BACK))
    print("wrote", os.path.normpath(OUT))
