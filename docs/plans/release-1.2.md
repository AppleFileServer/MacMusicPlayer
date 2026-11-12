# Plan: MacMusic 1.2 (requested as 1.1.1)

Draft for the owner's approval, 2026-11-11. Asked for by the Converter
session for the owner (as "1.1.1"). Its decision codes go into
`docs/decisions.md` when each step is made.

## Decision 0: the version number. DECIDED 2026-11-11: 1.2 (the owner)

`docs/plans/release-1.0.md`, Part E, says 1.0.x releases are "fixes only,
with no change to the file formats". Item 4 changes what a valid song file
is (a second sample rate) and the cache's format (version 2). 1.1 still
opens such a library, but it skips the new songs, with its "n files were not
recognised" alert. By the project's own rule, that makes this release
**1.2**.

The owner chose 1.2. The Converter 1.1 notes say "needs MacMusic 1.2".
Its decision codes are N12-n.

## Steps (each runnable and tested; your check in Mini vMac before each commit)

### A. Housekeeping (item 3): documentation only
1. `docs/outstanding-issues.md`:
   - "Known issues in 1.1": first start and Rescan are fast enough on the
     owner's 4 MB Plus (the owner, 2026-11-11);
   - a 1 MB Plus can't be tested (it's the only Plus, and taking SIMMs out
     isn't wanted); 1.1 Part 0's measurement stands in for it;
   - printing on the ImageWriter is still unchecked.
2. I4: closed once step D is done. It's accepted for 22,050 Hz songs (still
   about 0.9% fast) and fixed for 22,254 Hz songs. It moves to "Resolved".
3. Builds on today's uncommitted notes (the 1.1 test run, the issues log, the
   roadmap), which are committed first.

### B. Song details window: clicks in the box's margin (item 1)
Cause, confirmed in `resources/MacMusic.r`: each EditText item's rect is,
for example, Genre `{118, 96, 134, 342}`. The Dialog Manager draws the frame
3 px outside the rect but takes clicks only inside it. The 3 px band on each
side of the drawn box does nothing. The window's top is at y=56, so the
reported y=173 and y=191 are local y=117 and y=135, both in Genre's margin.

- **N12-1 (recommended): the click is moved into the field.** The song
  window gets its own filter, which calls `Modal_DialogFilter` first. On a
  mouseDown inside a field's drawn box (its rect outset 3 px) but outside the
  rect itself, the filter moves the click's point to the nearest point inside
  the rect and lets ModalDialog handle it as usual. The field is selected and
  the insertion point goes where it would for a click on the text. It needs
  no resource change and no layout change.
- **Not recommended: bigger rects.** The frame is always drawn 3 px outside
  the rect, so the dead band moves with it. The fields are 26 px apart, so
  frames 6 px bigger would overlap.
- **N12-2: only the song details window (DITL 131)** (the owner,
  2026-11-11). It's the only window with a column of fields close together.
- Test: a smoke-test step clicks Genre's margin and types, then checks that
  Genre got the text. If the smoke harness can't post clicks, this is a
  manual check, plus a host test of the point-moving function.

### C. Track order in the "All" lists (item 2)
Today (`Library_ListSongs`, `src/library.c:1152`), songs are sorted by track
only when one album is chosen (D13). Every other list is by title (D14).

- **Question for the owner first:** which route led to the album out of
  order, and was Shuffle on? If it was an album's own screen with Shuffle
  off, that's a separate bug, and it comes before anything below.
- **N12-3, PROVISIONAL (undocumented iPod behaviour): the "All" rows**
  (Artists > artist > All, Albums > All, Genres > genre > All, and so on)
  list songs by album (D14 order), then track, then title.
  **Music > Songs stays by title**, as on the iPod, as do Composers and
  Audiobooks.
- The same order is used for playing (N3 plays rows in screen order) and
  printing, since both call `Library_ListSongs`. The printed lists that go
  through "All" change to match. A merge sort with a key of (album, track,
  title) is used, like N11-23's sort; it isn't an insertion sort over 500
  songs.
- Test: a host test (`test_sorting`): album, then track, then title, and
  songs with no album last.

### D. Accepting 22,254 Hz songs (item 4)
The Converter now makes WAVs at 22,254 Hz, the Plus's rate (its 5a3acc7,
§18.25). The audio code already plays every sample at the hardware rate, so
**playback needs no change**. Only the checks and the time calculations do.

- **N12-4: accepted rates are 22,050 and 22,254 Hz exactly** (byte rate equal
  to the rate, block align 1). 22,255 and other near rates are refused.
  Confirmed by the Converter session (2026-11-11): its WAVs carry exactly
  22254 in both the rate and the byte rate (from `MacMusicFormat`; a real
  conversion made at 5a3acc7 checked), and its own check refuses anything
  else, 22,255 included.
