# Stage 10 plan: fast-forward and rewind

Status: **complete (2026-10-07)**: 10a, 10b and 10c done (F1-F8, G1-G4).
No code has been changed. Decision N8 (deferred since 5c-1, W4, and
Stage 7). Built as close to the Stage 7c scrubber as possible.

## What the 4G does (source)

Apple's iPod (Click Wheel) User's Guide, p. 16:
- Fast-forward: "Press and hold Next/Fast-forward."
- Rewind: "Press and hold Previous/Rewind."
- (A press, not held: Next skips to the next song; Previous starts the
  song over, and twice plays the previous song.)

The guide says nothing about speed, sound while held, the ends of a song,
pausing, or other screens; those are the decisions below, PROVISIONAL in
the code.

## The principle

Fast-forward and rewind change a **pending playback position**; they
never seek the audio while held. This is the scrubber's design (7c):
- while held, the song carries on playing and the progress bar and times
  race to the target;
- on release, one jump (a new stream with about a second of pre-fill);
- playback then carries on from the target.

It isn't an audible 16× fast-forward, and isn't meant to be.
Speeded-up audio on the 68000 would be an unnecessary audio-processing
problem, and seeking repeatedly would stall, since each seek costs about
a second of loading at Plus speed.

## Where the app is now (what is reused)

- **The scrubber's pending position (7c), all in `ui_nav.c`:**
  - `gScrubPending` and `gScrubTicks`: the target in song time;
  - Now Playing draws `gScrubTicks` instead of the playing position
    whenever a target is pending, in any mode;
  - `ApplyScrub()` sends it to playback with one `Playback_Seek`;
  - `CancelScrub()` drops it when the song changes.

  Fast-forward and rewind use these same variables and calls.
- **`UI_Idle()`** runs on every mouse sample while the wheel's button is
  held (`Input_TrackWheel` calls it through the wheel sink), and from the
  main loop otherwise. It already gives the scrubber its timed jump, and
  here it moves the target while a hold lasts.
- **`Input_TrackWheel`:** a click on Next or Previous acts on release,
  if the wheel wasn't turned. The wheel's step size already comes from
  the UI (`UI_WheelStepDegrees`), and whether a hold is allowed will be
  asked the same way.
- **Seeking to a song's very end** ends that song at once, and the
  normal end-of-song path plays the next as Repeat and Shuffle say (9d).
- **The keyboard:** → and ← (the emulator mapping, D8) act on key-down;
  System 6 reports no key releases, only auto-repeats while held.

## 10a: press and hold on Next and Previous (input only; no fast-forward)

10a establishes only these rules:
- **a short press skips,** as now;
- **a long press is recognised as a hold;**
- **releasing after a hold doesn't skip.**

There is no position movement and no fast-forward logic in 10a.

- **Where (F6):** only on Now Playing. The input asks the UI whether a
  hold is allowed (`UI_HoldAllowed`, like `UI_WheelStepDegrees`).
  - On every other screen, Next and Previous behave exactly as now: a
    press, held or not, skips on release.
  - Only on Now Playing while a song is playing or paused can a hold
    begin.
- **The mouse (F1):**
  - Next or Previous held for 30 ticks (about half a second) without the
    wheel having turned is a hold. The UI receives "hold Next" or "hold
    Previous" once, then "hold ended" on release, and no skip.
  - **Turning during a hold:** once a hold has begun, wheel movement is
    ignored until release.
  - **Turning before a hold:** if the wheel turns before the half
    second, it's a turn, as now (the volume on Now Playing), and no hold
    begins.
- **The keyboard (F2):** Option-→ and Option-← are the held Next and
  Previous.
  - The hold begins with the key's first auto-repeat (so after the
    Keyboard control panel's repeat delay).
  - It ends when the repeats stop: no repeat for twice the repeat rate,
    checked in `UI_Idle`.
  - A tap of Option-→ or Option-← without a repeat does nothing.
  - Plain → and ← are unchanged: they skip on key-down.
- **How a hold shows in 10a:** the play/pause icon at the top of Now
  Playing becomes a fast-forward (▶▶) or rewind (◀◀) icon while the hold
  lasts, and returns on release. The artwork is PROVISIONAL. This is the
  icon the finished feature will show; it's the only visible change in
  10a.
- **Unchanged:** Select, Play/Pause, Menu, wheel turning, the scrubber,
  volume and rating.
