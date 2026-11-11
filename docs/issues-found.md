# Issues found

A log of problems noticed while using MacMusic, kept by its user. Add a
row for anything that seems wrong, however small. When an entry is
looked into, it gets an issue number in `docs/outstanding-issues.md`
(I11, I12...), and the fix, if any, is noted here with its version.

How serious: **High** (crashes, loses songs or settings, or stops
playback), **Medium** (wrong, but there's a way round it), **Low** (looks
or feels wrong).

| Date | Version | What happened | Steps to make it happen again | How serious | Issue / fixed in |
|---|---|---|---|---|---|
| 2026-10-08 | 1.0 | Create New Library: the Save dialog's prompt is cut off, reading "Create new MacMusic" instead of "Create new MacMusic Library in:" (Standard File's Save prompt has room for about 20 characters). | File > Choose Library... > Create New Library... | Low | Fixed in 1.1: the prompt is "New library in:" (N11-1) |
| 2026-10-09 | 1.0 | On the real Macintosh Plus, MacMusic seems to go through the whole library every time it is opened (as it does when first started after switching on, or after choosing a new library), where before it didn't seem to. Not yet looked into. | Open MacMusic on the real Plus, quit it, and open it again; watch whether it reads the whole library again. | Medium | As 1.0 was made: it read every song file at each start, and spent most of the time sorting. Fixed in 1.1: a faster sort and grouping, a cache of the song details and an optimised build; on the real Plus a 316-song library starts in about 13 s (H1; N11-2 to N11-10, N11-23, N11-30, N11-32) |
| 2026-10-12 | 1.1 test (2d1694f) | On the real Plus, a plain start (nothing changed in the library, 307 songs) takes 32 s from double-clicking to MacMusic being usable, with no "Loading the library" message; after importing a song from the library's disk, 8 s of the logo, then 20 s with the message. 1.1's figures (12 s, then 7.5 s for 500 songs) came from Mini vMac, whose timings are 2-20 times off the Plus's (found in the Pokémon project). | Start MacMusic 1.1 with the Scratch library; time it to the main menu. | Medium | Fixed in 1.1: the grouping was most of it (N11-32), and the build wasn't optimised (N11-30); now about 13 s, with "Loading the library…" from the start (N11-29), and an import run reads the library once (N11-26) |
| 2026-10-12 | 1.1 test (2d1694f) | When MacMusic starts, its window stays hidden behind the Finder's windows until the library has loaded. Likely: under MultiFinder a program comes to the front only once it calls the event routines, and MacMusic reads its library first. | Start MacMusic under MultiFinder with a Finder window open over where it appears. | Low | Fixed in 1.1 (N11-28): MacMusic calls EventAvail before reading the library; to be checked on the Plus |
| 2026-11-11 | 1.1 | On the real Plus, while a disk is being read the music sounds really odd. Not yet looked into: which disk (floppy or the BlueSCSI), and whether MacMusic or another program was reading it, to be found. The owner asks whether MacMusic's undocumented use of the Sound Manager (I9) could affect floppy reading or writing; if so, the manual should warn about it. | Play a song on the Plus while a disk is read. | Medium | |
