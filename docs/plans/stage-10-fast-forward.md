# Stage 10 plan: fast-forward and rewind

Status: approved (2026-10-07), decisions F1-F8 made. 10a done.
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
