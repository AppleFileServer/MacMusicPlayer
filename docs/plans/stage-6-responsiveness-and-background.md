# Stage 6 plan: responsiveness and playing in the background

Status: revised 2026-10-06 after Stage 6a-4. 6a-1 to 6a-4 are done; the
remaining steps below replace the earlier 6b/6c plan, which assumed the
tick-based play position and the VBL silencing task that 6a-4 removed.
All decisions (S1-S8) are made; see the end.

## Where Stage 6 stands

| Step | Result | Commit |
|---|---|---|
| 6a-1 measure | Main loop ~58 passes/s while playing (fast emulator speed); the earlier once-a-second reading was a logging artefact | `bfdbfdb` (docs) |
| 6a-2 `SIZE` | Runs in the background under MultiFinder; 512 KB / 384 KB partition | `82c7079` |
| 6a-3 redraw | Now Playing redraw ~11x faster (was ~56 ticks a second at Plus speed) | `f34544c` |
| 6a-4 position | Position from the Sound Manager's read pointer; tick model and VBL silencing removed; underrun = stop, restart from the end of the valid audio | `99601ce` |

What 6a-4 changed for the rest of the plan:

- There is no interrupt-time code any more. The old 6b-1 "background
  safeguard" (moving the VBL task out of the app heap, or a Time Manager
  task) existed only to keep R6 silencing running while switched out; it
  is no longer needed.
- The old 6b-2 ("pause instead of skipping when starved", S5) is done:
  the 6a-4 underrun recovery restarts from the end of the valid audio,
  which is heard as a pause.
- What starvation now means: whenever the main loop does not run for
  longer than the ring (about 6.6 s), the Sound Manager replays stale
  ring audio until the app runs again, then the underrun is detected and
  playback recovers. Accepted for programs we don't control (a busy
  front program). But the main loop is also blocked inside the app's own
  modal loops, which we do control (see 6b-2).
- Tested in Mini vMac at 1x: playback continued through Finder Duplicate
  and Empty Wastebasket with the time in step.

## Remaining goals and success criteria

- **Switching (I1):** bringing another program to the front and back
  behaves like a well-mannered System 6 program: the window looks
  inactive in the background and active in front, redraws correctly,
  and playback is unaffected.
- **The app's own modal loops (I3, in part):** holding a menu, dragging
  the window, holding the wheel, or leaving any of the app's dialogs or
  Standard File open for a minute does not interrupt playback.
- **Responsiveness at Plus speed:** confirmed at 1x: a main-loop pass
  normally takes 1-2 ticks while playing, and wheel and buttons respond
  promptly.
- **No interrupt-time code is added.**

## 6b-1: switching in and out (verify first, then fix what is wrong)

1. Observe current behaviour (manual, Mini vMac at 1x): switch to the
   Finder and back while playing and while stopped; check the title bar
   highlighting, whether the iPod window redraws correctly after being
   covered and uncovered, desk accessories from the Apple menu, the Edit
   menu with a DA, Standard File and alerts while background running is
   enabled.
2. Fix only what is wrong. Likely, small:
   - handle suspend/resume (`osEvt`) and activate events in the main
     loop; with `needsActivateOnFGSwitch` MultiFinder already sends
     activate events, so this may only need the window's appearance
     handled (the app has no controls or text fields);
   - if anything in the window depends on being active, redraw on
     resume.
