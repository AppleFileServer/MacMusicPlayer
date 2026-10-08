#!/usr/bin/env python3
"""Writes docs/manual/chapter2.html, Chapter 2 of the MacMusic User's
Guide, "Using MacMusic" (Phase 6; outline.md): task pages on the
four-column grid (style guide §9). Each topic opens with its own column
(the topic, an introduction and a list of its tasks); each task takes a
column after it, with its pictures at the column's foot. A topic with
more tasks than fit carries on over the next page.

Build it with tools/manual/build-pdf.sh docs/manual/chapter2.html <pdf>."""
import os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from book import img, draft_title, opener, page, anchor

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "..", "docs", "manual", "chapter2.html")
COLUMN = 101            # a task column's width, in points
# Dialog and alert boxes are too wide to read at a column's width; they
# are pictured in Appendix A, at full size, instead.
DOUBLE = 216            # two columns together


class Task:
    def __init__(self, title, body, foot=(), double=False):
        self.title, self.body, self.foot, self.double = title, body, foot, double

    def html(self):
        foot = "".join(img(name, DOUBLE if self.double and width is None else width or COLUMN)
                       for name, width in self.foot)
        return ('<div class="column%s"><h3 id="%s">%s</h3>%s%s</div>'
                % (" double" if self.double else "", anchor(self.title), self.title,
                   self.body, '<div class="foot">%s</div>' % foot if foot else ""))


def T(title, body, *foot, double=False):
    """A task: its title, its text, and the figures at its foot (names,
    or (name, width) pairs)."""
    return Task(title, body, [f if isinstance(f, tuple) else (f, None) for f in foot], double)


class Topic:
    def __init__(self, title, intro, note, tasks, per_page=3):
        self.title, self.intro, self.note, self.tasks = title, intro, note, tasks
        self.per_page = per_page

    def html(self):
        items = "".join("<li>%s</li>" % t.title[3:] if t.title.startswith("To ")
                        else "<li>%s</li>" % t.title for t in self.tasks)
        first = ('<div class="column"><h2 class="topic" id="%s">%s</h2>%s'
                 '<ul class="boxes">%s</ul>%s</div>'
                 % (anchor(self.title), self.title, self.intro, items,
                    '<p class="grey">%s</p>' % self.note if self.note else ""))
        pages, cols, used = [], [first], 1
        for t in self.tasks:
            width = 2 if t.double else 1
            if used + width > 4:
                pages.append(cols)
                cols, used = [], 0
            cols.append(t.html())
            used += width
        pages.append(cols)
        return "\n".join('<div class="tasks page-break"><div class="task-page">%s</div></div>'
                         % "".join(c) for c in pages)


def steps(*parts):
    """Paragraphs: a part starting with "> " is a step (grey square), one
    starting with "~ " a grey note, "OR" the word OR; the rest plain."""
    out = []
    for p in parts:
        if p == "OR":
            out.append('<p class="or">OR</p>')
        elif p.startswith("> "):
            out.append('<p class="tstep">%s</p>' % p[2:])
        elif p.startswith("~ "):
            out.append('<p class="grey">%s</p>' % p[2:])
        else:
            out.append("<p>%s</p>" % p)
    return "".join(out)


