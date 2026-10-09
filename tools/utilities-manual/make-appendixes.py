#!/usr/bin/env python3
"""Writes docs/utilities-manual/appendixes.html, the appendixes of
*MacMusic Utilities* (Phase 6; docs/utilities-manual/outline.md):
  A  Messages: every message the Converter and the Transfer Utility
     show, quoted from their 1.0 source, with what it means and what to do;
  B  About the Files;
  C  Troubleshooting;
  D  Specifications and Licences.

The Backup Utility's parts follow when it exists (UB6). CHECK marks what
waits for the owner's Plus check of the Transfer Utility, or for
screenshots (UB7).

Build it with tools/utilities-manual/build-pdf.sh docs/utilities-manual/appendixes.html <pdf>."""
import os, re, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from ub import img, draft_title, page, anchor

OUT = os.path.join(HERE, "..", "..", "docs", "utilities-manual", "appendixes.html")


def appendix(letter, name, body, first=False):
    return ('<div class="chapter%s"><div class="appendix-title" id="appendix-%s" '
            'data-foot="Appendix %s: %s">'
            '<p class="ch">Appendix %s</p><h1 class="name">%s</h1></div>'
            '<div class="flow">%s</div></div>\n'
            % ("" if first else " page-break", letter.lower(), letter, name, letter, name, body))


def part(title):
    """A program's part of an appendix: a red margin head."""
    return '<div class="margin"><h2 class="margin-head" id="p-%s">%s</h2></div>' % (anchor(title), title)


def sub(title):
    return '<h3 class="ref-head loose" id="a-%s">%s</h3>' % (anchor(title), title)


def msg(text, meaning, figure=None):
    pic = '<figure class="window">%s</figure>' % img(*figure) if figure else ""
    return '<div class="msg"><p class="msg-text">%s</p>%s<p>%s</p></div>' % (text, pic, meaning)


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


# ---- Appendix A: Messages ---------------------------------------------------

