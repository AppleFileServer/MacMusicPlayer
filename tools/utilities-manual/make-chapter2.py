#!/usr/bin/env python3
"""Writes docs/utilities-manual/chapter2.html, Chapter 2 of *MacMusic
Utilities*, "Converting Songs": the MacMusic Converter Utility 1.0
(Phase 3; docs/utilities-manual/outline.md). Its introduction, two
lessons, the task pages and the Converter's reference.

Facts come from the Converter's 1.0 source (MainWindowView.swift,
AppModel.swift, AppDelegate.swift, the engine's messages) and the
owner's screenshots (figure codes cvNN), with captures of 1.0 made on
2026-10-10 (cv-...). CHECK marks what no screenshot shows yet (UB7).

Task pages: the User's Guide sets each task in a column a quarter of the
page wide, which suits its small Macintosh Plus screens. A modern window
can't be read at that width, so here each task sits in the wide text
column with its title in the margin (style addendum, "Task pages").

Build it with tools/utilities-manual/build-pdf.sh docs/utilities-manual/chapter2.html <pdf>."""
import os, re, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, "..", "manual"))
from ub import (img, draft_title, opener, page, anchor, TEXT, TASK, fig, steps, Parts, head, sub, cmd,
                table, Topic, T)
from callouts import figure

OUT = os.path.join(HERE, "..", "..", "docs", "utilities-manual", "chapter2.html")
WINDOW_SCALE = TEXT / 1080      # a whole Converter window across the text column

# ---- Pieces (tools/utilities-manual/ub.py) ---------------------------------

PARTS = Parts()
lesson, section = PARTS.lesson, PARTS.section
LESSONS, SECTIONS = PARTS.lessons, PARTS.sections


# ---- The window, labelled (the reference) -----------------------------------

# cv06: the window as it opens, Local File chosen, nothing yet. Points
# are the window's own pixels (1080 x 1264); the labels sit in the
# margin, their heights in points.
WINDOW = figure("figures/cv06.png", 1080, 1264, WINDOW_SCALE, [
    ("Local File|and YouTube", 33, 390, 127),
    ("Source", 61, 60, 215),
    ("Make", 139, 60, 488),
    ("Title", 163, 60, 570),
    ("File name", 184, 60, 643),
    ("Normalise", 236, 60, 827),
    ("Trim silence|at the ends", 270, 60, 961),
    ("Longer than", 313, 60, 1097),
    ("Convert", 344, 960, 1207),
])

# ---- The chapter ---------------------------------------------------------

INTRO = "".join([
    head("Introduction", "intro"),
    "<p>The <b class=\"term\">MacMusic Converter Utility</b> makes your music "
    "into files that MacMusic can play. MacMusic plays one kind of sound file "
    "only: a WAV file that is mono, with 22,050 samples a second, 8 bits each "
    "(<i>MacMusic User&rsquo;s Guide</i>, Appendix B). The Converter makes "
    "exactly that kind, from almost any sound file on your Mac, or from a "
    "YouTube video you own or have permission to save.</p>",
    "<p>It runs on your Mac, not on the Macintosh Plus. Chapter 3 explains how "
    "to take what it makes to the Plus.</p>",

    sub("What it makes"),
    "<p>Each time you convert a song, the Converter makes either or both of "
    "these:</p>",
    steps("* A <b class=\"term\">MacMusic WAV</b>: mono, 22,050 Hz, 8-bit. "
          "It&rsquo;s checked against MacMusic&rsquo;s own rules before it&rsquo;s "
          "kept, and it carries the song&rsquo;s title, so MacMusic fills the "
          "title in for you when you import it.",
          "* An <b class=\"term\">MP3</b>: 192 kbps, stereo if the song is. "
          "It&rsquo;s for listening on your Mac or elsewhere; it never goes to "
          "the Macintosh Plus."),

    sub("Where songs come from"),
    steps("* <b>A sound file on your Mac:</b> an MP3, AAC or M4A, WAV, FLAC, "
          "Opus and most other kinds.",
          "* <b>A YouTube video:</b> one at a time, by its address. The "
          "Converter doesn&rsquo;t sign in to YouTube, so private, "
          "age-restricted and members-only videos can&rsquo;t be saved. Use "
          "it only for videos you own or have permission to save."),

    sub("Safe with your files"),
    "<p>The Converter never changes the song you start from. It asks before "
    "it replaces anything. If you cancel, or quit while it works, nothing "
    "half-made is left behind, and every file is checked before it&rsquo;s "
    "kept.</p>",
    "<p>What it makes goes in a folder called MacMusic Converter, inside "
    "the Music folder in your home folder, unless you choose another.</p>",
])

