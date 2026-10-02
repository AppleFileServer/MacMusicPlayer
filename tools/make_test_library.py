#!/usr/bin/env python3
"""Writes a test iPod Library (song file format version 1, see
src/song_format.h) to a host folder, including deliberately invalid
files, so the scanner can be tested before the importer exists.

Songs are short silence (8-bit unsigned PCM centre value 0x80) to keep
the test disk small.

    tools/make_test_library.py OUTDIR

OUTDIR gets the song files plus a Playlists folder holding empty playlist
files (their contents are Stage 3c). tools/install-test-library.sh copies
it onto the test disk.
"""

import os
import struct
import sys

RATE = 22050


def pstr(s):
    b = s.encode("mac_roman")
    if len(b) > 63:
        raise ValueError("string over 63 bytes: %r" % s)
    return bytes([len(b)]) + b


def song(song_id, title, artist="", album="", genre="", composer="",
         track=0, seconds=2, audiobook=False, **override):
    """Returns the bytes of one song file; override fields to corrupt it."""
    audio = bytes([0x80]) * (RATE * seconds)
    strings = b"".join(pstr(s) for s in (title, artist, album, genre, composer))
    header_len = 38 + len(strings)
    fields = dict(magic=b"IPOD", version=1, header_len=header_len, id=song_id,
                  flags=1 if audiobook else 0, track=track,
                  duration=(len(audio) + RATE // 2) // RATE,
                  encoding=b"PCMu", rate=RATE, bits=8, channels=1,
                  audio_len=len(audio), strings=strings, audio=audio)
    fields.update(override)
    header = (fields["magic"]
              + struct.pack(">HIIHHI", fields["version"], fields["header_len"],
                            fields["id"], fields["flags"], fields["track"],
                            fields["duration"])
              + fields["encoding"]
              + struct.pack(">IHHI", fields["rate"], fields["bits"],
                            fields["channels"], fields["audio_len"])
              + fields["strings"])
    return header + fields["audio"]


VALID = [
    # id, title, artist, album, genre, composer, track
    (1, "Original Spin", "Scribe Tribe", "In Session", "Electronic", "", 1),
    (2, "My Voices Like You", "Scribe Tribe", "In Session", "Electronic", "", 2),
    (3, "You’re a Pill", "Scribe Tribe", "In Session", "Electronic", "", 3),
    (4, "Berceuse Op. 57", "Clara Nordahl", "Night Pieces", "Classical", "Frédéric Chopin", 2),
    (5, "Nocturne Op. 9 No. 2", "Clara Nordahl", "Night Pieces", "Classical", "Frédéric Chopin", 1),
    (6, "The Long Way Home", "The Lanterns", "Harbour Lights", "Rock", "", 1),
    (7, "Lighthouse", "The Lanterns", "Harbour Lights", "Rock", "", 2),
    (8, "Last Stop", "The Lanterns", "Northern Line", "Rock", "", 1),
    (9, "Midnight Express", "The Blue Hours", "Late Trains", "Jazz", "", 1),
    (10, "The Extraordinarily Long Song Title That Never Ends", "Northbound",
     "Signals", "Rock", "", 4),
    # D20: missing fields stay reachable through "All".
    (11, "No Album Here", "Kestrel", "", "Electronic", "", 0),
    (12, "No Artist Here", "", "Signals", "Rock", "", 2),
    (13, "Nothing But A Title", "", "", "", "", 0),
]

AUDIOBOOK = (14, "A Short History of Clocks", "R. Ellison",
             "A Short History of Clocks", "Audiobook", "", 1)

PLAYLISTS = ["Road Trip", "Quiet", "Reference"]


def write(outdir, name, data, kind="song"):
    with open(os.path.join(outdir, name), "wb") as f:
        f.write(data)
    print("%-8s %-10s %7d bytes" % (kind, name, len(data)))


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    outdir = sys.argv[1]
    os.makedirs(os.path.join(outdir, "Playlists"), exist_ok=True)

    for (i, title, artist, album, genre, composer, track) in VALID:
        write(outdir, "IPOD%04d" % i,
              song(i, title, artist, album, genre, composer, track))

    i, title, artist, album, genre, composer, track = AUDIOBOOK
    write(outdir, "IPOD%04d" % i,
          song(i, title, artist, album, genre, composer, track, audiobook=True),
          "audiobk")

    # Invalid files: each must be skipped, and must not stop the scan.
    bad = {
        "BAD-magic": song(90, "Bad Magic", magic=b"WAVE"),
        "BAD-version": song(91, "Bad Version", version=2),
        "BAD-rate": song(92, "Bad Rate", rate=44100),
        "BAD-stereo": song(93, "Bad Stereo", channels=2),
        "BAD-bits": song(94, "Bad Bits", bits=16),
        "BAD-encoding": song(95, "Bad Encoding", encoding=b"PCMs"),
        "BAD-string": song(96, "x", strings=bytes([64]) + b"y" * 64 + b"\0\0\0\0"),
        "BAD-truncated": song(97, "Truncated")[:2000],
        "BAD-duration": song(98, "Bad Duration", duration=99),
        "BAD-id0": song(0, "Zero ID"),
        "ZZ-dupID": song(1, "Duplicate of ID 1"),  # sorts after IPOD0001
        "BAD-empty": b"",
        "Read Me": b"Not a song.\r",
    }
    for name, data in bad.items():
        write(outdir, name, data, "invalid")

    for name in PLAYLISTS:
        write(os.path.join(outdir, "Playlists"), name, b"", "playlist")

    print("\n%d valid songs, 1 audiobook, %d invalid files, %d playlists"
          % (len(VALID), len(bad), len(PLAYLISTS)))


if __name__ == "__main__":
    main()
