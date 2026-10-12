#!/usr/bin/env python3
"""Writes docs/utilities-manual/chapter3.html, Chapter 3 of *MacMusic
Utilities*, "Moving Songs to the Macintosh": the MacMusic Transfer
Utility 1.0 (Phase 4; docs/utilities-manual/outline.md), as Chapters 2
and 4: introduction, two lessons, task pages, reference.

Facts come from the Transfer Utility's 1.0 source (its window, alerts
and messages) and its SPECIFICATION.md; the figures from the owner's
screenshots of 2026-10-10 (tuNN), and from captures of 1.0 itself made
the same day (tu-..., and the Plus's screens from Mini vMac, plus-...).
Its window is half again the Converter's, so its parts print across the
whole page. The real Plus check of 1.0 is still to do (the owner's), so
what the Plus shows is as Mini vMac showed it.

Build it with tools/utilities-manual/build-pdf.sh docs/utilities-manual/chapter3.html <pdf>."""
import os, re, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, "..", "manual"))
from ub import (img, draft_title, opener, page, anchor, TEXT, TASK, fig, steps, Parts, head, sub, cmd,
                table, Topic, T)
from callouts import figure

OUT = os.path.join(HERE, "..", "..", "docs", "utilities-manual", "chapter3.html")
PARTS = Parts()
lesson, section = PARTS.lesson, PARTS.section
LESSONS, SECTIONS = PARTS.lessons, PARTS.sections


def wide(name):
    """A part of the window across the whole page, so its text can be read."""
    return '<figure class="window wide">%s</figure>' % img(name, 448)


# The window with six songs added, two of them renamed (tu01), labelled:
# 1520 x 1440 pixels.
SCALE = TEXT / 1520
WINDOW = figure("figures/tu01.png", 1520, 1440, SCALE, [
    ("The songs", 40, 40, 237),
    ("What each|is called|on the Mac", 62, 350, 333),
    ("Add Files, Remove|and Clear All", 116, 90, 602),
    ("SCSI ID", 158, 60, 795),
    ("The image&rsquo;s|file name", 176, 60, 872),
    ("Size", 220, 60, 1094),
    ("Build Transfer|Image", 278, 1100, 1380),
])

INTRO = "".join([
    head("Introduction", "intro"),
    "<p>The <b class=\"term\">MacMusic Transfer Utility</b> takes your MacMusic WAV "
    "files to the Macintosh Plus. It puts them on a disk the Plus can use: "
    "a <b class=\"term\">disk image</b>, which the BlueSCSI in the Plus reads "
    "from its SD card as though it were another hard disk.</p>",
    "<p>You copy the disk image onto the card yourself, with the Finder; the "
    "Transfer Utility never writes to the card.</p>",

    sub("What it makes"),
    "<p>A disk called <b>Transfer</b>, holding a folder called <b>WAV Files</b> "
    "with your songs in it, kept in one file on your Mac, such as "
    "<b>HD60_512 Transfer.hda</b>. Its name tells the BlueSCSI what it is: "
    "a hard disk at SCSI ID 6 (Appendix B, &ldquo;Disk Images&rdquo;). It&rsquo;s made "
    "the size the songs need, with a quarter as much room again for more.</p>",

    sub("SCSI IDs"),
    "<p>Each disk on the Plus has a SCSI ID, 0 to 6 (7 is the Macintosh "
    "itself), and each disk image on the card needs an ID of its own. Your "
    "startup disk is usually 0. The Transfer Utility suggests 6.</p>",

    sub("Safe with your files"),
    "<p>It checks every song against MacMusic&rsquo;s own rules first, and "
    "refuses any that aren&rsquo;t MacMusic WAV files; it never changes a song; "
    "and it checks the finished disk, file by file, before it keeps it. "
    "Nothing half-made is left behind.</p>",
])