LESSON_1 = "".join([
    lesson(1, "Converting a Song From a File"),
    "<p>In this lesson you make a MacMusic WAV from an MP3 on your Mac. Any "
    "song you have will do; the pictures show &ldquo;Good Luck, "
    "Babe!&rdquo;.</p>",
    steps("> Open the MacMusic Converter Utility.",
          "It&rsquo;s in the Applications folder; double-click its icon. Its "
          "window opens with Local File chosen at the top."),
    fig("cv06-source", TASK),
    steps("> Click Choose File.",
          "A dialog box opens, showing the folders on your Mac."),
    fig("cv08", TEXT),
    steps("> Find the song, select it, and click Open."),
    fig("cv10", TEXT),
    steps("The window shows the song&rsquo;s name and folder, and under them "
          "what it is: an MP3, at 44,100 samples a second, in stereo, three "
          "minutes 38 seconds long.",
          "~ You can drag a song from the Finder onto the window instead."),
    fig("cv11-source", TASK),
    steps("> Under Output, click MacMusic WAV.",
          "The Title is filled in from the song&rsquo;s own details, if it has "
          "any. This is the title MacMusic will show; type another if you "
          "like.",
          "Leave File name empty, and the file is named after the title. "
          "Saves as shows the name it will have: Good Luck, Babe!.wav."),
    fig("cv12-output", TASK),
    steps("> Click Convert.",
          "At the foot of the window, the Converter says what it&rsquo;s doing "
          "as it works: checking the file, analysing the sound, making the "
          "MacMusic WAV, and checking the result. A song of a few minutes "
          "takes a few seconds."),
    fig("cv27-progress", TASK),
    steps("When it&rsquo;s finished, Results shows the new file, its length, "
          "size and loudest peak, with a Show in Finder button to find it, and "
          "what was done to the sound. (The picture shows another song&rsquo;s.)"),
    fig("cv-wav-only-results", TASK),
])

LESSON_2 = "".join([
    lesson(2, "Converting a YouTube Video"),
    "<p>Now you make a MacMusic WAV and an MP3 from a YouTube video. Use one "
    "you own or have permission to save; the pictures show a piece of music "
    "its channel offers free of copyright.</p>",
    steps("> Click YouTube at the top of the window.",
          "> In your web browser, copy the video&rsquo;s address. Then click in "
          "the box under YouTube Video, and choose Paste from the Edit menu.",
          "> Click Look Up.",
          "The Converter asks YouTube about the video."),
    fig("cv16-youtube", TASK),
    steps("In a moment you see its title, its channel and its length."),
    fig("cv17-youtube", TASK),
    steps("~ If the address is of a video in a playlist, the playlist is "
          "ignored: only the one video is converted.",
          "> Under Output, click Both.",
          "> Type a shorter title: Chill Dreamy.",
          "The title of a video is often long. MacMusic shows up to 63 "
          "characters of a title, and the WAV file&rsquo;s name can be no more "
          "than 31, so a short title is best. Saves as shows the two files "
          "you&rsquo;ll get."),
    fig("cv18-output", TASK),
    steps("> Click Convert.",
          "The Converter fetches the sound from YouTube first, then makes each "
          "file in turn."),
    fig("cv20-progress", TASK),
    steps("Results shows what was done, and the files made. Click Show in "
          "Finder to see them in their folder."),
    fig("cv28-results", TASK),
    steps("~ This video was normalised by &minus;2.4 dB, and its last three "
          "seconds of silence trimmed: those are choices under Sound, "
          "described in the tasks that follow."),
])

