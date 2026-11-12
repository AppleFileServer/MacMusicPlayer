#!/usr/bin/env python3
"""Writes docs/utilities-manual/card.html, the *MacMusic Quick Reference
Card*: one folded card, four pages the size of the manuals' (7.5 x 9 in),
in the manner of Apple's Macintosh Quick Reference Card (1988, part
030-3180-B): a red rule across the top of each page, each section headed
by a red square, and under it two columns, what to do and how, a thin
rule between the rows.

Pages 1 and 2: the three utilities, on your Mac (from *MacMusic
Utilities*, Chapters 2 to 4). Pages 3 and 4: MacMusic on the Macintosh
Plus (from the *MacMusic User's Guide*, Chapter 3 and Appendix C).
Nothing here that the two manuals don't say.

Build it with tools/utilities-manual/build-pdf.sh docs/utilities-manual/card.html <pdf>."""
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from ub import page

OUT = os.path.join(HERE, "..", "..", "docs", "utilities-manual", "card.html")

STYLE = """<style>
@page card { margin: 30pt 40pt 34pt 40pt; }
.card { page: card; font: 9pt/11pt var(--serif); }
.card .top { border-top: 2.2pt solid var(--red); padding-top: 9pt; margin-bottom: 4pt; }
.card .title { font: 19pt/22pt var(--serif); margin: 0 0 1pt 0; }
.card .title .apple { color: var(--red); font-family: var(--sans); }
.card .where { font: 9pt/11pt var(--serif); font-style: italic; color: #555; margin: 0 0 4pt 0; }
.card h2 { font: bold 10pt/12pt var(--serif); margin: 10pt 0 3pt 0; padding-left: 13pt; position: relative;
           break-after: avoid; }
.card h2::before { content: ""; position: absolute; left: 0; top: 2.5pt; width: 6.5pt; height: 6.5pt;
                   background: var(--red); }
.card h3 { font: italic bold 8.5pt/10pt var(--serif); margin: 4pt 0 1pt 0; }
.card table { width: 100%; border-collapse: collapse; break-inside: avoid; }
.card td { vertical-align: top; padding: 2pt 0 2.5pt 0; border-bottom: 0.5pt solid #777; }
.card td:first-child { width: 42%; padding-right: 10pt; }
.card table.three td:first-child { width: 36%; }
.card table.three td:nth-child(2) { width: 34%; padding-right: 8pt; }
.card th { font: italic bold 8.5pt/10pt var(--serif); text-align: left; padding: 0 0 2pt 0; }
.card ul { margin: 1pt 0 0 0; padding-left: 10pt; }
.card li { margin: 0; list-style: square; }
.card .note { font: 8pt/10pt var(--serif); color: #555; margin: 3pt 0 0 0; }
.card ol.journey { margin: 2pt 0 0 0; padding-left: 14pt; }
.card ol.journey li { list-style: decimal; margin: 0 0 1.5pt 0; }
.card kbd { font-size: 7.5pt; line-height: 9.5pt; }
.card .foot { font: 7.5pt/9.5pt var(--serif); color: #555; margin-top: 14pt; }
</style>
"""


def k(*names):
    return "".join("<kbd>%s</kbd>" % n for n in names)


def rows(*pairs, head=None, cls=""):
    h = ("<tr>%s</tr>" % "".join("<th>%s</th>" % c for c in head)) if head else ""
    body = "".join("<tr>%s</tr>" % "".join("<td>%s</td>" % c for c in r) for r in pairs)
    return '<table class="%s">%s%s</table>' % (cls, h, body)


def card(title, where, *parts, first=False):
    return ('<div class="card%s"><div class="top"><p class="title">%s</p><p class="where">%s</p></div>%s</div>\n'
            % ("" if first else " page-break", title, where, "".join(parts)))


def h2(t):
    return "<h2>%s</h2>" % t


# ---- Page 1: a song's journey, and the Converter ------------------------------

