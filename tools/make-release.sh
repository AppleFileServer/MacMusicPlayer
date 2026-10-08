#!/bin/sh
# Makes MacMusic's release files (1.0 Part D) from the committed source:
# builds it afresh from `git archive`, runs both test scripts, then writes
#
#   MacMusic-<v>.bin              the app as MacBinary ("has bundle" set)
#   MacMusic-<v>.dsk              an 800K floppy: MacMusic and its Read Me
#   MacMusic-<v>-source.zip       the source (git archive)
#   HD50_512 Scratch 1980 MB.hda  with --scratch: a BlueSCSI disk holding
#                                 MacMusic and a library of real songs
#   SHA256SUMS
#
# into ~/MacPlusProjects/releases/<name>/, outside the repository. When
# HEAD is tagged v<version> the name is MacMusic-<version>; otherwise it
# is a candidate for testing, MacMusic-<version>-candidate-<commit>.
#
# Usage: tools/make-release.sh [--scratch WORK] [--template HDA] [--out DIR]
#   --scratch WORK   make the Scratch image from WORK, a tools/library-image
#                    work folder with its songs made (the "songs" step done)
#   --template HDA   the BlueSCSI image whose partition map and driver it
#                    uses (default: the environment archive's
#                    bluescsi-template.hda, else the 1.0 release's image)
#   --out DIR        where to write (default as above)
# Mini vMac must be closed (for the smoke test). The working tree must be
# clean: the release is exactly a commit.

set -eu
cd "$(dirname "$0")/.."
PROJECT=$(pwd)
RETRO68=${RETRO68:-$HOME/Retro68-build}
HFS=$RETRO68/toolchain/bin
SCRATCH=
# X11: only a template's first 96 blocks (partition map, driver) are used.
TEMPLATE=$HOME/MacPlusProjects/environment-1.0/bluescsi-template.hda
[ -f "$TEMPLATE" ] || \
    TEMPLATE="$HOME/MacPlusProjects/releases/MacMusic-1.0/HD50_512 Scratch 1980 MB.hda"
OUT=

while [ $# -gt 0 ]; do
    case $1 in
        --scratch)  SCRATCH=$2; shift 2 ;;
        --template) TEMPLATE=$2; shift 2 ;;
        --out)      OUT=$2; shift 2 ;;
        *) echo "unknown option: $1" >&2; exit 2 ;;
    esac
done

if [ -n "$(git status --porcelain)" ]; then
    echo "The working tree has uncommitted changes: commit them first." >&2
    exit 1
fi
VERSION=$(sed -n 's/^project(MacMusic VERSION \([0-9.]*\)).*/\1/p' CMakeLists.txt)
COMMIT=$(git rev-parse --short HEAD)
if [ "$(git tag --points-at HEAD | grep -x "v$VERSION" || true)" ]; then
    NAME=MacMusic-$VERSION
else
    NAME=MacMusic-$VERSION-candidate-$COMMIT
fi
OUT=${OUT:-$HOME/MacPlusProjects/releases/$NAME}
if [ -e "$OUT" ]; then
    echo "$OUT already exists: remove it or choose another --out." >&2
    exit 1
fi
echo "Making $NAME from $COMMIT into $OUT"

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
mkdir "$WORK/src"
git archive HEAD | tar -x -C "$WORK/src"

echo "== build"
cmake -S "$WORK/src" -B "$WORK/build" \
      -DCMAKE_TOOLCHAIN_FILE="$RETRO68/build-target/cmake/retro68.toolchain.cmake" \
      > "$WORK/cmake.log" 2>&1 || { cat "$WORK/cmake.log"; exit 1; }
cmake --build "$WORK/build" > "$WORK/build.log" 2>&1 \
    || { grep -E "error|warning" "$WORK/build.log"; exit 1; }
if grep -q "warning:" "$WORK/build.log"; then
    grep "warning:" "$WORK/build.log"; echo "the build has warnings" >&2; exit 1
fi
APP=$WORK/build/MacMusic.bin

echo "== host tests"
if ! "$WORK/src/tests/run-host.sh" "$WORK/host" > "$WORK/host.log" 2>&1; then
    cat "$WORK/host.log"; exit 1
fi
tail -1 "$WORK/host.log"
echo "== smoke test"
if ! "$WORK/src/tests/run-smoke.sh" "$WORK/smoke" > "$WORK/smoke.log" 2>&1; then
    cat "$WORK/smoke.log"; exit 1
fi
tail -1 "$WORK/smoke.log"

mkdir -p "$OUT"
cp "$APP" "$OUT/$NAME.bin"

echo "== floppy"
python3 "$WORK/src/tools/make-readme.py" "$WORK/readme.txt"
FLOPPY=$OUT/$NAME.dsk
dd if=/dev/zero of="$FLOPPY" bs=1024 count=800 2>/dev/null
"$HFS/hformat" -l "MacMusic $VERSION" "$FLOPPY" > /dev/null
"$HFS/hmount" "$FLOPPY" > /dev/null
"$HFS/hcopy" -m "$APP" ":MacMusic"
"$HFS/hcopy" -r "$WORK/readme.txt" ":MacMusic Read Me"
"$HFS/hattrib" -t TEXT -c ttxt ":MacMusic Read Me"
"$HFS/hls" -l
"$HFS/humount" > /dev/null

echo "== source"
git archive --format=zip --prefix="MacMusic-$VERSION/" HEAD > "$OUT/$NAME-source.zip"

if [ -n "$SCRATCH" ]; then
    echo "== Scratch image"
    python3 "$WORK/src/tools/library-image/library_image.py" image \
        --work "$SCRATCH" --template "$TEMPLATE" --app "$APP" \
        --hfsutils "$HFS" --out "$OUT/HD50_512 Scratch 1980 MB.hda"
    rm -f "$SCRATCH/volume.dsk"         # the tool's 2 GB intermediate
fi

(cd "$OUT" && shasum -a 256 -- * > SHA256SUMS)
echo "== done: $OUT"
ls -l "$OUT"
