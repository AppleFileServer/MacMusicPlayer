#!/usr/bin/env python3
"""Writes docs/utilities-manual/chapter4.html, Chapter 4 of *MacMusic
Utilities*, "Backing Up Your Macintosh": the Macintosh Backup Utility
1.0 (Phase 7; docs/utilities-manual/outline.md). Its introduction, two
lessons, the task pages and the reference, as Chapter 2's.

Facts come from the Backup Utility's 1.0 source (its window, sheets and
messages) and its SPECIFICATION.md; the figures from the owner's
screenshots (bu01-bu14), taken with an earlier build: where 1.0 differs
(the card's registered name in the backup sheet, the Eject button, the
manifest's order), the text describes 1.0, and the figures are to be
replaced from new screenshots (CHECK). What no screenshot shows yet is
marked CHECK.

Build it with tools/utilities-manual/build-pdf.sh docs/utilities-manual/chapter4.html <pdf>."""
import os, re, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, "..", "manual"))
from ub import (img, draft_title, opener, page, anchor, TEXT, TASK, fig, steps, Parts, head, sub, cmd,
                table, Topic, T)
from callouts import figure

OUT = os.path.join(HERE, "..", "..", "docs", "utilities-manual", "chapter4.html")
PARTS = Parts()
lesson, section = PARTS.lesson, PARTS.section
LESSONS, SECTIONS = PARTS.lessons, PARTS.sections
CARD = "Mac Plus BlueSCSI"


def wide(name):
    """A strip of the window across the whole page (the margin column too),
    so its text can be read."""
    return '<figure class="window wide">%s</figure>' % img(name, 448)

# The window after a backup (bu08), labelled: 2000 x 1400 pixels across
# the text column.
SCALE = TEXT / 2000
WINDOW = figure("figures/bu08.png", 2000, 1400, SCALE, [
    ("Status", 22, 40, 150),
    ("Toolbar", 6, 1530, 50),
    ("The card", 46, 60, 320),
    ("What&rsquo;s on it", 124, 60, 820),
    ("History", 72, 940, 350),
    ("What the|snapshot holds", 150, 940, 920),
])

INTRO = "".join([
    head("Introduction", "intro"),
    "<p>Everything on your Macintosh Plus lives on the BlueSCSI&rsquo;s SD card: "
    "its disks, with the System, your programs, and MacMusic and its songs. "
    "Cards wear out, and a power cut while the Plus is using the card can "
    "damage a disk on it. The <b class=\"term\">Macintosh Backup Utility</b> "
    "keeps copies of the card on your Mac, so that whatever happens to the "
    "card, its disks can be put back as they were.</p>",
    "<p>It works rather as Time Machine does for your Mac: each backup is "
    "kept, dated, and older ones are thinned out as time goes on.</p>",

    sub("Snapshots"),
    "<p>Each backup is a <b class=\"term\">snapshot</b>: a complete copy of "
    "everything on the card at that moment, in a folder of its own on your "
    "Mac, with a record (its <b class=\"term\">manifest</b>) of every file "
    "and a check-sum of each. A snapshot stands on its own: you can restore "
    "from it even if every other snapshot were lost.</p>",

    sub("Safe with your card"),
    "<p>The Backup Utility never writes to the card it backs up. It reads "
    "every file, copies it, and checks the copy against what it read; only a "
    "backup in which every file checks becomes a snapshot. It never deletes "
    "a backup unless you ask, or under the thinning rule you can turn off.</p>",

    sub("Where the backups go"),
    "<p>In a folder called Mac Plus Backups, in your home folder, unless you "
    "choose another in Settings. A card of 2 GB of disks takes about 2 GB "
    "for each snapshot, so keep an eye on the room on your Mac.</p>",
])

LESSON_1 = "".join([
    lesson(1, "Backing Up Your Card"),
    "<p>In this lesson you back up the card for the first time, and register "
    "it, so the Backup Utility knows it from then on.</p>",
    steps("> Shut the Macintosh Plus down: choose Shut Down from the Special "
          "menu, and switch it off when it says you may.",
          "The card must never come out while the Plus is using it. Shut "
          "Down puts every disk away properly first."),
    wide("bu01-status"),
    steps("> Take the card out of the BlueSCSI and put it in your Mac&rsquo;s "
          "card slot (or a card reader).",
          "> Open the Macintosh Backup Utility.",
          "It recognises a BlueSCSI card by its disks and its settings file, "
          "and asks to register it the first time. (CHECK: the registration "
          "sheet isn&rsquo;t yet pictured.)",
          "> Type a name for the card, or keep &ldquo;Mac Plus BlueSCSI&rdquo;, and "
          "click Register.",
          "The window lists the card&rsquo;s disks and other files."),
    fig("bu05-contents", TASK),
    steps("> Click Back Up Now, the first button in the toolbar.",
          "A sheet says what will be copied and how much room it needs, and "
          "what&rsquo;s free."),
    fig("bu13-sheet", TEXT),
    steps("> Click Back Up.",
          "The Backup Utility copies each file, checking each as it goes. A "
          "card of 2 GB takes a few minutes; the window says where it&rsquo;s "
          "got to."),
    wide("bu14-status"),
    steps("When it&rsquo;s done, it says &ldquo;Backed up and verified,&rdquo; and the "
          "snapshot appears in the history on the right.",
          "> Click Eject, put the card back in the BlueSCSI, and switch the Plus "
          "on.",
          "~ Put the card back before you switch the Plus on, never after."),
])

