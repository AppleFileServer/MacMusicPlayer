#!/bin/sh
# MacMusic's emulator smoke test (1.0 Part B2): builds MacMusic with the
# smoke test built in (-DMACMUSIC_SMOKE_TEST=ON, in build-smoke), runs it
# in Mini vMac with LaunchAPPL and checks its log. Exit status 0 only if
# the log ends with "FAILURES: 0" and "DONE". About 1-2 minutes.
#
# Usage: tests/run-smoke.sh [build folder]   (default: build-smoke)

set -u
cd "$(dirname "$0")/.." || exit 2
OUT=${1:-build-smoke}
RETRO68=${RETRO68:-$HOME/Retro68-build}
TIMEOUT=${SMOKE_TIMEOUT:-300}   # seconds; a hang fails instead of waiting

if pgrep -x minivmac > /dev/null; then
    echo "Mini vMac is already running: quit it first."; exit 2
fi
mkdir -p "$OUT"
cmake -S . -B "$OUT" -DMACMUSIC_SMOKE_TEST=ON \
      -DCMAKE_TOOLCHAIN_FILE="$RETRO68/build-target/cmake/retro68.toolchain.cmake" \
      > "$OUT/cmake.log" 2>&1 || { cat "$OUT/cmake.log"; exit 2; }
cmake --build "$OUT" > "$OUT/build.log" 2>&1 \
    || { grep -E "error|warning" "$OUT/build.log"; echo "build failed"; exit 2; }
grep -E "warning:" "$OUT/build.log"

"$RETRO68/toolchain/bin/LaunchAPPL" --timeout "$TIMEOUT" \
    "$OUT/MacMusic.APPL" > "$OUT/smoke.log" 2>&1
status=$?
grep -v '^[.#]*$' "$OUT/smoke.log"

if [ $status -eq 0 ] && grep -qx "FAILURES: 0" "$OUT/smoke.log" \
   && [ "$(grep -v '^[.#]*$' "$OUT/smoke.log" | tail -1)" = "DONE" ]; then
    echo "smoke test: passed"; exit 0
fi
echo "smoke test: FAILED (LaunchAPPL exit $status)"; exit 1