PAGE_1 = card(
    '<span class="apple">&#9632;</span> MacMusic Quick Reference Card',
    "The MacMusic utilities on your Mac, and MacMusic on the Macintosh Plus",
    h2("A song&rsquo;s journey"),
    '<ol class="journey">'
    "<li><b>Convert it</b> to a MacMusic WAV with the MacMusic Converter Utility.</li>"
    "<li><b>Put it on a disk</b> for the Plus with the MacMusic Transfer Utility: HD60_512 "
    "Transfer.hda.</li>"
    "<li><b>Shut the Plus down</b> (Shut Down, in the Special menu), switch it off, and put "
    "its SD card in your Mac.</li>"
    "<li><b>Back the card up</b> with the Macintosh Backup Utility: Back Up Now, then Back Up.</li>"
    "<li><b>Copy the disk image</b> onto the card&rsquo;s top level; eject the card, put it "
    "back in the BlueSCSI, and switch the Plus on.</li>"
    "<li><b>Import the song</b> in MacMusic from the Transfer disk&rsquo;s WAV Files folder "
    "(page 4).</li></ol>",

    h2("MacMusic Converter Utility"),
    rows(("To convert a sound file", "Click Local File, then Choose File; or drag the song "
          "onto the window. Click MacMusic WAV, then Convert."),
         ("To convert a YouTube video", "Click YouTube, paste the address, click Look Up "
          "(or press Return), then Convert. Only videos you own or may save."),
         ("To make an MP3 too", "Under Output, click Both (or MP3 for the MP3 alone). An MP3 "
          "made from an MP3 asks first: Re-encode, WAV Only or Cancel."),
         ("To set the title MacMusic shows", "Type it in Title. File name, left empty, follows "
          "the title; Saves as shows the names."),
         ("The loudness", "Nothing to set: every song is brought to &minus;16 LUFS, "
          "with a limiter at &minus;1 dBFS."),
         ("To trim silence at the ends", "Turn on Trim silence at the ends, and choose 2 or 3 "
          "seconds in Longer than."),
         ("To choose where files go", "Under Save To, click Choose. Use Default: the MacMusic "
          "Converter folder in your Music folder."),
         ("To find what was made", "Click Show in Finder beside the file in Results."),
         ("To cancel", "Click Cancel, or press " + k("Esc") + ". Nothing half-made is kept."),
         ("If a file of that name is there", "Keep Both (the next free name), Replace, or "
          "Cancel.")),
    '<p class="note">A MacMusic WAV is mono, 22,254 samples a second, 8-bit (MacMusic 1.2 '
    "or later): about 1.3 MB a minute. Its name is at most 31 characters, with .wav.</p>",
    first=True,
)

# ---- Page 2: the Transfer and Backup Utilities --------------------------------

PAGE_2 = card(
    "On your Mac",
    "The MacMusic Transfer Utility and the Macintosh Backup Utility",
    h2("MacMusic Transfer Utility"),
    rows(("To add songs", "Drag MacMusic WAV files, or a folder of them, onto the window; "
          "or click Add Files. Remove and Clear All take them off the list."),
         ("To choose the SCSI ID", "Choose 0 to 6 from the SCSI ID menu (6 to start). Each "
          "disk image on the card needs an ID of its own; 0 is usually your startup disk."),
         ("To make the disk image", "Click Build Transfer Image. It&rsquo;s saved in the "
          "MacMusic Transfer folder in your Music folder."),
         ("To copy it to the card", "Click Show in Finder, and drag the image onto the "
          "card&rsquo;s top level, not into a folder."),
         ("To see what was checked", "Click What was checked, under Result."),
         ("To cancel", "Click Cancel, or press " + k("Esc") + ". Nothing is saved."),
         ("If a song can&rsquo;t be used", "It isn&rsquo;t a MacMusic WAV: make one with "
          "the Converter, and add that."),
         ("If a name is changed", "Shown in orange under On the Mac: 31 characters at most, "
          "no colons, Clash.wav beside clash.wav becomes Clash 2.wav.")),

    h2("Macintosh Backup Utility"),
    rows(("Before the card comes out", "Shut the Plus down with Shut Down, in the Special "
          "menu, and switch it off when it says you may."),
         ("To back up the card", "Put it in your Mac. Click Back Up Now, then Back Up. "
          "Nothing is ever written to the card."),
         ("To register a new card", "The first time it&rsquo;s inserted: type a name, and "
          "click Register."),
         ("To put the card back", "Click Eject; put it in the BlueSCSI before you switch the "
          "Plus on."),
         ("To restore a disk image", "Select a snapshot, click Restore&hellip;, then Chosen "
          "files and folders; tick the image, choose a folder, click Restore. Then drag it "
          "onto the card."),
         ("To restore everything", "Restore&hellip;, The whole snapshot, into a new folder."),
         ("If the card itself has failed", "Format a new card in Disk Utility (ExFAT, Master "
          "Boot Record); Control-click a snapshot, Restore to a New Card&hellip;."),
         ("To check a snapshot", "Select it, and click Verify Backup."),
         ("To keep or delete a snapshot", "Control-click it: Keep (Never Thin), or "
          "Delete&hellip; (to the Trash)."),
         ("To change the settings", "Settings, in the Macintosh Backup Utility menu "
          "(" + k("&#8984;", ",") + "): the backup folder, and thinning.")),
    '<p class="note">Backups are kept in Mac Plus Backups, in your home folder. Thinning keeps '
    "a card&rsquo;s newest snapshot of each day for a week, each week for a month, each month "
    "for a year; at least two, and any marked Keep.</p>",
)

# ---- Page 3: MacMusic's controls ------------------------------------------------

