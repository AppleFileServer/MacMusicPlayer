# Stage 7 plan: Now Playing volume, scrubber and rating

Status: **complete (2026-10-07)**: 7a (`cede6c4`), 7b (`3016fe2`), 7c
(`de22d4d`) and 7d. Fast-forward/rewind remains deferred (N8). Decisions N1-N8 are recorded at the end.

## What the 4G iPod does (sources)

From Apple's iPod (4th generation, Click Wheel) user guide, 2004:

- **Volume:** "Change the volume: From the Now Playing screen, use the
  Click Wheel" (Using iPod Controls, p. 15).
- **Scrubber:** "Skip to any point in a song: From the Now Playing screen,
  press the Select button to show the scrubber bar. Then scroll to any
  point in the song" (p. 16).
- **Rating:** "You can assign a rating to a song (from 1 to 5 stars)...
  From the Now Playing screen, press the Select button twice. Use the
  Click Wheel to select a rating" (Rating Songs, p. 19).
- **Fast-forward/rewind:** "Press and hold Next/Fast-forward",
  "Press and hold Previous/Rewind" (p. 16): deferred from Stage 5c-1
  (W4); see N8.

The iPod colour-display guide (same software generation) adds that the
rating screen shows "either bullet points, stars, or a combination of
both" (p. 18).

**What is confirmed and what is provisional (N7):** the interactions
and functional elements below are treated as confirmed. Neither guide has
a screenshot of these screens or gives the timings, so the exact 160 x 128
pixel artwork and layout, and the timeouts, are PROVISIONAL in code until
an actual 4G screen photograph or reference is obtained.

- Volume: a horizontal bar with a small speaker icon at the left and a
  larger one at the right, shown in place of the progress bar and times
  while the wheel turns; it disappears a short time after turning stops.
- Scrubber: the progress bar with a diamond-shaped marker at the play
  position; turning moves the marker, and the elapsed and remaining
  times follow it.
- Rating: five bullets in place of the progress bar; turning clockwise
  turns them into stars from the left, counter-clockwise back to bullets.
- Pressing Select cycles: normal, scrubber, rating, normal. Each mode
  returns to normal after a few seconds without input.

## Where the app is now

