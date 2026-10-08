#!/usr/bin/env python3
"""Writes docs/manual/chapter1.html, Chapter 1 of the MacMusic manual,
"Learning MacMusic" (Phase 6; outline.md), for review on its own: a
title page for the draft, the opener spread, the chapter's contents,
the introduction, ten lessons and "What Now?".

Build it with tools/manual/build-pdf.sh docs/manual/chapter1.html <pdf>."""
import os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from callouts import figure
from book import img, draft_title, opener, page

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "..", "docs", "manual", "chapter1.html")

# Lesson 5: the parts of the MacMusic window (the window crop, 206 x 320
# Mac pixels, at a point a pixel).
WINDOW = figure("figures/c1-parts.png", 206, 320, 1.0, [
    ("Close box", 30, 8, 8),
    ("Highlighted item", 56, 24, 56),
    ("MacMusic&rsquo;s screen", 104, 21, 104),
    ("MENU", 184, 94, 186),
    ("Click wheel", 212, 58, 212),
    ("Previous", 242, 42, 242),
    ("Title bar", 9, 150, 9, "right"),
    ("Battery|(just for show)", 38, 178, 38, "right"),
    ("Next", 242, 162, 242, "right"),
    ("Centre button", 266, 116, 252, "right"),
    ("Play/Pause", 298, 110, 298, "right"),
])

# Lesson 8: Now Playing (the screen crop, 174 x 142 pixels), as in the
# sample spread.
NOW_PLAYING = figure("figures/c1-now-playing.png", 174, 142, 200 / 174, [
    ("Play icon", 16, 13, 15),
    ("Place in the list", 38, 22, 34),
    ("Title, artist and album", 58, 52, 49),
    ("Progress bar", 127, 60, 110),
    ("Time played", 143, 22, 124),
    ("Shuffle and|Repeat icons", 38, 150, 33, "right"),
    ("Time left", 143, 154, 124, "right"),
])


def lesson(number, title, anchor):
    return ('<div class="margin"><div class="lesson-head" id="%s">'
            '<span class="number">%d</span><span class="title">%s</span></div></div>'
            % (anchor, number, title))


LESSONS = [
    (1, "Making a Backup Copy", "l1"),
    (2, "Installing MacMusic", "l2"),
    (3, "Starting MacMusic", "l3"),
    (4, "Creating Your MacMusic Library", "l4"),
    (5, "The MacMusic Window", "l5"),
    (6, "Using the Click Wheel", "l6"),
    (7, "Importing a Song", "l7"),
    (8, "Playing a Song", "l8"),
    (9, "Making a Playlist", "l9"),
    (10, "Quitting", "l10"),
]

CONTENTS = "\n".join(
    ['<li class="part"><a href="#intro">Introduction: What Is MacMusic?</a></li>',
     '<li class="part"><a href="#lessons">Lessons</a></li>']
    + ['<li class="lesson"><span class="n">%d</span><a href="#%s">%s</a></li>' % (n, a, t)
       for n, t, a in LESSONS]
    + ['<li class="part"><a href="#whatnow">What Now?</a></li>'])

