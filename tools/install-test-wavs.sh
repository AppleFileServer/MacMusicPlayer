#!/bin/sh
# Copies WAV files made by make_test_wavs.py into a "Test WAVs" folder at
# the top of the test disk (created if missing), for Import Song... tests.
# This folder is only a test convenience: the app uses the normal Open
# dialog and does not depend on it.
#
#     tools/install-test-wavs.sh HOSTDIR
#
# All HFS paths are absolute; do not use hcd (see install-test-library.sh).

set -e

PROJECT=$(cd "$(dirname "$0")/.." && pwd)
TOOLS="$HOME/Retro68-build/toolchain/bin"
DISK="$PROJECT/testdisks/MusicTest.dsk"
SRC="$1"

[ -d "$SRC" ] || { echo "usage: $0 HOSTDIR" >&2; exit 1; }

if pgrep -x minivmac >/dev/null; then
    echo "Quit Mini vMac first." >&2
    exit 1
fi

"$TOOLS/hmount" "$DISK" >/dev/null
# Always unmount: hfsutils only writes changes back to the image then.
trap '"$TOOLS/humount" >/dev/null 2>&1 || true' EXIT

ROOT=$("$TOOLS/hpwd")
DEST="${ROOT}Test WAVs"

# hls exits 0 even for a missing path, so look for the name instead.
if ! "$TOOLS/hls" "$ROOT" | grep -qx "Test WAVs"; then
    "$TOOLS/hmkdir" "$DEST"
fi

count=0
for f in "$SRC"/*; do
    [ -f "$f" ] || continue
    name=$(basename "$f")
    "$TOOLS/hdel" "$DEST:$name" 2>/dev/null || true
    "$TOOLS/hcopy" -r "$f" "$DEST:$name"
    # D29: type WAVE, unknown creator; Notes.wav gets an unrelated type,
    # since the app must recognise WAVs by content, not by type.
    case "$name" in
        Notes.wav) "$TOOLS/hattrib" -t TEXT -c ttxt "$DEST:$name" ;;
        *)         "$TOOLS/hattrib" -t WAVE -c '????' "$DEST:$name" ;;
    esac
    count=$((count + 1))
done

"$TOOLS/hls" -l "$DEST"

echo "Installed $count files into Test WAVs."