TOPICS = [
    Topic("Finding Music",
          "<p>Everything in your library can be found from the Music screen. "
          "Choose Music from the main menu, and you can look by playlist, "
          "artist, album, song title, genre or composer, with audiobooks "
          "kept apart.</p>",
          "Your music is sorted by the details you give each song. The more "
          "you fill in, the more ways you can find it.",
          [T("To Scroll a List", steps(
               "> Turn the click wheel: clockwise to move down the list, "
               "anticlockwise to move up.",
               "When a list is longer than the screen, a scroll bar appears at "
               "its right, and the list scrolls as the highlight reaches the "
               "top or the bottom.",
               "OR",
               "Press the Down Arrow or Up Arrow key."), "c2-scroll"),
           T("To Go Back", steps(
               "> Click MENU, at the top of the click wheel.",
               "You go back one screen, to the list you came from, with the "
               "same item highlighted.",
               "OR",
               "Press the Delete key.",
               "~ On the main menu, MENU does nothing: there&rsquo;s nowhere "
               "further back to go."), "c1-wheel-1"),
           T("To Find a Song by Its Title", steps(
               "> Choose Songs from the Music screen.",
               "Every song in your library is listed in alphabetical order of "
               "title. A leading &ldquo;The&rdquo; is ignored, so &ldquo;The "
               "Long Way Round&rdquo; is listed under L.",
               "> Turn the click wheel to the song, and click the centre button "
               "to play it.",
               "~ Audiobooks have a list of their own, and aren&rsquo;t listed "
               "here."), "c2-songs"),
           T("To Find an Artist&rsquo;s Songs", steps(
               "> Choose Artists from the Music screen.",
               "> Choose an artist.",
               "You see the artist&rsquo;s albums.",
               "> Choose an album to see its songs, in track order, or choose "
               "All to see every song by that artist."), "c2-artists"),
           T("To Find an Album", steps(
               "> Choose Albums from the Music screen.",
               "Every album in your library is listed in alphabetical order.",
               "> Choose an album.",
               "Its songs are listed in track order, as on the record."), "c2-albums"),
           T("To Use &ldquo;All&rdquo;", steps(
               "The Artists, Albums and Genres lists each begin with All.",
               "> Choose All at the top of the list.",
               "All gathers up everything the list leads to: All in Genres "
               "lists every artist; All in Artists, every album; and All in "
               "an artist&rsquo;s albums, every song by that artist.",
               "~ The Composers list has no All."), "c2-genres"),
           T("To Find Songs by Genre or Composer", steps(
               "> Choose Genres from the Music screen, and then a genre.",
               "You see the artists with songs in that genre; choose one for "
               "their albums, and then the songs.",
               "> Or choose Composers, and then a composer.",
               "That composer&rsquo;s songs are listed straight away."), "c2-composers"),
           ]),

    Topic("Playlists and Audiobooks",
          "<p>The playlists you&rsquo;ve made, and your audiobooks, each have a "
          "list of their own in the Music screen.</p>",
          "To make and change playlists, see &ldquo;Working With "
          "Playlists&rdquo; later in this chapter.",
          [T("To Play a Playlist", steps(
               "> Choose Playlists from the Music screen, and then a playlist.",
               "Its songs are listed in the order you added them.",
               "> Choose a song.",
               "It plays, followed by the rest of the playlist, in order.",
               "OR",
               "Highlight the playlist and click the bottom of the click wheel "
               "to play it from the start."), "c2-playlists", "c2-playlist-songs"),
           T("To Find an Audiobook", steps(
               "> Choose Audiobooks from the Music screen.",
               "Every song marked as an audiobook is listed here, and only "
               "here: audiobooks aren&rsquo;t listed under Songs, Artists or "
               "the other lists, and they&rsquo;re never shuffled.",
               "~ A title too long for the screen ends in &ldquo;&hellip;&rdquo;."),
             "c2-audiobooks"),
           T("To Mark a Song as an Audiobook", steps(
               "> Highlight the song in a list, and choose Edit Song from the "
               "Edit menu.",
               "> Click the Audiobook box, so that a cross appears in it, and "
               "click Save.",
               "The song moves from the other lists to Audiobooks.",
               "~ You can mark a song as an audiobook when you import it, "
               "too.")),
           ]),

    Topic("Playing Songs",
          "<p>You can play a single song, a whole list, or everything you "
          "have.</p>",
          "The music plays on whatever you do in the MacMusic window or the "
          "menu bar.",
          [T("To Play a Song", steps(
               "> Highlight a song in any list, and click the centre button.",
               "The song plays, and the Now Playing screen appears. When it "
               "ends, the next song in the list follows, and so on to the end "
               "of the list.",
               "OR",
               "Press Return or Enter."), "c2-songs"),
           T("To Play a Whole List", steps(
               "You can play everything an item leads to (an artist, an "
               "album, a genre or a playlist) without opening it.",
               "> Highlight the item and click the bottom of the click "
               "wheel.",
               "Every song it leads to plays, from the first.",
               "OR",
               "Press the Space bar.",
               "~ If Shuffle is on in Settings, the songs play in a random "
               "order."), "c2-albums"),
           T("To Pause and Resume", steps(
               "> On Now Playing, click the bottom of the click wheel.",
               "The music pauses, and the play icon changes to two bars.",
               "> Click it again to carry on.",
               "OR",
               "Press the Space bar.",
               "~ On a list, this plays the highlighted item instead; use it on "
               "Now Playing to pause."), "c2-paused"),
           T("To Skip to the Next Song", steps(
               "> Click the right side of the click wheel.",
               "The next song in the list starts. After the last song, Next "
               "goes back to the first.",
               "OR",
               "Press the Right Arrow key.",
               "~ Next works from any screen while music is playing.")),
           T("To Go Back a Song or Start It Again", steps(
               "> Click the left side of the click wheel.",
               "If more than three seconds of the song have played, it starts "
               "again from the beginning. Otherwise, the song before it "
               "plays.",
               "OR",
               "Press the Left Arrow key.")),
           T("To Return to Now Playing", steps(
               "While a song is playing or paused, Now Playing is the last "
               "item in the main menu.",
               "> Click MENU until you reach the main menu, then choose Now "
               "Playing.",
               "When the music stops, the item goes away again.")),
           T("What Now Playing Shows",
             '<ul class="boxes"><li>where the song is in the list (&ldquo;2 of '
             '3&rdquo;)</li><li>its title, artist and album</li><li>a bar '
             'showing how much has played</li><li>the time played, and the '
             'time left</li><li>at the top right, the Shuffle and Repeat '
             'icons, when they&rsquo;re on</li></ul>', "c2-now-playing"),
           ]),

    Topic("Shuffling",
          "<p>Shuffle plays songs in a random order, like a radio station that "
          "only plays your records.</p>",
          "Audiobooks are never shuffled.",
          [T("To Shuffle Every Song", steps(
               "> Choose Shuffle Songs from the main menu.",
               "Every song in your library plays, in a random order, and Now "
               "Playing appears."), "c2-shuffle-songs"),
           T("To Shuffle a List", steps(
               "> Choose Settings from the main menu.",
               "> Highlight Shuffle, and click the centre button until it says "
               "Songs.",
               "Each click shows the next setting: Off, Songs, Albums.",
               "> Play a list.",
               "The song you chose plays first; the rest of the list follows "
               "in a random order.",
               "~ The setting stays until you change it."), "c2-settings"),
           T("To Shuffle by Albums", steps(
               "> In Settings, set Shuffle to Albums.",
               "> Play a list.",
               "Its albums play in a random order, each one&rsquo;s songs in "
               "track order, as their records would.")),
           ]),

    Topic("Moving Through a Song",
          "<p>While a song plays, the Now Playing screen lets you skip ahead, "
          "go back, or jump to any place in it, and set how loud it is.</p>",
          "The music keeps playing while you do any of these.",
          [T("To Fast-Forward or Rewind", steps(
               "> Position the pointer on the right side of the click wheel and "
               "hold down the mouse button. To rewind, use the left side.",
               "After half a second the play icon changes to two arrows, and "
               "the song moves on at four times its normal speed; after two "
               "seconds, sixteen times.",
               "> Release the mouse button.",
               "The song carries on from the new place.",
               "OR",
               "Hold down Option and the Right Arrow or Left Arrow key."), "c2-ff"),
           T("To Use the Scrubber", steps(
               "> On Now Playing, click the centre button.",
               "A diamond, the <b>scrubber</b>, appears on the progress bar.",
               "> Turn the click wheel.",
               "One full turn moves through the whole song. When you stop "
               "turning, the song jumps to the new place.",
               "~ The scrubber goes away by itself after five seconds."), "c2-scrubber"),
           T("To Change the Volume", steps(
               "> On Now Playing, turn the click wheel: clockwise for louder, "
               "anticlockwise for softer.",
               "The volume bar takes the place of the progress bar. One full "
               "turn goes from the softest setting to the loudest.",
               "OR",
               "Press the Down Arrow key for louder, the Up Arrow key for "
               "softer, one step at a time.",
               "~ MacMusic remembers the volume for next time."), "c2-volume"),
           ]),

    Topic("Headphones, Speakers and Tape",
          "<p>The sound output port on the back of your Macintosh lets you "
          "listen through headphones, play your music through a speaker or "
          "a stereo, or record it onto tape.</p>",
          "The Macintosh has one sound channel, so its sound, and MacMusic&rsquo;s, "
          "is mono. While something is plugged into the port, the built-in "
          "speaker is silent. For more about the port, see the "
          "<i>Macintosh Owner&rsquo;s Guide</i>.",
          [T("To Listen Through Headphones", steps(
               "> On Now Playing, turn the click wheel anticlockwise to the "
               "softest volume.",
               "> Plug the headphones into the sound output port, and put them "
               "on.",
               "> Play a song, and turn the click wheel clockwise, a step at a "
               "time, to a comfortable level.",
               "~ The port takes a mono 3.5 mm plug. Stereo headphones need an "
               "adaptor to hear the music in both ears.")),
           T("To Play Through a Speaker or Stereo", steps(
               "Use an amplified speaker, or a stereo&rsquo;s auxiliary "
               "or line input; not a microphone or record-player input.",
               "> Turn the speaker&rsquo;s volume all the way down, and turn "
               "MacMusic&rsquo;s down too.",
               "> Connect the cable: a mono 3.5 mm plug at the Macintosh, and "
               "an adaptor if the inputs are stereo, so the sound comes from "
               "both sides.",
               "> Play a song. Bring up the speaker&rsquo;s volume to a "
               "comfortable level, then set MacMusic&rsquo;s to suit.",
               "~ The sound is usually clearest with MacMusic fairly loud and "
               "the speaker turned lower.")),
           T("To Record Onto Tape", steps(
               "> Connect the Macintosh to the tape recorder&rsquo;s auxiliary "
               "or line input, not its microphone input, which would "
               "distort.",
               "> Set the recorder to record, and pause it.",
               "> Play the song. Set MacMusic&rsquo;s volume, then the "
               "recorder&rsquo;s recording level, so that the meters stay "
               "just short of the red in the loudest parts.",
               "> Choose the song again from its list to start it from the "
               "beginning, and release the recorder&rsquo;s pause.",
               "~ MacMusic&rsquo;s volume has eight steps, so make fine "
               "adjustments on the recorder.")),
           ]),

    Topic("Settings",
          "<p>Settings, in the main menu, holds MacMusic&rsquo;s preferences, "
          "and a few facts about your library. Legal shows MacMusic&rsquo;s "
          "legal statement.</p>",
          "MacMusic remembers your settings from one time to the next.",
          [T("To Change Shuffle or Repeat", steps(
               "> Choose Settings from the main menu.",
               "> Highlight Shuffle or Repeat, and click the centre button.",
               "Each click shows the next setting beside the item.",
               "Shuffle: Off, Songs or Albums (see &ldquo;Shuffling&rdquo;).",
               "Repeat: Off; One, the same song again and again; or All, back "
               "to the start of the list when it ends.",
               "~ With Repeat One, Next and Previous still go to another "
               "song."), "c2-settings"),
           T("To See About", steps(
               "> Choose About from Settings.",
               "You see how many songs your library holds, out of the 500 it "
               "can; how much room is left on the library&rsquo;s disk "
               "(Available); and MacMusic&rsquo;s version."), "c2-about"),
           T("To Reset All Settings", steps(
               "> Choose Reset All Settings from Settings.",
               "Cancel is highlighted, so a stray click does no harm.",
               "> Highlight Reset, and click the centre button.",
               "Shuffle and Repeat go back to Off. The volume stays as it "
               "is."), "c2-reset"),
           ]),

    Topic("Ratings, Backlight and the Clock",
          "<p>A few finishing touches: stars for your favourite songs, a "
          "backlight for show, and a clock.</p>",
          None,
          [T("To Rate a Song", steps(
               "> On Now Playing, click the centre button twice.",
               "The first click shows the scrubber; the second, the song&rsquo;s "
               "rating.",
               "> Turn the click wheel to give the song from one to five "
               "stars.",
               "The rating is saved with the song. After five seconds, the "
               "progress bar comes back.",
               "~ MacMusic keeps ratings, but doesn&rsquo;t use them for "
               "anything yet."), "c2-rating"),
           T("To Turn the Backlight On or Off", steps(
               "> Choose Backlight from the main menu.",
               "MacMusic&rsquo;s screen turns light on dark. Choose Backlight "
               "again to turn it back.",
               "~ It&rsquo;s just for show, and it&rsquo;s off again the next "
               "time you start MacMusic."), "c2-backlight-off", "c2-backlight-on"),
           T("To See the Clock", steps(
               "> Choose Extras from the main menu, and then Clock.",
               "The Clock shows the time and date from your Macintosh&rsquo;s "
               "own clock, and keeps up to the minute.",
               "~ To set the time, use the Alarm Clock or the Control Panel desk "
               "accessory; see the <i>Macintosh Owner&rsquo;s Guide</i>."),
             "c2-extras", "c2-clock"),
           ]),

    Topic("Adding Songs",
          "<p>Songs come into your library by importing them. MacMusic plays "
          "WAV sound files of one particular kind: mono, 22,050 samples a "
          "second, 8 bits. To make them from your own music, see "
          "<i>MacMusic Utilities</i>.</p>",
          "A library can hold as many as 500 songs.",
          [T("To Import a Song", steps(
               "> Choose Import Song from the File menu.",
               "> Select a WAV file, and click Open.",
               "If the file is on the same disk as your library, MacMusic says "
               "it will move the file into the library.",
               "> Click OK.",
               "A dialog box shows the song&rsquo;s details, filled in from the "
               "file where MacMusic can.",
               "> Type or change any of the details, and click Import.",
               "~ Each detail can be up to 63 characters long.")),
           T("To Import From a Floppy Disk or Another Hard Disk", steps(
               "MacMusic first asks: &ldquo;Importing from another disk will "
               "take some time. Do you still want to import?&rdquo;",
               "> Click Import.",
               "> When MacMusic says the file will be moved, click OK, or Just "
               "Copy to keep the original.",
               "OK copies the song into your library, then removes the "
               "original from the other disk.",
               "~ If the disk is locked, the original can&rsquo;t be removed, "
               "and MacMusic tells you it has been left in place.")),
           T("To Keep the Original", steps(
               "A file is normally moved into your library, so that it "
               "isn&rsquo;t left in two places.",
               "> When MacMusic says the file will be moved, click Just "
               "Copy.",
               "Your library gets a copy, and the original stays where it "
               "was. Copying takes longer than moving.")),
           T("If the Song Is Already There", steps(
               "MacMusic recognises a song by its sound, even under another "
               "name. If you import one that&rsquo;s already in your library, "
               "it tells you, and names the song.",
               "> Click Cancel to stop, or Import Anyway to have it "
               "twice.")),
           T("If the File Isn&rsquo;t the Right Kind", steps(
               "If the file isn&rsquo;t a WAV file, MacMusic says so. If "
               "it&rsquo;s a WAV file of another kind, it says what the file is, "
               "and what&rsquo;s needed: &ldquo;Only mono, 22050 Hz, 8-bit PCM "
               "WAV files can be imported.&rdquo;",
               "> Click OK.",
               "To prepare files MacMusic can play, see <i>MacMusic "
               "Utilities</i>.")),
           T("If the Library Is Full", steps(
               "When your library holds 500 songs, MacMusic says &ldquo;The "
               "MacMusic Library is full (500 songs). Export a song to make "
               "room.&rdquo;",
               "> Click OK.",
               "To make room, export songs you no longer want (see "
               "&ldquo;Changing and Removing Songs&rdquo;), or start another "
               "library.")),
           T("If There Isn&rsquo;t Room on the Disk", steps(
               "Music takes room: about 1.3 MB for each minute. If the "
               "library&rsquo;s disk is too full for a song, MacMusic says "
               "so.",
               "> Click OK, make room on the disk, and import the song "
               "again.",
               "~ Settings, About shows how much room is left.")),
           ]),

    Topic("Changing and Removing Songs",
          "<p>You can change any of a song&rsquo;s details at any time, and "
          "remove a song you no longer want.</p>",
          "Taking a song off a playlist doesn&rsquo;t remove it from your "
          "library; see &ldquo;Working With Playlists.&rdquo;",
          [T("To Change a Song&rsquo;s Details", steps(
               "> Highlight the song in a list, or play it, so that it&rsquo;s "
               "on Now Playing.",
               "> Choose Edit Song from the Edit menu.",
               "You see the same details as when you imported it.",
               "> Make your changes, and click Save.",
               "The song is filed under its new details, and you stay on the "
               "same screen."), "c2-edit-menu"),
           T("To Export (Remove) a Song", steps(
               "To remove a song, you export it: MacMusic gives it back as a "
               "WAV file, just as you imported it, and takes it out of your "
               "library and every playlist.",
               "> Highlight the song, and choose Export Song from the File "
               "menu.",
               "> Choose where to save the file (anywhere outside the library "
               "folder), and click Save.",
               "~ If you don&rsquo;t want the file, drag it to the "
               "Wastebasket."), "c2-export-menu"),
           T("To Bring an Exported Song Back", steps(
               "> Import the exported file again.",
               "MacMusic fills in the song&rsquo;s details from the file where "
               "it can, as it did the first time.",
               "~ The song comes back without its rating, and isn&rsquo;t on any "
               "playlists; add it to them again.")),
           ]),

    Topic("Working With Playlists",
          "<p>A playlist lists songs from your library in the order you "
          "choose. You can have as many as ten.</p>",
          "The commands for playlists are in the Playlists menu, in the menu "
          "bar.",
          [T("To Make a Playlist", steps(
               "> Choose New Playlist from the Playlists menu.",
               "> Type a name, and click Create.",
               "The name can be up to 31 characters. The playlist appears "
               "under Playlists in the Music screen, and in the Playlists menu "
               "as Add to &ldquo;<i>name</i>.&rdquo;")),
           T("To Add a Song", steps(
               "> Highlight a song in any list.",
               "> Choose Add to &ldquo;<i>name</i>&rdquo; from the Playlists "
               "menu.",
               "The song goes at the end of the playlist.",
               "~ The Add to commands are dimmed unless a song is "
               "highlighted."), "c1-add-to"),
           T("To Add a Song Again", steps(
               "A song can be on a playlist more than once. If it&rsquo;s "
               "already there, MacMusic asks first.",
               "> Click Add Again to add it once more, or Cancel.")),
           T("To Remove a Song From a Playlist", steps(
               "> Open the playlist (Music, then Playlists, then the playlist), "
               "and highlight the song.",
               "> Choose Remove from Playlist from the Playlists menu.",
               "The song comes off the playlist, but stays in your "
               "library.")),
           T("To Rename a Playlist", steps(
               "> Highlight the playlist in the Playlists list, or open it.",
               "> Choose Rename &ldquo;<i>name</i>&rdquo; from the Playlists "
               "menu.",
               "> Type the new name, and click Rename."), "c2-rename"),
           T("To Delete a Playlist", steps(
               "> Highlight the playlist, or open it.",
               "> Choose Delete &ldquo;<i>name</i>&rdquo; from the Playlists "
               "menu.",
               "MacMusic asks you to confirm.",
               "> Click Delete.",
               "The playlist goes; its songs stay in your library.")),
           T("Playlists in the Finder", steps(
               "Each playlist is a text file in the Playlists folder inside "
               "your library, listing its songs, one a line.",
               "You can open one with TeachText to read it, but change "
               "playlists only with MacMusic.")),
           ]),

    Topic("Using More Than One Library",
          "<p>You might keep one library on your hard disk and another on a "
          "different disk. MacMusic uses one library at a time, and "
          "remembers the last five you&rsquo;ve used.</p>",
          "The library in use is checked in the File menu.",
          [T("To Create Another Library", steps(
               "> Choose Choose Library from the File menu.",
               "MacMusic asks whether to use an existing library or create a "
               "new one.",
               "> Click Create New Library. Choose where to put it, name it, "
               "and click Save.",
               "MacMusic switches to the new, empty library.",
               "~ If you choose a folder that&rsquo;s already a library, "
               "MacMusic offers to use it instead."), "c2-choose"),
           T("To Open an Existing Library", steps(
               "> Choose Choose Library from the File menu, and click Use "
               "Existing Library.",
               "> Open the library&rsquo;s folder, select its MacMusic Library "
               "Info file, or any of its songs, and click Open.",
               "MacMusic reads the library, and shows the main menu.")),
           T("To Switch to a Library You&rsquo;ve Used Before", steps(
               "The libraries you&rsquo;ve used are listed in the File menu.",
               "> Choose one from the File menu.",
               "~ A library whose disk isn&rsquo;t there is dimmed. Insert the "
               "disk, or switch on the hard disk, and try again."), "c2-recent"),
           T("To Rescan a Library", steps(
               "MacMusic reads your library each time it starts. To have it "
               "read the library again:",
               "> Choose Rescan Library from the File menu.",
               "~ A large library takes a while: a minute and a half or so for "
               "500 songs.")),
           T("To See Library Info", steps(
               "> Choose Library Info from the File menu.",
               "You see how many songs and playlists the library has, how much "
               "room its songs take, how much is free on its disk, and how "
               "many files couldn&rsquo;t be read.",
               "> Click OK.")),
           T("If MacMusic Can&rsquo;t Find Your Library", steps(
               "If the library&rsquo;s disk isn&rsquo;t there when MacMusic "
               "starts (a floppy not inserted, say, or a hard disk not switched "
               "on), MacMusic says so.",
               "> Insert the disk or switch it on, and click Find Existing; or "
               "click Create New.")),
           T("To Carry On Without a Library", steps(
               "> When MacMusic asks for a library, click Not Now.",
               "MacMusic starts with nothing in it. You can choose a library at "
               "any time with Choose Library in the File menu.")),
           ]),

    Topic("MacMusic and Your Other Applications",
          "<p>MacMusic plays on while you do other things. Under MultiFinder "
          "you can work in another application with the music playing, and "
          "desk accessories can be used at any time.</p>",
          "If another application keeps the Macintosh completely busy for "
          "more than about six seconds, you may hear a moment of music "
          "repeat, then a pause; the music then carries on by itself.",
          [T("To Keep Playing While You Work", steps(
               "> Under MultiFinder, start a song, then open another "
               "application or click in one of its windows.",
               "The music plays on. Click in the MacMusic window to come back "
               "to it.",
               "~ Without MultiFinder, quitting MacMusic to open another "
               "application stops the music."), "c2-background", double=True),
           T("To Use Desk Accessories", steps(
               "> Choose a desk accessory from the Apple menu.",
               "The music plays on while you use it.",
               "~ Click in the MacMusic window to bring it to the front "
               "again."), "c2-da-crop"),
           T("To See How Much Memory MacMusic Uses", steps(
               "> Under MultiFinder, click the desktop to make the Finder "
               "active, and choose About the Finder from the Apple menu.",
               "A bar shows how much memory each open application has. MacMusic "
               "asks for 640K."), "c2-about-finder", double=True),
           T("To Change MacMusic&rsquo;s Memory Size", steps(
               "MacMusic works best with 640K of memory, and needs at least "
               "512K.",
               "> Quit MacMusic. In the Finder, select its icon and choose Get "
               "Info from the File menu.",
               "> Type a new size in the Application Memory Size box, and close "
               "the window."), ("c2-get-info", 150), double=True),
           ]),
]


def contents():
    rows = []
    for t in TOPICS:
        rows.append('<li class="topic"><a href="#%s">%s</a></li>' % (anchor(t.title), t.title))
        rows += ['<li class="task"><a href="#%s">%s</a></li>' % (anchor(k.title), k.title)
                 for k in t.tasks]
    return "\n".join(rows)


DRAFT = draft_title("Chapter 2: Using MacMusic",
                    "A draft for review, 8 October 2026. In the finished book this "
                    "chapter follows Chapter 1, and its pages are numbered on from "
                    "there.")
PART = (opener(2, "Using MacMusic", "c2-opener")
        + '<div class="chapter chapter-contents"><div class="flow">'
          '<div class="margin"><h2 class="contents-head">Contents</h2></div>'
          '<ul class="toc">%s</ul></div></div>\n' % contents()
        + "\n".join(t.html() for t in TOPICS))


# The chapter for the book (make-book.py), and on its own for review.
if __name__ == "__main__":
    open(OUT, "w").write(page("MacMusic User&rsquo;s Guide: Chapter 2", DRAFT + PART))
    print("wrote", os.path.normpath(OUT))
