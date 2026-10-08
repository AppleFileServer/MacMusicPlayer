#!/bin/sh
# Checks that everything MacMusic's build, tests and releases need from
# outside the repository is present and working (Part F, F3). Changes
# nothing. Each ✗ is needed and names the section of docs/environment.md
# that says how to get it back; each ! is only needed for some things.
#
# Usage: tools/check-environment.sh
#   RETRO68   the Retro68 build folder   (default ~/Retro68-build)
#   MINIVMAC  the Mini vMac folder       (default ~/minivmac)
#   LAUNCHAPPL_CFG  LaunchAPPL's settings (default ~/.LaunchAPPL.cfg)
# Exit status 0 if nothing is missing (warnings allowed).

cd "$(dirname "$0")/.." || exit 2
RETRO68=${RETRO68:-$HOME/Retro68-build}
MINIVMAC=${MINIVMAC:-$HOME/minivmac}
CFG=${LAUNCHAPPL_CFG:-$HOME/.LaunchAPPL.cfg}
BIN=$RETRO68/toolchain/bin
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
missing=0

ok()   { echo "  ✓ $1"; }
bad()  { echo "  ✗ $1  (docs/environment.md §$2)"; missing=$((missing + 1)); }
warn() { echo "  ! $1  (docs/environment.md §$2)"; }
sha()  { shasum -a 256 "$1" 2>/dev/null | cut -c1-16; }

echo "This Mac's tools"
if command -v cmake > /dev/null; then
    v=$(cmake --version | head -1 | sed 's/[^0-9.]//g')
    case $v in 3.1[5-9]*|3.[2-9]*|[4-9].*) ok "CMake $v" ;;
               *) bad "CMake $v is older than 3.15" 2 ;; esac
else bad "CMake not found" 2; fi
echo 'int main(void) { return "\pok"[0] != 2; }' > "$TMP/t.c"
if cc -fpascal-strings -o "$TMP/t" "$TMP/t.c" 2> /dev/null && "$TMP/t"; then
    ok "the host compiler ($(cc --version | head -1))"
else bad "the host compiler (cc -fpascal-strings) doesn't work" 2; fi
if python3 -c 'import sys; assert sys.version_info >= (3, 8)' 2> /dev/null; then
    ok "Python $(python3 -c 'import platform; print(platform.python_version())')"
else bad "Python 3.8 or later not found" 2; fi
for t in afconvert mdls afinfo; do
    command -v $t > /dev/null || warn "$t not found (only tools/library-image needs it)" 2
done

echo "Retro68 ($RETRO68)"
if [ -x "$BIN/m68k-apple-macos-gcc" ]; then
    printf 'int f(int x) { return x + 1; }\n' > "$TMP/m.c"
    if "$BIN/m68k-apple-macos-gcc" -c -o "$TMP/m.o" "$TMP/m.c" 2> "$TMP/gcc.log"; then
        ok "gcc $("$BIN/m68k-apple-macos-gcc" -dumpversion) compiles"
    else bad "gcc doesn't compile: $(head -1 "$TMP/gcc.log")" 3; fi
else bad "gcc not found in $BIN" 4; fi
printf "data 'TEST' (1) { \$\"00\" };\n" > "$TMP/t.r"
if [ ! -x "$BIN/Rez" ]; then bad "Rez not found" 4
elif (cd "$TMP" && "$BIN/Rez" t.r -o t.bin > rez.log 2>&1); then ok "Rez"
else bad "Rez doesn't start (Homebrew's boost?): $(head -1 "$TMP/rez.log")" 3; fi
dd if=/dev/zero of="$TMP/t.dsk" bs=1024 count=800 2> /dev/null
if [ -x "$BIN/hformat" ] && "$BIN/hformat" -l T "$TMP/t.dsk" > /dev/null 2>&1; then
    "$BIN/humount" > /dev/null 2>&1    # hformat leaves the new disk current
    ok "hfsutils (hformat, hcopy, hmount...)"
else bad "hfsutils missing or not working" 4; fi
if [ ! -x "$BIN/LaunchAPPL" ]; then bad "LaunchAPPL not found" 4
elif ! "$BIN/LaunchAPPL" --help > /dev/null 2>&1; then
    bad "LaunchAPPL doesn't start (Homebrew's boost?)" 3
elif strings "$BIN/LaunchAPPL" | grep -qx -- "-nWa"; then
    bad "LaunchAPPL is not patched (tools/patches/retro68-launchappl.patch)" 4
