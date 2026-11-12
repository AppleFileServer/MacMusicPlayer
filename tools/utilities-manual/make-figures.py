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
- a part of a window (for the task pages' narrow columns) is cut
  square, inside the window, with a thin grey rule round it;
- a chapter's opener art (style guide §8.6), in two halves: for the
  owner's own edition, a scene from macOS's Macintosh wallpaper (the
  original Macintosh's own 512 x 342 screens, Apple's artwork) in flat
  pale-grey pixels (the owner's choice, 2026-10-09), written only to the
  private assets (UTILITIES_ASSETS, default
  ~/MacPlusProjects/manual-assets-private/utilities), never the
  repository; in the repository, a plain stand-in: the Converter's
  window enlarged and lightened;
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
BACKUP = ((408, 212, 2407, 1611), 56)          # the Backup Utility's window (a toolbar window's corners)
BACKUP_SHEET = ((888, 720, 1927, 1101), 56)    # its "Back up ...?" sheet
TRANSFER = ((648, 192, 2167, 1631), 36)        # the Transfer Utility's window
FINDER = ((504, 276, 2599, 1505), 56)          # the Finder window, installing (cv01-cv05)
ARCHIVE = ((1048, 518, 1791, 789), 36)         # Archive Utility, expanding the zip
APP_MENU = ((67, 6, 489, 53), ((80, 62, 715, 435), 28))      # its name, then the menu
WINDOW_MENU = ((658, 6, 815, 53), ((660, 62, 1199, 579), 28))

# The captures of 2026-10-10 (taken-2026-10-10/): each window captured
# alone, so its frame is the whole image (sized when it's read); an alert
# captured as a region has 20 pixels round it; a sheet is measured within
# its window.
def whole(radius):
    return ("whole", radius)
REGION_ALERT = ((20, 20, 539, 555), 56)        # "Stop ... and quit?"
RESTORE_CONFLICT = ((20, 20, 539, 719), 56)    # "... is already there"
ORIGIN = ((0, 0, 0, 0), 0)                     # a part cut straight from a capture
T = "taken-2026-10-10/"
# The captures of 2026-11-11 (Claude's, with the owner's leave): the
# Converter 1.1, each window alone (1080 x 1264), so a part is cut from
# the whole window, W.
T2 = "taken-2026-11-11/"
W = ((0, 0, 1079, 1263), 36)
SHEETS = {      # sheets and alerts, measured within their windows' captures
    "cv-reencode": ((280, 378, 799, 885), 56),
    "bu-restore": ((440, 306, 1559, 1093), 72),
    "bu-delete": ((740, 464, 1259, 935), 56),
    "cv23": ((280, 394, 799, 869), 56),         # Converter 1.1 (T2)
}

# A part of a window: (left, top, right, bottom) within it, in the
# window's own pixels. Each section is its heading and its grey box.
def section(top, bottom, heading=75, below=20):
    return (0, top - heading, 1080, bottom + below)


# Code: (screenshot, kind, frame). The codes are the outline's
# (docs/utilities-manual/outline.md); a part's code names what it shows.
FIGURES = {
    **{"cv%02d" % n: ("converter/cv%02d" % n, "window", FINDER) for n in (1, 3, 4, 5)},
    "cv02": ("converter/cv02", "window", ARCHIVE),
    "cv06": (T2 + "cv06", "window", whole(36)),      # Converter 1.1
    "cv07": ("converter/cv07", "window", CONVERTER_A),
    "cv08": ("converter/cv08", "window", OPEN_PANEL),
    "cv09": ("converter/cv09", "window", OPEN_PANEL),
    "cv10": ("converter/cv10", "window", OPEN_PANEL),
    **{"cv%02d" % n: ("converter/cv%02d" % n, "window", CONVERTER_B)
       for n in list(range(11, 23)) + list(range(24, 29))},
    "cv23": (T2 + "cv23", "window", None),           # its sheet, in SHEETS
    "cv29": ("converter/cv29", "menu", APP_MENU),
    "cv30": ("converter/cv30", "window", ABOUT),
    "cv31": ("converter/cv31", "menu", APP_MENU),
    "cv32": ("converter/cv32", "menu", APP_MENU),
    "cv34": ("converter/cv34", "menu", WINDOW_MENU),
    # The Macintosh Backup Utility (the owner's screenshots, 2026-10-09).
    **{"bu%02d" % n: ("backup/bu%02d" % n, "window", BACKUP) for n in (1, 5, 7, 8, 14)},
    "bu13-sheet": ("backup/bu13", "window", BACKUP_SHEET),
    # The MacMusic Transfer Utility (the owner's screenshots, 2026-10-10).
    **{"tu%02d" % n: ("new-2026-10-10/n%02d" % n, "window", TRANSFER) for n in (1, 2, 3, 5, 6, 7)},
    # The captures of 2026-10-10 (Claude's, with the owner's leave).
    "tu01": (T + "a02", "window", whole(36)),          # six songs, two renamed
    "tu-refused": (T + "a01", "window", whole(36)),
    "tu-progress": (T + "a06b", "window", whole(36)),
    "tu-checked": (T + "a08", "window", whole(36)),
    "tu-already": (T + "a04", "window", ((500, 466, 1019, 973), 56)),
    "tu-id0": (T + "a11", "window", ((500, 500, 1019, 939), 56)),
    "tu-quit": (T + "a12", "window", REGION_ALERT),
    "plus-wav-files": (T + "p03", "part", (ORIGIN, (0, 64, 2048, 1432))),
    "cv-refused": (T + "c01", "window", whole(36)),
    "cv-not-retrieved": (T + "c04", "window", whole(36)),
    "cv-reencode": (T2 + "cv-reencode", "window", None),  # its sheet, in SHEETS
    "cv-wav-only": (T2 + "cv-wav-only", "window", whole(36)),
    "cv-cancelled": (T + "c10", "window", whole(36)),
    "cv-finder": (T + "c07", "window", whole(56)),
    "cv-quit": (T + "c11", "window", REGION_ALERT),
    "bu-verified": (T + "b06", "window", whole(56)),
    "bu-restore": (T + "b08", "window", None),
    "bu-conflict": (T + "b09", "window", RESTORE_CONFLICT),
    "bu-delete": (T + "b12", "window", None),
    "bu-menu": (T + "b11", "window", ((1102, 360, 1463, 689), 28)),
    "bu-settings": (T + "b14", "window", whole(36)),
    # Parts of the Backup Utility's window, cut from its figures (so the
    # corners are already the page's white): (figure, part).
    "bu01-status": (None, "partof", ("bu01", (0, 100, 2000, 290))),
    "bu14-status": (None, "partof", ("bu14", (0, 100, 1160, 280))),
    "bu05-card": (None, "partof", ("bu05", (0, 250, 920, 720))),
    "bu05-contents": (None, "partof", ("bu05", (0, 760, 920, 1396))),
    "bu08-history": (None, "partof", ("bu08", (920, 262, 2000, 820))),
    "bu08-manifest": (None, "partof", ("bu08", (920, 840, 2000, 1396))),
    "bu08-toolbar": (None, "partof", ("bu08", (1480, 0, 2000, 104))),
    # Installing (Chapter 1): the Finder window's sidebar and first rows.
    "cv01-zip": (None, "partof", ("cv01", (0, 0, 1320, 420))),
    "cv03-app": (None, "partof", ("cv03", (0, 0, 1320, 420))),
    "cv04-drag": (None, "partof", ("cv04", (0, 0, 1320, 420))),
    # Parts of the Transfer Utility's window.
    "tu01-files": (None, "partof", ("tu01", (0, 100, 1520, 650))),
    "tu01-disk": (None, "partof", ("tu01", (0, 690, 1520, 1220))),
    "tu-refused-files": (None, "partof", ("tu-refused", (0, 100, 1520, 790))),
    "tu-progress-foot": (None, "partof", ("tu-progress", (0, 1300, 1520, 1436))),
    "tu-checked-result": (None, "partof", ("tu-checked", (0, 460, 1520, 1290))),
    "cv-wav-only-results": (None, "partof", ("cv-wav-only", (0, 850, 1080, 1125))),
    "cv-cancelled-results": (None, "partof", ("cv-cancelled", (0, 970, 1080, 1130))),
    "cv-refused-source": (None, "partof", ("cv-refused", (0, 186, 1080, 497))),
    "cv-not-retrieved-youtube": (None, "partof", ("cv-not-retrieved", (0, 186, 1080, 583))),
    "bu-verified-snapshot": (None, "partof", ("bu-verified", (920, 835, 2000, 1235))),
    "bu-warnings-status": (None, "partof", ("bu-verified", (0, 104, 2000, 262))),
    "tu03-save-to": (None, "partof", ("tu03", (0, 620, 1520, 790))),
    "tu03-result": (None, "partof", ("tu03", (0, 950, 1520, 1290))),
    "tu05-drop": (None, "partof", ("tu05", (0, 100, 1520, 470))),
    "tu07-advanced": (None, "partof", ("tu07", (0, 1085, 1520, 1290))),
    "tu01-build": (None, "partof", ("tu01", (0, 1320, 1520, 1436))),
    # Parts, for the task pages.
    "cv06-source": (T2 + "cv06", "part", (W, section(261, 330))),
    "cv06-sound": (T2 + "cv06", "part", (W, section(801, 1125))),
    "cv06-convert": (T2 + "cv06", "part", (W, (4, 1140, 1076, 1242))),
    "cv11-source": ("converter/cv11", "part", (CONVERTER_B, section(261, 418))),
    "cv11-output": ("converter/cv11", "part", (CONVERTER_B, section(549, 934))),
    "cv12-output": ("converter/cv12", "part", (CONVERTER_B, section(314, 597))),
    "cv14-output": ("converter/cv14", "part", (CONVERTER_B, section(314, 733))),
    # Converter 1.1 (T2): the YouTube video is the owner's own "MacMusic".
    "cv15-save-to": (T2 + "cv15", "part", (W, (0, 345, 1080, 545))),
    "cv15-advanced": (T2 + "cv15", "part", (W, (0, 545, 1080, 1125))),
    "cv16-youtube": (T2 + "cv16", "part", (W, (0, 150, 1080, 410))),
    "cv17-youtube": (T2 + "cv17", "part", (W, (0, 150, 1080, 440))),
    "cv18-output": (T2 + "cv17", "part", (W, (0, 470, 1080, 890))),
    "cv22-output": (T2 + "cv22", "part", (W, (0, 470, 1080, 890))),
    "cv20-progress": (T2 + "cv20", "part", (W, (4, 1130, 1076, 1242))),
    "cv27-progress": (T2 + "cv27b", "part", (W, (4, 1130, 1076, 1242))),
    "cv28-results": (T2 + "cv28", "part", (W, (0, 700, 1080, 1125))),
    # Chapter openers: (window, the part enlarged across the spread).
    "c2-opener": (T2 + "cv17", "opener", (W, (0, 0, 1080, 648))),
    "c4-opener": ("backup/bu08", "opener", (BACKUP, (0, 0, 2000, 1200))),
    "c3-opener": ("new-2026-10-10/n01", "opener", (TRANSFER, (0, 0, 1520, 912))),
    "c1-opener": ("converter/cv05", "opener", (FINDER, (0, 0, 2096, 1230))),
}

# The own edition's openers: a scene from macOS's Macintosh wallpaper for
# each chapter (the owner's choice of scenes, 2026-10-09).
WALLPAPER = ("/System/Library/ExtensionKit/Extensions/WallpaperMacintoshExtension.appex"
             "/Contents/Resources")
PRIVATE = os.environ.get("UTILITIES_ASSETS", os.path.expanduser(
    "~/MacPlusProjects/manual-assets-private/utilities"))
SCENES = {
    "c1-opener": "StartupWelcome",      # Getting Started: "Welcome to Macintosh"
    "c2-opener": "AlertSounds",         # Converting Songs: the Sound control panel
    "c3-opener": "FinderWindow",        # Moving Songs to the Macintosh: a disk's window
    "c4-opener": "QuitDialog",          # Backing Up Your Macintosh: "Save changes before quitting?" (FinderTrash looked too like c3's)
}
SCENE_SCALE = 5         # a Macintosh pixel, in the spread's pixels (150 per inch)
SCENE_GREY = 205        # a black pixel's tone

RULE = 2        # a part's rule, in the screenshots' pixels
SPREAD = (2250, 1350)   # an opener spread, 15 x 9 in at 150 per inch
PALE = 0.14     # how much of the window shows in an opener (the rest is white)

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
    im = Image.open(found[0])
    if im.mode in ("RGBA", "LA", "P"):     # a window captured alone: its corners are clear
        white = Image.new("RGBA", im.size, "white")
        white.alpha_composite(im.convert("RGBA"))
        im = white
    return im.convert("RGB")


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
    if kind == "partof":            # a part of a figure already made
        figure, (l, t, r, b) = frame
        part = Image.open(os.path.join(OUT, figure + ".png")).convert("RGB").crop((l, t, r, b))
        ImageDraw.Draw(part).rectangle((0, 0, part.width - 1, part.height - 1), outline=(GREY,) * 3, width=RULE)
        part.convert("L").save(os.path.join(OUT, code + ".png"), optimize=True)
        return part, None
    im = screenshot(name)
    if kind in ("part", "opener"):
        ((wl, wt, _, _), _), (l, t, r, b) = frame
        crop = im.crop((wl + l, wt + t, wl + r, wt + b))
        if kind == "part":
            d = ImageDraw.Draw(crop)
            d.rectangle((0, 0, crop.width - 1, crop.height - 1), outline=(GREY,) * 3, width=RULE)
            crop.convert("L").save(os.path.join(OUT, code + ".png"), optimize=True)
            return crop, None
        art = crop.convert("L").resize(SPREAD, Image.LANCZOS)
        art = Image.eval(art, lambda v: round(255 - (255 - v) * PALE))
        halves(art, OUT, code)
        return art, None
    if frame is None:
        frame = SHEETS[code]
    if frame[0] == "whole":
        frame = ((0, 0, im.width - 1, im.height - 1), frame[1])
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


def halves(art, folder, code):
    half = SPREAD[0] // 2
    art.crop((0, 0, half, SPREAD[1])).save(os.path.join(folder, code + "-l.png"), optimize=True)
    art.crop((half, 0, SPREAD[0], SPREAD[1])).save(os.path.join(folder, code + "-r.png"), optimize=True)


def scenes():
    """The own edition's openers, from the wallpaper's scenes, if this Mac
    has them; into the private assets only."""
    if not os.path.isdir(WALLPAPER):
        print("no Macintosh wallpaper on this Mac: the openers stay plain")
        return
    folder = os.path.join(PRIVATE, "figures")
    os.makedirs(folder, exist_ok=True)
    for code, scene in SCENES.items():
        screen = Image.open(os.path.join(WALLPAPER, scene + ".png")).convert("L")
        screen = screen.point(lambda v: SCENE_GREY if v < 128 else 255)
        big = screen.resize((screen.width * SCENE_SCALE, screen.height * SCENE_SCALE), Image.NEAREST)
        x, y = (big.width - SPREAD[0]) // 2, (big.height - SPREAD[1]) // 2
        halves(big.crop((x, y, x + SPREAD[0], y + SPREAD[1])), folder, code)
        print("wrote %s (private): %s" % (code, scene))


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
    scenes()
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
