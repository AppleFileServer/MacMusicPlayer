# Stage 8b: Settings menu audit and specification

Status: **decided (2026-10-07)**, S-1 to S-11. A document only; no code changed.
Part of Stage 8 (`stage-8-library-and-settings.md`, decision L9).
Building the Settings screens is a later stage (S-8).

## Source

Apple's iPod (Click Wheel) User's Guide (June 2004, revised March 2005),
for the 4G with iPod software 3.x. The page numbers below are that
guide's. The guide describes each setting in its own section but gives
**no complete list of the Settings menu in order**, so the order below
(from the 4G's menu as commonly shown) is PROVISIONAL.

## Where the app is now

- **The iPod main menu:** Music, Extras, Settings, Shuffle Songs,
  Backlight, then Now Playing while a song is playing.
  - Extras and Settings are stubs (D3).
  - Backlight is shown but does nothing.
- **Existing settings:**
  - the volume (7b): set on Now Playing, kept in the prefs;
  - Shuffle Songs (main menu): plays the whole library in random order,
    skipping audiobooks.
- **Ratings (7d)** are per song, in the song files. They aren't a
  setting.
- **Storage:** under Option A (8a, one library at a time) every setting
  is the app's, not a library's, and lives in the prefs file. No 4G
  setting below is library-specific.

## The audit, item by item

### About (p. 31)
- **4G:** "how many songs are stored on iPod, how much disk space is
  left, and other information": songs, capacity, available space,
  software version, serial number.
- **Mac Plus:** makes sense as a read-only iPod screen.
- **Possible:** yes:
  - Songs: the library's count;
  - Capacity and Available: the library disk's size and free space;
  - Version: MacMusic's version.

  No serial number.
- **Verdict: adapt.** The rows are Songs, Capacity, Available and
  Version. It overlaps with Library Info (8d), which is the fuller Mac
  view; this is the iPod's own summary.

### Shuffle (p. 32): Off, Songs, Albums
- **4G:** "Set Shuffle to Songs or to Albums." With Songs, a list plays
  in random order; with Albums, each album plays in order and the next
  album is chosen at random.
- **Mac Plus:** makes sense.
- **Possible:** yes. It only changes the order of the play queue, which
  is built when play starts. No audio cost, and gapless playback is
  unaffected.
- **Verdict: replicate.** Stored in the prefs. The main menu's Shuffle
  Songs stays as it is (the whole library, audiobooks skipped).
  Audiobooks are never shuffled (S-2).

### Repeat (p. 32): Off, One, All
- **4G:** "repeat a song over and over, or ... a sequence of songs."
- **Mac Plus:** makes sense.
- **Possible:** yes, in the play queue:
  - **One:** the song is queued again after itself, so it repeats
    gaplessly;
  - **All:** after the last song, the queue starts again from the first.

  This changes today's rules: N4 stops at the end of the last song, and
  N6 makes Next wrap. Off keeps today's behaviour.
- **Verdict: replicate.** Stored in the prefs. Next and Previous with
  Repeat One are an open detail (S-3).

### Backlight Timer (p. 34), and the main menu's Backlight
- **4G:** how long the screen's backlight stays on after a button press.
  The main menu's Backlight item turns it on.
- **Mac Plus:** the screen is black and white with no backlight to show.
- **Investigated (S-4): show "backlight on" as the LCD inverted** (black
  pixels white, white pixels black).
  - **Cost: none.** The LCD is drawn off screen and copied to the window
    in one CopyBits call. Inverting is that call's notSrcCopy mode
    instead of srcCopy, the same speed, and none of the drawing code
    changes.
  - **Look:** white text on a black screen. A lit 4G LCD stays dark text
    on a brighter background, so this is a symbol of "lit" rather than a
    likeness, and an inverted screen is easy to tell apart at a glance.
  - **What it would bring back:** if Backlight means inverting, the
    main menu's Backlight item and the Backlight Timer both become
    meaningful (S-9, S-10).
- **Verdict:** the main menu's Backlight item inverts the LCD, as a
  novelty (S-9). **The Backlight Timer is omitted** (S-10).

### Audiobooks (p. 31): Slower, Normal, Faster
- **4G:** plays audiobooks faster or slower "than normal", keeping the
  voice's pitch.
- **Mac Plus:** pitch-preserving time-stretching in real time is far
  beyond a 68000 already busy reading the disk. Changing the playback
  rate would raise or lower the pitch, which isn't the 4G's behaviour.
- **Verdict: omit.**

### EQ (p. 34)
- **4G:** equalizer presets (Rock, Jazz and others).
- **Mac Plus:** real-time filtering of 22,000 samples a second on the
  68000, alongside the disk reads, isn't feasible. The Plus's 8-bit
  output would also make the effect crude.
- **Verdict: omit.**

### Sound Check (pp. 33-34)
- **4G:** uses iTunes' volume adjustment so songs play at the same
  relative level.
- **Mac Plus:** possible in a simple form: a per-song level measured at
  import and applied while filling the ring, through a 256-byte table.
  That costs some processor time per sample and loses detail at 8 bits,
  and existing songs would need measuring.
- **Verdict: defer.** Possible later, not in the first Settings.

### Contrast (p. 52)
- **4G:** the LCD's contrast.
- **Mac Plus:** the simulated LCD is drawn in 1 bit; contrast has no
  meaning.
- **Verdict: omit.**

### Clicker (p. 33): Off, Speaker, Headphones, Both
- **4G:** a click from the iPod's speaker while scrolling.
- **Mac Plus:** the Plus has one sound output, used by the looped music
  while a song plays. A click can't play over the music: mixing it into
  the ring would arrive seconds late, and a beep stops the music. While
  nothing is playing (stopped or paused) the sound channel is idle, so a
  short click can play then.
- **Verdict: adapt, On and Off.** The click sounds only when no audio is
  playing; while a song plays the wheel is silent. Soon: in the first
  Settings build (S-1).

### Main Menu (p. 35)
- **4G:** choose which items appear in the main menu (Songs, Artists,
  Playlists and others can be added, and items removed).
- **Mac Plus:** makes sense.
- **Possible:** yes: an on/off list, kept in the prefs, used when the
  main menu is built.
- **Verdict: defer.** Worth having, but it isn't needed first, and the
  main menu's own contents (Extras, Backlight) are being decided here.

### Date & Time (p. 35)
- **4G:** sets the iPod's clock.
- **Mac Plus:** the Mac's clock (Alarm Clock desk accessory or Control
  Panel) is the clock; the app needs no second one.
