#!/usr/bin/env python3
"""Makes the figures of *MacMusic Utilities* from the owner's macOS
screenshots (UB4; docs/utilities-manual/style-addendum.md):

- each is cropped to its window, dialog, alert or menu, by the
  rectangles recorded below (measured from the outline macOS draws
  round each one);
- everything outside the rounded frame is painted the page's white, so
  nothing of the desktop shows in the corners (the owner's note,
  2026-10-09); a menu keeps its name's highlight in the menu bar too;
- a grey outline, 2 pixels wide, is drawn just inside the frame: a white
  window needs an edge on a white page, and macOS's corners are smooth
  curves rather than circular arcs, so it also covers the slivers of
  desktop a circular mask leaves where each curve meets the sides;
- the result is greyscale, in docs/utilities-manual/figures/png.

The screenshots stay private, in
~/MacPlusProjects/manual-assets-private/utilities-screenshots
(or UTILITIES_SCREENSHOTS), never in the repository.

Each figure is checked: a band just inside its frame must hold no
desktop colours (a frame measured too big lets the desktop in).

Usage (with Pillow, as the User's Guide's tools):
  make-figures.py              all the figures
  make-figures.py --sheet DIR  also a review sheet of each figure's corners
"""
import glob, os, sys
from PIL import Image, ImageChops, ImageDraw, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "..", "docs", "utilities-manual", "figures", "png")
SHOTS = os.environ.get("UTILITIES_SCREENSHOTS", os.path.expanduser(
    "~/MacPlusProjects/manual-assets-private/utilities-screenshots"))

# The frames, in the screenshots' pixels (2816 x 1762, two to a point):
# left, top, right, bottom (inclusive), and the corner radius.
CONVERTER_A = ((720, 262, 1799, 1525), 36)     # the Converter's window, first position
CONVERTER_B = ((854, 270, 1933, 1533), 36)     # moved, from cv11 on
OPEN_PANEL = ((528, 262, 2287, 1157), 36)      # the Open dialog
ALERT = ((1134, 664, 1653, 1139), 56)          # "... already exists"
ABOUT = ((1124, 344, 1691, 725), 36)           # the About box
APP_MENU = ((67, 6, 489, 53), ((80, 62, 715, 435), 28))      # its name, then the menu
WINDOW_MENU = ((658, 6, 815, 53), ((660, 62, 1199, 579), 28))

# Code: (screenshot, kind, frame). The codes are the outline's
# (docs/utilities-manual/outline.md).
FIGURES = {
    "cv06": ("converter/cv06", "window", CONVERTER_A),
    "cv07": ("converter/cv07", "window", CONVERTER_A),
    "cv08": ("converter/cv08", "window", OPEN_PANEL),
    "cv09": ("converter/cv09", "window", OPEN_PANEL),
    "cv10": ("converter/cv10", "window", OPEN_PANEL),
    **{"cv%02d" % n: ("converter/cv%02d" % n, "window", CONVERTER_B)
       for n in list(range(11, 23)) + list(range(24, 29))},
    "cv23": ("converter/cv23", "window", ALERT),
    "cv29": ("converter/cv29", "menu", APP_MENU),
    "cv30": ("converter/cv30", "window", ABOUT),
    "cv31": ("converter/cv31", "menu", APP_MENU),
    "cv32": ("converter/cv32", "menu", APP_MENU),
    "cv34": ("converter/cv34", "menu", WINDOW_MENU),
}

SCALE = 4       # the masks are drawn this much larger, then reduced: smooth curves
# macOS's corners aren't circular arcs but smooth ("continuous") curves
# that start bending about 1.2 radii from the corner. This is the shape,
# measured from the Converter's window (cv12, its four corners alike):
# how far in from the side the window starts, row by row from the top,
# for a radius of 36. Other radii scale it (the alert's 56 fits).
CORNER_36 = [43.5, 26.2, 22, 19, 17, 15, 14, 12, 11, 10, 9, 8, 7, 7, 6, 5, 5, 4, 4,
             3, 3, 3, 2, 2, 2, 2, 1.5] + [1] * 15 + [0.8, 0.2, 0]

OUTLINE = 2     # the outline's width, in the screenshots' pixels
GREY = 150      # and its tone (macOS's own outline is about this)


def screenshot(name):
    found = glob.glob(os.path.join(SHOTS, name + " *.png"))
    if len(found) != 1:
        sys.exit("%s: expected one screenshot, found %d" % (name, len(found)))
    return Image.open(found[0]).convert("RGB")


def inset(radius, y):
    """How far in from the side the frame starts, `y` pixels from its top
    or bottom, for a corner of `radius`."""
    k = radius / 36
    u = y / k
    i = int(u)
    if i >= len(CORNER_36) - 1:
        return 0.0
    f = u - i
    return k * (CORNER_36[i] * (1 - f) + CORNER_36[i + 1] * f)


def outline_of(box, radius, style, grow=0.0):
    """The frame as a polygon (pixel edges: the box spans l..r+1), or a
    circle-cornered rectangle for a capsule; `grow` < 0 shrinks it."""
    l, t, r, b = box
    l, t, r, b = l - grow, t - grow, r + 1 + grow, b + 1 + grow
    radius = max(1.0, radius + grow)
    if style == "capsule":
        return ("rounded", (l, t, r, b), radius)
    extent = len(CORNER_36) * radius / 36
    ys = [i / 4 for i in range(int(extent * 4) + 1)]
    pts = [(l + inset(radius, y), t + y) for y in reversed(ys)]                # top-left, upwards
    pts += [(r - inset(radius, y), t + y) for y in ys]                         # top-right, downwards
    pts += [(r - inset(radius, y), b - y) for y in reversed(ys)]               # bottom-right, downwards
    pts += [(l + inset(radius, y), b - y) for y in ys]                         # bottom-left, upwards
    return ("polygon", pts, None)