A = "".join([
    "<p>This appendix lists the messages each program shows, grouped by what "
    "you were doing, with what each one means and what to do. Many messages "
    "have a Details line under them: click it to see the technical detail, "
    "which helps if you need to find out more.</p>",

    part("MacMusic Converter Utility"),

    sub("Choosing a Sound File"),
    msg(q("The file couldn&rsquo;t be found. It may have been moved or deleted."),
        "The song was moved, renamed or deleted after you chose it. Choose it again."),
    msg(q("The file couldn&rsquo;t be read as audio. It may be damaged, or not a sound file."),
        "The Converter couldn&rsquo;t make sense of it. Check it plays on your Mac; "
        "if it doesn&rsquo;t, it&rsquo;s damaged, or isn&rsquo;t a sound file."),
    msg(q("The file has no sound in it."),
        "It may be a video with no soundtrack, or an empty file."),
    msg(q("The file&rsquo;s sound is damaged, so it can&rsquo;t be converted."),
        "Part of the sound can&rsquo;t be read. Try another copy of the song."),

    sub("Choosing a YouTube Video"),
    msg(q("Paste the address of a YouTube video."), "The box is empty."),
    msg(q("That isn&rsquo;t a web address.") + " and " + q("That isn&rsquo;t a YouTube address."),
        "Copy the address from your web browser&rsquo;s address bar while the video is showing."),
    msg(q("That&rsquo;s a YouTube address, but not of one video (a playlist or a "
          "channel, perhaps)."),
        "Open one video from the playlist or channel, and copy its address."),
    msg(q("The video isn&rsquo;t available: it may have been removed, or the address is wrong."),
        "Check the address, and that the video still plays in your browser."),
    msg(q("The video is private."), "Only its owner can see it, so it can&rsquo;t be saved."),
    msg(q("YouTube needs signing in to play this video (it may be age-restricted or "
          "for members only). This app doesn&rsquo;t sign in, so it can&rsquo;t be "
          "saved here."), "The Converter never signs in to YouTube."),
    msg(q("The video isn&rsquo;t available in your country."), "YouTube blocks it where you are."),
    msg(q("YouTube couldn&rsquo;t be reached. Check the internet connection, and try again."),
        "Your Mac isn&rsquo;t online, or YouTube isn&rsquo;t answering."),
    msg(q("YouTube has changed in a way this version of yt-dlp can&rsquo;t handle. "
          "yt-dlp may need updating (see BUILDING.md)."),
        "YouTube changes from time to time. See Appendix C, &ldquo;YouTube Stops Working.&rdquo;"),
    msg(q("The video couldn&rsquo;t be retrieved."),
        "Something else went wrong while fetching it; the Details say what. Try again later."),
    msg(q("yt-dlp or Deno, which YouTube needs, couldn&rsquo;t be found.") + " and "
        + q("FFmpeg, which the conversion needs, couldn&rsquo;t be found."),
        "The programs inside the Converter are missing: the Converter has been "
        "damaged. Copy it again from the USB drive."),

    sub("Converting"),
    msg("<b>The file is already an MP3</b><br>" + q("Making a new MP3 from it re-encodes it, "
        "which loses a little quality; a higher bitrate can&rsquo;t bring it back. Make "
        "the MP3 anyway, or just the MacMusic WAV?"),
        "Re-encode makes the MP3 anyway; WAV Only makes just the MacMusic WAV; "
        "Cancel makes nothing. (CHECK: not yet pictured.)"),
    msg("<b>" + q("Chill Dreamy.mp3") + " already exists in " + q("MacMusic Files") + "</b><br>"
        + q("Keep Both saves the new file under the next free name. Replace puts it in "
            "place of what&rsquo;s there."),
        "Keep Both gives the new file the next free name (Chill Dreamy 2.mp3); "
        "Replace puts it in place of the old one; Cancel makes nothing. The song "
        "you&rsquo;re converting from is never replaced.", ("cv23", 180)),
    msg(q("The folder to save to isn&rsquo;t available, or can&rsquo;t be written to. "
          "Choose another."),
        "The folder is on a disk that isn&rsquo;t connected, or is locked. Choose "
        "another under Save To."),
    msg(q("There isn&rsquo;t enough room on the disk to save the converted files."),
        "Make room on the disk, or save to another one. A MacMusic WAV takes about "
        "1.3 MB a minute (Appendix B)."),
    msg(q("The conversion failed."),
        "FFmpeg, which does the work, stopped; the Details say why. Try again; if "
        "it fails again, the song itself may be at fault."),
    msg(q("Cancelled. Nothing was saved.") + " and " + q("Cancelled. The files already "
        "finished were kept."),
        "You clicked Cancel. With Both, a file finished before you cancelled is kept."),
    msg("<b>Stop the conversion and quit?</b><br>" + q("The file being converted won&rsquo;t "
        "be saved. Anything already finished is kept."),
        "You chose Quit while a song was converting. Keep Converting carries on; "
        "Stop and Quit stops and quits. (CHECK: not yet pictured.)"),

    sub("In the Results"),
    "<p>After a conversion, Results says what was done to the sound, and may add "
    "a note. Notes are for your information; the file is fine.</p>",
    msg(q("Normalised by &minus;2.4 dB.") + ", or " + q("... (the most allowed)."),
        "How much louder (+) or quieter (&minus;) Normalise made the song. A song is "
        "raised by 20 dB at most, so a very quiet one may not reach the level: "
        "&ldquo;the most allowed.&rdquo;"),
    msg(q("The source is too quiet to normalise: it never rises above &minus;50 dBFS."),
        "The song is nearly silent, so Normalise left it alone."),
    msg(q("Trimmed 0.0 s from the start and 3.0 s from the end.") + ", "
        + q("No long silence at the ends to trim.") + " and "
        + q("The whole source is silent, so nothing was trimmed."),
        "What Trim silence at the ends did."),
    msg(q("The source says it lasts ... s, but only ... s of it could be read: it may be "
          "damaged or cut short."),
        "The song ends sooner than it should. The file made has what could be read."),
    msg(q("The source peaks above full scale (... dBFS), so its loudest moments are clipped "
          "slightly. Normalise would lower it."),
        "The song is too loud for a file to hold. Turn on Normalise and convert it again."),
    msg(q("The title has nothing MacMusic can show, so the WAV has none."),
        "The title uses only letters the Macintosh can&rsquo;t show (Japanese, say). "
        "Type one MacMusic can show, or type it in MacMusic when you import."),
    msg(q("The WAV peaks at ... dBFS, below the target: its channels differ, and mixing them "
          "into one lowers the peak."),
        "A stereo song&rsquo;s two channels are mixed into one for MacMusic, and the "
        "mix is a little quieter. Nothing is wrong."),
    msg(q("The MP3 peaks at ... dBFS: encoding pushed it past full scale, so it may clip on "
          "playback."),
        "Making the MP3 made it a fraction too loud. Lower Normalise to (Advanced) a "
        "little, and convert it again."),
    msg(q("failed: nothing was saved"),
        "A file failed its final checks (its length, its format or its sound), so it "
        "wasn&rsquo;t kept; the reason is shown under it. Convert it again. This "
        "shouldn&rsquo;t happen: if it does again, note the message."),

    part("MacMusic Transfer Utility"),
    "<p class=\"grey\">CHECK: these are quoted from the Transfer Utility 1.0&rsquo;s "
    "source; they&rsquo;ll be pictured, and checked against its screens, when its "
    "chapter is written.</p>",

    sub("Adding Files"),
    msg(q("It isn&rsquo;t a WAV file."),
        "Only WAV files can go on the disk. Convert the song with the Converter first."),
    msg(q("It isn&rsquo;t in MacMusic&rsquo;s format: ..."),
        "It&rsquo;s a WAV file, but not the kind MacMusic plays; the message says how it "
        "differs. Convert it with the Converter first."),
    msg(q("&ldquo;...&rdquo; changed or disappeared after it was added. Add it again."),
        "The file was changed, moved or deleted between adding it and building the disk."),

    sub("Building the Disk Image"),
    msg(q("Add some WAV files first."), "There&rsquo;s nothing to put on the disk."),
    msg(q("These files can&rsquo;t go on the disk: .... Remove them, or convert them with the "
          "MacMusic Converter Utility."),
        "Some files in the list were refused. Remove them, or convert them first."),
    msg(q("SCSI ID ... can&rsquo;t be used: choose 0 to 6 (7 is the Mac itself)."),
        "Choose another SCSI ID."),
    msg("<b>ID 0 is your boot disk&rsquo;s</b><br>" + q("An image with ID 0 would clash with the "
        "disk your Macintosh Plus starts up from. Choose another ID unless you mean to "
        "replace it on the card."),
        "Choose another ID, or Use ID 0 Anyway if you really mean to replace your "
        "startup disk&rsquo;s image."),
    msg(q("BlueSCSI uses one image per SCSI ID: make sure no other image on the card uses "
          "ID ...."),
        "A reminder: two images with the same ID on the card would clash."),
    msg(q("... MB of files is too much: a System 6 disk holds at most 2 GB."),
        "Make two disks, each with some of the songs, on different SCSI IDs."),
    msg("<b>&ldquo;HD60_512 Transfer.hda&rdquo; is already in &ldquo;MacMusic Transfer&rdquo;</b><br>"
        + q("Keep Both saves the new one as &ldquo;&hellip; Transfer 2.hda&rdquo;. Both would use "
            "SCSI ID 6, so put only one of them on the card."),
        "Keep Both, Replace or Cancel, as the Converter&rsquo;s question."),
    msg(q("The folder to save in isn&rsquo;t available, or can&rsquo;t be written to: ..."),
        "Choose another folder."),
    msg(q("There isn&rsquo;t room for the disk image: it needs ... MB, and ... MB is free."),
        "Make room on your Mac&rsquo;s disk, or save it to another."),
    msg(q("The disk template can&rsquo;t be used: ..."),
        "A disk template chosen in Advanced isn&rsquo;t a usable BlueSCSI disk image. "
        "Click Use Default."),
    msg(q("The disk couldn&rsquo;t be made: ...") + " and " + q("The files (...) don&rsquo;t fit in "
        "the formatted volume (...)."),
        "Something went wrong while making the disk; the message says what. Nothing "
        "was saved. Try again."),
    msg(q("The finished disk failed its checks, so it wasn&rsquo;t kept: ..."),
        "The disk was checked before it was kept, and something wasn&rsquo;t right, so "
        "it was thrown away. Try again; this shouldn&rsquo;t happen."),
    msg(q("Cancelled. Nothing was saved."), "You clicked Cancel."),
    msg("<b>Stop building the disk and quit?</b><br>" + q("The disk being built won&rsquo;t be "
        "saved. Your files and any finished disk are left as they are."),
        "You chose Quit while a disk was being built."),
])

