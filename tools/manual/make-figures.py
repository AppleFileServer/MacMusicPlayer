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
  crop:     full | window | lcd | box | menu | x,y,w,h   (Mac screen pixels)
            window: the MacMusic window, found in the shot (its two
                    straight sides, 206 pixels apart), the desktop
                    cleared from its rounded corners;
            lcd:    the window's screen;
            box:    the largest dialog or alert box (a rectangle of black
                    lines below the menu bar);
            menu:   a pulled-down menu, with its title in the menu bar
                    and its shadow
  options:  zoom=N        enlargement (default 4)
            ink=RRGGBB    the colour black pixels print in (default 000000)
            paper=RRGGBB  the colour white pixels print in (default ffffff)
            erase=x,y,w,h paint an area white; repeatable
            nopointer     paint out the arrow pointer wherever it shows
                          (it is found exactly, from its own bitmap)
The screenshot settings (where the screen starts, the zoom) are on a line
    screen <x0> <y0> <zoom>   (default: 384 198 4, the owner's Mini vMac)."""

import os, sys
from PIL import Image

FULL = (0, 0, 512, 342)
WINDOW_WIDTH = 206          # the MacMusic window, frame included
LCD = (16, 24, 174, 142)    # its screen, from the window's top-left corner


# The arrow pointer, 16 x 16 (the Toolbox's arrow cursor): its image and
# its mask, one row a number, the leftmost pixel the highest bit.
ARROW = (0x0000, 0x4000, 0x6000, 0x7000, 0x7800, 0x7C00, 0x7E00, 0x7F00,
         0x7F80, 0x7C00, 0x6C00, 0x4600, 0x0600, 0x0300, 0x0300, 0x0000)
ARROW_MASK = (0xC000, 0xE000, 0xF000, 0xF800, 0xFC00, 0xFE00, 0xFF00, 0xFF80,
              0xFFC0, 0xFFE0, 0xFE00, 0xEF00, 0xCF00, 0x8780, 0x0780, 0x0380)


def remove_pointer(screen):
    """Whitens the arrow pointer wherever it shows on the screen: every
    place where the pixels under its mask are exactly its image."""
    px = screen.load()
    cells = [(dx, dy, bool(ARROW[dy] & (0x8000 >> dx)))
             for dy in range(16) for dx in range(16) if ARROW_MASK[dy] & (0x8000 >> dx)]
    found = 0
    for y in range(342 - 15):
        for x in range(512 - 15):
            if all((px[x + dx, y + dy] == 0) == black for dx, dy, black in cells):
                for dx, dy, _ in cells:
                    px[x + dx, y + dy] = 255
                found += 1
    return found


def find_window(screen):
    """(x, y, w, h) of the MacMusic window: its left and right frames are
    unbroken black lines 206 pixels apart; of every such pair, the one
    whose shorter line is longest (a menu's edge or the LCD's frame can
    happen to be 206 pixels from something too, but runs shorter)."""
    px = screen.load()

    def longest(x):
        best = run = 0
        for y in range(40, 339):
            run = run + 1 if px[x, y] == 0 else 0
            best = max(best, run)
        return best

    runs = [longest(x) for x in range(512)]
    best, where = 0, None
    for x in range(0, 512 - WINDOW_WIDTH + 1):
        score = min(runs[x], runs[x + WINDOW_WIDTH - 1])
        if score >= 100 and score > best:
            best, where = score, x
    if where is None:
        sys.exit("no MacMusic window found")
    x = where
    bottom = 341
    for y in range(341, 300, -1):
        if sum(px[i, y] == 0 for i in range(x + 10, x + WINDOW_WIDTH - 10)) > 150:
            bottom = y
            break
    return (x, 20, WINDOW_WIDTH, bottom - 20 + 1)


def find_rects(screen, min_width=60, min_height=30):
    """Rectangles of black lines (dialog boxes, alerts, menus), largest
    first: a top and a bottom line with matching ends (give or take a
    shadow's two pixels) and black sides between them."""
    px = screen.load()
    lines = []
    for y in range(19, 342):
        x = 0
        while x < 512:
            if px[x, y] == 0:
                start = x
                while x < 512 and px[x, y] == 0:
                    x += 1
                if x - start >= min_width:
                    lines.append((y, start, x - 1))
            x += 1
    found = []
    for i, (ty, tx0, tx1) in enumerate(lines):
        for by, bx0, bx1 in lines[i + 1:]:
            if by - ty < min_height or abs(bx0 - tx0) > 2 or abs(bx1 - tx1) > 2:
                continue
            x0, x1 = min(tx0, bx0), max(tx1, bx1)
            left = sum(px[x0, y] == 0 or px[x0 + 1, y] == 0 for y in range(ty, by + 1))
            right = sum(px[x1, y] == 0 or px[x1 - 1, y] == 0 for y in range(ty, by + 1))
            if min(left, right) >= 0.9 * (by - ty + 1):
                found.append((x0, ty, x1 - x0 + 1, by - ty + 1))
    found.sort(key=lambda r: -r[2] * r[3])
    return found


def find_menu(screen):
    """(left, right, bottom) of a pulled-down menu. Its top edge is the
    menu bar's own bottom line, so it is found from its title, which is
    highlighted (white on black) in the menu bar: the menu's left frame
    drops from the title's left end to the bottom; the right frame is the
    first column to its right that is black almost all the way down (a
    highlighted item, a bar across, can't be)."""
    px = screen.load()
    title = None
    for x in range(0, 500):
        if all(px[x + i, 17] == 0 for i in range(12)) and px[x - 1, 17] != 0:
            title = x
            break
    if title is None:
        sys.exit("no highlighted menu title found")
    for left in range(title - 3, title + 4):
        y = 20
        while y < 341 and px[left, y] == 0:
            y += 1
        if y - 20 >= 20:
            break
    else:
        sys.exit("no menu frame found below the title")
    bottom = y - 1
    rows = range(22, bottom - 2)
    for right in range(left + 30, 512):              # the right frame: a full column
        if sum(px[right, r] == 0 for r in rows) >= 0.9 * len(rows):
            return left, right, bottom
    sys.exit("no menu right frame found")


def menu_bar_end(screen):
    """The x just past the menu bar's last title (before the icons at its
    right end)."""
    px = screen.load()
    end = 0
    for x in range(0, 440):
        if any(px[x, y] == 0 for y in range(2, 18)):
            end = x
    return end + 1


def clear_around_menu(part, screen, cx):
    """Whitens whatever shows beside and below a menu crop, below the
    menu bar, except the menu itself and its shadow (two pixels to the
    right and below, starting two pixels in); and in the menu bar, the
    end of the title before the highlighted one."""
    left, right, bottom = find_menu(screen)
    px = part.load()
    w, h = part.size
    for y in range(0, 19):
        for x in range(0, max(0, left - 1 - cx)):
            px[x, y] = 255
    for y in range(20, h):
        for x in range(w):
            sx = cx + x
            inside = left <= sx <= right and y <= bottom
            shadow = (right < sx <= right + 2 and y >= 22) or \
                     (bottom < y <= bottom + 2 and left + 2 <= sx <= right + 2)
            if not (inside or shadow):
                px[x, y] = 255


def crop_box(screen, kind):
    if kind == "full":
        return FULL
    if kind == "window":
        return find_window(screen)
    if kind == "lcd":
        x, y, w, h = find_window(screen)
        return (x + LCD[0], y + LCD[1], LCD[2], LCD[3])
    rects = find_rects(screen)
    if kind == "box":
        rects = [r for r in rects if r[1] > 22]
        if not rects:
            sys.exit("no dialog box found")
        return rects[0]                              # exactly the box's frame
    if kind == "menu":
        x0, x1, bottom = find_menu(screen)
        x1 = max(x1 + 3, menu_bar_end(screen) + 4)      # the whole menu bar's titles
        left = max(0, x0 - 6)
        return (left, 0, min(x1 - left + 1, 512 - left), bottom + 3)
    return tuple(map(int, kind.split(",")))


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
        opts = {"zoom": "4", "ink": "000000", "paper": "ffffff"}
        erase = []
        for w in words[3:]:
            if w == "nopointer":
                opts["nopointer"] = True
                continue
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
        if opts.get("nopointer") and not remove_pointer(screen):
            sys.exit("%s: no pointer found" % name)
        cx, cy, cw, ch = crop_box(screen, crop)
        part = screen.crop((cx, cy, cx + cw, cy + ch))
        if crop == "window":
            clear_outside(part, cx, cy)
        if crop == "menu":
            clear_around_menu(part, screen, cx)
        zoom = int(opts["zoom"])
        part = part.resize((cw * zoom, ch * zoom), Image.NEAREST)
        ink = tuple(int(opts["ink"][i:i + 2], 16) for i in (0, 2, 4))
        paper = tuple(int(opts["paper"][i:i + 2], 16) for i in (0, 2, 4))
        rgb = Image.new("RGB", part.size, paper)
        rgb.paste(Image.new("RGB", part.size, ink), mask=part.point(lambda v: 255 - v))
        rgb.save(os.path.join(out, name + ".png"), optimize=True)
        print("%-14s %-7s %dx%d" % (name, crop, cw, ch))


main()