LESSON_2 = "".join([
    lesson(2, "Restoring a Disk Image"),
    "<p>Say one of the Plus&rsquo;s disks has gone wrong. Here you get it back "
    "from a snapshot, as it was then.</p>",
    steps("> In the history, select the snapshot to restore from.",
          "Its contents are listed below it, its disks first."),
    fig("bu08-manifest", TASK),
    steps("> Click Restore… in the toolbar.",
          "> Click &ldquo;Chosen files and folders,&rdquo; and tick the disk image.",
          "> Click Choose, choose a folder on your Mac, and click Restore.",
          "The disk image is copied into the folder, and checked against the "
          "snapshot. (CHECK: the Restore sheet isn&rsquo;t yet pictured.)",
          "> Shut the Plus down, put its card in your Mac, and drag the restored "
          "disk image onto the card, replacing the damaged one.",
          "~ Only you ever change the card: the Backup Utility doesn&rsquo;t write "
          "to a card that holds anything. If a card itself has failed, see "
          "&ldquo;To Restore Onto a New Card&rdquo; in the tasks that follow.",
          "> Eject the card, put it back in the BlueSCSI, and switch the Plus on."),
])

TOPICS = [
    Topic("Backing Up",
          "<p>A backup copies everything on the card into a new snapshot, and "
          "checks every file. It never starts by itself.</p>",
          [T("To Back Up the Card", steps(
               "> Shut the Plus down with Shut Down, and switch it off.",
               "> Put the card in your Mac.",
               "The window says the card is connected.",
               "> Click Back Up Now, then Back Up.",
               "~ If the Backup Utility is in the background when the card goes in, "
               "a notification says the card is connected."), "bu13-sheet"),
           T("If a Disk Wasn&rsquo;t Shut Down Cleanly", steps(
               "A disk records whether it was put away properly. If one on the card "
               "wasn&rsquo;t (the power failed while the Plus was using it, say), the "
               "Backup Utility warns you before the backup, and marks the snapshot "
               "in the history.",
               "The backup goes ahead: it keeps the disk as it is. An earlier "
               "snapshot, from before the power cut, may be the better one to "
               "restore from. (CHECK: the warning isn&rsquo;t yet pictured.)")),
           T("To Cancel a Backup", steps(
               "> Click Cancel, or press Esc.",
               "It stops. What it had copied isn&rsquo;t kept as a snapshot; your "
               "earlier snapshots aren&rsquo;t touched."), "bu14-status"),
           T("If the Backup Isn&rsquo;t Verified", steps(
               "The window says why, and the job isn&rsquo;t kept as a snapshot:",
               "* <b>The card changed during the backup:</b> a file was different "
               "afterwards. Make sure nothing else is using the card, and back up "
               "again.",
               "* <b>The backup failed:</b> a file couldn&rsquo;t be read or written; "
               "the message names it.",
               "* <b>The card was removed during the backup.</b>",
               "Each leaves an incomplete job, listed under the history; see &ldquo;To "
               "Clear Incomplete Jobs.&rdquo;")),
           ]),

    Topic("Your Card",
          "<p>The Backup Utility remembers each card you register, by name, and "
          "keeps its snapshots together.</p>",
          [T("To Register a Card", steps(
               "The first time a card is inserted, the Backup Utility asks to "
               "register it.",
               "> Type a name, such as Mac Plus BlueSCSI, and click Register.",
               "OR",
               "Click Not Now, and register it later with Register… in the window.",
               "~ Nothing is written to the card: the names are kept in the backup "
               "folder.")),
           T("If It Asks Whether It&rsquo;s the Same Card", steps(
               "A card that&rsquo;s been reformatted looks new to your Mac, though it "
               "has the same name, size and format. The Backup Utility asks.",
               "> Click &ldquo;Yes, the Same Card&rdquo; to keep its backups together, or "
               "&ldquo;No, a Different Card&rdquo; to register it as a new one.")),
           T("To Eject the Card", steps(
               "> Click Eject, at the top of the window, after the backup.",
               "OR",
               "Eject it in the Finder.",
               "~ Always eject the card before you take it out of your Mac.")),
           ]),

    Topic("The History",
          "<p>The history lists the snapshots, newest first, and shows what the "
          "selected one holds.</p>",
          [T("To See What a Snapshot Holds", steps(
               "> Select the snapshot.",
               "Its files are listed below: the disk images first, then the "
               "settings and log, with macOS&rsquo;s own hidden files last.",
               "> To see its folder, click Browse Backups in the toolbar."), "bu08-history"),
           T("To Check a Snapshot", steps(
               "> Select it and click Verify Backup in the toolbar.",
               "Every file is read again and checked against the snapshot&rsquo;s "
               "record. It says &ldquo;Verified: 13 files match,&rdquo; or names each file "
               "that doesn&rsquo;t, and how. (CHECK: not yet pictured.)")),
           T("To Keep a Snapshot", steps(
               "> Control-click the snapshot, and choose Keep (Never Thin).",
               "It&rsquo;s marked with a pin, and thinning never removes it. Choose "
               "Don&rsquo;t Keep to take the mark off.")),
           T("To Delete a Snapshot", steps(
               "> Control-click it, choose Delete…, and click Move to Trash.",
               "It goes to the Trash, so it can be put back until the Trash is "
               "emptied. If it&rsquo;s the card&rsquo;s only backup, you&rsquo;re told so "
               "first.")),
           T("To Clear Incomplete Jobs", steps(
               "Backups that failed, were cancelled, or were interrupted (if your "
               "Mac stopped during one) are listed under the history as incomplete "
               "jobs. They hold no snapshot.",
               "> Click Clear…, then Move to Trash.")),
           ]),

    Topic("Restoring",
          "<p>Restoring copies files out of a snapshot, and checks each against "
          "the snapshot&rsquo;s record. It never writes to a card that holds anything.</p>",
          [T("To Restore Files to a Folder", steps(
               "> Select a snapshot, and click Restore….",
               "> Click &ldquo;Chosen files and folders,&rdquo; and tick them.",
               "> Click Choose, choose a folder, and click Restore.",
               "If a file of the same name is there, you&rsquo;re asked: Keep Both, "
               "Replace or Skip, with &ldquo;Do the same for the rest.&rdquo;")),
           T("To Restore the Whole Card to a Folder", steps(
               "> Click Restore…, keep &ldquo;The whole snapshot, into a new folder,&rdquo; "
               "choose where, and click Restore.",
               "A new folder, &ldquo;Restored&rdquo; and the snapshot&rsquo;s name, holds "
               "everything the card did.")),
           T("To Restore Onto a New Card", steps(
               "If the card itself has failed, a snapshot can be put onto a new one.",
               "> Format the new card in Disk Utility: Erase, Format ExFAT, Scheme "
               "Master Boot Record. (The Backup Utility never formats a card.)",
               "> Insert it, Control-click the snapshot, and choose Restore to a New "
               "Card….",
               "> Choose the new card (&ldquo;Empty, and ready&rdquo;), click Restore to "
               "Card…, and confirm.",
               "Every file is written and checked. Then click &ldquo;Use as "
               + CARD + "&rsquo;s Replacement,&rdquo; so the new card carries on the old "
               "one&rsquo;s name and backups, and Eject.",
               "~ CHECK: not yet pictured, nor tried on a real card.")),
           ]),

    Topic("Older Snapshots and Settings",
          "<p>After each backup, older snapshots of the card are thinned out, so "
          "the backups don&rsquo;t fill your Mac.</p>",
          [T("How Older Snapshots Are Thinned", steps(
               "For each card, the Backup Utility keeps:",
               "* the newest snapshot of each day, for a week;",
               "* the newest of each week, for a month;",
               "* the newest of each month, for a year.",
               "It always keeps at least two, and never one marked Keep. Where it "
               "can, it keeps one whose disks were all shut down cleanly. Those "
               "removed go to the Trash, and the window says which.")),
           T("To Change the Settings", steps(
               "> Choose Settings from the Macintosh Backup Utility menu (or press "
               "Command-comma).",
               "> To keep backups elsewhere, click Choose… beside Backup folder; Use "
               "Default goes back to Mac Plus Backups.",
               "> To keep every snapshot, turn off &ldquo;Thin older snapshots after each "
               "backup.&rdquo; (CHECK: not yet pictured.)")),
           ]),
]