BODY = """
DRAFT_TITLE
OPENER

<div class="chapter chapter-contents">
 <div class="flow">
  <div class="margin"><h2 class="contents-head">Contents</h2></div>
  <ul class="toc">
CONTENTS
  </ul>
 </div>
</div>

<div class="chapter page-break">
 <div class="flow">

  <div class="margin"><h2 class="margin-head" id="intro">What Is MacMusic?</h2></div>
  <p>MacMusic turns your Macintosh into a music player. It plays songs
  stored on your disk through the Macintosh&rsquo;s own speaker, and you
  work it much as you would a pocket music player: by turning a wheel
  and clicking buttons, all with the mouse.</p>
  <p>Before you start the lessons, it helps to know the three ideas
  MacMusic is built on. None of them is new; you probably have all
  three at home already.</p>

  <div class="margin"><h3 class="margin-sub">Your record shelf</h3></div>
  <figure class="beside">IMG_LIBRARY</figure>
  <p>MacMusic keeps every song in one place, a folder on your hard disk
  called the <b class="term">MacMusic Library</b>. Like a shelf of
  records, it&rsquo;s where your music lives. You put a song in it by
  <b class="term">importing</b> it, and MacMusic looks after it from
  then on: it files the song under a number of its own, and keeps its
  title, artist and album with it.</p>
  <p>A library can hold as many as 500 songs, if your disk has room
  for them.</p>

  <div class="margin"><h3 class="margin-sub">A mix tape</h3></div>
  <figure class="beside">IMG_PLAYLIST</figure>
  <p>A <b class="term">playlist</b> is like a mix tape you&rsquo;ve
  made for a long drive: a few favourite songs, in the order you want
  to hear them. Unlike a tape, though, a playlist doesn&rsquo;t copy
  anything. The songs stay where they are in the library, and the
  playlist simply lists them. A song can be on several playlists at
  once, and taking it off one doesn&rsquo;t remove it from your
  library.</p>
  <p>You can have as many as ten playlists.</p>

  <div class="margin"><h3 class="margin-sub">A pocket music player</h3></div>
  <figure class="beside">IMG_PLAYER</figure>
  <p>The <b class="term">MacMusic window</b> looks like a pocket music
  player, and it works like one. At the top is MacMusic&rsquo;s screen,
  where you see lists of your music and what&rsquo;s playing. Below it
  is the <b class="term">click wheel</b>: you turn it to move through a
  list, and click it to choose. Everything you do to <i>play</i> music
  happens in this window.</p>
  <p>Everything you do to <i>look after</i> your music&mdash;importing
  songs, making playlists, choosing a library&mdash;happens in the
  usual Macintosh way, with commands in the menu bar at the top of the
  screen.</p>

  <div class="margin"><h3 class="margin-sub">What you&rsquo;ll learn</h3></div>
  <p>The lessons that follow take about half an hour. You&rsquo;ll
  install MacMusic on your hard disk, create a library, import the
  song that comes on the MacMusic disk, play it, and put it on a
  playlist of your own.</p>
  <p>You should already know how to use your Macintosh: how to point,
  click and drag with the mouse, and how to choose commands from menus.
  If you don&rsquo;t, spend some time with the <i>Macintosh Owner&rsquo;s
  Guide</i> that came with your computer, and its guided tour disk
  first.</p>

  <div class="margin"><h2 class="margin-head" id="lessons">Lessons</h2></div>
  <p class="lessons-intro">Do the lessons in order; each one picks up
  where the last one left off. Each step is shown like this:</p>
  <p class="step">A step tells you what to do.</p>
  <p>The plain text after it tells you what happens.</p>

LESSON_1
  <p>Before you use MacMusic, make a copy of the MacMusic disk, and
  keep the original somewhere safe. Then, if anything happens to the
  disk you use, you can always make another copy.</p>
  <p class="step">Lock the MacMusic disk.</p>
  <p>Turn the disk over and slide the small tab in its corner so that
  you can see through the hole. A locked disk can be read, but nothing
  on it can be changed or thrown away.</p>
  <p class="step">Copy the disk onto a blank 800K disk.</p>
  <p>Insert the MacMusic disk, and then the blank disk (if the Finder
  asks, click Initialize to prepare the blank disk first). Drag the MacMusic disk&rsquo;s
  icon onto the blank disk&rsquo;s icon. The Finder copies everything on
  it, asking you to swap the two disks when it needs to; follow its
  messages.</p>
  <p class="grey">If you&rsquo;re not sure how to initialise or copy a
  disk, see the <i>Macintosh Owner&rsquo;s Guide</i>.</p>
  <p>Put the original MacMusic disk away, and use the copy for the
  rest of the lessons.</p>

LESSON_2
  <p>MacMusic runs from your hard disk: an Apple Hard Disk 20SC or
  other SCSI hard disk. Your library goes on the hard disk too, as
  songs take far more room than a floppy disk can hold.</p>
  <p>The MacMusic disk holds three things: the MacMusic application;
  <b class="term">Song.wav</b>, a short song for the lessons (half a
  minute of &ldquo;Chariots of Fire&rdquo; by Vangelis); and
  <i>MacMusic Read Me</i>, which you can open with TeachText.</p>
  <p class="step">Insert your copy of the MacMusic disk, and open it.</p>
  <p class="step">Drag the MacMusic icon and the Song.wav icon to your
  hard disk&rsquo;s window.</p>
  <figure class="beside">IMG_HARD_DISK</figure>
  <p>To drag both at once, click one, then hold down the Shift key
  and click the other; then drag either of them. The Finder copies them
  to the hard disk.</p>
  <p>You can put MacMusic in any folder you like. For the lessons,
  leave both icons in the hard disk&rsquo;s window, where
  you&rsquo;ll find them easily.</p>
  <p class="grey">Your hard disk&rsquo;s name and window will look
  different from the picture.</p>
  <p class="step">Eject the MacMusic disk.</p>
  <p>Choose Eject from the File menu. You won&rsquo;t need the disk
  again in these lessons.</p>

LESSON_3
  <p class="step">Double-click the MacMusic icon on your hard disk.</p>
  <p>The MacMusic window opens, and the Macintosh menu bar shows
  MacMusic&rsquo;s menus: File, Edit and Playlists. For a moment the
  Apple logo appears on MacMusic&rsquo;s screen, as on a music player
  starting up.</p>
  <figure class="wide">IMG_START</figure>
  <p>Under MultiFinder, MacMusic starts the same way. It needs 512K of
  memory, and works best with 640K.</p>

LESSON_4
  <p>The first time you start MacMusic, it doesn&rsquo;t know where your
  music is, so it asks:</p>
  <figure class="dialog">IMG_FIRST_TIME</figure>
  <p><b>Create New</b> makes a new, empty library. <b>Find Existing</b>
  opens a library you already have, on this disk or another.
  <b>Not Now</b> carries on with no library at all.</p>
  <p class="step">Click Create New.</p>
  <p>A dialog box asks where to put the library, and what to call
  it.</p>
  <figure class="dialog">IMG_CREATE</figure>
  <p class="step">Make sure your hard disk&rsquo;s name is shown above
  the list, and click Save.</p>
  <p>If another disk&rsquo;s name is shown, click Drive until your hard
  disk&rsquo;s name appears. The name MacMusic suggests,
  &ldquo;MacMusic Library,&rdquo; is a good one; you can type another
  if you like.</p>
  <p>MacMusic makes the library folder, and its screen shows the
  <b class="term">main menu</b>. Your library is ready, but empty.</p>
  <p class="margin margin-note grey">MacMusic remembers your library,
  and opens it every time it starts.</p>
  <p>If you look in your hard disk&rsquo;s window in the Finder later,
  you&rsquo;ll see the new folder. Inside it are a Playlists folder,
  a file called <b class="term">MacMusic Library Info</b>, which tells
  MacMusic that the folder is a library, and, once you&rsquo;ve
  imported some songs, the songs themselves:</p>
  <figure class="dialog">IMG_LIBRARY_FOLDER</figure>
  <div class="important">
   <p class="margin important-head">Important</p>
   <p>Leave the files in your MacMusic Library to MacMusic. Don&rsquo;t
   rename, move or add files inside the library with the Finder;
   use MacMusic&rsquo;s commands, and your library will always be in
   order.</p>
  </div>

LESSON_5
  <p>Take a moment to look at the MacMusic window.</p>
  <figure class="wide">WINDOW_FIGURE</figure>
  <p>You&rsquo;ll use the click wheel for almost everything you do in
  the window:</p>
  <ul class="boxes">
   <li>Turn the wheel to move the highlight up and down a list.</li>
   <li>Click the <b class="term">centre button</b> to choose the
   highlighted item.</li>
   <li>Click <b class="term">MENU</b>, at the top of the wheel, to go
   back to the screen before.</li>
   <li>Click the bottom of the wheel to play or pause, and the left
   and right sides to go to the previous or next song.</li>
  </ul>
  <p>The window&rsquo;s title bar and close box work like any other
  window&rsquo;s: drag the title bar to move the window, and click the
  close box to quit MacMusic.</p>

LESSON_6
  <p>The main menu lists the things you can do: Music, Extras,
  Settings, Shuffle Songs and Backlight. A <b>&gt;</b> at the right of
  an item means it leads to another list.</p>
  <p><b class="run-in">To turn the click wheel:</b> Position the pointer
  on the wheel, hold down the mouse button, and move the pointer round
  the wheel, as though you were turning it with your finger.</p>
  <p class="step">Turn the click wheel clockwise, slowly.</p>
  <p>The highlight moves down the list, one item for every short turn
  you make. Turn it anticlockwise, and the highlight moves up.</p>
  <figure class="row">IMG_WHEEL_1 IMG_WHEEL_2 IMG_WHEEL_3</figure>
  <p class="step">Turn the wheel until Music is highlighted, and click
  the centre button.</p>
  <figure class="beside">IMG_MUSIC</figure>
  <p>The Music screen appears. It sorts your music in several ways:
  by playlist, artist, album, song title, genre and composer, and it
  keeps audiobooks apart. The list is longer than the screen; the bar
  at the right shows that there&rsquo;s more below, and turning the
  wheel scrolls to it.</p>
  <p class="step">Click MENU, at the top of the wheel.</p>
  <p>You&rsquo;re back at the main menu. MENU always takes you back one
  screen, so you can never get lost.</p>
  <p class="grey">You can also use the keyboard: the Up Arrow and Down
  Arrow keys move the highlight, Return chooses, and Delete goes
  back.</p>
  <p>Your library is still empty, so there&rsquo;s nothing to play yet.
  The next lesson puts a song in it.</p>

LESSON_7
  <p>To put a song in your library, you <b class="term">import</b> it.
  MacMusic plays songs stored as WAV sound files; Song.wav, the song
  you copied to your hard disk in Lesson 2, is one.</p>
  <p class="step">Choose Import Song from the File menu.</p>
  <figure class="beside">IMG_IMPORT_MENU</figure>
  <p>The commands in the File menu, and in the Edit and Playlists
  menus beside it, look after your library. The Import Song dialog box
  appears, listing the files on your hard disk.</p>
  <figure class="dialog">IMG_IMPORT_OPEN</figure>
  <p class="step">Select Song.wav and click Open.</p>
  <p>MacMusic tells you what it&rsquo;s about to do:</p>
  <figure class="dialog">IMG_IMPORT_MOVE</figure>
  <p>Because Song.wav is on the same disk as your library, MacMusic
  moves it into the library, which is quick. (From a floppy disk or
  another hard disk, MacMusic copies the song and then removes the
  original from that disk, unless you choose Just Copy. Chapter 2
  explains.)</p>
  <p class="step">Click OK.</p>
  <p>A dialog box shows the song&rsquo;s details: its file name and
  length at the top, then its title, artist and so on. MacMusic has
  filled in the title from the file&rsquo;s name. You can do
  better.</p>
  <figure class="dialog">IMG_IMPORT_INFO</figure>
  <p class="step">Type the song&rsquo;s title:</p>
  <div class="type-this">Chariots of Fire</div>
  <p>The title is already selected, so what you type replaces it.</p>
  <p class="step">Press the Tab key, and type the artist&rsquo;s name:</p>
  <div class="type-this">Vangelis</div>
  <p class="step">Press Tab again, and type the album&rsquo;s name:</p>
  <div class="type-this">Chariots of Fire</div>
  <p class="grey">You can leave the other boxes empty. You can change
  any of a song&rsquo;s details later, with Edit Song in the Edit
  menu.</p>
  <p class="margin margin-note grey">Your own music has to be made into
  WAV files before MacMusic can play it. <i>MacMusic Utilities</i>, a
  separate manual, explains how.</p>
  <p class="step">Click Import.</p>
  <p>For a moment you see &ldquo;Importing&hellip;&rdquo;, and the song
  is in your library.</p>

LESSON_8
  <p class="step">Turn the wheel to highlight Music, and click the centre
  button. Then highlight Songs, and click the centre button again.</p>
  <figure class="beside">IMG_SONGS</figure>
  <p>The Songs screen lists every song in your library by title, in
  alphabetical order. For now there&rsquo;s just one, &ldquo;Chariots
  of Fire.&rdquo;</p>
  <p class="step">With &ldquo;Chariots of Fire&rdquo; highlighted, click
  the centre button.</p>
  <p>In a moment you&rsquo;ll hear the opening bars through the
  Macintosh&rsquo;s speaker, and the <b class="term">Now Playing</b>
  screen shows what&rsquo;s playing:</p>
  <figure class="wide">NOW_PLAYING_FIGURE</figure>
  <p class="step">Click the bottom of the click wheel.</p>
  <figure class="beside">IMG_PAUSED</figure>
  <p>The music pauses, and the play icon changes to two bars. Click
  the bottom of the wheel again, and it carries on from the same
  place.</p>
  <p class="margin margin-note grey">The Space bar pauses and plays,
  too.</p>
  <p class="step">Turn the click wheel a little, either way.</p>
  <p>On Now Playing, turning the wheel changes the volume:
  clockwise for louder, anticlockwise for softer. A volume bar takes
  the place of the progress bar for a couple of seconds.</p>
  <p>When the song ends, the music stops. In a bigger library, the
  next song in the list follows on by itself.</p>

LESSON_9
  <p>A playlist of one song is a modest start, but every collection
  begins somewhere.</p>
  <p class="step">Choose New Playlist from the Playlists menu.</p>
  <figure class="beside">IMG_NEW_PLAYLIST</figure>
  <p>The Playlists menu holds everything to do with playlists. Below
  the dotted line is an &ldquo;Add to&rdquo; command for each
  playlist you have, so the picture shows a few more than you&rsquo;ll
  see.</p>
  <p>A dialog box asks for the new playlist&rsquo;s name, suggesting
  &ldquo;Untitled Playlist.&rdquo;</p>
  <figure class="dialog">IMG_NAME_IT</figure>
  <p class="step">Type a name for the playlist, and click Create.</p>
  <div class="type-this">Film Themes</div>
  <p class="step">On MacMusic&rsquo;s screen, go to Music, then Songs,
  and highlight &ldquo;Chariots of Fire.&rdquo;</p>
  <p>Click MENU to go back, if Now Playing is showing.</p>
  <p class="step">Choose Add to &ldquo;Film Themes&rdquo; from the
  Playlists menu.</p>
  <figure class="beside">IMG_ADD_TO</figure>
  <p>The highlighted song is added at the end of the playlist.</p>
  <p class="step">Go back to the Music screen, choose Playlists, and
  then Film Themes.</p>
  <figure class="beside">IMG_PLAYLIST_SONGS</figure>
  <p>There&rsquo;s your song. Choose it to play it, and the rest of
  the playlist plays after it, in order.</p>
  <p class="grey">MacMusic can have as many as ten playlists. If you
  try to make an eleventh, it tells you so:</p>
  <figure class="dialog small">IMG_TEN_LIMIT</figure>

LESSON_10
  <p class="step">Choose Quit from the File menu.</p>
  <figure class="beside">IMG_QUIT</figure>
  <p>The music stops, the MacMusic window closes, and you&rsquo;re
  back in the Finder.</p>
  <p class="grey">You can also quit by pressing Command-Q, or by
  clicking the close box in the MacMusic window.</p>
  <p>MacMusic remembers your library, the volume, and your settings.
  Next time you start it, it opens your library straight away, with
  everything as you left it.</p>

  <div class="margin"><h2 class="margin-head" id="whatnow">What Now?</h2></div>
  <p>You&rsquo;ve learned the basics: you&rsquo;ve set up MacMusic,
  imported a song, played it and made a playlist. That&rsquo;s most of
  what you&rsquo;ll ever need.</p>
  <p>When you&rsquo;ve imported more of your music, you might like to
  try:</p>
  <ul class="boxes">
   <li><b>Shuffle Songs,</b> in the main menu, which plays every song
   in your library in a random order;</li>
   <li><b>Artists</b> and <b>Albums,</b> in the Music screen, which
   find your music the way you remember it;</li>
   <li><b>Settings,</b> where Shuffle and Repeat change how lists
   play;</li>
   <li><b>Extras,</b> where the Clock keeps you company.</li>
  </ul>
  <p>Chapter 2, &ldquo;Using MacMusic,&rdquo; shows how to do each of
  these, and everything else MacMusic does, one task at a time.
  Chapter 3, &ldquo;MacMusic Reference,&rdquo; describes every menu,
  command and screen.</p>
  <p>Or simply explore. Nothing you do on MacMusic&rsquo;s screen can
  harm your songs, and MENU always brings you back.</p>
 </div>
</div>
"""