PAGE_3 = card(
    "MacMusic on the Macintosh Plus",
    "The click wheel, the keyboard, and MacMusic&rsquo;s screens",
    h2("The click wheel and the keyboard"),
    rows(("Move up or down a list", "Drag round the wheel", k("&uarr;") + " " + k("&darr;")),
         ("Choose the highlighted item", "Click the centre button", k("Return") + " or " + k("Enter")),
         ("Go back one screen", "Click MENU", k("Delete")),
         ("Play; play or pause on Now Playing", "Click the bottom of the wheel", k("Space bar")),
         ("Next song", "Click the right of the wheel", k("&rarr;")),
         ("Song before, or start it again", "Click the left of the wheel", k("&larr;")),
         ("Fast-forward or rewind", "Hold down the right or left", k("Option") + " "
          + k("&rarr;") + " or " + k("&larr;") + ", held"),
         ("Change the volume (Now Playing)", "Turn the wheel: clockwise for louder",
          k("&darr;") + " louder, " + k("&uarr;") + " softer"),
         ("Use the scrubber, or rate a song", "Click the centre button, then turn the wheel",
          k("Return") + ", then " + k("&uarr;") + " " + k("&darr;")),
         ("Quit MacMusic", "", k("&#8984;") + " " + k("Q")),
         head=("To", "With the mouse", "With the keyboard"), cls="three"),
    '<p class="note">The keys work while the MacMusic window is in front. A hold starts after '
    "half a second, and works only on Now Playing.</p>",

    h2("The main menu"),
    rows(("Music", "Playlists, Artists, Albums, Songs, Genres, Composers, Audiobooks"),
         ("Extras", "The Clock"),
         ("Settings", "Shuffle, Repeat, About, Legal, Reset All Settings"),
         ("Shuffle Songs", "Every song but the audiobooks, in a random order"),
         ("Backlight", "The screen light on dark, and back"),
         ("Now Playing", "While a song is playing or paused")),

    h2("Settings"),
    rows(("Shuffle", "Off; Songs; Albums (each in track order)"),
         ("Repeat", "Off; One (the same song again); All (the list again)"),
         ("To change one", "Choose it, and click the centre button for the next value.")),
)

# ---- Page 4: MacMusic's menus, and importing -----------------------------------

PAGE_4 = card(
    "MacMusic on the Macintosh Plus",
    "Adding songs, the menus, and playlists",
    h2("Adding songs from the Transfer disk"),
    rows(("To import a song", "Choose Import Song from the File menu; open the Transfer "
          "disk, then WAV Files; select the song, and click Open."),
         ("&ldquo;Importing from another disk will take some time&rdquo;", "Click Import."),
         ("&ldquo;The file will be moved&rdquo;", "Click OK to move it into your library, or "
          "Just Copy to leave it on the Transfer disk too."),
         ("The song&rsquo;s details", "Type or change them (63 characters each), and click "
          "Import."),
         ("&ldquo;Import another song?&rdquo;", "Click Import Another for the next song, or "
          "Done when you&rsquo;ve finished.")),

    h2("The menus"),
    rows(("File", "Import Song&hellip;, Export Song&hellip; (gives the song back as a WAV "
          "file, and takes it out of the library), Choose Library&hellip;, Rescan Library, "
          "Library Info&hellip;, Page Setup&hellip;, Print Library&hellip; ("
          + k("&#8984;", "P") + "), the libraries used before, Quit ("
          + k("&#8984;", "Q") + ")"),
         ("Edit", "Edit Song&hellip;: the highlighted song&rsquo;s details, or Now "
          "Playing&rsquo;s"),
         ("Playlists", "New Playlist&hellip;, Rename&hellip;, Delete&hellip;, Remove from "
          "Playlist, Add to &ldquo;<i>name</i>&rdquo;"),
         ("Apple", "About MacMusic&hellip;, and your desk accessories: Control Panel sets the "
          "speaker volume, Alarm Clock the clock")),

    h2("Playlists"),
    rows(("To make one", "Choose New Playlist&hellip; from the Playlists menu, and type a "
          "name."),
         ("To add a song", "Highlight it, and choose Add to &ldquo;<i>name</i>&rdquo; from the "
          "Playlists menu."),
         ("To take a song off", "In the playlist, highlight it, and choose Remove from "
          "Playlist. It stays in the library.")),

    h2("Printing (MacMusic 1.1)"),
    rows(("To print a list of the library", "Choose Print Library&hellip; ("
          + k("&#8984;", "P") + "); tick Songs, Albums or Artists (with each "
          "album&rsquo;s songs, or each artist&rsquo;s albums), click Print, then OK."),
         ("To choose the paper", "Choose Page Setup&hellip;: A4 or US Letter. The first "
          "print asks for it."),
         ("To stop printing", k("&#8984;") + " " + k(".") + ". The music pauses while "
          "it prints.")),

    '<p class="note"><b>Limits:</b> 500 songs and 10 playlists in a library; 640K under '
    "MultiFinder (it works in 512K); MacMusic WAV files only, mono, 22,254 or 22,050 Hz, "
    "8-bit.</p>",
    '<p class="foot">See the <i>MacMusic User&rsquo;s Guide</i> for MacMusic, and <i>MacMusic '
    "Utilities</i> for the utilities. &copy; 2026 Jack McAteer. Apple, Macintosh and Mac are "
    "trademarks of Apple Inc.; YouTube is a trademark of Google LLC. For the owner&rsquo;s own "
    "use.</p>",
)

if __name__ == "__main__":
    body = STYLE + PAGE_1 + PAGE_2 + PAGE_3 + PAGE_4
    open(OUT, "w").write(page("MacMusic Quick Reference Card", body))
    print("wrote", os.path.normpath(OUT))
