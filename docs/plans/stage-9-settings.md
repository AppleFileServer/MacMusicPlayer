# Stage 9 plan: the Settings menu

Status: approved (2026-10-07), decisions T1-T9 made. 9a and 9b done;
9c-9e remain. Builds the specification decided in Stage 8b
(`stage-8b-settings.md`, S-1 to S-11).

## What Stage 8b decided

| Order | Item | Values | Notes |
|---|---|---|---|
| 1 | About | Songs "n of 500", Available, Version | Read-only |
| 2 | Shuffle | Off, Songs, Albums | Audiobooks never shuffled (S-2) |
| 3 | Repeat | Off, One, All | Next and Previous still skip with Repeat One (S-3) |
| 4 | Clicker | On, Off | Clicks only while no audio is playing |
| 5 | Legal | The project's legal statement, scrolling | `docs/legal/legal-statement.md` |
| 6 | Reset All Settings | Cancel, Reset | Leaves the volume alone (S-5) |

Also from 8b:
- **Backlight:** the main menu's Backlight item inverts the LCD, as a
  novelty, until selected again (S-9). There is no timer (S-10).
- **Storage:** Shuffle, Repeat and Clicker are kept in the prefs.
- **Not included:** Extras is audited later (S-7).

## Where the app is now

- **The main menu's Settings item** is a stub (D3), and Backlight does
  nothing.
- **List rows** show a title and, optionally, an arrow. There's no way yet
  to show a value on the right ("Shuffle  Off"), and no screen for
  scrolling text.
- **Playback:**
  - the play queue is a copy of the chosen list's song IDs, in list order;
  - **Shuffle Songs** in the main menu shuffles the whole library,
    skipping audiobooks;
  - at the end of the last song playback stops (N4), and Next on the last
    song wraps to the first (N6).
- **Gapless audio (5c):** the next song is handed to the audio stream as
  soon as a song starts (G3). Each song in the stream carries a **tag**,
  its queue position, and the app follows the audio by watching the tag
  increase.
- **The sound channel** plays one looped sampled sound for the music
  (6a-4). While stopped or paused it is quiet.
- **The prefs file** is version 3 (8d-3).

## The parts, each built, tested and committed separately

### 9a: the Settings screen, its values, Reset, and Backlight (no new audio behaviour)
- **The Settings screen:** the main menu's Settings item opens a list
  screen with the six items in the order above.
- **Values on the right:** a list row can show a value ("Off", "Songs")
  at its right edge, which needs new drawing for list rows.
- **Choosing a value (T2):** Select on Shuffle, Repeat or Clicker steps to
  the next value in place, and the row updates. The values are saved
  (prefs version 4; versions 1-3 are still read). In 9a they're stored
  only; 9c-9e make them act.
- **Reset All Settings:** opens a two-row screen, Cancel then Reset.
  Reset sets Shuffle, Repeat and Clicker to their defaults (T1) and goes
  back to Settings. The volume is not touched (S-5).
- **Backlight:** the main menu's Backlight item inverts the LCD (the
  window copy uses notSrcCopy), and selecting it again turns it back.
  It isn't remembered between launches (S-9).
- **Tests:**
  - the values cycle, are saved and read back, and the prefs versions
    are read correctly;
  - Reset sets the defaults and leaves the volume alone;
  - render checks of the Settings screen and of the inverted LCD;
  - manual.

### 9b: About and Legal
- **About:** a list screen with values on the right:
  - Songs: "3 of 500";
  - Available: the free space on the library's disk;
  - Version: "0.1".

  Read-only; Menu goes back.
- **Legal:** a text screen showing the legal statement, wrapped to the
  LCD's width.
  - The wheel scrolls it one line per step, and the keyboard arrows do
    the same; Menu goes back.
  - The text is a `TEXT` resource of about 5 KB in the app, made from
    `docs/legal/legal-statement.md`, with its Markdown formatting removed
    (T6).
- **Tests:** render checks of both screens, scrolling to the end and
  back, and long words and paragraphs wrapping; manual.
- **Done:** Legal is shown in Geneva 9 (asked for in review: Chicago has
  no 9-point bitmap on System 6), 9 lines at a time; the text resource
  is `resources/legal.r`, made by `tools/make-legal-r.py`.