- **Verdict: omit.**

### Language (p. 35)
- **4G:** the iPod's menu language.
- **Mac Plus:** the app is English only.
- **Verdict: omit.**

### Legal (p. 63 holds the guide's own legal text)
- **4G:** Apple's copyright and legal notices, as scrolling text.
- **Mac Plus:** Apple's notices don't apply to this app, but the
  project's own legal statement does.
- **Verdict: adapt.** Legal shows the project's legal statement (kept in
  `docs/legal/legal-statement.md`) as scrolling text. The wheel scrolls
  it, as on the 4G. In the app it is a text resource of about 5 KB.
  The project's name in it is MacMusic (S-11).

### Reset All Settings (p. 52)
- **4G:** "Settings > Reset All Settings > Reset": the settings return
  to their defaults. Songs are not affected.
- **Mac Plus:** makes sense.
- **Possible:** yes. It resets the prefs' settings only, never the
  library, ratings or playlists.
- **Verdict: replicate**, with a confirmation screen (Cancel, Reset).
  Whether it also resets the volume: S-5.

### Not in the 4G
- Volume Limit and Compilations arrived on later iPods, so they're not
  included.
- Contacts' sort and display settings aren't in this guide.

## Draft specification: the MacMusic Settings menu

| Order | Item | Values (default first) | Stored | Notes |
|---|---|---|---|---|
| 1 | About | Read-only: Songs ("n of 500"), Available (free space on the library's disk), Version | n/a | Adapted (S-6) |
| 2 | Shuffle | Off, Songs, Albums | Prefs | Replicated; audiobooks never shuffled (S-2) |
| 3 | Repeat | Off, One, All | Prefs | Replicated; Next and Previous still skip with Repeat One (S-3) |
| 4 | Clicker | On, Off | Prefs | Adapted: clicks only while no audio is playing |
| 5 | Legal | Read-only scrolling text | App resource | Adapted: the project's legal statement (S-11) |
| 6 | Reset All Settings | Cancel, Reset | n/a | Replicated; resets Shuffle, Repeat and Clicker, not the volume (S-5) |

Backlight is not a Settings item: it stays in the main menu, where
selecting it inverts the LCD (S-9).

**How each item behaves (PROVISIONAL screen details):**
- **Shuffle and Repeat:** each row shows its current value on the right.
  Select steps through the values in place, without opening another
  screen.
- **About:** opens a list of its rows, each with a value on the right.
  Capacity is shown as songs out of the 500-song limit (S-6).
- **Legal:** opens the text; the wheel scrolls it, Menu goes back.
- **Reset All Settings:** opens a two-row confirmation screen.

**Stored in the prefs file:** version 3 adds the Shuffle, Repeat and
Clicker bytes. Version 1 and 2 files are still read, with defaults for the new
settings.

**Deferred:** Sound Check and Main Menu, possible later stages.

**Omitted:** Backlight Timer, Audiobooks speed, EQ, Contrast, Date &
Time and Language.

**Unchanged:** the volume stays on Now Playing (7b), and Shuffle Songs
stays in the main menu.

## Building it later

The specification is built in a later stage, with these parts:
1. the Settings screen, About, Legal, and the prefs version 3;
2. Clicker ("soon"), with a check that it never sounds while a song
   plays;
3. Shuffle, with Songs and Albums applied when play starts;
4. Repeat, with One and All in the queue and gapless playback;
5. Reset All Settings;
6. Backlight in the main menu (the inverted LCD).

Each part gets diagnostics and a manual check.

## Decisions (2026-10-07)

- **S-1:** the audit table is approved, except that Clicker is adapted
  (On/Off, clicking only while no audio is playing, built soon) and Legal
  shows the project's legal statement.
- **S-2:** audiobooks are never shuffled.
- **S-3:** with Repeat One, Next and Previous still move to another song.
- **S-4:** investigate Backlight as an inverted screen: done (above).
- **S-5:** Reset All Settings leaves the volume alone (it is the Mac's
  system volume).
- **S-6:** About shows Songs as "n of 500" and Available as the free
  space on the library's disk.
- **S-7:** Extras is audited the same way later.
- **S-8:** the Settings are built as a stage after Stage 8.

## Follow-up decisions (2026-10-07)

- **S-9:** the main menu's Backlight item inverts the LCD (black pixels
  white, white pixels black) as a novelty. Selecting it again turns it
  back. PROVISIONAL: it isn't remembered between launches.
- **S-10:** no Backlight Timer.
- **S-11:** the legal statement names the project **MacMusic**: the app
  is being renamed from MacMusicPlayer (`rename-macmusic.md`). The licence the statement refers to will
  be written later, in the product documentation, so the wording stays
  as it is.
