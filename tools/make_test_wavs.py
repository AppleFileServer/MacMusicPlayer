#!/usr/bin/env python3
"""Writes test WAV files for Stage 3B (Import Song...) to a host folder:
valid files the importer must accept, and invalid ones it must reject
with a specific message.

    tools/make_test_wavs.py OUTDIR

tools/install-test-wavs.sh copies them into "Test WAVs" on the test disk.
The app itself knows nothing about that folder: files are picked with
the normal Open dialog.
"""

import math
import os
import struct
import sys

RATE = 22050


def chunk(cid, data):
    pad = b"\0" if len(data) % 2 else b""
    return cid + struct.pack("<I", len(data)) + data + pad


def fmt(tag=1, channels=1, rate=RATE, bits=8, block=None, byterate=None):
    block = block if block is not None else channels * bits // 8
    byterate = byterate if byterate is not None else rate * block
    return chunk(b"fmt ", struct.pack("<HHIIHH", tag, channels, rate,
                                      byterate, block, bits))


def tone(seconds, freq=440.0, channels=1, bytes_per_sample=1):
    n = int(RATE * seconds)
    if bytes_per_sample == 1 and channels == 1:
        return bytes(int(128 + 90 * math.sin(2 * math.pi * freq * i / RATE))
                     for i in range(n))
    # Other shapes only need the right size and some content.
    return bytes((i * 7) & 0xFF for i in range(n * channels * bytes_per_sample))


def info(**tags):
    body = b"INFO"
    for key, value in tags.items():
        body += chunk(key.encode("ascii"), value.encode("latin-1") + b"\0")
    return chunk(b"LIST", body)


def riff(*chunks, form=b"WAVE", size_override=None):
    body = form + b"".join(chunks)
    size = size_override if size_override is not None else len(body)
    return b"RIFF" + struct.pack("<I", size) + body


TONE = tone(3)

VALID = {
    # The plain case: Title should default to "Tone".
    "Tone.wav": riff(fmt(), chunk(b"data", TONE)),
    # Same audio as Tone.wav: importing both triggers the duplicate warning.
    "Tone copy.wav": riff(fmt(), chunk(b"data", TONE)),
    # INFO tags fill the fields (Latin-1 e-acute becomes Mac Roman).
    "Tagged.wav": riff(info(INAM="Café Song", IART="Test Artist",
                            IPRD="Test Album", IGNR="Jazz", ITRK="7"),
                       fmt(), chunk(b"data", tone(2, 660))),
    # No metadata; upper-case extension is still removed from the title.
    "Clip.WAV": riff(fmt(), chunk(b"data", tone(1, 880))),
    # Chunks in an unusual order, plus an odd-length chunk needing padding.
    "Odd Order.wav": riff(chunk(b"junk", b"abc"), chunk(b"data", tone(1, 330)),
                          fmt()),
    # A RIFF size field that is wrong (D27: tolerated).
    "Bad RIFF Size.wav": riff(fmt(), chunk(b"data", tone(1, 550)),
                              size_override=12345),
}

INVALID = {
    "Stereo.wav": riff(fmt(channels=2), chunk(b"data", tone(1, channels=2))),
    "44100 Hz.wav": riff(fmt(rate=44100), chunk(b"data", tone(1))),
    "16-bit.wav": riff(fmt(bits=16), chunk(b"data", tone(1, bytes_per_sample=2))),
    "Compressed.wav": riff(fmt(tag=2), chunk(b"data", tone(1))),
    "Float.wav": riff(fmt(tag=3, bits=32), chunk(b"data", tone(1, bytes_per_sample=4))),
    "Extensible.wav": riff(fmt(tag=0xFFFE), chunk(b"data", tone(1))),
    "Inconsistent.wav": riff(fmt(byterate=11025), chunk(b"data", tone(1))),
    "No Data.wav": riff(fmt()),
    "No Format.wav": riff(chunk(b"data", tone(1))),
    "Empty Audio.wav": riff(fmt(), chunk(b"data", b"")),
    "Truncated.wav": riff(fmt(), chunk(b"data", tone(1)))[:4000],
    "Not WAVE.avi": riff(fmt(), chunk(b"data", tone(1)), form=b"AVI "),
    "Sound.aiff": b"FORM" + struct.pack(">I", 4) + b"AIFF",
    "Notes.wav": b"This is a text file named .wav\r",
}


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    outdir = sys.argv[1]
    os.makedirs(outdir, exist_ok=True)
    for group, files in (("valid", VALID), ("invalid", INVALID)):
        for name, data in files.items():
            with open(os.path.join(outdir, name), "wb") as f:
                f.write(data)
            print("%-8s %-20s %7d bytes" % (group, name, len(data)))
    print("\n%d valid, %d invalid" % (len(VALID), len(INVALID)))


if __name__ == "__main__":
    main()
