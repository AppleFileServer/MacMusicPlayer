#!/usr/bin/env python3
"""Makes the MacMusic manual's figures (Phase 4/5, decision F5) from the
owner's Mini vMac screenshots, which are kept outside the repository.

Each screenshot shows the Mac Plus's 512 x 342 one-bit screen at an exact
whole-number zoom, so the screen is recovered pixel for pixel (each
block's centre sampled), then cropped (figures.md: F, W, L, M, D) and
written as a PNG enlarged by whole pixels, so it stays sharp in print.

Usage (with Pillow; see docs/environment.md):
    make-figures.py <figures.txt> <out folder>

figures.txt has one figure per line (# starts a comment):
    <name> <screenshot file> <crop> [options]
  crop:     full | window | lcd | x,y,w,h   (in Mac screen pixels)
  options:  zoom=N        enlargement (default 4)
            ink=RRGGBB    the colour black pixels print in (default 000000)
            erase=x,y,w,h paint an area white (the pointer, say); repeatable
The screenshot settings (where the screen starts, the zoom) are on a line
    screen <x0> <y0> <zoom>   (default: 384 198 4, the owner's Mini vMac)."""

import os, sys
from PIL import Image

CROPS = {
    "full": (0, 0, 512, 342),
    "window": (153, 20, 206, 320),   # the MacMusic window, frame included, where the owner keeps it
    "lcd": (169, 44, 174, 142),      # its screen
}


def mac_screen(path, x0, y0, k):
    """The emulated screen, one value per Mac pixel: 0 black, 255 white."""
    shot = Image.open(path).convert("L")
    screen = Image.new("L", (512, 342), 255)
    px, out = shot.load(), screen.load()
    for j in range(342):
        for i in range(512):
            out[i, j] = 0 if px[x0 + i * k + k // 2, y0 + j * k + k // 2] < 128 else 255
    return screen


def clear_outside(part, cx, cy):
    """Whitens the desktop around a window crop (the rounded corners, the
    title bar's ends). The desktop is a checkerboard, black where the
    screen's x + y is even; the window's frame is found on each row as
    the first black pixel out of step with it, from each side, taken
    with the black pixels joined to it. A straight frame line falls in
    step on every other row, so each row also takes its neighbours'
    edges. Everything outside the edges is made white."""
    px = part.load()
    w, h = part.size

    def desktop(x, y):
        return (px[x, y] == 0) == ((cx + x + cy + y) % 2 == 0)

    def edge(y, xs):
        for x in xs:
            if px[x, y] == 0 and not desktop(x, y):
                step = -1 if xs.step > 0 else 1
                while 0 <= x + step < w and px[x + step, y] == 0:
                    x += step
                return x
        return None

    left = [edge(y, range(0, w)) for y in range(h)]
    right = [edge(y, range(w - 1, -1, -1)) for y in range(h)]
    for y in range(h):
        near = [y] if y in (0, h - 1) else [y - 1, y, y + 1]
        ls = [left[n] for n in near if left[n] is not None]
        rs = [right[n] for n in near if right[n] is not None]
        lo = min(ls) if ls else w
        hi = max(rs) if rs else -1
        for x in range(w):
            if x < lo or x > hi:
                px[x, y] = 255


def main():
    spec, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    x0, y0, k = 384, 198, 4
    cache = {}
    for line in open(spec, encoding="utf-8"):
        line = line.split("#", 1)[0].strip()
        if not line:
            continue
        words = line.split()
        if words[0] == "screen":
            x0, y0, k = map(int, words[1:4])
            continue
        name, source, crop = words[0], os.path.expanduser(words[1].replace("%20", " ")), words[2]
        opts = {"zoom": "4", "ink": "000000"}
        erase = []
        for w in words[3:]:
            key, value = w.split("=", 1)
            if key == "erase":
                erase.append(tuple(map(int, value.split(","))))
            else:
                opts[key] = value
        if source not in cache:
            cache[source] = mac_screen(source, x0, y0, k)
        screen = cache[source].copy()
        for (ex, ey, ew, eh) in erase:
            screen.paste(255, (ex, ey, ex + ew, ey + eh))
        cx, cy, cw, ch = CROPS[crop] if crop in CROPS else tuple(map(int, crop.split(",")))
        part = screen.crop((cx, cy, cx + cw, cy + ch))
        if crop == "window":
            clear_outside(part, cx, cy)
        zoom = int(opts["zoom"])
        part = part.resize((cw * zoom, ch * zoom), Image.NEAREST)
        ink = tuple(int(opts["ink"][i:i + 2], 16) for i in (0, 2, 4))
        rgb = Image.new("RGB", part.size, (255, 255, 255))
        rgb.paste(Image.new("RGB", part.size, ink), mask=part.point(lambda v: 255 - v))
        rgb.save(os.path.join(out, name + ".png"), optimize=True)
        print("%-14s %-7s %dx%d" % (name, crop, cw, ch))


main()
