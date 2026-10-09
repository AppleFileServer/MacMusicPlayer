#!/bin/sh
# Builds a PDF from one of MacMusic Utilities' HTML files: copies it, the
# User's Guide's styles (docs/manual/manual.css) and this book's
# (docs/utilities-manual/utilities.css), its figures and the fonts into
# build-utilities/, then prints it with headless Chrome, as the User's
# Guide's tools/manual/build-pdf.sh does.
#   MANUAL_TOOLS      default ~/MacPlusProjects/manual-tools
# The owner's own edition's figures (UB1; the openers' wallpaper scenes,
# Apple's artwork), if there, replace the plain ones in build-utilities/
# only, never in the repository:
#   UTILITIES_ASSETS  default ~/MacPlusProjects/manual-assets-private/utilities
#
# Usage: tools/utilities-manual/build-pdf.sh <html file> <out.pdf> [<anchors.json>]

set -eu
cd "$(dirname "$0")/../.."
TOOLS=${MANUAL_TOOLS:-$HOME/MacPlusProjects/manual-tools}
CHROME=$TOOLS/chrome-headless-shell-mac-arm64/chrome-headless-shell
B=build-utilities
rm -rf "$B/fonts" "$B/figures" "$B/page.html"; mkdir -p "$B/fonts" "$B/figures"
cp "$1" "$B/page.html"
cp docs/manual/manual.css docs/utilities-manual/utilities.css "$TOOLS/paged.polyfill.js" "$B/"
cp docs/utilities-manual/figures/png/*.png "$B/figures/"
ASSETS=${UTILITIES_ASSETS:-$HOME/MacPlusProjects/manual-assets-private/utilities}
[ -d "$ASSETS/figures" ] && cp "$ASSETS"/figures/*.png "$B/figures/"
cp "$TOOLS"/fonts/built/*.ttf "$B/fonts/"
"$TOOLS/venv/bin/python" tools/manual/print-pdf.py "$CHROME" "$B/page.html" "$2" ${3:+"$3"}
