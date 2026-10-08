#!/bin/sh
# Copies a test library made by make_test_library.py into an existing
# library folder at the top of the test disk: "MacMusic Library", or
# "iPod Library" (its name before Fix 2, 2026-10-08).
#
# First let the app create the folder: run tools/run-persistent.sh,
# choose the MusicTest disk and Save "MacMusic Library", then quit.
#
#     tools/install-test-library.sh HOSTDIR
#
# All HFS paths are absolute (volume name first). Do not use hcd here: it
# changes hfsutils' current folder, which persists between commands, and a
# path starting with ":" is relative to it.

set -e

PROJECT=$(cd "$(dirname "$0")/.." && pwd)
TOOLS="${RETRO68:-$HOME/Retro68-build}/toolchain/bin"
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

# hmount leaves the current folder at the volume root, e.g. "MusicTest:".
ROOT=$("$TOOLS/hpwd")
# hls exits 0 even for a missing path, so look for the names instead.
if "$TOOLS/hls" "$ROOT" | grep -qx "MacMusic Library"; then
    NAME="MacMusic Library"
else
    NAME="iPod Library"
fi
LIB="${ROOT}${NAME}"

if ! "$TOOLS/hls" "$ROOT" | grep -qx "$NAME" \
   || ! "$TOOLS/hls" "$LIB" | grep -qx "Playlists"; then
    echo "No \"MacMusic Library\" folder (with Playlists) at the top of the test disk." >&2
    echo "Create it with the app first (File > Choose Library...)." >&2
    exit 1
fi

# Copies one host file to an HFS path, replacing any existing file.
install_file() {
    "$TOOLS/hdel" "$2" 2>/dev/null || true
    "$TOOLS/hcopy" -r "$1" "$2"
    "$TOOLS/hattrib" -t "$3" -c iPdM "$2"
}

songs=0
for f in "$SRC"/*; do
    [ -f "$f" ] || continue
    install_file "$f" "$LIB:$(basename "$f")" iPdS
    songs=$((songs + 1))
done

playlists=0
for f in "$SRC"/Playlists/*; do
    [ -f "$f" ] || continue
    install_file "$f" "$LIB:Playlists:$(basename "$f")" TEXT
    playlists=$((playlists + 1))
done

"$TOOLS/hls" -l "$LIB"

echo "Installed $songs files into $NAME and $playlists into Playlists."