PICTURES = {
    "IMG_LIBRARY": img("c1-library", 150),
    "IMG_PLAYLIST": img("c1-playlist", 100),
    "IMG_PLAYER": img("c1-parts", 82),
    "IMG_HARD_DISK": img("c1-hard-disk", 150),
    "IMG_START": img("c1-start", 448),
    "IMG_FIRST_TIME": img("c1-first-time", 240),
    "IMG_CREATE": img("c1-create", 240),
    "IMG_LIBRARY_FOLDER": img("c1-library", 240),
    "IMG_WHEEL_1": img("c1-wheel-1", 96),
    "IMG_WHEEL_2": img("c1-wheel-2", 96),
    "IMG_WHEEL_3": img("c1-wheel-3", 96),
    "IMG_MUSIC": img("c1-music", 100),
    "IMG_IMPORT_MENU": img("c1-import-menu", 112),
    "IMG_IMPORT_OPEN": img("c1-import-open", 240),
    "IMG_IMPORT_MOVE": img("c1-import-move", 240),
    "IMG_IMPORT_INFO": img("c1-import-info", 240),
    "IMG_SONGS": img("c1-songs", 82),
    "IMG_PAUSED": img("c1-paused", 100),
    "IMG_NEW_PLAYLIST": img("c1-new-playlist", 112),
    "IMG_NAME_IT": img("c1-name-it", 240),
    "IMG_ADD_TO": img("c1-add-to", 112),
    "IMG_PLAYLIST_SONGS": img("c1-playlist", 100),
    "IMG_TEN_LIMIT": img("c1-ten-limit", 240),
    "IMG_QUIT": img("c1-quit", 112),
}

