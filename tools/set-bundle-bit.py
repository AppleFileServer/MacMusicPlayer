#!/usr/bin/env python3
"""Sets the "has bundle" Finder flag in a MacBinary file (Fix 1, K2).

The System 6 Finder shows an application's own icons (its BNDL, FREF and
ICN# resources) only if this flag is set, and Retro68's Rez has no option
for it. The "inited" flag is cleared too, so the Finder reads the bundle
afresh when the file is copied to a disk. Run after each build on
build/MacMusic.bin, which tools/run-persistent.sh and tools/library-image
install from; the build does this itself (CMakeLists.txt).

Usage: set-bundle-bit.py FILE.bin
"""

import sys

HAS_BUNDLE = 0x20       # high byte of fdFlags: 0x2000
INITED = 0x01           # high byte of fdFlags: 0x0100

def crc16(data):
    """MacBinary II's header CRC: CRC-16/CCITT, as XMODEM."""
    crc = 0
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc

def main(path):
    with open(path, 'r+b') as f:
        header = bytearray(f.read(128))
        if len(header) < 128 or header[0] != 0 or header[74] != 0 \
                or not 1 <= header[1] <= 63:
            sys.exit('%s: not a MacBinary file' % path)
        if crc16(header[:124]) != int.from_bytes(header[124:126], 'big'):
            sys.exit('%s: the MacBinary header\'s CRC is wrong' % path)
        header[73] = (header[73] | HAS_BUNDLE) & ~INITED & 0xFF
        header[124:126] = crc16(header[:124]).to_bytes(2, 'big')
        f.seek(0)
        f.write(header)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    main(sys.argv[1])