- `UI_HandleEvent`: on Now Playing the wheel does nothing ("until volume
  (Stage 7)"); Select is handled by `HandleSelect`.
- Now Playing layout (`lcd_layout.h`, measured from Apple's screenshot):
  progress bar at x 8-150, y 99-107; times at y 114-120.
- Song files: the chunk's 16-bit flags field uses only bit 0 (audiobook).
- Playback (6a-4): the position is the Sound Manager's; a seek is a new
  stream from a sample (`Audio_Play` / `StartCurrent`), which reads a 4 s
  pre-fill first (about a second at Plus speed).

## Plan: four steps, each built, tested and committed separately

### 7a: Now Playing modes (no new audio behaviour)
- A mode for the Now Playing screen: normal, volume, scrubber, rating.
- Wheel in normal mode enters volume mode; Select cycles normal,
  scrubber, rating, normal; Menu leaves Now Playing as now (any mode).
- Each mode returns to normal after a timeout (N5) without input; the
  main loop's once-a-second redraw also handles the timeout.
- Drawing: the area of the progress bar and times is replaced by the
  mode's own bar (stubs at first, final drawing in 7b-7d). The title,
  artist, album, counter and play icon stay as they are.
- Play/Pause, Next and Previous keep working in every mode.
- Tests: host UI harness for the mode transitions and timeouts; render
  checks of each mode's screen (off-screen, compared pixel by pixel as in
  5c-1).

### 7b: Volume (N1, N2)
- Turning the wheel in volume mode changes the volume one step per wheel
  step (15 degrees, as for lists); the bar shows the level.
- Mechanism (N1): the Plus's hardware volume through the Sound Manager
  (`SetSoundVol`, 8 levels, 0-7). Instant, no processor cost, and doesn't
  touch the audio already in the ring.
- (N2) It simply changes the system volume. The chosen level is saved in
  the prefs, and MacMusicPlayer sets it when it starts, so it opens at the
  same setting next time.
- Done: the level is saved when volume mode ends (timeout, Select,
  Next/Previous, Menu) or on quit, not on every step: saving on every
  step was tried and made the wheel lag (each save is several disk
  accesses). The prefs file is now version 2 (version 1 is still read).
- Tests: the level steps 0-7 and back; the bar drawing; the level
  survives quit and relaunch; manual listening.

### 7c: Scrubber (N3)
- In scrubber mode the diamond marks the position; turning moves it, with
  the times following.
- (N3) One complete revolution of the wheel is one complete traversal of
  the track: the marker moves by (degrees turned / 360) x the song's
  length. The wheel's usual 15-degree steps would give only 24 positions
  per revolution, so in scrubber mode the wheel reports finer movement
  (about 3 degrees: 120 positions per revolution).
- (N3) The audio jumps when turning stops (about half a second without
  movement), not on every step: each jump is a new stream with a
  pre-fill, about a second at Plus speed, so seeking on every step would
  stall. While turning, the marker and times move but the audio carries
  on from where it was.
- Paused: the marker moves and the new position is used when play
  resumes.
- Gapless: a seek starts a new stream at the current song; the next song
  is queued again as usual.
- Done: Select, Menu and Play/Pause apply a pending position at once;
  Next/Previous and a natural song change drop it. The seek also
  happens while the button is still held. The wheel now redraws once
  per mouse sample rather than once per step, so the fine steps don't
  lag. The 3-degree step and 0.5 s wait are PROVISIONAL.
- Tests: diagnostic seeking to set positions (start, middle, near the
  end) and checking the Sound Manager position afterwards; manual.

### 7d: Rating (N4)
- In rating mode turning sets 0-5 stars; the bullets/stars show it.
- Storage (N4): bits 1-3 of the song chunk's flags field. **Rating 0
  means unrated; 1-5 are the five ratings** (values 6 and 7 are invalid
  and read as unrated). Bits 1-3 are genuinely unused (only bit 0,
  audiobook, is defined), and files without a rating read as 0, so the
  chunk format version doesn't change. The song file is changed by
  rewriting those two bytes in place. Export removes the chunk, so a
  rating doesn't leave the library (as other details).
- (N4) Written only when rating mode is exited (timeout or Select), never
  on each wheel movement; the in-memory library is updated at once.
- Writing while the song plays: the audio has the file open read-only;
  opening a second path with write access is allowed by the File
  Manager, but this is checked in a diagnostic first.
- Done: the rating is kept by song ID, so it is saved to the song that
  was rated even if the song changes first; it is also saved on quit. A
  write is refused if the file's song ID doesn't match. Writing while
  the song plays was verified (no underruns). On an error the library
  keeps the old rating and a message is shown.
- Tests: host tests for reading/writing the rating bits; a diagnostic
  that rates the playing song and rescans; manual.

### Not in Stage 7: press-and-hold fast-forward and rewind (N8)
- Deferred again (from 5c-1, W4): holding Next or Previous to move
  through the song is left for later.

## Decisions (decided 2026-10-07)

- **N1:** volume by the Plus hardware's 8 levels (`SetSoundVol`).
- **N2:** it simply changes the system volume; a preference remembers the
  chosen level and MacMusicPlayer sets it on start, so it opens at the same
  setting next time.
- **N3:** one complete revolution of the wheel = one complete traversal of
  the track (finer wheel resolution in scrubber mode); the audio jumps
  when turning stops.
- **N4:** rating in bits 1-3 of the chunk flags, 0 = unrated, 1-5 the
  ratings; no format version change; written only when rating mode is
  exited.
- **N5:** timeouts 2 s (volume) and 5 s (scrubber, rating), PROVISIONAL.
- **N6:** order 7a, 7b, 7c, 7d.
- **N7:** interactions and functional elements confirmed; exact 160 x 128
  pixel artwork and layout PROVISIONAL unless a 4G screen photograph or
  reference is obtained.
- **N8:** press-and-hold fast-forward/rewind deferred; not in Stage 7.