3. No scrap handling is needed (the app doesn't use the clipboard).

## 6b-2: keep the ring fed inside the app's own modal loops (S6, approved)

The main loop doesn't run while the app is inside these, so playback
starves after about 6.6 s. Each has a supported, non-interrupt way to
run our own code repeatedly while it waits; calling `Audio_Service`
there keeps the ring full. This runs in the app's normal context, so
File Manager calls are allowed.

| Blocking loop | Where | Hook |
|---|---|---|
| Wheel drag | `Input_TrackWheel` (`while (StillDown())`) | Our own loop: call the service directly |
| Menus held down | `MenuSelect` | `MenuHook` low-memory global (called repeatedly while a menu is down) |
| Window drag | `DragWindow` | `DragHook` low-memory global (called repeatedly during the drag) |
| Our dialogs | 2 x `ModalDialog`, 9 x `Alert` | Filter procedure (called for every event, including null events) |
| Standard File | 4 x `SFGetFile`/`SFPutFile` | The dialog hook (called repeatedly) |

- One function, e.g. `Playback_ServiceAudio()`, does only the refill and
  underrun check (`Audio_Service` plus queueing the next song); no
  drawing, since those loops own the screen.
- Hooks are installed only around the call and restored afterwards
  (low-memory hooks are shared with the system).
- Import and export keep pausing playback (A5); their dialogs would be
  covered by the same mechanism, but changing A5 is not part of this
  step.
- Testing: hold a menu for 30 s, drag the window around for 30 s, hold
  the wheel for 30 s, leave each kind of dialog open for 30 s, all while
  playing: no interruption. A diagnostic counts underruns (expect none).

## 6b-3: a larger stack (S7, approved: 24 KB; issue I10)

- At startup, before anything is allocated, lower `ApplLimit` to reserve
  a 24 KB stack instead of 8 KB. Costs 16 KB of the 512 KB partition.
- Testing: the stack-measurement method from 6a-2 (throwaway build)
  confirms the new size; normal regression.

## 6b-4: confirm responsiveness at Plus speed

- A throwaway timing build (as 6a-1, logging to a closed folder) run
  once in Mini vMac at 1x while playing: passes per second, wait and
  redraw times. Expected after 6a-3: ~1-2 ticks per pass. Only if not:
  investigate before Stage 7.

## Deferred until the BlueSCSI card is available (S3)

- Rebuild the Scratch image (I7, plan first).
- On the real Plus: the 6a-4 read-pointer check passes (I9); the stress
  test (Finder Duplicate/Empty Wastebasket); 6b-2's modal-loop tests;
  responsiveness.

## Not in Stage 6 (S4 revised)

- **6c, interrupt-time disk refill: dropped from Stage 6 (S4).**
  Its purpose was to survive starvation longer than the ring. After
  6a-4 that is an accepted limitation for programs we don't control,
  6b-2 covers the app's own modal loops without interrupt code, and on
  the Plus interrupt-time reads are done by the processor inside the
  interrupt (no DMA), with a risk of disturbing the sound that would
  need real-hardware measurement. Kept in the issues log (I3) as a
  possible later item.
- A cosmetic point, not planned: during total starvation the elapsed
  time may run on with the stale audio and step back when the underrun
  is detected. Only visible when the app gets no time, i.e. when it
  can't redraw anyway.

## Order

1. 6b-1 (verify switching, fix what's wrong).
2. 6b-2 (modal-loop servicing, S6).
3. 6b-3 (24 KB stack, S7).
4. 6b-4 (1x responsiveness check).
5. Hardware checks when the card is available.

Each step is built, tested (diagnostics plus manual Mini vMac at 1x) and
committed separately after approval.

## Decisions

### Decided earlier (stand)
- **S1:** Stage 6 before Stage 7.
- **S2:** keep playing in the background; starvation behaves as a pause
  (now delivered by the 6a-4 underrun recovery).
- **S3:** no real hardware for now; Plus checks deferred.
- **S5:** pause, not skip (done in 6a-4).

### Decided 2026-10-06
- **S4 (revised):** interrupt-time refill (6c) is dropped from Stage 6;
  kept in the issues log (I3) as a possible later item.
- **S6:** yes: keep the ring fed inside the app's own modal loops (6b-2).
- **S7:** yes: reserve a 24 KB stack (6b-3).
- **S8:** the order above: 6b-1, 6b-2, 6b-3, then 6b-4.
