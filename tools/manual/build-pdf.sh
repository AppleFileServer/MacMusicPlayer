#!/bin/sh
# Builds a PDF from one of the manual's HTML files (Phase 4): copies it,
# docs/manual/manual.css, the figures and the fonts into build-manual/,
# then prints it with headless Chrome (tools/manual/print-pdf.py), Paged.js
# laying out the pages.
# The tools live outside the repository (docs/environment.md):
#   MANUAL_TOOLS   default ~/MacPlusProjects/manual-tools
#
# Usage: tools/manual/build-pdf.sh <html file> <out.pdf>

set -eu
cd "$(dirname "$0")/../.."
TOOLS=${MANUAL_TOOLS:-$HOME/MacPlusProjects/manual-tools}
CHROME=$TOOLS/chrome-headless-shell-mac-arm64/chrome-headless-shell
SRC=$1
OUT=$2
B=build-manual
rm -rf "$B"; mkdir -p "$B/fonts" "$B/figures"
cp "$SRC" "$B/page.html"
cp docs/manual/manual.css "$TOOLS/paged.polyfill.js" "$B/"
cp docs/manual/figures/png/*.png "$B/figures/"
cp "$TOOLS"/fonts/built/*.ttf "$B/fonts/"
"$TOOLS/venv/bin/python" tools/manual/print-pdf.py "$CHROME" "$B/page.html" "$OUT"