DRAFT = draft_title(
    "Chapter 1: Learning MacMusic",
    "A draft for review, 8 October 2026. In the finished book this chapter "
    "follows About This Manual, and its pages are numbered on from there.")
body = BODY.replace("DRAFT_TITLE\n", "")
body = body.replace("OPENER", opener(1, "Learning MacMusic", "c1-opener"))
body = body.replace("CONTENTS", CONTENTS)
body = body.replace("WINDOW_FIGURE", WINDOW).replace("NOW_PLAYING_FIGURE", NOW_PLAYING)
for key in sorted(PICTURES, key=len, reverse=True):     # IMG_PLAYLIST_SONGS first
    body = body.replace(key, PICTURES[key])
for n, title, anchor in reversed(LESSONS):              # LESSON_10 before LESSON_1
    body = body.replace("LESSON_%d\n" % n, lesson(n, title, anchor) + "\n")
# Each margin head stays on the page with what follows it: its figure
# beside, if any, its first paragraph, and a large figure straight after.
body = re.sub(r'(<div class="margin">(?:(?!</div>).)*</div>(?:</div>)?)(\s*<figure class="beside">.*?</figure>)?'
              r'(\s*<p.*?</p>)(\s*<figure class="wide">.*?</figure>)?',
              r'<div class="keep">\1\2\3\4</div>', body, flags=re.S)
PART = body

# The chapter for the book (make-book.py), and on its own for review.
if __name__ == "__main__":
    open(OUT, "w").write(page("MacMusic User&rsquo;s Guide: Chapter 1", DRAFT + PART))
    print("wrote", os.path.normpath(OUT))