- **N12-5: each song keeps its own rate.** `SongInfo` gets a `sampleRate`,
  read from the song file (offset 18, already in the format) and kept in the
  library's song record.
  - `src/wav.c:241,247`: accept either rate. The refusal message becomes
    "Only mono, 22,050 or 22,254 Hz, 8-bit PCM WAV files…".
  - `src/import.c:119`: duration from the WAV's own rate.
  - `src/songfile.c:49,76`: accept either rate; the expected seconds come
    from the song's rate.
  - **`src/songfile.c:223-225`: the chunk is built with the song's own rate.**
    Today it always writes 22,050, so Edit Song or a rating change would
    quietly re-label a 22,254 song as 22,050. This must be fixed with the
    rest, not later.
  - `src/playback.c:59,239,365`: times and seeking use the current song's
    rate, so gapless mixing of the two rates stays right.
- **N12-6: the cache goes to version 2,** adding each song's rate.
  - 1.1 treats a version-2 cache as unknown, ignores it and rewrites its own.
    As built (step D1): 1.2 still reads 1.1's version 1, its songs all
    22,050 Hz, so upgrading costs no scan; only going back to 1.1 does. The
    rate lives in the entry's four spare bytes; a version 1 cache is kept
    until a scan changes something.
  - 1.0 ignores the cache.
- **N12-7: what older versions do with a 22,254 song** (documented, not
  changed): 1.0 and 1.1 skip it with their "not recognised" alert. The file
  stays in the library, unchanged, and returns when this release opens the
  library again.
- **Export is unchanged:** it restores the original WAV byte for byte, at
  whichever rate it came in.
- **Tools and tests:**
  - `tools/library-image/library_image.py`: a `--rate 22050|22254` option,
    default 22,254 (the Converter's), so test images can hold both;
  - host tests: WAV check (both rates, and 22,255 refused), song file round
    trip at 22,254, Edit Song keeps the rate, cache version 2 round trip,
    seconds;
  - smoke test: a 22,254 Hz song is imported and played; its time and the
    progress bar are right, it plays gaplessly after a 22,050 one, and its
    rate is kept after Edit Song.
- The Transfer Utility's own WAV check is left to its session (as asked).

### E. The manuals (items 4 and 5)
- **User's Guide** (`docs/user-guide.md`, then the TeachText Read Me and
  `docs/manual/`):
  - the song format, line 16: 22,050 or 22,254 Hz; the `afconvert` example
    changed to `UI8@22254`;
  - "Songs play about 0.9% fast": now only for 22,050 Hz songs; songs from
    the Converter 1.1 play at true speed;
  - the cache, and opening in 1.1 (N12-6, N12-7).
- ***MacMusic Utilities*, Chapter 2** (`tools/utilities-manual/make-chapter2.py`):
  - Normalise (14 mentions) is replaced by loudness levelling to −16 LUFS with
    a limiter at −1 dBFS (the Converter's L10), and nothing to set;
  - the format becomes 22,254 Hz, the Plus's own rate;
  - `outline.md` lines 43, 59-60, 72, 77 updated to match;
  - **figures:** any screenshot showing the Normalise setting needs retaking
    from Converter 1.1. These are the owner's screenshots (UB4), so the
    owner is asked for them.
- **The new captures (2026-11-11, Claude's, with the owner's leave):**
  Converter 1.1 (45c037b, built in the scratchpad), each window captured
  alone at 1080 x 1264, in
  `manual-assets-private/utilities-screenshots/taken-2026-11-11/`:
  cv06, cv15 (from a copy in /Applications, so the FFmpeg line reads
  `/Applications/…1.1.app`), cv16, cv17, cv20, cv22, cv23, cv27, cv28,
  cv-reencode, cv-wav-only. The Save To folder is the owner's "Music
  Downloads". The YouTube video is the owner's own "MacMusic" (4:02),
  so the text's "Chill Dreamy" example and cv18's shortened title need
  rewriting. (1.1 still has its "Analysing the sound" stage; it passed
  too quickly to capture.) The Version row and
  cv30 still read 1.0 (cv30 left as it is: the owner, 2026-11-11).
  `make-figures.py` needs these as `whole(36)` entries and new part
  rectangles (the layout moved).
- An update card like 1.1's (MD10), since users would notice the change:
  made, as 1.2 is a version with changes a user would notice.

### F. Release
Version in `CMakeLists.txt`, `CHANGELOG.md`, the issues log, and
`tools/make-release.sh`. Then the manual checklist's new items, in Mini vMac
and on the Plus. Then the tag, followed by the hand-made extras that
RESTART.md lists (the BlueSCSI disk, the PDFs, SHA256SUMS).

## Answers so far (2026-11-11)
1. 1.2.
2. The route and Shuffle: the owner is unsure. N12-3 goes ahead as PROVISIONAL;
   an album's own screen was checked in Mini vMac and is in track order.
3. The click fix: the song details window only (N12-2).
4. Chapter 2's screenshots: Claude takes them, but only after the owner has
   authorised the next commit and push.

## Questions for the owner (as first asked)
1. 1.2 or 1.1.1 (Decision 0)?
2. Item 2: which route led to the album, and was Shuffle on? Is N12-3 (the
   "All" rows by album, then track) the order wanted?
3. N12-2: the click fix in the song window only, or in every dialog?
4. Chapter 2's new screenshots, from Converter 1.1.