LESSON_1 = "".join([
    lesson(1, "Putting Songs on a Disk for the Plus"),
    "<p>In this lesson you put songs made by the Converter on a disk image, "
    "copy it to the card, and find the songs on the Plus.</p>",
    steps("> Open the MacMusic Transfer Utility.",
          "Its window waits for songs."),
    wide("tu05-drop"),
    steps("> Drag MacMusic WAV files onto the window, or a folder of them, or "
          "click Add Files.",
          "Each song is listed with its name on the Mac, its format, length and "
          "size, and a tick if it can go on the disk."),
    wide("tu01-files"),
    steps("Under Disk, the SCSI ID is 6, and the file name and size of the disk "
          "image are shown."),
    wide("tu01-disk"),
    steps("> Click Build Transfer Image.",
          "The Transfer Utility makes the disk, copies the songs onto it, and "
          "checks it, saying at the foot of the window what it&rsquo;s doing."),
    wide("tu-progress-foot"),
    steps("When it&rsquo;s done, Result shows the disk image, and what to do "
          "with it."),
    wide("tu03-result"),
    steps("> Click Show in Finder.",
          "> Shut the Plus down with Shut Down, switch it off, take the card out "
          "of the BlueSCSI, and put it in your Mac.",
          "> Drag the disk image onto the card, into its top level, not into a "
          "folder; then eject the card.",
          "> Put the card back in the BlueSCSI, and switch the Plus on.",
          "A disk called Transfer appears on the Plus&rsquo;s desktop, with your "
          "songs in its WAV Files folder.",
          "> Open the Transfer disk, and its WAV Files folder.",
          "~ The songs show most clearly by name: choose by Name from the View "
          "menu."),
    fig("plus-wav-files", TEXT),
    steps("> Import them with MacMusic&rsquo;s Import Song: see the <i>MacMusic "
          "User&rsquo;s Guide</i>, Lesson 7."),
])

LESSON_2 = "".join([
    lesson(2, "Adding More Songs Later"),
    "<p>Later you&rsquo;ll have more songs for the Plus. There are two ways.</p>",
    steps("* <b>On the Plus:</b> the Transfer disk has room to spare (a quarter "
          "again), so songs can be copied onto it there, from a floppy disk, say.",
          "* <b>A new disk image:</b> add all the songs, old and new, to the "
          "Transfer Utility, and build again. It asks what to do with the image "
          "already in the folder: Replace it, or Keep Both."),
    steps("> To replace the disk on the card, drag the new image onto the card "
          "as before; the Finder asks whether to replace the old one, which has "
          "the same name.",
          "~ MacMusic&rsquo;s library on the Plus is separate from the Transfer disk: "
          "a song imported into it stays there, whatever becomes of the disk."),
])

