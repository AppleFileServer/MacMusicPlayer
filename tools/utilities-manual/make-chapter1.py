#!/usr/bin/env python3
"""Writes docs/utilities-manual/chapter1.html, Chapter 1 of *MacMusic
Utilities*, "Getting Started" (Phase 5; docs/utilities-manual/outline.md):
a song's journey in one picture, what's on the USB drive (UB8,
provisional), installing the utilities (the owner's screenshots of
installing the Converter, cv01-cv05), a short tour of a song's whole
journey pointing to the chapters, and What Now?

Build it with tools/utilities-manual/build-pdf.sh docs/utilities-manual/chapter1.html <pdf>."""
import os, re, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from ub import img, draft_title, opener, page, anchor, TEXT, TASK, fig, steps, Parts, head, sub, table

OUT = os.path.join(HERE, "..", "..", "docs", "utilities-manual", "chapter1.html")
PARTS = Parts()
lesson, section = PARTS.lesson, PARTS.section
LESSONS = PARTS.lessons
RED = "#e0442b"


def journey():
    """A song's journey (the introduction's picture): what it is at each
    step, and the program that takes it to the next. An SVG in the text
    column, Helvetica, the programs in red-ruled boxes."""
    W = TEXT
    rows = [
        ("thing", "Your music: a sound file, or a YouTube video"),
        ("program", "MacMusic Converter Utility", "Chapter 2"),
        ("thing", "A MacMusic WAV file, on your Mac"),
        ("program", "MacMusic Transfer Utility", "Chapter 3"),
        ("thing", "A disk image, HD60_512 Transfer.hda"),
        ("step", "You copy it onto the BlueSCSI’s SD card"),
        ("thing", "The Transfer disk, on the Macintosh Plus"),
        ("step", "MacMusic’s Import Song (MacMusic User’s Guide)"),
        ("thing", "A song in your MacMusic library"),
    ]
    out, y = [], 6
    for i, r in enumerate(rows):
        if i:
            out.append('<line x1="%d" y1="%d" x2="%d" y2="%d" stroke="%s" stroke-width="1.2"/>'
                       '<polygon points="%d,%d %d,%d %d,%d" fill="%s"/>'
                       % (W // 2 - 70, y - 14, W // 2 - 70, y - 4, RED,
                          W // 2 - 74, y - 7, W // 2 - 66, y - 7, W // 2 - 70, y - 2, RED))
        if r[0] == "program":
            out.append('<rect x="20" y="%d" width="%d" height="24" rx="5" fill="white" stroke="%s" stroke-width="1.2"/>'
                       '<text x="30" y="%d" font-weight="bold">%s</text>'
                       '<text x="%d" y="%d" text-anchor="end" fill="#8c8c8c">%s</text>'
                       % (y, W - 40, RED, y + 16, r[1], W - 30, y + 16, r[2]))
            y += 24 + 16
        elif r[0] == "step":
            out.append('<text x="30" y="%d" font-style="italic" fill="#555">%s</text>' % (y + 12, r[1]))
            y += 16 + 16
        else:
            out.append('<text x="30" y="%d">%s</text>' % (y + 12, r[1]))
            y += 16 + 16
    # The Backup Utility, beside the card.
    y += 4
    out.append('<rect x="20" y="%d" width="%d" height="38" rx="5" fill="white" stroke="#555" stroke-width="1" stroke-dasharray="3 2"/>'
               '<text x="30" y="%d" font-weight="bold">Macintosh Backup Utility</text>'
               '<text x="%d" y="%d" text-anchor="end" fill="#8c8c8c">Chapter 4</text>'
               '<text x="30" y="%d" fill="#555">keeps copies of the SD card on your Mac, to put back</text>'
               % (y, W - 40, y + 15, W - 30, y + 15, y + 30))
    y += 44
    return ('<figure class="journey"><svg width="%dpt" height="%dpt" viewBox="0 0 %d %d" '
            'font-family="Helvetica, Arial, sans-serif" font-size="9.5">%s</svg></figure>'
            % (W, y, W, y, "".join(out)))


INTRO = "".join([
    head("From Your Music to MacMusic", "intro"),
    "<p>MacMusic plays songs on the Macintosh Plus, from its own kind of "
    "sound file. The three MacMusic utilities run on your Mac, and between "
    "them take a song from wherever it is now to the Plus, and keep the "
    "Plus&rsquo;s disks safe.</p>",
    journey(),
    "<p>The <b>MacMusic Converter Utility</b> makes a song into a MacMusic WAV "
    "file. The <b>MacMusic Transfer Utility</b> puts MacMusic WAV files on a "
    "disk for the Plus, as a disk image that its BlueSCSI reads from its SD "
    "card. And the <b>Macintosh Backup Utility</b> keeps copies of the card on "
    "your Mac, so that if anything on it is ever lost, it can be put back.</p>",

    section("What&rsquo;s on the USB Drive", "usb"),
    "<p>The USB drive in the box holds:</p>",
    steps("* the three utilities, each as a zip file;",
          "* this manual and the <i>MacMusic User&rsquo;s Guide</i>, to read on your "
          "Mac;",
          "* the MacMusic disk, for the Macintosh Plus;",
          "* the utilities&rsquo; source code and licences;",
          "* a Read Me."),
    '<p class="grey">PROVISIONAL: the drive&rsquo;s contents are settled when '
    "it&rsquo;s made (UB8).</p>",

    section("What You Need", "need"),
    steps("* A Mac with Apple silicon and macOS 26, with a slot or reader for SD "
          "cards.",
          "* The Macintosh Plus, with System 6.0.7, MacMusic, and a BlueSCSI.",
          "* For YouTube, an internet connection."),
])

LESSON_1 = "".join([
    lesson(1, "Installing the Utilities"),
    "<p>Each utility comes as a zip file. Installing one takes a minute; "
    "the pictures show the MacMusic Converter Utility, and the others go "
    "the same way.</p>",
    steps("> Plug in the USB drive, and open it in the Finder.",
          "> Double-click the utility&rsquo;s zip file.",
          "macOS expands it."),
    fig("cv01-zip", TEXT),
    fig("cv02", 200),
    steps("The utility appears beside the zip file."),
    fig("cv03-app", TEXT),
    steps("> Drag the utility onto Applications, in the sidebar of the Finder "
          "window.",
          "It&rsquo;s copied into your Applications folder."),
    fig("cv04-drag", TEXT),
    steps("> Open the Applications folder, and double-click the utility to open "
          "it.",
          "~ If macOS won&rsquo;t open it the first time, saying it can&rsquo;t check it, "
          "open System Settings, click Privacy &amp; Security, and click Open "
          "Anyway beside its name; then open it again. (The utilities are made "
          "for your own use, and aren&rsquo;t registered with Apple.)",
          "> Do the same for the MacMusic Transfer Utility and the Macintosh "
          "Backup Utility."),
    fig("cv05", TEXT),
])

LESSON_2 = "".join([
    lesson(2, "A Song&rsquo;s Journey"),
    "<p>This lesson follows one song from your Mac to the Plus, a step to "
    "each program, and says where each step is explained in full. Have a "
    "song of your own ready: an MP3, say.</p>",
    steps("> Make it a MacMusic WAV: open the MacMusic Converter Utility, click "
          "Choose File, choose the song, click MacMusic WAV, and click Convert.",
          "The WAV file is saved in the MacMusic Converter folder in your Music "
          "folder. (Chapter 2, Lesson 1.)",
          "> Put it on a disk for the Plus: open the MacMusic Transfer Utility, "
          "drag the WAV file onto its window, and click Build Transfer Image.",
          "The disk image, HD60_512 Transfer.hda, is saved in the MacMusic "
          "Transfer folder in your Music folder. (Chapter 3, Lesson 1.)",
          "> Shut the Plus down with Shut Down in the Special menu, switch it off, "
          "and put its SD card in your Mac.",
          "> Back the card up: the Macintosh Backup Utility offers to when the card "
          "appears; click Back Up Now, then Back Up. (Chapter 4, Lesson 1.)",
          "A backup before you change the card means nothing can be lost.",
          "> Drag the disk image onto the card, eject it, put it back in the "
          "BlueSCSI, and switch the Plus on.",
          "The Transfer disk appears on the Plus&rsquo;s desktop.",
          "> In MacMusic, choose Import Song from the File menu, and open the "
          "Transfer disk&rsquo;s WAV Files folder. (<i>MacMusic User&rsquo;s Guide</i>, "
          "Lesson 7.)",
          "The song is in your library, ready to play."),
])

WHAT_NOW = "".join([
    head("What Now?", "whatnow"),
    "<p>Each utility has a chapter of its own, with lessons, a task for "
    "everything it does, and a reference to every part of its window:</p>",
    steps("* Chapter 2, &ldquo;Converting Songs&rdquo;: the MacMusic Converter Utility;",
          "* Chapter 3, &ldquo;Moving Songs to the Macintosh&rdquo;: the MacMusic Transfer "
          "Utility;",
          "* Chapter 4, &ldquo;Backing Up Your Macintosh&rdquo;: the Macintosh Backup "
          "Utility."),
    "<p>The appendixes list every message the utilities show, describe the "
    "files they make, and say what to do when something goes wrong.</p>",
])


def contents():
    rows = ['<li class="part"><a href="#intro">From Your Music to MacMusic</a></li>',
            '<li class="topic"><a href="#usb">What&rsquo;s on the USB Drive</a></li>',
            '<li class="topic"><a href="#need">What You Need</a></li>',
            '<li class="part"><a href="#lessons">Lessons</a></li>']
    rows += ['<li class="lesson"><span class="n">%d</span><a href="#%s">%s</a></li>' % (n, a, t)
             for n, t, a in LESSONS]
    rows.append('<li class="part"><a href="#whatnow">What Now?</a></li>')
    return "\n".join(rows)


def chapter(*parts, cls="chapter page-break"):
    return '<div class="%s"><div class="flow">%s</div></div>\n' % (cls, "".join(parts))


PART = (opener(1, "Getting Started", "c1-opener")
        + '<div class="chapter chapter-contents"><div class="flow">'
          '<div class="margin"><h2 class="contents-head">Contents</h2></div>'
          '<ul class="toc">%s</ul></div></div>\n' % contents()
        + chapter(INTRO)
        + chapter('<span id="lessons"></span>', LESSON_1) + chapter(LESSON_2)
        + chapter(WHAT_NOW))
PART = re.sub(r'(<div class="margin">(?:(?!</div>).)*</div>(?:</div>)?)(\s*<p.*?</p>)',
              r'<div class="keep">\1\2</div>', PART, flags=re.S)

DRAFT = draft_title("Chapter 1: Getting Started",
                    "A draft for review, 10 October 2026. The USB drive&rsquo;s "
                    "contents are provisional (UB8).")

if __name__ == "__main__":
    open(OUT, "w").write(page("MacMusic Utilities: Chapter 1", DRAFT + PART))
    print("wrote", os.path.normpath(OUT))
