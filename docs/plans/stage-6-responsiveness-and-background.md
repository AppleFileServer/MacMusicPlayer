# Stage 6 plan: responsiveness and playing in the background

Status: proposed, not started (saved while Stage 5c-1 is done first).
Decisions S1-S4 below are still open.

Covers outstanding issues I2 (main loop runs about once a second while
playing), I1 (audio repeats with another program's window in front) and,
optionally, I3 (silence after a long menu hold). See
`docs/outstanding-issues.md`.

Recommended before Stage 7 (Now Playing volume, scrubber and rating),
because those are driven by turning the wheel while a song plays, and the
wheel would feel delayed with a main loop that comes round once a second.

## 6a: find out where the time goes (I2), measuring first

1. A throwaway timing build logs, for each main-loop pass while playing:
   time waiting in `WaitNextEvent`, time in the audio refill (and bytes
   read), time redrawing, and the rest.
2. Measure in Mini vMac and on the real Mac Plus (the once-a-second
   behaviour may be partly an emulator effect). The user plays a song for
   about a minute with the iPod in front, then quits; the log is read
   from the disk image or SD card. The Mac Plus run needs the BlueSCSI
   Scratch image rebuilt first (issue I7), planned before touching the
   card.
3. Fix what the measurements point to, after reporting them:
   - waiting for events (MultiFinder giving the Finder long turns): sleep
     0 while playing, or poll events without yielding;
   - disk reads: smaller reads spread over passes, or interrupt-time
     reads (6c);
   - redraw: draw less, or only what changed.

## 6b: playing in the background (I1, the 5b-1 plan)

1. The app's own `SIZE` resource replaces Retro68's default: can run in
   the background and receives suspend/resume events; memory stays 1 MB.
2. Handle suspend/resume: window drawn inactive/active and redrawn
   correctly; while playing, ask for time every tick in the background.
3. Keep the silencing safety net (R6) working in the background. The 5b-1
   test showed MultiFinder suspends the app's VBL task when switched out
   (57 runs in 2,818 ticks). Measure again with background running; if it
   is still suspended, first try a VBL task record outside the app heap,
   otherwise stop the sound when the app gets no time for longer than the
   ring lasts. Silence, never repeated audio.

## 6c (optional): interrupt-time disk reads (I3)

Keeps the ring full during long menu holds and dialogs. Decide after 6a:
worth it if the reads are part of the slowness, or if long menu holds
must not go silent.

## Testing

- Diagnostics in Mini vMac: timing log; background play (no repeats,
  correct time on return); silencing in the background.
- Manual (user) in Mini vMac: wheel and clicks responsive while playing;
  Finder window in front 30 s or more (music continues, or silence if the
  Finder is busy, never a repeat); a large Finder copy while playing;
  window active/inactive appearance.
- The Mac Plus, with the rebuilt card image.

## Decisions

- S1: do Stage 6 before Stage 7 (recommended)?
- S2: in the background, keep playing (recommended) or pause?
- S3: rebuild the BlueSCSI Scratch image (I7) early in 6a so timing can be
  measured on the real Plus? (Plan presented before touching the card.)
- S4: decide on 6c after 6a's measurements, or include it from the start?