TOPICS = [
    Topic("Choosing the Songs",
          "<p>Only MacMusic WAV files can go on the disk: those the Converter "
          "makes.</p>",
          [T("To Add Songs", steps(
               "> Drag WAV files, or a folder of them, onto the window.",
               "OR",
               "Click Add Files, choose them, and click Open.",
               "A folder adds the WAV files in it, not those in folders inside it. "
               "The same file added twice is listed once.")),
           T("To Remove Songs", steps(
               "> Select them in the list and click Remove; or click Clear All.",
               "The files themselves aren&rsquo;t touched.")),
           T("If a Song Can&rsquo;t Be Used", steps(
               "Its Status says Can&rsquo;t be used; point to it to see why: it isn&rsquo;t "
               "a WAV file, or isn&rsquo;t in MacMusic&rsquo;s format (mono, 22,050 Hz, "
               "8-bit).",
               "> Remove it, or make a MacMusic WAV of it with the Converter, and add "
               "that instead.",
               "~ The disk isn&rsquo;t built while a refused song is in the list."),
             ("tu-refused-files", "wide")),
           T("If a Name Is Changed", steps(
               "A Macintosh file name is at most 31 characters, without colons, and "
               "two names that differ only in capitals count as the same. Where a "
               "name has to change, the new one is shown in orange under On the "
               "Mac: a long name shortened, a colon made a hyphen, a second "
               "&ldquo;Clash.wav&rdquo; beside &ldquo;clash.wav&rdquo; made &ldquo;Clash "
               "2.wav,&rdquo; &ldquo;Rock: Live.wav&rdquo; made &ldquo;Rock- Live.wav.&rdquo; The "
               "file on your Mac keeps its name."), ("tu01-files", "wide")),
           ]),

    Topic("The Disk",
          "<p>The disk&rsquo;s SCSI ID, its name and its size are under Disk.</p>",
          [T("To Choose the SCSI ID", steps(
               "> Choose an ID from the SCSI ID menu: 0 to 6.",
               "The file name changes to match: HD50_512 Transfer.hda for 5. "
               "Choose an ID no other disk image on the card uses.",
               "~ ID 0 is usually your startup disk&rsquo;s: if you choose it, you&rsquo;re "
               "asked to confirm (Use ID 0 Anyway), since putting the image on the "
               "card would replace your startup disk."), ("tu-id0", 200)),
           T("How the Size Is Worked Out", steps(
               "The songs&rsquo; size, plus a quarter as much again for adding more, "
               "plus 4 MiB for the disk&rsquo;s own needs, rounded up to whole "
               "megabytes; at least 10 MiB, and at most 2 GB, the largest disk "
               "System 6 can use. Click &ldquo;How is this worked out?&rdquo; under Size "
               "to see the sums.",
               "~ Sizes are shown as the Finder shows them (46.1 MB). On the Plus, "
               "sizes are in K of 1,024 bytes, so the same disk shows there as "
               "about 45,000K.")),
           T("To Choose Where It&rsquo;s Saved", steps(
               "> Under Save To, click Choose and choose a folder. Use Default goes "
               "back to the MacMusic Transfer folder in your Music folder.")),
           T("If an Image Is Already There", steps(
               "If the folder already has an image of that name, you&rsquo;re asked:",
               "* <b>Keep Both</b> saves the new one as &ldquo;HD60_512 Transfer "
               "2.hda.&rdquo; Both use the same SCSI ID, so put only one on the card.",
               "* <b>Replace</b> puts the new one in place of the old.",
               "* <b>Cancel</b> builds nothing."), ("tu-already", 200)),
           ]),

    Topic("Building",
          "<p>Building makes the disk image, copies the songs onto it, and "
          "checks it, in a minute or so.</p>",
          [T("To Build the Disk Image", steps(
               "> Click Build Transfer Image.",
               "The foot of the window says what it&rsquo;s doing, with a progress bar "
               "and Cancel. When it&rsquo;s done, Result shows the image, its size and "
               "the room left on it, and Verified.")),
           T("To Cancel", steps(
               "> Click Cancel, or press Esc.",
               "Nothing is saved.",
               "~ If you quit while it&rsquo;s building, you&rsquo;re asked first: Keep "
               "Building, or Stop and Quit. Stop and Quit leaves your songs, and any "
               "disk image already finished, as they are."), ("tu-quit", 160)),
           T("To See What Was Checked", steps(
               "> Click &ldquo;What was checked&rdquo; under Result.",
               "It lists each check the finished disk passed: its layout, its "
               "volume, and each song, byte for byte. Copy Summary copies the lot, "
               "to keep or to send. Any name that had to change is listed at the "
               "end."), ("tu-checked-result", "wide")),
           ]),

    Topic("On the Card and the Plus",
          "<p>The Transfer Utility&rsquo;s work ends on your Mac; the last steps are "
          "yours.</p>",
          [T("To Copy the Disk Image to the Card", steps(
               "> Shut the Plus down with Shut Down, and switch it off.",
               "> Take the card out of the BlueSCSI, and put it in your Mac.",
               "> Click Show in Finder, and drag the image onto the card&rsquo;s top "
               "level.",
               "> Eject the card in the Finder, and put it back in the BlueSCSI.",
               "~ Before you copy it, think of backing the card up with the Macintosh "
               "Backup Utility (Chapter 4).")),
           T("To Use the Disk on the Plus", steps(
               "> Switch the Plus on.",
               "The Transfer disk appears on the desktop. Open it, and its WAV Files "
               "folder, to see the songs.",
               "> In MacMusic, choose Import Song from the File menu, and open the "
               "WAV Files folder (<i>MacMusic User&rsquo;s Guide</i>, &ldquo;Adding "
               "Songs&rdquo;).",
               "~ After each song, MacMusic asks whether to import another: "
               "click Import Another to bring in the next from the disk, or "
               "Done.")),
           T("If the Disk Doesn&rsquo;t Appear", steps(
               "See Appendix C, &ldquo;The disk doesn&rsquo;t appear.&rdquo; Most often another "
               "image on the card has the same SCSI ID, or the image went into a "
               "folder on the card rather than its top level.")),
           ]),
]

