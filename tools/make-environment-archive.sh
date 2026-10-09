#!/bin/sh
# Makes the environment archive (Part F, F4): everything outside the
# repository that building and testing MacMusic need, as it is now, so
# it can be put back in a year (docs/environment.md). Into
# ~/MacPlusProjects/environment-<version>/ (or --out DIR):
#
#   toolchain.tar.gz        Retro68's built toolchain and its CMake file
#   homebrew-libs.tar.gz    the Homebrew libraries the toolchain loads
#   minivmac.tar.gz         Mini vMac: app, configuration, source, ROM,
#                           System disks, autoquit disk, disk links
#   testdisks.tar.gz        testdisks/MusicTest.dsk and Transfer.dsk
#   converter-tools.tar.gz  the MacMusic Converter Utility's tools (FFmpeg,
#                           yt-dlp, Deno), pinned (its BUILDING.md)
#   MacMusicConverter.bundle  the converter's repository (git bundle, with tags)
#   manual-tools.tar.gz     the manual's tools (headless Chrome, Paged.js,
#                           the fonts, the Python venv); not the owner's
#                           screenshots or the private cover assets (E2, E3)
#   LaunchAPPL.cfg          ~/.LaunchAPPL.cfg
#   bluescsi-template.hda   a BlueSCSI image's partition map and driver
#   MacMusic.bundle         the whole repository (git bundle, with tags)
#   restore.sh, README.md, MANIFEST.txt, SHA256SUMS
#
# For the owner's use only: the ROM, the System disk and the driver are
# Apple's copyright.
#
# Usage: tools/make-environment-archive.sh [--out DIR]
#   RETRO68, MINIVMAC   where they are (defaults ~/Retro68-build, ~/minivmac)
#   TEMPLATE            a BlueSCSI image to take the template from
#   MANUAL_TOOLS        the manual's tools (default ~/MacPlusProjects/manual-tools)
#   CONVERTER, CONVERTER_TOOLS   the converter's repository and tools (defaults
#                       ~/MacPlusProjects/MacMusicConverter, ~/MacPlusProjects/converter-tools)

set -eu
cd "$(dirname "$0")/.."
PROJECT=$(pwd)
RETRO68=${RETRO68:-$HOME/Retro68-build}
MINIVMAC=${MINIVMAC:-$HOME/minivmac}
MANUAL_TOOLS=${MANUAL_TOOLS:-$HOME/MacPlusProjects/manual-tools}
CONVERTER=${CONVERTER:-$HOME/MacPlusProjects/MacMusicConverter}
CONVERTER_TOOLS=${CONVERTER_TOOLS:-$HOME/MacPlusProjects/converter-tools}
VERSION=$(sed -n 's/^project(MacMusic VERSION \([0-9.]*\)).*/\1/p' CMakeLists.txt)
OUT=$HOME/MacPlusProjects/environment-$VERSION
[ "${1:-}" = "--out" ] && OUT=$2

if [ -e "$OUT" ]; then
    echo "$OUT already exists: remove it or choose another --out." >&2
    exit 1
fi
if pgrep -x minivmac > /dev/null; then
    echo "Mini vMac is running: quit it first (its disks must not be changing)." >&2
    exit 1
fi
tools/check-environment.sh > /dev/null || {
    echo "tools/check-environment.sh reports something missing: fix that first." >&2
    exit 1
}
mkdir -p "$OUT"
echo "Making the environment archive in $OUT"

echo "== the toolchain"
tar -czf "$OUT/toolchain.tar.gz" -C "$RETRO68" toolchain build-target/cmake

echo "== the Homebrew libraries"
LIBS=$(mktemp -d)
trap 'rm -rf "$LIBS"' EXIT
otool -L "$RETRO68"/toolchain/bin/* \
         "$RETRO68"/toolchain/libexec/gcc/m68k-apple-macos/*/* 2> /dev/null \
    | grep -o '/opt/homebrew/[^ ]*' | sort -u > "$LIBS/names"
mkdir "$LIBS/files"
while read -r name; do          # the path the toolchain asks for, the real file
    real=$(python3 -c 'import os, sys; print(os.path.realpath(sys.argv[1]))' "$name")
    cp "$real" "$LIBS/files/$(basename "$name")"
    echo "$name $real" >> "$LIBS/files/PATHS"
done < "$LIBS/names"
tar -czf "$OUT/homebrew-libs.tar.gz" -C "$LIBS/files" .

echo "== Mini vMac"
tar -czf "$OUT/minivmac.tar.gz" -C "$(dirname "$MINIVMAC")" \
    --exclude "$(basename "$MINIVMAC")/build" "$(basename "$MINIVMAC")"

echo "== the test disks, settings and template"
tar -czf "$OUT/testdisks.tar.gz" testdisks/MusicTest.dsk testdisks/Transfer.dsk
cp "$HOME/.LaunchAPPL.cfg" "$OUT/LaunchAPPL.cfg"
TEMPLATE=${TEMPLATE:-$HOME/MacPlusProjects/releases/MacMusic-1.0/HD50_512 Scratch 1980 MB.hda}
dd if="$TEMPLATE" of="$OUT/bluescsi-template.hda" bs=512 count=96 2> /dev/null

echo "== the manual's tools"
tar -czf "$OUT/manual-tools.tar.gz" -C "$(dirname "$MANUAL_TOOLS")" \
    --exclude '__pycache__' "$(basename "$MANUAL_TOOLS")"

echo "== the converter: its tools and its repository"
tar -czf "$OUT/converter-tools.tar.gz" -C "$(dirname "$CONVERTER_TOOLS")" "$(basename "$CONVERTER_TOOLS")"
git -C "$CONVERTER" bundle create "$OUT/MacMusicConverter.bundle" --all 2> /dev/null
git -C "$CONVERTER" bundle verify "$OUT/MacMusicConverter.bundle" > /dev/null 2>&1

echo "== the repository"
git bundle create "$OUT/MacMusic.bundle" --all 2> /dev/null
git bundle verify "$OUT/MacMusic.bundle" > /dev/null 2>&1

cp tools/environment-restore.sh "$OUT/restore.sh"
cp tools/environment-archive-readme.md "$OUT/README.md"
{
    echo "MacMusic environment archive, made $(date '+%Y-%m-%d %H:%M')"
    echo "from commit $(git rev-parse --short HEAD) ($(git describe --tags --always))"
    echo
    tools/check-environment.sh
    echo
    echo "Homebrew libraries:"
    cat "$LIBS/files/PATHS"
} > "$OUT/MANIFEST.txt"

(cd "$OUT" && shasum -a 256 -- * > SHA256SUMS)
echo "== done"
ls -l "$OUT"
du -sh "$OUT"
