#!/usr/bin/env python3
"""Writes a test iPod Library (song file format version 2, see
src/song_format.h) to a host folder, including deliberately invalid
files, so the scanner can be tested on the host and on the Mac.

A version 2 song file is a WAV file with an "iPod" chunk added at the
end. Songs are short silence (8-bit unsigned PCM centre value 0x80) to
keep the test disk small.

    tools/make_test_library.py OUTDIR

OUTDIR gets the song files plus a Playlists folder of playlist files in
the Stage 3C format: one song per line, "<song ID><tab><title>", where
only the ID counts (P1). tools/install-test-library.sh copies it onto
the test disk.
"""

import os
import struct
import sys

RATE = 22050
SAMPLE = 8192


def pstr(s):
    b = s.encode("mac_roman")
    if len(b) > 63:
        raise ValueError("string over 63 bytes: %r" % s)
    return bytes([len(b)]) + b


def fingerprint(audio):
    """D31: running sums over the start, middle and end 8 KB samples."""
    if len(audio) <= 3 * SAMPLE:
        parts = [audio]
    else:
        mid = (len(audio) - SAMPLE) // 2
        parts = [audio[:SAMPLE], audio[mid:mid + SAMPLE], audio[-SAMPLE:]]
    a = b = 0
    for part in parts:
        for byte in part:
            a = (a + byte) & 0xFFFFFFFF
            b = (b + a) & 0xFFFFFFFF
    return ((b << 16) ^ a) & 0xFFFFFFFF


def wav(audio, odd_tail=False):
    """A plain WAV: fmt and data chunks (data padded to even length).
    odd_tail adds an odd-length final chunk with no pad byte, so the
    file length is odd."""
    fmt = struct.pack("<HHIIHH", 1, 1, RATE, RATE, 1, 8)
    body = b"WAVE" + b"fmt " + struct.pack("<I", len(fmt)) + fmt
    body += b"data" + struct.pack("<I", len(audio)) + audio
    if len(audio) & 1:
        body += b"\0"
    if odd_tail:
        body += b"note" + struct.pack("<I", 3) + b"abc"
    return b"RIFF" + struct.pack("<I", len(body)) + body


def song(song_id, title, artist="", album="", genre="", composer="",
         track=0, seconds=2, audiobook=False, file_name=None, odd_tail=False,
         **override):
    """Returns the bytes of one song file; override fields to corrupt it."""
    audio = bytes([0x80]) * (RATE * seconds)
    original = wav(audio, odd_tail)
    if file_name is None:
        file_name = title[:27] + ".wav"
    chunk_start = len(original) + (len(original) & 1)
    strings = b"".join(pstr(x) for x in
                       (title, artist, album, genre, composer, file_name))
    f = dict(chunk_id=b"iPod", version=1, id=song_id,
             flags=1 if audiobook else 0, track=track,
             duration=(len(audio) + RATE // 2) // RATE,
             encoding=b"PCMu", rate=RATE, bits=8, channels=1,
             audio_offset=44, audio_len=len(audio),
             fingerprint=fingerprint(audio), orig_len=len(original),
             orig_riff=len(original) - 8, orig_type=b"WAVE",
             orig_creator=b"????", strings=strings, chunk_start=chunk_start,
             end_mark=b"iPod", riff_size=None, data_id=b"data")
    f.update(override)
    payload = (struct.pack(">HIHHI", f["version"], f["id"], f["flags"],
                           f["track"], f["duration"])
               + f["encoding"]
               + struct.pack(">IHHIIIII", f["rate"], f["bits"], f["channels"],
                             f["audio_offset"], f["audio_len"],
                             f["fingerprint"], f["orig_len"], f["orig_riff"])
               + f["orig_type"] + f["orig_creator"] + f["strings"])
    if len(payload) & 1:
        payload += b"\0"
    payload += struct.pack(">I", f["chunk_start"]) + f["end_mark"]
    data = bytearray(original + b"\0" * (chunk_start - len(original))
                     + f["chunk_id"] + struct.pack("<I", len(payload))
                     + payload)
    riff = f["riff_size"] if f["riff_size"] is not None else len(data) - 8
    data[4:8] = struct.pack("<I", riff)
    data[36:40] = f["data_id"]
    return bytes(data)


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

# Playlist contents. Each exercises the reader's tolerance (P1, P8):
#   Quiet:     CR endings; unknown ID 999 (hidden on screen, kept in the
#              file); a blank line; lines not starting with a digit
#              (ignored); a stale title for ID 9 (the current title shows).
#   Road Trip: LF endings and no line ending after the last line.
#   Reference: CRLF endings, IDs with no title at all.
PLAYLISTS = {
    "Quiet": b"5\tNocturne Op. 9 No. 2\r4\tBerceuse Op. 57\r"
             b"999\tA Song That Is Not In The Library\r\r"
             b"# not a song line\r  7\tleading space, ignored\r"
             b"9\tAn Old Title For Midnight Express\r",
    "Road Trip": b"6\tThe Long Way Home\n10\tThe Extraordinarily Long Song "
                 b"Title That Never Ends\n11\tNo Album Here",
    "Reference": b"4\r\n12\r\n13\r\n",
}


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

    # A valid song whose original file length was odd (pad byte before
    # the chunk).
    write(outdir, "IPOD0015",
          song(15, "Odd Length Original", "Kestrel", odd_tail=True))

    # Invalid files: each must be skipped, and must not stop the scan.
    plain = wav(bytes([0x80]) * RATE)
    bad = {
        "BAD-chunkid": song(90, "Bad Chunk ID", chunk_id=b"iPoe"),
        "BAD-version": song(91, "Bad Version", version=2),
        "BAD-rate": song(92, "Bad Rate", rate=44100),
        "BAD-stereo": song(93, "Bad Stereo", channels=2),
        "BAD-bits": song(94, "Bad Bits", bits=16),
        "BAD-encoding": song(95, "Bad Encoding", encoding=b"PCMs"),
        "BAD-string": song(96, "x", strings=bytes([64]) + b"y" * 64 + b"\0" * 5),
        "BAD-truncated": song(97, "Truncated")[:2000],
        "BAD-duration": song(98, "Bad Duration", duration=99),
        "BAD-id0": song(0, "Zero ID"),
        "BAD-riffsize": song(80, "Bad RIFF Size", riff_size=1000),
        "BAD-endmark": song(81, "Bad End Mark", end_mark=b"iPoX"),
        "BAD-chunkstart": song(82, "Bad Chunk Start", chunk_start=100),
        "BAD-origlen": song(83, "Bad Original Length", orig_len=50),
        "BAD-audiolen": song(84, "Audio Past End", audio_len=10 ** 6),
        "BAD-datahdr": song(85, "No Data Header", data_id=b"dat_"),
        "BAD-noname": song(86, "No File Name", file_name=""),
        "BAD-plainwav": plain,
        "ZZ-dupID": song(1, "Duplicate of ID 1"),  # sorts after IPOD0001
        "BAD-empty": b"",
        "Read Me": b"Not a song.\r",
    }
    for name, data in bad.items():
        write(outdir, name, data, "invalid")

    for name, data in PLAYLISTS.items():
        write(os.path.join(outdir, "Playlists"), name, data, "playlist")

    print("\n%d valid songs, 1 audiobook, %d invalid files, %d playlists"
          % (len(VALID) + 1, len(bad), len(PLAYLISTS)))


if __name__ == "__main__":
    main()
