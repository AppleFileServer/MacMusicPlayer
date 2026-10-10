#!/bin/sh
# MacMusic's host tests (1.0 Part B1): builds MacMusic's portable modules
# for this Mac with tests/host's stand-in Toolbox, runs each test program
# and reports. Exit status 0 only if every check passed.
#
# Usage: tests/run-host.sh [build folder]   (default: build-host)

set -u
cd "$(dirname "$0")/.." || exit 2
OUT=${1:-build-host}
mkdir -p "$OUT"

CC=${CC:-cc}
FLAGS="-std=c99 -fpascal-strings -Wall -Wextra -Wno-multichar
       -Wno-unknown-pragmas -Wno-unused-parameter -Itests/host/inc
       -Itests/host -Isrc"
HARNESS="tests/host/macfs.c tests/host/mactoolbox.c tests/host/hosttest.c"
MODULES="src/songfile.c src/library.c src/libcache.c src/libfolder.c src/fileops.c
         src/prefs.c src/settings.c src/shuffle.c"

failed=0
# test name, then any arguments it is run with ("-" for none)
for run in "songfile -" "library -" "playlists -" "marker -" "prefs -" \
           "prefs locked-old" "prefs both" "shuffle -" \
           "sorting -" "cache -"; do
    set -- $run
    name=$1
    exe="$OUT/test_$name"
    if [ ! -x "$exe" ] || [ -n "$(find src tests/host -newer "$exe" \
                                   -name '*.[ch]' | head -1)" ]; then
        # shellcheck disable=SC2086
        if ! $CC $FLAGS -o "$exe" "tests/host/test_$name.c" $HARNESS \
                $MODULES 2> "$OUT/test_$name.log"; then
            echo "test_$name: build failed:"; cat "$OUT/test_$name.log"
            failed=1; continue
        fi
        if [ -s "$OUT/test_$name.log" ]; then     # warnings: shown, not fatal
            echo "test_$name: compiler warnings:"; cat "$OUT/test_$name.log"
        fi
    fi
    if [ "$2" = "-" ]; then "$exe" || failed=1
    else "$exe" "$2" || failed=1
    fi
done

if [ $failed -eq 0 ]; then echo "host tests: all passed"
else echo "host tests: FAILED"
fi
exit $failed