else ok "LaunchAPPL, patched"; fi
CMAKEFILE=$RETRO68/build-target/cmake/retro68.toolchain.cmake
if [ -f "$CMAKEFILE" ]; then
    root=$(sed -n 's/^set( RETRO68_ROOT "\(.*\)" CACHE.*/\1/p' "$CMAKEFILE")
    if [ "$root" = "$RETRO68/toolchain" ]; then ok "the CMake toolchain file"
    else warn "the toolchain file says RETRO68_ROOT=$root; pass -DRETRO68_ROOT=$RETRO68/toolchain to CMake" 4; fi
else bad "the CMake toolchain file $CMAKEFILE" 4; fi
if command -v brew > /dev/null; then
    pinned=$(brew list --pinned 2> /dev/null | tr '\n' ' ')
    for f in boost gmp mpfr libmpc gettext zstd; do
        case " $pinned " in *" $f "*) ;; *) unpinned="$unpinned $f" ;; esac
    done
    if [ -n "${unpinned:-}" ]; then
        warn "Homebrew libraries not pinned:$unpinned (an upgrade can break the toolchain)" 3
    else ok "Homebrew libraries pinned"; fi
fi

echo "Mini vMac ($MINIVMAC)"
if [ -x "$MINIVMAC/minivmac.app/Contents/MacOS/minivmac" ]; then ok "minivmac.app"
else bad "minivmac.app" 5; fi
case $(sha "$MINIVMAC/vMac.ROM") in
    dd908e2b65772a6b) ok "the Mac Plus ROM" ;;
    "") bad "the Mac Plus ROM (vMac.ROM)" 6 ;;
    *) bad "vMac.ROM isn't the Plus ROM 1.0 was tested with" 6 ;;
esac
if [ -f "$MINIVMAC/System607-hfs.img" ]; then ok "the System 6.0.7 disk"
else bad "the System 6.0.7 disk (System607-hfs.img)" 6; fi
[ "$(sha "$MINIVMAC/autoquit-1.1.1.dsk")" = e5cd4597ac614fd6 ] \
    && ok "the autoquit disk" || bad "the autoquit disk (autoquit-1.1.1.dsk)" 6
for n in 1 2 3; do
    [ -e "$MINIVMAC/disk$n.dsk" ] || bad "disk$n.dsk (the disk links)" 6
done
[ -e "$MINIVMAC/disk1.dsk" ] && [ -e "$MINIVMAC/disk2.dsk" ] && [ -e "$MINIVMAC/disk3.dsk" ] \
    && ok "the disk links"
if [ -f "$CFG" ]; then
    if grep -q "minivmac-dir = $MINIVMAC\$" "$CFG"; then
        ok "LaunchAPPL's settings ($CFG)"
    else warn "$CFG names another Mini vMac folder" 6; fi
else bad "LaunchAPPL's settings ($CFG)" 6; fi

echo "Test disks and release inputs"
for d in MusicTest Transfer; do
    [ -f "testdisks/$d.dsk" ] && ok "testdisks/$d.dsk" || bad "testdisks/$d.dsk" 7
done
template=
for t in "${TEMPLATE:-}" "$HOME/MacPlusProjects/environment-1.0/bluescsi-template.hda" \
         "$HOME/MacPlusProjects/BlueSCSI-backups/Scratch.before-0692d4f.hda" \
         "$HOME/MacPlusProjects/releases/MacMusic-1.0/HD50_512 Scratch 1980 MB.hda"; do
    [ -n "$t" ] && [ -f "$t" ] && { template=$t; break; }
done
[ -n "$template" ] && ok "a BlueSCSI template: $template" \
    || warn "no BlueSCSI template (only Scratch images need it)" 8
[ -f "$HOME/MacPlusProjects/library-work/pick.json" ] && ok "the song work folder" \
    || warn "no ~/MacPlusProjects/library-work (only Scratch images need it)" 8
M=${MANUAL_TOOLS:-$HOME/MacPlusProjects/manual-tools}
[ -x "$M/chrome-headless-shell-mac-arm64/chrome-headless-shell" ] && [ -f "$M/paged.polyfill.js" ] \
    && [ -x "$M/venv/bin/python" ] && ls "$M"/fonts/built/*.ttf > /dev/null 2>&1 \
    && ok "the manual's tools ($M)" \
    || warn "the manual's tools aren't all in $M (only the manual's PDF needs them)" 10
[ -d "$HOME/MacPlusProjects/environment-1.0" ] && ok "the environment archive" \
    || warn "no environment archive in ~/MacPlusProjects/environment-1.0" 1

echo
if [ $missing -eq 0 ]; then echo "environment: ready"; exit 0; fi
echo "environment: $missing missing"; exit 1