REFERENCE = "".join([
    section("Backup Utility Reference", "reference"),
    "<p>This section describes the window, what happens when you back up, "
    "and where the backups are kept.</p>",

    section("The Backup Utility Window"),
    '<figure class="wide">%s</figure>' % WINDOW,
    cmd("Status", "What&rsquo;s happening: waiting for a card (with the reminder to "
        "shut the Plus down first), the card connected, backing up (with the file "
        "it&rsquo;s on, and Cancel), or the result. The last backup&rsquo;s date is on "
        "the right, with Eject when the card can be ejected."),
    cmd("Toolbar", "Back Up Now; Browse Backups (the snapshot&rsquo;s folder in the "
        "Finder); Verify Backup; Restore…; Open Backup Folder; Settings."),
    cmd("The card", "Its registered name, its own name, where it is, its format "
        "and how full it is; Choose Card (to choose a card or folder yourself) "
        "and Forget."),
    cmd("What&rsquo;s on it", "Every file the card holds, its disk images first, "
        "and any warnings: a disk not shut down cleanly, a file that can&rsquo;t be "
        "read."),
    cmd("History", "The snapshots: date, card, files, size, the room each takes "
        "on your Mac, and whether it&rsquo;s verified (with a warning sign if a disk "
        "in it wasn&rsquo;t shut down cleanly); a pin for one marked Keep. Below: "
        "how many, how much room, and any incomplete jobs. Control-click a "
        "snapshot for Restore…, Restore to a New Card…, Verify Backup, Show in "
        "Finder, Keep and Delete…."),
    cmd("What the snapshot holds", "The selected snapshot&rsquo;s files, each with "
        "its kind, size and whether it was verified, and the result of Verify "
        "Backup."),
    fig("bu08-toolbar", 200, "The toolbar."),

    section("What Happens When You Back Up"),
    table(("Stage", "What it does"),
          ("Looking at the card", "Lists everything on it, reading only; reads each "
           "disk&rsquo;s own record of whether it was shut down cleanly."),
          ("Copying", "Copies each file, a piece at a time, working out its "
           "check-sum (SHA-256) as it reads; then reads the copy back to check it."),
          ("Checking the card hasn&rsquo;t changed", "Looks at every file on the card "
           "again: any that changed or went makes the backup unverified."),
          ("Finishing", "Writes the snapshot&rsquo;s record, reads it back, and only "
           "then gives the snapshot its name; then thins older ones."),
          cls="ref stages"),

    section("Where the Backups Are"),
    "<p>In the backup folder (Mac Plus Backups, unless you chose another):</p>",
    table(("Folder", "Holds"),
          ("Snapshots", "a folder for each snapshot, named by its date and time "
           "(2026-10-09_18-11-03_3d70), holding Files, the card&rsquo;s copy, and "
           "SnapshotManifest.json, its record"),
          ("Catalog", "the index of snapshots (rebuilt from the snapshots if it&rsquo;s "
           "ever lost), the registered cards, and the snapshots marked Keep"),
          ("Logs", "for the Backup Utility&rsquo;s own use"),
          cls="ref spec"),
    "<p>A snapshot&rsquo;s Files folder is an ordinary folder: you can copy a disk "
    "image straight out of it in the Finder. Don&rsquo;t change anything inside a "
    "snapshot: Verify Backup would find it no longer matches.</p>",

    section("Limits"),
    table(("", ""),
          ("What&rsquo;s backed up", "The card&rsquo;s files and folders, not the card "
           "itself (its partition table, free space or deleted files)"),
          ("Each snapshot", "A full copy; only never-used space inside a disk image "
           "is left out"),
          ("Cards", "exFAT or FAT32, as BlueSCSI uses"),
          ("A new card", "Empty, exFAT (or FAT32, if no file is 4 GB or more), big "
           "enough"),
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


PART = (opener(4, "Backing Up Your Macintosh", "c4-opener")
        + '<div class="chapter chapter-contents"><div class="flow">'
          '<div class="margin"><h2 class="contents-head">Contents</h2></div>'
          '<ul class="toc">%s</ul></div></div>\n' % contents()
        + chapter(INTRO)
        + chapter('<span id="lessons"></span>', LESSON_1) + chapter(LESSON_2)
        + chapter('<span id="tasks"></span>', "".join(t.html() for t in TOPICS), cls="chapter tasks-wide page-break")
        + chapter(REFERENCE))
PART = re.sub(r'(<div class="margin">(?:(?!</div>).)*</div>(?:</div>)?)(\s*<p.*?</p>)',
              r'<div class="keep">\1\2</div>', PART, flags=re.S)

DRAFT = draft_title("Chapter 4: Backing Up Your Macintosh",
                    "A draft for review, 10 October 2026. The figures are from an "
                    "earlier build of the Backup Utility, to be replaced with "
                    "screenshots of 1.0; CHECK marks what no screenshot shows yet.")

if __name__ == "__main__":
    open(OUT, "w").write(page("MacMusic Utilities: Chapter 4", DRAFT + PART))
    print("wrote", os.path.normpath(OUT))