def draw(d, shape, fill):
    kind, geometry, radius = shape
    if kind == "rounded":
        l, t, r, b = geometry
        d.rounded_rectangle((l * SCALE, t * SCALE, r * SCALE - 1, b * SCALE - 1),
                            radius=radius * SCALE, fill=fill)
    else:
        d.polygon([(x * SCALE, y * SCALE) for x, y in geometry], fill=fill)


def mask(size, shapes, outline=False):
    """White where the figure is (or, with `outline`, where its outline
    is), in the crop's pixels."""
    w, h = size
    m = Image.new("L", (w * SCALE, h * SCALE), 0)
    d = ImageDraw.Draw(m)
    for box, radius, style in shapes:
        draw(d, outline_of(box, radius, style), 255)
    if outline:     # less the same shapes shrunk by OUTLINE: the ring
        d = ImageDraw.Draw(m)
        for box, radius, style in shapes:
            draw(d, outline_of(box, radius, style, grow=-OUTLINE), 0)
    return m.resize((w, h), Image.LANCZOS)


def shapes_of(kind, frame):
    """The crop's box in the screenshot, and its shapes relative to it."""
    if kind == "window":
        (l, t, r, b), radius = frame
        return (l, t, r, b), [((0, 0, r - l, b - t), radius, "macos")]
    name, ((pl, pt, pr, pb), radius) = frame          # a menu: its name, then the panel
    nl, nt, nr, nb = name
    l, t, r, b = min(nl, pl), nt, max(nr, pr), pb
    capsule = (nb - nt + 1) // 2
    return (l, t, r, b), [((nl - l, nt - t, nr - l, nb - t), capsule, "capsule"),
                          ((pl - l, pt - t, pr - l, pb - t), radius, "macos")]


def desktop_in_band(crop, m, band=4):
    """Strongly coloured pixels just inside the outline (the next `band`
    pixels): the desktop let in by a frame measured too big."""
    inside = m.point(lambda v: 255 if v > 250 else 0)
    under = inside.filter(ImageFilter.MinFilter(2 * (OUTLINE + 1) + 1))
    core = inside.filter(ImageFilter.MinFilter(2 * (OUTLINE + 1 + band) + 1))
    ring = ImageChops.subtract(under, core)
    r, g, b = crop.split()
    colour = ImageChops.subtract(ImageChops.lighter(ImageChops.lighter(r, g), b),
                                 ImageChops.darker(ImageChops.darker(r, g), b))
    strong = colour.point(lambda v: 255 if v > 60 else 0)
    return ImageChops.multiply(strong, ring).histogram()[255]


def make(code, entry):
    name, kind, frame = entry
    im = screenshot(name)
    box, shapes = shapes_of(kind, frame)
    crop = im.crop((box[0], box[1], box[2] + 1, box[3] + 1))
    m = mask(crop.size, shapes)
    figure = Image.composite(crop, Image.new("RGB", crop.size, "white"), m)
    figure = Image.composite(Image.new("RGB", crop.size, (GREY,) * 3), figure,
                             mask(crop.size, shapes, outline=True))
    # Menus are translucent, tinted by the desktop behind them: not checked.
    leaked = desktop_in_band(crop, m) if kind == "window" else None
    figure.convert("L").save(os.path.join(OUT, code + ".png"), optimize=True)
    return figure, leaked


def corners(figure, n=48, zoom=4):
    """The four corners, enlarged, side by side: for reviewing the crops."""
    w, h = figure.size
    sheet = Image.new("RGB", (4 * n * zoom + 30, n * zoom), (200, 0, 0))
    for i, (x, y) in enumerate([(0, 0), (w - n, 0), (0, h - n), (w - n, h - n)]):
        sheet.paste(figure.crop((x, y, x + n, y + n)).resize((n * zoom, n * zoom), Image.NEAREST),
                    (i * (n * zoom + 10), 0))
    return sheet


def main():
    sheet_dir = sys.argv[sys.argv.index("--sheet") + 1] if "--sheet" in sys.argv else None
    os.makedirs(OUT, exist_ok=True)
    bad = []
    rows = []
    for code, entry in FIGURES.items():
        figure, leaked = make(code, entry)
        note = "" if leaked is None else (" desktop in the frame: %d px" % leaked if leaked else "")
        print("wrote %s.png (%d x %d)%s" % (code, figure.width, figure.height, note))
        if leaked:
            bad.append(code)
        if sheet_dir:
            rows.append((code, corners(figure)))
    if sheet_dir:
        os.makedirs(sheet_dir, exist_ok=True)
        for i in range(0, len(rows), 8):
            part = rows[i:i + 8]
            s = Image.new("RGB", (part[0][1].width, sum(r.height + 24 for _, r in part)), "white")
            y = 0
            for code, r in part:
                ImageDraw.Draw(s).text((4, y + 4), code, fill="black")
                s.paste(r, (0, y + 24))
                y += r.height + 24
            s.save(os.path.join(sheet_dir, "corners-%d.png" % (i // 8 + 1)))
    if bad:
        sys.exit("the desktop shows inside: " + ", ".join(bad))


main()