REFERENCE = "".join([
    section("Transfer Utility Reference", "reference"),
    "<p>This section describes each part of the Transfer Utility&rsquo;s window, "
    "what happens when you build, and Advanced.</p>",

    section("The Transfer Utility Window"),
    '<figure class="wide">%s</figure>' % WINDOW,
    cmd("Files", "The songs: each file&rsquo;s name, its name on the Mac (orange if "
        "it had to change), its format, length and size, and its status (a tick, "
        "or Can&rsquo;t be used); Add Files, Remove, Clear All; how many, how long "
        "and how big altogether."),
    cmd("Disk", "SCSI ID (0 to 6; 0 marked as your boot disk); the image&rsquo;s file "
        "name; the reminder that each ID needs its own image; the volume "
        "(&ldquo;Transfer&rdquo;, its songs in &ldquo;WAV Files&rdquo;); the size, with How is "
        "this worked out?"),
    cmd("Save To", "The folder the image is saved in, Choose and Use Default."),
    cmd("Advanced", "The disk template (below)."),
    cmd("Result", "After building: the image, the number of songs, its size, the "
        "room left on it, Verified; Copy Summary and Show in Finder; what to do "
        "next; What was checked."),
    cmd("Build Transfer Image", "Starts building; dimmed until there are songs and "
        "none is refused. While it builds: what it&rsquo;s doing, a progress bar, and "
        "Cancel."),
    wide("tu03-save-to"),

    section("What Happens When You Build"),
    table(("Stage", "What it does"),
          ("Checking the files", "Each song checked again, as it is now."),
          ("Preparing the disk", "The image made, its start copied from the disk "
           "template."),
          ("Formatting the HFS volume", "The Transfer disk made inside it, as a "
           "Macintosh disk."),
          ("Copying the files", "Each song copied into WAV Files, under its Mac name."),
          ("Verifying the disk", "The whole disk read back and checked, song by "
           "song, byte for byte."),
          ("Finishing", "Only then is the image given its name."),
          cls="ref stages"),

    section("Advanced"),
    wide("tu07-advanced"),
    cmd("Disk template", "The start of every disk the Transfer Utility makes: "
        "Apple&rsquo;s partition map and hard disk driver, which the Plus needs to "
        "use a hard disk. The Transfer Utility has its own; Choose uses another "
        "BlueSCSI disk image&rsquo;s, and Use Default goes back."),

    section("Limits"),
    table(("", ""),
          ("Songs", "MacMusic WAV files only"),
          ("Folders", "One level: a folder&rsquo;s WAV files, not those in folders inside it"),
          ("Names on the Mac", "31 characters at most; no colons; capitals don&rsquo;t "
           "tell names apart"),
          ("SCSI ID", "0 to 6"),
          ("The disk", "10 MiB to 2 GB"),
          cls="ref spec"),
])


def contents():
    rows = ['<li class="part"><a href="#intro">Introduction</a></li>',
            '<li class="part"><a href="#lessons">Lessons</a></li>']
    rows += ['<li class="lesson"><span class="n">%d</span><a href="#%s">%s</a></li>' % (n, a, t)
             for n, t, a in LESSONS]
    rows.append('<li class="part"><a href="#tasks">Tasks</a></li>')
    for t in TOPICS:
        rows.append('<li class="topic"><a href="#%s">%s</a></li>' % (anchor(t.title), t.title))
    rows += ['<li class="part"><a href="#%s">%s</a></li>' % (a, t) for t, a in SECTIONS[:1]]
    rows += ['<li class="topic"><a href="#%s">%s</a></li>' % (a, t) for t, a in SECTIONS[1:]]
    return "\n".join(rows)


def chapter(*parts, cls="chapter page-break"):
    return '<div class="%s"><div class="flow">%s</div></div>\n' % (cls, "".join(parts))


PART = (opener(3, "Moving Songs to the Macintosh", "c3-opener")
        + '<div class="chapter chapter-contents"><div class="flow">'
          '<div class="margin"><h2 class="contents-head">Contents</h2></div>'
          '<ul class="toc">%s</ul></div></div>\n' % contents()
        + chapter(INTRO)
        + chapter('<span id="lessons"></span>', LESSON_1) + chapter(LESSON_2)
        + chapter('<span id="tasks"></span>', "".join(t.html() for t in TOPICS), cls="chapter tasks-wide page-break")
        + chapter(REFERENCE))
PART = re.sub(r'(<div class="margin">(?:(?!</div>).)*</div>(?:</div>)?)(\s*<p.*?</p>)',
              r'<div class="keep">\1\2</div>', PART, flags=re.S)

DRAFT = draft_title("Chapter 3: Moving Songs to the Macintosh",
                    "A draft for review, 10 October 2026. The figures are from the "
                    "owner's screenshots and from captures of 1.0 (the Plus&rsquo;s "
                    "screens from Mini vMac). The Transfer Utility 1.0&rsquo;s check "
                    "on the real Plus is still to do.")

if __name__ == "__main__":
    open(OUT, "w").write(page("MacMusic Utilities: Chapter 3", DRAFT + PART))
    print("wrote", os.path.normpath(OUT))
