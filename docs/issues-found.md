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
| 2026-10-08 | 1.0 | Create New Library: the Save dialog's prompt is cut off, reading "Create new MacMusic" instead of "Create new MacMusic Library in:" (Standard File's Save prompt has room for about 20 characters). | File > Choose Library... > Create New Library... | Low | Fix in 1.0.1 with a shorter prompt, such as "New library in:" (libfolder.c, Folder_Choose) |