# ---- Appendix B: About the Files ---------------------------------------------

B = "".join([
    sub("MacMusic WAV Files"),
    "<p>MacMusic plays one kind of sound file only: a WAV file of plain sound "
    "(&ldquo;PCM&rdquo;), mono, with 22,050 samples a second, each of 8 bits. "
    "The <i>MacMusic User&rsquo;s Guide</i>, Appendix B, explains why. The "
    "Converter makes exactly this kind, and puts the song&rsquo;s title inside "
    "it, where MacMusic finds it when you import the song.</p>",
    "<p>A MacMusic WAV takes 22,050 bytes for each second of sound:</p>",
    table(("", "Holds"),
          ("A minute of music", "about 1.3 MB"),
          ("A 3&frac12;-minute song", "about 4.6 MB"),
          ("An 800K floppy disk", "about 37 seconds"),
          ("A 20 MB hard disk", "about 15 minutes"),
          ("A 100 MB disk", "about 75 minutes"), cls="ref spec"),
    "<p>Its name is at most 31 characters, with .wav, since that&rsquo;s the "
    "longest a Macintosh file name can be.</p>",

    sub("MP3 Files"),
    "<p>The Converter&rsquo;s MP3s are 192 kbps, stereo if the song is: about "
    "1.4 MB a minute. They&rsquo;re for your Mac, a phone or any other player; "
    "MacMusic doesn&rsquo;t play them. An MP3&rsquo;s name keeps every character "
    "of the title.</p>",

    sub("Disk Images"),
    "<p>A <b class=\"term\">disk image</b> is a whole hard disk kept in one "
    "file. A BlueSCSI, fitted to the Macintosh Plus&rsquo;s SCSI port, reads "
    "disk images from its SD card, and the Plus sees each as a hard disk of its "
    "own.</p>",
    "<p>The BlueSCSI reads what a disk image is from its name:</p>",
    table(("Part of the name", "Means"),
          ("HD", "a hard disk"),
          ("6", "its SCSI ID, 0 to 6 (7 is the Macintosh itself)"),
          ("0", "the first (and only) disk at that ID"),
          ("_512", "blocks of 512 bytes, the Plus&rsquo;s own"),
          ("Transfer", "a label, for you"),
          (".hda", "a hard disk image"), cls="ref narrow"),
    "<p>So <b>HD60_512 Transfer.hda</b> is a hard disk at SCSI ID 6. Each image "
    "on the card needs an ID of its own.</p>",
    "<p>A disk made by the Transfer Utility starts as Apple&rsquo;s hard disks "
    "do, with a partition map and Apple&rsquo;s hard disk driver (from its disk "
    "template), and holds one disk, called Transfer, with one folder, WAV Files, "
    "holding the songs. Its size is the songs&rsquo;, plus a quarter as much "
    "again for adding more, plus 4 MB for the disk&rsquo;s own needs, rounded up "
    "to a whole megabyte: at least 10 MB, and at most 2 GB, the largest disk "
    "System 6 can use.</p>",
    "<p class=\"grey\">CHECK: confirmed on the Macintosh Plus with the Transfer "
    "Utility&rsquo;s first disk (2026-10-09); its 1.0 check is pending.</p>",

    sub("Names on the Macintosh"),
    "<p>Macintosh file names are at most 31 characters, can&rsquo;t contain a "
    "colon, and ignore the difference between capital and small letters. So the "
    "Transfer Utility changes names where it must: a long name is shortened; a "
    "colon becomes a hyphen (Rock: Live.wav becomes Rock- Live.wav); and of two "
    "names that differ only in capitals, the second gets a number (clash.wav "
    "becomes clash 2.wav). Letters the Macintosh can&rsquo;t show are changed "
    "for the nearest it can. The window shows every new name before the disk is "
    "made.</p>",
])

