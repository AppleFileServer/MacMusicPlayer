#!/bin/sh
# Copies the built app onto the persistent test disk and starts Mini vMac
# directly, so the prefs file and MacMusic Library survive between runs.
#
# LaunchAPPL cannot be used for this: it boots a fresh temporary disk on
# every launch and discards it afterwards.
#
# Setup (done once): ~/minivmac/disk2.dsk is a symlink to
# testdisks/MusicTest.dsk; disk1.dsk is the System 6.0.7 disk.
#
# In Mini vMac: open the MusicTest disk and double-click MacMusic.

set -e

PROJECT=$(cd "$(dirname "$0")/.." && pwd)
TOOLS="${RETRO68:-$HOME/Retro68-build}/toolchain/bin"
DISK="$PROJECT/testdisks/MusicTest.dsk"
APP_BIN="$PROJECT/build/MacMusic.bin"
MINIVMAC="${MINIVMAC:-$HOME/minivmac}/minivmac.app"

if pgrep -x minivmac >/dev/null; then
    echo "Mini vMac is running. Quit it first: the disk image must not be" >&2
    echo "changed while it is mounted in the emulator." >&2
    exit 1
fi

[ -f "$APP_BIN" ] || { echo "Build the app first: $APP_BIN not found" >&2; exit 1; }
[ -f "$DISK" ]    || { echo "Test disk not found: $DISK" >&2; exit 1; }

"$TOOLS/hmount" "$DISK" >/dev/null
"$TOOLS/hdel" ":MacMusic" 2>/dev/null || true
"$TOOLS/hdel" ":MacMusicPlayer" 2>/dev/null || true   # before the rename
"$TOOLS/hcopy" -m "$APP_BIN" ":MacMusic"
"$TOOLS/hls" -l
"$TOOLS/humount" >/dev/null

# Run the executable directly: "open" on the bundle fails with
# LaunchServices error -10825 on this Mac.
nohup "$MINIVMAC/Contents/MacOS/minivmac" >/dev/null 2>&1 &