TOPICS = [
    Topic("Choosing a Song",
          "<p>The Converter works from one song at a time: a sound file on your "
          "Mac (Local File) or a YouTube video (YouTube). Choose which at the "
          "top of the window.</p>",
          [T("To Convert a Sound File", steps(
               "> Click Local File at the top of the window.",
               "> Click Choose File, select the song, and click Open.",
               "OR",
               "Drag the song from the Finder onto the window.",
               "The window shows what the song is. To choose a different one, "
               "click Choose Another."), "cv06-source"),
           T("If a Sound File Can&rsquo;t Be Used", steps(
               "The Converter says why, under the song&rsquo;s name: it couldn&rsquo;t "
               "be read as audio (it may be damaged, or not a sound file), it has "
               "no sound in it, or its sound is damaged. Click Details to see more.",
               "> Choose another song, or another copy of it (Appendix A)."),
             "cv-refused-source"),
           T("To Convert a YouTube Video", steps(
               "> Click YouTube at the top of the window.",
               "> Paste the video&rsquo;s address into the box, and click Look Up "
               "(or press Return).",
               "Its title, channel and length appear. Only one video is "
               "converted, even if the address is of a playlist.",
               "~ Only for videos you own or have permission to save."), "cv17-youtube"),
           T("If a Video Can&rsquo;t Be Used", steps(
               "The Converter says why, in place of the video&rsquo;s details:",
               "* The video is private.",
               "* YouTube needs signing in to play it (it may be age-restricted "
               "or for members only). The Converter doesn&rsquo;t sign in.",
               "* It isn&rsquo;t available in your country.",
               "* It isn&rsquo;t available: it may have been removed, or the "
               "address is wrong.",
               "* The address isn&rsquo;t of one video (a playlist or a channel, "
               "perhaps), or isn&rsquo;t a YouTube address.",
               "* YouTube couldn&rsquo;t be reached: check the internet "
               "connection, and try again.",
               "If it says YouTube has changed in a way it can&rsquo;t handle, "
               "see Appendix C, &ldquo;Troubleshooting.&rdquo; Click Details for "
               "what YouTube said."), "cv-not-retrieved-youtube"),
           ]),

    Topic("Choosing What to Make",
          "<p>A MacMusic WAV is the kind of file MacMusic plays. An MP3 is for "
          "listening anywhere else. The Converter makes either, or both from "
          "the one job.</p>",
          [T("To Make a MacMusic WAV, an MP3 or Both", steps(
               "> Under Output, click MacMusic WAV, MP3 or Both.",
               "Saves as lists the file, or files, you&rsquo;ll get. The "
               "Converter remembers your choice for next time."), "cv14-output"),
           T("If the Song Is Already an MP3", steps(
               "Making a new MP3 from an MP3 re-encodes it, which loses a little "
               "quality. The window warns you when you choose MP3 or Both.",
               "When you click Convert, it asks:",
               "* <b>Re-encode</b> makes the MP3 anyway.",
               "* <b>WAV Only</b> makes just the MacMusic WAV.",
               "* <b>Cancel</b> makes nothing."), "cv11-output", ("cv-reencode", 200)),
           ]),

    Topic("Naming the Song",
          "<p>Each song has a title, which MacMusic shows, and a file name, "
          "which is what the file is called on your Mac and on the "
          "Macintosh Plus.</p>",
          [T("To Set the Title MacMusic Shows", steps(
               "> Type it in the Title box.",
               "It&rsquo;s filled in for you from the song&rsquo;s own details, "
               "or the video&rsquo;s title. It goes inside the WAV file, and "
               "MacMusic puts it in the Title box when you import the song. "
               "MacMusic keeps up to 63 characters.")),
           T("To Set the File Name", steps(
               "> Type it in the File name box, without .wav or .mp3.",
               "Left empty, the file is named after the title.",
               "A MacMusic WAV&rsquo;s name can be no more than 31 characters, "
               "with .wav (the Macintosh&rsquo;s limit), so a long name is cut, "
               "at a word where it can be. Colons are left out, and letters the "
               "Macintosh can&rsquo;t show are changed for the nearest it can. "
               "An MP3 keeps its whole name.",
               "Saves as shows the names before you convert."), "cv18-output"),
           ]),

    Topic("The Sound",
          "<p>Two choices under Sound change the music a little, and both "
          "start off. The Converter remembers them for next time.</p>",
          [T("To Even Out the Loudness", steps(
               "> Turn on Normalise.",
               "The song is made louder or quieter so that its loudest moment "
               "comes to a set level (&minus;1.0 dBFS, just below the most a "
               "file can hold). Songs from different places then play at more "
               "nearly the same volume.",
               "~ It sets the loudest peak, not how loud a song sounds overall, "
               "so songs may still sound different."), "cv06-sound"),
           T("To Trim Silence at the Ends", steps(
               "> Turn on &ldquo;Trim silence at the ends,&rdquo; and choose 2 or 3 "
               "seconds in Longer than.",
               "Silence longer than that at the start or the end is cut down to "
               "half a second. Silence in the middle of a song is never "
               "touched.")),
           T("To Change the Normalise Level", steps(
               "> Click Advanced, near the foot of the window, to open it.",
               "> Click the arrows beside Normalise to.",
               "The level goes from &minus;6.0 to 0.0 dBFS, half a step at a "
               "time. The window&rsquo;s description of Normalise changes to "
               "match."), "cv15-advanced"),
           ]),

    Topic("Saving",
          "<p>The Converter saves everything in one folder, and never replaces "
          "a file without asking.</p>",
          [T("To Choose Where Files Are Saved", steps(
               "> Under Save To, click Choose, select a folder, and click Open.",
               "To go back to the MacMusic Converter folder in your Music "
               "folder, click Use Default. That folder is made the first time "
               "it&rsquo;s needed."), "cv15-save-to"),
           T("If a File Is Already There", steps(
               "A mark beside a name in Saves as means the folder already has a "
               "file of that name. When you click Convert, the Converter "
               "asks:",
               "* <b>Keep Both</b> saves the new file under the next free name: "
               "Chill Dreamy 2.wav, say.",
               "* <b>Replace</b> puts the new file in place of the old one.",
               "* <b>Cancel</b> makes nothing.",
               "~ The song you&rsquo;re converting from is never replaced: if it "
               "has the same name, the new file takes the next free one."),
             "cv22-output", ("cv23", 200)),
           ]),

    Topic("While It Works",
          "<p>A conversion takes a few seconds, or a minute or two from "
          "YouTube. The foot of the window says what&rsquo;s happening.</p>",
          [T("To Cancel a Conversion", steps(
               "> Click Cancel, or press Esc.",
               "It stops, and nothing half-made is kept: Results says so."),
             "cv20-progress", "cv-cancelled-results"),
           T("If You Quit While It Works", steps(
               "The Converter asks first:",
               "* <b>Keep Converting</b> carries on.",
               "* <b>Stop and Quit</b> stops it and quits. The file being made "
               "isn&rsquo;t saved; anything already finished is kept."), ("cv-quit", 160)),
           ]),

    Topic("Afterwards",
          "<p>Results, at the foot of the window, shows what was made.</p>",
          [T("To Find the Files", steps(
               "> Click Show in Finder beside a file in Results.",
               "A Finder window opens with the file selected."), "cv28-results", "cv-finder"),
           T("To Put the Songs on the Macintosh Plus", steps(
               "The MacMusic WAV files are the ones for the Plus. Chapter 3, "
               "&ldquo;Moving Songs to the Macintosh,&rdquo; explains how to put "
               "them on a disk for it, and the <i>MacMusic User&rsquo;s "
               "Guide</i> how to import them into MacMusic.")),
           ]),
]