# ---- Appendix C: Troubleshooting ------------------------------------------------

C = "".join([
    sub("On Your Mac"),
    trouble("A program won&rsquo;t open the first time",
            "The utilities are made for your own use, and aren&rsquo;t registered "
            "with Apple, so macOS may refuse to open one, saying it can&rsquo;t check "
            "it. If it does, open System Settings, click Privacy &amp; Security, and "
            "click Open Anyway beside the program&rsquo;s name; then open it again. "
            "After that it opens normally. (A program copied from the USB drive "
            "usually opens straight away.)",
            "CHECK: not yet seen with these programs on macOS 26."),
    trouble("YouTube stops working",
            "If the Converter says YouTube has changed in a way its yt-dlp can&rsquo;t "
            "handle, the Converter needs a newer yt-dlp inside it. That means "
            "rebuilding the Converter from its source, as its BUILDING.md describes "
            "(tools/update-yt-dlp.sh). Converting files from your Mac still works "
            "meanwhile."),
    trouble("Songs aren&rsquo;t equally loud",
            "Normalise evens out each song&rsquo;s loudest moment, not how loud it "
            "sounds overall, so a quiet song with one loud drum may still sound "
            "quieter than others. MacMusic&rsquo;s own volume sets the level you "
            "hear."),
    trouble("A file name has changed",
            "A MacMusic WAV&rsquo;s name is at most 31 characters, with no colons: "
            "see Appendix B, &ldquo;Names on the Macintosh.&rdquo; Type a shorter "
            "File name in the Converter for a better one."),

    sub("On the Macintosh Plus"),
    trouble("The disk doesn&rsquo;t appear",
            "Check that the image&rsquo;s name starts HD and a SCSI ID (0 to 6), "
            "that no other image on the card has the same ID, and that the card "
            "went back into the BlueSCSI before you switched the Plus on.",
            "CHECK: to be completed after the Transfer Utility&rsquo;s Plus check."),
    trouble("A song won&rsquo;t import",
            "MacMusic says why: see Appendix A of the <i>MacMusic User&rsquo;s "
            "Guide</i>. A file made by the Converter, or put on a disk by the "
            "Transfer Utility, has already been checked against MacMusic&rsquo;s "
            "rules, so the commonest cause is a file that came another way."),
])

