#!/bin/sh
# Puts MacMusic's build and test environment back from this archive
# (made by tools/make-environment-archive.sh; see README.md). Anything
# already in place is left alone and reported, so it is safe to run on a
# Mac where only some pieces are missing.
#
# Usage: ./restore.sh [--project DIR] [--replace-libs]
#   --project DIR    the MacMusic repository (default
#                    ~/MacPlusProjects/MacMusicPlayer); cloned from the
#                    bundle if it isn't there
#   --replace-libs   also put the archived Homebrew libraries over newer
#                    ones (only if the toolchain no longer starts after a
#                    Homebrew upgrade; the replaced files are kept as
#                    *.before-restore)
#   RETRO68, MINIVMAC, LAUNCHAPPL_CFG, MANUAL_TOOLS   where to restore
#                    to (defaults ~/Retro68-build, ~/minivmac,
#                    ~/.LaunchAPPL.cfg, ~/MacPlusProjects/manual-tools)

set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
PROJECT=$HOME/MacPlusProjects/MacMusicPlayer
REPLACE_LIBS=
while [ $# -gt 0 ]; do
    case $1 in
        --project) PROJECT=$2; shift 2 ;;
        --replace-libs) REPLACE_LIBS=1; shift ;;
        *) echo "unknown option: $1" >&2; exit 2 ;;
    esac
done
RETRO68=${RETRO68:-$HOME/Retro68-build}
MINIVMAC=${MINIVMAC:-$HOME/minivmac}
CFG=${LAUNCHAPPL_CFG:-$HOME/.LaunchAPPL.cfg}
MANUAL_TOOLS=${MANUAL_TOOLS:-$HOME/MacPlusProjects/manual-tools}
ORIGINAL_RETRO68=/Users/jack/Retro68-build      # where it was archived from
ORIGINAL_PROJECT=/Users/jack/MacPlusProjects/MacMusicPlayer
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

kept()  { echo "  - $1: already there, left alone"; }
done_() { echo "  + $1"; }

echo "Checking the archive"
(cd "$HERE" && shasum -a 256 -c SHA256SUMS --quiet) || {
    echo "The archive's files don't match SHA256SUMS: it is damaged." >&2
    exit 1
}

echo "Restoring"
if [ -e "$PROJECT/.git" ]; then kept "the repository ($PROJECT)"
else
    mkdir -p "$(dirname "$PROJECT")"
    git clone -q "$HERE/MacMusic.bundle" "$PROJECT"
    done_ "the repository, cloned into $PROJECT"
fi

if [ -e "$RETRO68/toolchain" ]; then kept "the toolchain ($RETRO68/toolchain)"
else
    mkdir -p "$RETRO68"
    tar -xzf "$HERE/toolchain.tar.gz" -C "$RETRO68"
    if [ "$RETRO68" != "$ORIGINAL_RETRO68" ]; then
        # The toolchain file records where the toolchain is.
        for f in "$RETRO68/build-target/cmake/retro68.toolchain.cmake" \
                 "$RETRO68/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake"; do
            sed -i '' "s|$ORIGINAL_RETRO68/toolchain|$RETRO68/toolchain|" "$f"
        done
    fi
    done_ "the toolchain, into $RETRO68"
fi

mkdir "$TMP/libs"
tar -xzf "$HERE/homebrew-libs.tar.gz" -C "$TMP/libs"
while read -r name real; do
    file=$TMP/libs/$(basename "$name")
    if [ -e "$name" ] && [ -z "$REPLACE_LIBS" ]; then continue; fi
    if [ -e "$name" ]; then
        cmp -s "$file" "$name" && continue
        cp -p "$name" "$name.before-restore"
        cp "$file" "$name"
        done_ "Homebrew library $name (replaced)"
    else
        mkdir -p "$(dirname "$real")"
        cp "$file" "$real"
        [ -e "$name" ] || { mkdir -p "$(dirname "$name")"; ln -s "$real" "$name"; }
        done_ "Homebrew library $name"
    fi
done < "$TMP/libs/PATHS"

if [ -e "$MINIVMAC" ]; then kept "Mini vMac ($MINIVMAC)"
else
    tar -xzf "$HERE/minivmac.tar.gz" -C "$TMP"
    mkdir -p "$(dirname "$MINIVMAC")"
    mv "$TMP/minivmac" "$MINIVMAC"
    if [ "$PROJECT" != "$ORIGINAL_PROJECT" ]; then
        ln -sf "$PROJECT/testdisks/MusicTest.dsk" "$MINIVMAC/disk2.dsk"
        ln -sf "$PROJECT/testdisks/Transfer.dsk" "$MINIVMAC/disk3.dsk"
    fi
    done_ "Mini vMac, the ROM and the System disks, into $MINIVMAC"
fi

if [ -e "$PROJECT/testdisks/MusicTest.dsk" ]; then kept "the test disks"
else
    tar -xzf "$HERE/testdisks.tar.gz" -C "$PROJECT"
    done_ "the test disks, into $PROJECT/testdisks"
fi

if [ -e "$CFG" ]; then kept "LaunchAPPL's settings ($CFG)"
else
    sed -e "s|^minivmac-dir = .*|minivmac-dir = $MINIVMAC|" \
        -e "s|^system-image = .*|system-image = $MINIVMAC/System607-hfs.img|" \
        "$HERE/LaunchAPPL.cfg" > "$CFG"
    done_ "LaunchAPPL's settings, $CFG"
fi

if [ -e "$MANUAL_TOOLS" ]; then kept "the manual's tools ($MANUAL_TOOLS)"
elif [ -e "$HERE/manual-tools.tar.gz" ]; then
    tar -xzf "$HERE/manual-tools.tar.gz" -C "$TMP"
    mkdir -p "$(dirname "$MANUAL_TOOLS")"
    mv "$TMP/manual-tools" "$MANUAL_TOOLS"
    done_ "the manual's tools, into $MANUAL_TOOLS"
    # E1: the venv points at the Python it was made with; if that has
    # changed, say how to make it again.
    if ! "$MANUAL_TOOLS/venv/bin/python" -c 'import fontTools, PIL, websocket' 2> /dev/null; then
        echo "  ! the manual's Python venv doesn't run here (Python has changed). Remake it:"
        echo "      rm -rf $MANUAL_TOOLS/venv && python3 -m venv $MANUAL_TOOLS/venv \\"
        echo "        && $MANUAL_TOOLS/venv/bin/pip install fonttools pillow websocket-client"
    fi
fi

echo
echo "Checking"
RETRO68=$RETRO68 MINIVMAC=$MINIVMAC LAUNCHAPPL_CFG=$CFG \
    "$PROJECT/tools/check-environment.sh"