### 9c: Shuffle
- **What it applies to:** playing from any list (songs, an album, an
  artist, a playlist and so on) builds the queue as follows.
  - **Off:** in list order, as now.
  - **Songs:** in random order. The chosen song plays first, then the
    rest in random order (T3).
  - **Albums:** the albums in random order, each album's songs in track
    order, starting with the chosen song's album (T3).
- **Audiobooks are never shuffled** (S-2). A list played from Audiobooks
  stays in order.
- **The main menu's Shuffle Songs** stays as it is: always the whole
  library, songs shuffled, audiobooks skipped.
- **Changing Shuffle while a song plays** affects the next list played,
  not the queue already playing (T4).
- **Gapless playback** is unaffected, because only the order of the queue
  changes.
- **Tests:** a diagnostic that plays lists with each setting:
  - every song exactly once;
  - the chosen song first;
  - albums kept together in track order;
  - audiobooks in order.

### 9d: Repeat (the most delicate part)
- **Off:** as now: stops after the last song (N4).
- **One:** the song repeats without a gap, by queueing the same song again
  after itself. Next and Previous still move to another song (S-3).
- **All:** after the last song the queue starts again from the first,
  also without a gap. With Shuffle on, the order is reshuffled each time
  round (T5).
- **The technical change:** today each song in the audio stream is tagged
  with its queue position, and the app follows the audio by watching the
  tag increase. With Repeat, the same position comes round again (One),
  or goes back to 0 (All).
  - **Change:** each song handed to the stream gets a fresh sequence
    number, and a small table maps sequence numbers to queue positions.
  - **Where:** this changes how playback follows the audio (`FollowAudio`
    and queueing ahead in `playback.c`), not the audio code.
  - **Behaviour with Repeat Off:** must stay exactly as now.
- **Changing Repeat while playing** takes effect from the next song
  boundary not yet handed to the stream. Because the next song is queued
  as soon as one starts (G3), a change made during the last song may only
  apply after the following one (T7).
- **Tests:** a diagnostic with short songs:
  - Repeat One plays the same song three times gaplessly (no underruns,
    the position resets each time);
  - Repeat All wraps from the last song to the first;
  - Off still stops at the end;
  - Next and Previous with Repeat One;
  - turning Repeat off during playback.

  Plus a manual listening check at 1×.

### 9e: Clicker
- **When it clicks:** turning the wheel (and the keyboard arrows) on any
  screen makes a short click, only while no audio is playing (stopped or
  paused). While a song plays the wheel is silent (8b).
- **The sound:** a very short click, a few milliseconds, made in memory
  (T8). It's played on the app's own sound channel while that channel is
  idle.
- **Risk, checked first with a diagnostic:** 6a-4 found that after the
  channel is quietened, every other start of the looped music fails, so
  starts are verified and retried. A click on the same channel could
  change that pattern. The diagnostic plays clicks, then starts and
  resumes music many times, and checks that every start still succeeds
  within its retries. If it doesn't, the click could use a second
  channel, if Sound Manager 2.0 on the Plus allows one, or the plan comes
  back to you.
- **Tests:** that diagnostic; clicks never sound while playing; Clicker
  Off is silent; manual listening at 1×.

## Not in Stage 9
- Extras (audited later, S-7).
- Sound Check and Main Menu customisation (deferred in 8b).
- On-The-Go (deferred), and using ratings in shuffle (issues log).
- Fast-forward and rewind (N8).

## Decisions (2026-10-07)

- **T1:** defaults: Shuffle Off, Repeat Off, Clicker On.
- **T2:** Select steps through a setting's values in place.
- **T3:** with Shuffle on, the chosen song plays first, then the rest
  shuffled; with Albums, its album first.
- **T4:** changing Shuffle applies to the next list played.
- **T5:** Repeat All with Shuffle reshuffles each time round.
- **T6:** Legal is the statement as plain text, headings on their own
  lines, bold removed.
- **T7:** a change to Repeat during playback may take effect one song
  later (gapless queueing); accepted.
- **T8:** the click is a short synthesized tick.
- **T9:** the order 9a, 9b, 9c, 9d, 9e.