# ---- Appendix D: Specifications and Licences ------------------------------------

D = "".join([
    sub("What You Need"),
    table(("", ""),
          ("Your Mac", "a Mac with Apple silicon and macOS 26"),
          ("The Macintosh", "a Macintosh Plus with System 6.0.7, MacMusic 1.0, and a "
           "BlueSCSI (for the Transfer Utility&rsquo;s disks)"),
          ("YouTube", "an internet connection; the Converter doesn&rsquo;t sign in"),
          cls="ref spec"),

    sub("MacMusic Converter Utility 1.0"),
    table(("", ""),
          ("Reads", "a sound file (MP3, AAC or M4A, WAV, FLAC, Opus and most others), "
           "or one YouTube video"),
          ("Makes", "a MacMusic WAV (mono, 22,050 Hz, 8-bit), an MP3 (192 kbps), or both"),
          ("Normalise", "the loudest peak to &minus;6.0 to 0.0 dBFS (&minus;1.0 to start)"),
          ("Trim silence", "longer than 2 or 3 seconds at the start or end, to half a second"),
          ("Saves to", "~/Music/MacMusic Converter, or a folder you choose"),
          cls="ref spec"),

    sub("MacMusic Transfer Utility 1.0"),
    table(("", ""),
          ("Reads", "MacMusic WAV files, and folders of them (one level)"),
          ("Makes", "a BlueSCSI hard disk image: HD<i>n</i>0_512 Transfer.hda, SCSI ID 0 to 6"),
          ("The disk", "Apple&rsquo;s partition map and driver; one HFS volume, Transfer, "
           "with a folder, WAV Files"),
          ("Size", "the songs plus a quarter, plus 4 MB; 10 MB to 2 GB"),
          ("Saves to", "~/Music/MacMusic Transfer, or a folder you choose"),
          cls="ref spec"),

    sub("Software Inside the Utilities"),
    "<p>The utilities do their work with these programs, which come inside them, "
    "each under its own licence:</p>",
    table(("Program", "Version", "Licence", "In"),
          ("FFmpeg", "9.0.2", "GPL version 3", "the Converter"),
          ("yt-dlp", "2026.08.19", "the Unlicense", "the Converter"),
          ("Deno", "2.9.7", "MIT", "the Converter"),
          ("hfsutils", "3.2.6", "GPL version 2", "the Transfer Utility"),
          cls="ref narrow"),
    "<p>The Converter shows their licences from Advanced, Licences. The source "
    "code of the GPL programs, as used, comes with each program&rsquo;s release "
    "on the USB drive (CHECK: the drive&rsquo;s contents are settled when it&rsquo;s "
    "made, UB8).</p>",
    "<p>The Transfer Utility&rsquo;s disk template holds Apple&rsquo;s hard disk "
    "driver. Like everything on the USB drive, it&rsquo;s for the owner&rsquo;s "
    "own use only.</p>",
])

DRAFT = draft_title("Appendixes A to D",
                    "A draft for review, 9 October 2026, for the MacMusic Converter "
                    "Utility and the MacMusic Transfer Utility; the Macintosh Backup "
                    "Utility&rsquo;s parts follow when it exists. CHECK marks what "
                    "waits for screenshots or the Plus check.")
PART = (appendix("A", "Messages", A)
        + appendix("B", "About the Files", B)
        + appendix("C", "Troubleshooting", C)
        + appendix("D", "Specifications and Licences", D))
# A subheading stays on the page with its first message.
PART = re.sub(r'(<h3 class="ref-head loose"[^>]*>.*?</h3>)(<div class="msg">.*?</div>)',
              r'<div class="keep">\1\2</div>', PART, flags=re.S)

if __name__ == "__main__":
    open(OUT, "w").write(page("MacMusic Utilities: Appendixes", DRAFT + PART))
    print("wrote", os.path.normpath(OUT))
