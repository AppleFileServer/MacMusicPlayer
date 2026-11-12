#!/usr/bin/env python3
"""Writes docs/utilities-manual/appendixes.html, the appendixes of
*MacMusic Utilities* (Phase 6; docs/utilities-manual/outline.md):
  A  Messages: every message each utility shows, quoted from its 1.0
     source, with what it means and what to do;
  B  About the Files;
  C  Troubleshooting;
  D  Specifications and Licences.

The Backup Utility's parts are quoted from its 1.0 source (Sources/,
Errors.swift and the window's sheets). CHECK marks what waits for the
owner's Plus check of the Transfer Utility, a real card, or the USB
drive (UB7, UB8).

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
        "Cancel makes nothing.", ("cv-reencode", 180)),
    msg("<b>" + q("MacMusic.wav") + " already exists in " + q("Music Downloads") + "</b><br>"
        + q("Keep Both saves the new file under the next free name. Replace puts it in "
            "place of what&rsquo;s there."),
        "Keep Both gives the new file the next free name (MacMusic 2.wav); "
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
        "Stop and Quit stops and quits.", ("cv-quit", 150)),

    sub("In the Results"),
    "<p>After a conversion, Results says what was done to the sound, and may add "
    "a note. Notes are for your information; the file is fine.</p>",
    msg(q("Levelled from &minus;24.7 LUFS by +8.7 dB; the limiter took up to 2.7 dB off "
          "the loudest moments."),
        "How loud the song was, and how much louder (+) or quieter (&minus;) it was "
        "made to reach the Converter&rsquo;s loudness, &minus;16 LUFS; then how much "
        "the limiter took off its loudest moments, if it took anything. A song is "
        "raised by 20 dB at most, so a very quiet one may not reach the loudness: "
        "&ldquo;(the most allowed).&rdquo;"),
    msg(q("The source is too quiet to level: it never rises above &minus;50 dBFS."),
        "The song is nearly silent, so it was left as it was."),
    msg(q("Levelling it took up to ... dB off its loudest moments, so it may sound "
          "squashed. A lower loudness target would take off less."),
        "The limiter had to work hard (more than 6 dB) to bring a song with very "
        "loud moments to the loudness. Listen to it; the Converter&rsquo;s loudness "
        "can&rsquo;t be changed."),
    msg(q("It comes out at ... LUFS, not the &minus;16.0 LUFS asked for: the limiter took "
          "more off its loudest moments."),
        "The song is a little quieter than the others, because the limiter took "
        "off more than expected. Nothing is wrong."),
    msg(q("Trimmed 0.0 s from the start and 3.0 s from the end.") + ", "
        + q("No long silence at the ends to trim.") + " and "
        + q("The whole source is silent, so nothing was trimmed."),
        "What Trim silence at the ends did."),
    msg(q("The source says it lasts ... s, but only ... s of it could be read: it may be "
          "damaged or cut short."),
        "The song ends sooner than it should. The file made has what could be read."),
    msg(q("The source peaks above full scale (... dBFS), so its loudest moments are clipped "
          "slightly."),
        "The song you started from was already too loud for a file to hold, and "
        "being made no quieter, its loudest moments stay clipped. Nothing more can "
        "be done with that copy."),
    msg(q("The title has nothing MacMusic can show, so the WAV has none."),
        "The title uses only letters the Macintosh can&rsquo;t show (Japanese, say). "
        "Type one MacMusic can show, or type it in MacMusic when you import."),
    msg(q("The MP3 peaks at ... dBFS: encoding pushed it past full scale, so it may clip on "
          "playback."),
        "Making the MP3 made it a fraction too loud, so it may crackle at its loudest "
        "moments on your Mac. The MacMusic WAV isn&rsquo;t affected."),
    msg(q("failed: nothing was saved"),
        "A file failed its final checks (its length, its format, its sound or its "
        "loudness), so it "
        "wasn&rsquo;t kept; the reason is shown under it. Convert it again. This "
        "shouldn&rsquo;t happen: if it does again, note the message."),

    part("MacMusic Transfer Utility"),
    "<p class=\"grey\">These are quoted from the Transfer Utility 1.0&rsquo;s source; "
    "those Chapter 3 pictures were checked against its screens.</p>",

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
        "You chose Quit while a disk was being built. Keep Building carries on; Stop "
        "and Quit stops and quits.", ("tu-quit", 150)),

    part("Macintosh Backup Utility"),
    "<p class=\"grey\">These are quoted from the Backup Utility 1.0&rsquo;s source; "
    "those Chapter 4 pictures were checked against its screens.</p>",

    sub("Your Card"),
    msg(q("Waiting for a card"),
        "No card is chosen. Shut the Plus down, and put its card in your Mac; it&rsquo;s "
        "recognised when it appears. Or click Choose Card."),
    msg("<b>Register this card?</b><br>" + q("... looks like a BlueSCSI card. Give it a name, "
        "and its backups are kept together under it, and it&rsquo;s recognised whenever "
        "it&rsquo;s inserted."),
        "Type a name and click Register, or click Not Now. Nothing is written to the "
        "card."),
    msg("<b>Is this ...?</b><br>" + q("It has the same name, size and format as ..., but macOS "
        "gives it a different identity, as it does a card that&rsquo;s been reformatted."),
        "Yes, the Same Card keeps its backups together; No, a Different Card registers "
        "it as a new one."),
    msg(q("... is connected") + " (a notification)",
        "The card went in while the Backup Utility was in the background."),
    msg(q("macOS hasn&rsquo;t allowed this app to read the card. Allow it in System Settings, "
          "Privacy &amp; Security, Files &amp; Folders (Removable Volumes), then choose the "
          "card again."),
        "macOS asks once whether the Backup Utility may read removable disks; the answer "
        "was no. Change it where the message says."),
    msg(q("The card couldn&rsquo;t be read: ..."),
        "The card, or a file on it, can&rsquo;t be read. Eject it, put it in again, and "
        "try again; if it still fails, the card may be failing: restore its backup onto "
        "a new one."),
    msg(q("The card couldn&rsquo;t be ejected"),
        "Something on your Mac is using the card. Close it, and try again, or eject the "
        "card in the Finder."),

    sub("Backing Up"),
    msg(q("... wasn&rsquo;t shut down cleanly: the Plus may have lost power while using it. "
          "This backup keeps it as it is; an earlier snapshot may be the better one to "
          "restore."),
        "A disk on the card wasn&rsquo;t put away properly. The backup goes ahead; think "
        "which snapshot to restore from if you need to.", ("bu-verified-snapshot", 260)),
    msg(q("The card holds a BlueSCSI firmware update package; it&rsquo;s backed up like "
          "everything else."),
        "A file that updates the BlueSCSI itself is on the card. It&rsquo;s left out when "
        "restoring onto a new card, unless you ask for it."),
    msg(q("There isn&rsquo;t room for this backup in the backup folder. Free some space, or "
          "choose another folder in Settings.") + " and " + q("There isn&rsquo;t room for "
          "the backup: it needs about ... MB, and ... MB is free."),
        "Make room on your Mac, or choose a backup folder on another disk."),
    msg(q("The backup folder isn&rsquo;t available, or can&rsquo;t be written to: ..."),
        "The folder is on a disk that isn&rsquo;t connected, or is locked. Connect it, or "
        "choose another in Settings."),
    msg(q("Backed up and verified") + " and " + q("Backed up and verified, with warnings"),
        "Every file was copied and checked; the snapshot is in the history. With "
        "warnings: see the warning above."),
    msg(q("Not verified: the card changed during the backup"),
        "A file was different afterwards: " + q("It was changed during the backup.") + ", "
        + q("Its size changed during the backup.") + ", " + q("It was replaced during the "
        "backup.") + " or " + q("It disappeared from the card during the backup.") + " "
        "Make sure nothing else is using the card, and back up again."),
    msg(q("The backup failed") + " and " + q("The backup couldn&rsquo;t start"),
        "A file couldn&rsquo;t be read or written; the message names it."),
    msg(q("The backup was cancelled") + " and " + q("The card was removed during the backup"),
        "Nothing is kept as a snapshot. Each ends with " + q("It isn&rsquo;t a restore point; "
        "your earlier backups aren&rsquo;t affected.")),
    msg(q("A snapshot called ... is already there, so this one wasn&rsquo;t saved."),
        "A snapshot of the same name is already in the backup folder (most unlikely, "
        "as each name ends with a number of its own). Back up again."),

    sub("The History"),
    msg(q("Verified: ... files match") + " and " + q("... problems found"),
        "Verify Backup&rsquo;s result. A problem is a file missing, the wrong size, "
        "&ldquo;its contents have changed,&rdquo; one that can&rsquo;t be read, or one "
        "&ldquo;not in the manifest.&rdquo; A snapshot with problems may be damaged: "
        "restore from another."),
    msg("<b>Delete the snapshot ...?</b><br>" + q("It goes to the Trash, so it can be put back "
        "until the Trash is emptied."),
        "Move to Trash or Cancel. If it&rsquo;s the card&rsquo;s only backup, it says so.",
        ("bu-delete", 180)),
    msg("<b>Clear the incomplete jobs?</b><br>" + q("They hold no restore points: backups that "
        "failed, were cancelled, or found the card changed. Your snapshots aren&rsquo;t "
        "affected."), "Move to Trash or Cancel."),
    msg(q("Interrupted: it has no record (the app or the Mac stopped during the backup)."),
        "An incomplete job left by a backup that never finished. Clear it."),
    msg(q("Its folder and its manifest don&rsquo;t agree.") + " and " + q("The snapshot&rsquo;s "
        "manifest isn&rsquo;t valid: ..."),
        "Something in the backup folder has been changed by hand. Don&rsquo;t restore from "
        "that snapshot."),
    msg(q("The snapshot couldn&rsquo;t be deleted"),
        "The message says why; a snapshot that&rsquo;s locked, or on a disk that&rsquo;s "
        "gone, can&rsquo;t be."),

    sub("Restoring"),
    msg("<b>&ldquo;...&rdquo; is already there</b><br>" + q("Keep Both saves the backup&rsquo;s "
        "copy under the next free name. Replace puts it in place of what&rsquo;s there."),
        "Keep Both, Replace or Skip; with Do the same for the rest, the answer goes for "
        "every such file.", ("bu-conflict", 160)),
    msg(q("Restored ... files, each checked against the backup.") + " and " + q("... file(s) "
        "couldn&rsquo;t be restored."),
        "The result, with any files skipped or kept under a new name listed below it. A "
        "file that fails says why: " + q("The backup&rsquo;s copy doesn&rsquo;t match its "
        "manifest: the backup may be damaged.")),
    msg(q("... isn&rsquo;t a complete, verified snapshot, so it can&rsquo;t be restored from."),
        "Only a verified snapshot can be restored from."),

    sub("Restoring Onto a New Card"),
    msg(q("Empty, and ready."), "The card can be restored onto."),
    msg(q("The card isn&rsquo;t empty (...). Restore only onto a newly formatted card, so "
          "nothing on it can be replaced."),
        "The Backup Utility won&rsquo;t write to a card that holds anything. Use a new "
        "card, or erase this one in Disk Utility if you&rsquo;re sure."),
    msg(q("The card is formatted as ...; BlueSCSI needs exFAT (or FAT32). Format it as exFAT "
          "first."),
        "In Disk Utility: Erase, Format ExFAT, Scheme Master Boot Record."),
    msg(q("The card is too small: it needs ... MB, and has ... MB free."), "Use a bigger card."),
    msg("<b>Restore onto ...?</b><br>" + q("... is written to ... The Macintosh Plus will use "
        "these disks when the card goes in its BlueSCSI. This isn&rsquo;t a copy of the whole "
        "old card, only of its files."), "Restore to Card, or Cancel."),
    msg(q("Restored ... files; all ... checked on the card.") + " and " + q("Some files "
        "don&rsquo;t match: ..."),
        "The result. If some don&rsquo;t match, the new card may be faulty: try another."),
    msg(q("Recorded as ...&rsquo;s replacement: its backups carry on under that name."),
        "After Use as ...&rsquo;s Replacement."),
])

# ---- Appendix B: About the Files ---------------------------------------------

B = "".join([
    sub("MacMusic WAV Files"),
    "<p>MacMusic plays one kind of sound file only: a WAV file of plain sound "
    "(&ldquo;PCM&rdquo;), mono, with 8 bits to each sample, at 22,254 or 22,050 "
    "samples a second. The <i>MacMusic User&rsquo;s Guide</i>, Appendix B, "
    "explains why. The Converter makes them at 22,254, the Macintosh "
    "Plus&rsquo;s own rate, so they play at their true speed and pitch "
    "(MacMusic 1.2 or later; earlier versions play only 22,050). It puts the "
    "song&rsquo;s title inside the file, where MacMusic finds it when you "
    "import the song.</p>",
    "<p>A MacMusic WAV takes 22,254 bytes for each second of sound:</p>",
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
    "names that differ only in capitals, the second gets a number (Clash.wav, "
    "beside clash.wav, becomes Clash 2.wav). Letters the Macintosh can&rsquo;t show are changed "
    "for the nearest it can. The window shows every new name before the disk is "
    "made.</p>",

    sub("The Backup Folder"),
    "<p>The Macintosh Backup Utility keeps everything in one folder, Mac Plus "
    "Backups in your home folder unless you choose another:</p>",
    table(("", ""),
          ("Snapshots", "a folder for each snapshot, named by its date and time and a "
           "number (2026-10-10_12-05-13_4914, say), holding Files, a copy of everything "
           "on the card, and SnapshotManifest.json, the record of every file with its "
           "size and check-sum (SHA-256)"),
          ("Catalog", "the list of snapshots, for quick browsing (catalog.json); the "
           "registered cards (cards.json); the snapshots marked Keep (pins.json)"),
          ("Logs", "made with the folder; 1.0 keeps nothing in it"),
          cls="ref spec"),
    "<p>A snapshot is never changed once it&rsquo;s made, and each stands on its "
    "own: its disk images are ordinary files, which can be copied straight back "
    "onto a card with the Finder. A backup still in progress is in a hidden "
    "folder, and only gets its name when every file has been checked. Leave "
    "the folder&rsquo;s contents to the Backup Utility: don&rsquo;t rename or "
    "change them.</p>",
    "<p>Each snapshot takes as much room as the card&rsquo;s files: a card with "
    "2 GB of disks takes about 2 GB a backup. Thinning keeps that in check.</p>",
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
            "Songs made by the Converter 1.1 are all brought to the same loudness. "
            "Songs made by the Converter 1.0 had only their loudest moments evened "
            "out, so they may sound quieter or louder than the rest: convert them "
            "again. A very quiet song can be raised only 20 dB, and may stay a "
            "little quiet."),
    trouble("A file name has changed",
            "A MacMusic WAV&rsquo;s name is at most 31 characters, with no colons: "
            "see Appendix B, &ldquo;Names on the Macintosh.&rdquo; Type a shorter "
            "File name in the Converter for a better one."),

    sub("Backups"),
    trouble("The card isn&rsquo;t recognised",
            "The Backup Utility knows a BlueSCSI card by its bluescsi.ini, its log, or "
            "its disk images. If it doesn&rsquo;t recognise one, click Choose Card and "
            "choose it yourself.",
            "CHECK: not yet seen with a real card in this Mac."),
    trouble("The card can&rsquo;t be read",
            "The first time, macOS asks whether the Backup Utility may read removable "
            "disks. If you said no, allow it in System Settings, Privacy &amp; "
            "Security, Files &amp; Folders, and choose the card again."),
    trouble("The backup says the card changed",
            "Something else read or wrote the card during the backup: the Finder "
            "making its hidden files, perhaps, or another program. Back up again."),
    trouble("The backups take too much room",
            "Turn on thinning in Settings, delete snapshots you don&rsquo;t need, or "
            "choose a backup folder on a bigger disk. Deleted snapshots go to the "
            "Trash: empty it to get the room back."),
    trouble("A disk on the Plus has gone wrong",
            "Restore its disk image from a snapshot from before it went wrong "
            "(Chapter 4, Lesson 2). A snapshot marked with a warning sign has a disk "
            "that wasn&rsquo;t shut down cleanly; one before it may be better."),
    trouble("The card itself has failed",
            "Restore the newest snapshot onto a new card (Chapter 4, &ldquo;To Restore "
            "Onto a New Card&rdquo;)."),

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
          ("The Macintosh", "a Macintosh Plus with System 6.0.7, MacMusic 1.2 or later, and a "
           "BlueSCSI (for the Transfer Utility&rsquo;s disks)"),
          ("YouTube", "an internet connection; the Converter doesn&rsquo;t sign in"),
          ("Backups", "a slot or reader for SD cards; room on your Mac for the "
           "snapshots"),
          cls="ref spec"),

    sub("MacMusic Converter Utility 1.1"),
    table(("", ""),
          ("Reads", "a sound file (MP3, AAC or M4A, WAV, FLAC, Opus and most others), "
           "or one YouTube video"),
          ("Makes", "a MacMusic WAV (mono, 22,254 Hz, 8-bit; for MacMusic 1.2 or later), "
           "an MP3 (192 kbps), or both"),
          ("Loudness", "every song to &minus;16 LUFS, a limiter at &minus;1 dBFS; raised "
           "20 dB at most"),
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

    sub("Macintosh Backup Utility 1.0"),
    table(("", ""),
          ("Reads", "a BlueSCSI SD card (exFAT or FAT32), or a folder you choose; never "
           "writes to it"),
          ("Makes", "snapshots: a full copy of the card, every file checked (SHA-256)"),
          ("Keeps", "with thinning, the newest of each day for a week, each week for a "
           "month, each month for a year; at least two; any marked Keep"),
          ("Restores", "files, or the whole snapshot, to a folder; or everything onto a "
           "new, empty card"),
          ("Saves to", "~/Mac Plus Backups, or a folder you choose"),
          cls="ref spec"),
    "<p>The Backup Utility uses only macOS&rsquo;s own software.</p>",

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
                    "A draft for review, 10 October 2026, for all three utilities. "
                    "CHECK marks what waits for the Plus check, a real card, or the "
                    "USB drive (UB8).")
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