- **Tests:**
  - a diagnostic for press versus hold timing, turning before and during
    a hold, the other screens, and the keyboard repeat timing;
  - then the manual check, at 1×, playing and paused:
    - short Next, short Previous;
    - hold Next, hold Previous;
    - turning the wheel during a hold;
    - Select, Play/Pause;
    - the keyboard equivalents.

  Only when that is solid does 10b begin.

## 10b: fast-forward and rewind (the scrubber's pending position)

- **Starting a hold:**
  - Now Playing returns to normal mode first (any pending scrubber
    position is applied).
  - The hold notes the starting position: the playing position, or the
    paused one.
  - It notes the tick it began, and sets `gScrubPending`.
- **While held (F4, time-based):** the target is worked out afresh from
  the time held, not by adding a fixed amount each pass, so a busy Plus
  doesn't change where it lands.
  - Elapsed hold time t (in ticks, since the hold began):
    - up to 2 s: 4× (the target moves 4t);
    - after 2 s: 16× (8 s of song for the first 2 s, then 16 × (t − 2 s)).
  - `gScrubTicks` is the starting position plus (fast-forward) or minus
    (rewind) that offset, kept between 0:00 and the song's end.
  - `UI_Idle` recomputes it on every pass while the hold lasts, and the
    scrubber's settle timer is not used during a hold.
- **Sound while held (F3):** the song carries on playing from where it
  was; only the display races to the target.
- **Release:** `ApplyScrub()` straight away, with no half-second settle:
  one jump, and playback carries on from the target.
- **The ends (F5):**
  - **Fast-forward** stops at the song's end. Released there, the jump
    to the end finishes the song at once, and the next song plays as
    Repeat and Shuffle say (9d), with no new code for this.
  - **Rewind** stops at 0:00.
- **Paused (F7):** the target moves, and on release it becomes the
  paused position (`Playback_Seek` while paused, as the scrubber does).
  The song stays paused, and resumes from there.
- **If the song changes during a hold:** for example, the playing song
  reaches its natural end while the display is racing. The pending
  position is dropped, as for the scrubber (`CancelScrub`). The hold
  carries on from the new song's position.
- **Next and Previous during a hold:** a hold on one button ignores the
  other.
- **Tests:**
  - a diagnostic holding for set times, with the target checked against
    the formula: 1 s gives 4 s of song, and 3 s gives 8 + 16 = 24 s;
  - a single jump on release (counted), and no seeks while held;
  - both ends, including fast-forward to the end with Repeat Off, One
    and All;
  - rewind to 0:00;
  - paused;
  - a song change during a hold;
  - no underruns;
  - the manual check at 1×.

## 10c: Shuffle and Repeat icons on Now Playing (approved 2026-10-07)

Asked for after the 10b check: with Repeat set to One, a song
fast-forwarded to its end replays (correctly), but nothing on screen said
Repeat was on. Now Playing will show when Shuffle and Repeat are in
effect.

### References found (2026-10-07)

- **The 4G Click Wheel guide (2004)** doesn't describe shuffle or repeat
  icons. Its only title-bar items are the battery ("upper-right corner")
  and the optional clock.
- **The iPod classic guide** (the later click-wheel generation), on the
  Now Playing screen:
  - "**Shuffle icon**: Appears if iPod classic is set to shuffle songs or
    albums."
  - "**Repeat icon**: Appears if iPod classic is set to repeat all songs.
    The **Repeat Once** icon appears if iPod classic is set to repeat one
    song."

  The guide's labelled diagram of the screen couldn't be retrieved; the
  only image fetched was a table layout.
- **Descriptions of the shapes** (other sources, consistent with each
  other):
  - shuffle: two crossed arrows;
  - repeat: two arrows, each pointing at the other's tail (a loop);
  - Repeat One: the same with a small "1".
- **Placement:** not stated in any source found. On the later iPods both
  icons are recalled as sitting together on the right of the title bar,
  just left of the battery, but this is unconfirmed.
- **So:** the icons' meaning and shapes follow Apple's descriptions; their
  exact artwork and position are PROVISIONAL, as N7.

### Room in our title bar

Now Playing's bar is 18 px tall, and the icons are at icon height
(y 3-11):
- the play/pause icon at x 9-17 (fast-forward and rewind to x 18);
- "Now Playing" centred, about x 42-118 in Chicago 12 (to be measured
  in 10c);
- the battery at x 134-153.