REFERENCE = "".join([
    section("Converter Reference", "reference"),
    "<p>This section describes each part of the Converter&rsquo;s window, "
    "what happens when you convert a song, and the menus.</p>",

    section("The Converter Window"),
    '<figure class="wide">%s</figure>' % WINDOW,
    cmd("Local File and YouTube", "Choose where the song comes from: a sound "
        "file on your Mac, or a YouTube video. The rest of the window follows "
        "your choice."),
    cmd("Source", "With Local File: the song&rsquo;s name and folder, what it "
        "is (its kind, samples a second, mono or stereo, and length), and Choose "
        "File or Choose Another. If the file can&rsquo;t be read, or holds no "
        "sound, it says so here."),
    cmd("YouTube Video", "With YouTube: the box for the address, Look Up, and "
        "the video&rsquo;s title, channel and length once it&rsquo;s found, or why "
        "it can&rsquo;t be used."),
    cmd("Output", "Make: MacMusic WAV, MP3 or Both. Title: the title MacMusic "
        "shows. File name: what the files are called (empty: named after the "
        "title). Saves as: the names they&rsquo;ll have, with a mark if a file "
        "of that name is already there."),
    cmd("Sound", "Normalise; Trim silence at the ends; Longer than (2 or 3 "
        "seconds)."),
    cmd("Save To", "The folder the files are saved in, with Choose and, if "
        "you&rsquo;ve chosen one, Use Default."),
    cmd("Advanced", "Click it to open or close it: see &ldquo;Advanced&rdquo; "
        "below."),
    cmd("Results", "After a conversion: what was done to the sound "
        "(normalised by how much, trimmed by how much), and each file made, with "
        "its length, size and loudest peak, and Show in Finder. If something "
        "went wrong, the message is here."),
    cmd("Convert and Cancel", "Convert starts (or press Return); it&rsquo;s "
        "dimmed until there&rsquo;s a song to convert. While it works, the foot "
        "of the window shows what it&rsquo;s doing, a progress bar, and Cancel "
        "(or press Esc)."),

    '<div class="keep">',
    section("What Happens When You Convert"),
    "<p>The foot of the window names each stage as it comes:</p>",
    table(("Stage", "What it does"),
          ("Retrieving the audio from YouTube", "With YouTube only: fetches the "
           "video&rsquo;s sound, not its pictures."),
          ("Checking the file", "Reads the song, and makes sure it holds sound."),
          ("Analysing the sound", "Finds its loudest peak and any silence at the "
           "ends, if Normalise or Trim is on."),
          ("Making the MacMusic WAV, Making the MP3", "Makes each file, in a "
           "hidden place in the folder."),
          ("Checking the results", "Checks each file is what it should be (a "
           "MacMusic WAV by MacMusic&rsquo;s own rules) before giving it its "
           "name."), cls="ref stages"),
    "</div>",
    "<p>Nothing is given its name until it&rsquo;s been checked, so a file you "
    "see in the folder is always complete.</p>",

    section("Advanced"),
    fig("cv15-advanced", TEXT),
    cmd("Normalise to", "The level Normalise brings the loudest peak to: "
        "&minus;6.0 to 0.0 dBFS, in half steps; &minus;1.0 to start with."),
    cmd("MacMusic WAV", "Fixed: 22,050 Hz, mono, 8-bit, what MacMusic plays."),
    cmd("MP3", "192 kbps, stereo if the song is. An MP3 made from a compressed "
        "song (an MP3, or YouTube&rsquo;s sound) is no better than the song."),
    cmd("FFmpeg, yt-dlp and Deno", "The programs inside the Converter that do "
        "the work: FFmpeg reads and makes the sound, yt-dlp and Deno fetch from "
        "YouTube. Their versions are shown here; see Appendix D."),
    cmd("Version", "The Converter&rsquo;s version, 1.0. Licences opens the "
        "licences of the programs inside it, a file called THIRD-PARTY-NOTICES.md, "
        "in the app your Mac uses for such files."),

    section("The Menus"),
    "<p>The Converter&rsquo;s menus are the usual ones of a Mac program.</p>",
    '<figure class="row">%s%s</figure>' % (img("cv31", 150), img("cv34", 128)),
    cmd("MacMusic Converter Utility menu", "About MacMusic Converter Utility "
        "shows its version; Hide, Hide Others and Show All; Quit MacMusic "
        "Converter Utility (Command-Q)."),
    cmd("Edit menu", "Cut, Copy and Paste, for the Title, File name and "
        "address boxes."),
    cmd("View menu", "Enter Full Screen."),
    cmd("Window menu", "Close (Command-W), Minimize (Command-M), Zoom, and "
        "macOS&rsquo;s other ways to arrange the window."),
    fig("cv30", 160, "The About box."),

    section("Limits"),
    table(("", ""),
          ("Songs at a time", "One"),
          ("YouTube", "One video at a time; no playlists; no signing in"),
          ("A MacMusic WAV&rsquo;s name", "31 characters at most, with .wav"),
          ("A title, as MacMusic keeps it", "63 characters"),
          ("Normalise to", "&minus;6.0 to 0.0 dBFS"),
          ("Trim silence longer than", "2 or 3 seconds"),
          ("MP3", "192 kbps"), cls="ref spec"),
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


LESSONS_HTML = chapter('<span id="lessons"></span>', LESSON_1) + chapter(LESSON_2)
TASKS_HTML = chapter('<span id="tasks"></span>', "".join(t.html() for t in TOPICS),
                     cls="chapter tasks-wide page-break")
REFERENCE_HTML = chapter(REFERENCE)

PART = (opener(2, "Converting Songs", "c2-opener")
        + '<div class="chapter chapter-contents"><div class="flow">'
          '<div class="margin"><h2 class="contents-head">Contents</h2></div>'
          '<ul class="toc">%s</ul></div></div>\n' % contents()
        + chapter(INTRO) + LESSONS_HTML + TASKS_HTML + REFERENCE_HTML)

# Each margin head stays on the page with its first paragraph.
PART = re.sub(r'(<div class="margin">(?:(?!</div>).)*</div>(?:</div>)?)(\s*<p.*?</p>)',
              r'<div class="keep">\1\2</div>', PART, flags=re.S)

DRAFT = draft_title("Chapter 2: Converting Songs",
                    "A draft for review, 10 October 2026. In the finished book this "
                    "chapter follows Chapter 1, and its pages are numbered on from "
                    "there.")

if __name__ == "__main__":
    open(OUT, "w").write(page("MacMusic Utilities: Chapter 2", DRAFT + PART))
    print("wrote", os.path.normpath(OUT))