That leaves about x 20-40 on the left and only x 120-132 on the right:
room for one icon on the right, not two, while the title stays centred.

### Placement (G1): below the title bar, top right (proposed 2026-10-07)

Asked for in review: put the icons at the top right of Now Playing,
below the battery and below the title bar's rule, rather than in the
title bar.

**Why it fits:** below the rule (y 17), the strip down to the song
title has only the "3 of 12" counter, on the left at y 24-30 in the
small font. The title's ink starts at y 39, and the right side of the
strip is empty.

**Proposed layout:** two 11 x 9 px icons on the counter's row, y 23-31
(centred on the counter's 24-30), right-aligned with the progress bar
and the remaining time (which end at x 150-151):
- repeat at x 141-151, under the battery (x 134-153);
- shuffle at x 128-138, to its left, with a 2 px gap.

Each icon is drawn only when on. With one icon showing, it takes the
rightmost place (x 141-151), so a lone shuffle icon sits under the
battery too.

**What this avoids:**
- **The title bar is unchanged:** it still matches the measured 4G
  screenshot, and "Now Playing" stays centred.
- **No other screen changes.**
- **No overlap:** nothing clashes with the counter (which ends well
  before x 100, even at "500 of 500"), the song title, or the
  scrubber, volume or rating bars further down.

**Not from a reference:** the later iPods put these icons in the title
bar (as far as recalled). This place is a MacMusic decision for
legibility on the 160 x 128 screen, PROVISIONAL like the artwork.

**The earlier options, set aside:** in the title bar, (a) both right
with the title shifted, (b) split either side of the title, or (c) both
right but smaller. There is room in the title bar for only one icon
beside the battery while the title stays centred.

### What they show (G2, G3, G4)

- **Repeat:** the Repeat setting, which acts on what is playing at once
  (9d). The loop for All; the loop with a "1" for One (G4); nothing for
  Off.
- **Shuffle:** whether the songs now playing were shuffled (the queue's
  own order, kept by playback for Repeat All's reshuffle, 9d). Shuffle
  Songs from the main menu shows it too. The Shuffle setting affects only
  the next list played (T4), so the icon follows what is actually
  playing. Shuffle by Albums shows the same icon (G3), as the classic
  guide's "shuffle songs or albums" suggests.

### Code, tests and order

- **Code:**
  - two (three, with Repeat One) small bitmaps and a draw call in
    `lcd_draw.c`;
  - `LCDDraw_NowPlaying` gains the two states;
  - `Playback_ShuffleMode()` gives the queue's mode;

  Playback and the settings are unchanged.
- **Tests:**
  - render checks of every combination, including with the fast-forward
    icon;
  - that nothing overlaps the counter, the song title or the title bar;
  - that the Repeat icon follows a change at once, and the Shuffle icon
    only with the next list;
  - manual at 1×.
- **Order:** a separate step after 10b (committed).

### Decisions (2026-10-07)

- **G1:** below the title bar at the top right, on the counter's row:
  repeat under the battery, shuffle to its left. PROVISIONAL.
- **G2:** the shuffle icon is shown whenever any shuffle is happening:
  the songs now playing were shuffled, whether by the Shuffle setting
  (Songs or Albums) or by Shuffle Songs in the main menu. It isn't shown
  when they play in order, including a list played in order because it
  is audiobooks (S-2), or because Shuffle was changed only after it
  started (T4).
- **G3:** Shuffle by Albums shows the same shuffle icon.
- **G4 (reversed in review, 2026-10-07):** Repeat One shows the same loop
  as Repeat All, without a "1": the icon only says Repeat is on.

## Not in Stage 10
- Audible fast-forward (see the principle above).
- Fast-forward and rewind away from Now Playing (F6).
- The other deferred items: On-The-Go, the Extras audit, using ratings.

## Decisions (2026-10-07)

- **F1:** a hold is about half a second (30 ticks), PROVISIONAL.
- **F2:** Option-→ / Option-← are the held Next / Previous.
- **F3:** the song carries on while held; one jump on release.
- **F4:** 4× for the first 2 seconds held, then 16×, computed from the
  time held (not added up per pass), PROVISIONAL.
- **F5:** fast-forward stops at the song's end and the next song plays on
  release; rewind stops at 0:00.
- **F6:** Now Playing only. Elsewhere Next and Previous are unchanged.
- **F7:** paused stays paused, at the new position.
- **F8:** 10a then 10b. 10a stays tiny: input and the icon only.
