#!/usr/bin/env python3
"""Makes the MacMusic User's Guide's cover pictures (style guide §12,
SG4, MM1a):

- docs/manual/figures/png/mm-icon.png: MacMusic's own 32 x 32 icon
  (resources/finder-icons.r, ICN# 128), for the plain edition's cover,
  title page and back cover, in place of a logo.
- The own-use edition's cover, only when the private assets are there
  (~/MacPlusProjects/manual-assets-private, or MANUAL_ASSETS): MacDraw's
  cover with its title replaced by "MacMusic User's Guide" and its
  screen by MacMusic's Now Playing (figure c0-cover-screen, made by
  make-figures.py), written as cover-macmusic.jpg in that folder, never
  in the repository (it holds Apple's artwork).

Usage (with Pillow): make-cover.py"""
import os, re, sys
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..", "..")
FIGURES = os.path.join(ROOT, "docs", "manual", "figures", "png")
PRIVATE = os.environ.get("MANUAL_ASSETS",
                         os.path.expanduser("~/MacPlusProjects/manual-assets-private"))
TOOLS = os.environ.get("MANUAL_TOOLS", os.path.expanduser("~/MacPlusProjects/manual-tools"))
FONT = os.path.join(TOOLS, "fonts", "built", "MacMusicGaramond100-Regular.ttf")

# Where things are on the MacDraw cover scan (4489 x 5360), measured.
SCREEN = (1503, 704, 4406, 2652)        # the screen: the black bezel's inner edges
TITLE = (1420, 380, 2420, 632)          # "MacDraw", to be painted out
TITLE_LEFT, TITLE_BASE = 1436, 600      # where the new title is set
TITLE_CAP = 168                         # MacDraw's capital height, in pixels
HOLES = 205                             # the wire binding's holes, left of here


def icon():
    """MacMusic's icon, black on white, enlarged 8 times."""
    text = open(os.path.join(ROOT, "resources", "finder-icons.r")).read()
    block = text[text.index("resource 'ICN#' (128"):]
    rows = re.findall(r"([0-9A-F]{8})", block[:block.index("/* mask */")])[:32]
    im = Image.new("L", (32, 32), 255)
    for y, row in enumerate(rows):
        bits = int(row, 16)
        for x in range(32):
            if bits & (1 << (31 - x)):
                im.putpixel((x, y), 0)
    im = im.resize((256, 256), Image.NEAREST).convert("RGB")
    im.save(os.path.join(FIGURES, "mm-icon.png"), optimize=True)
    print("wrote mm-icon.png")


def cover():
    scan = os.path.join(PRIVATE, "macdraw-cover-4489.png")
    if not os.path.exists(scan):
        print("no private assets: the plain edition's cover will be used")
        return
    im = Image.open(scan).convert("RGB")
    draw = ImageDraw.Draw(im)

    # The binding holes down the left edge go (the owner's choice), painted
    # in the paper's own colour, sampled just to their right.
    strip = im.crop((HOLES, 0, HOLES + 20, im.height)).resize((1, 1), Image.BOX)
    draw.rectangle((0, 0, HOLES, im.height), fill=strip.getpixel((0, 0)))

    # The title: paint out "MacDraw" with the paper's white, and set ours
    # in the book's Garamond at the same capital height.
    draw.rectangle(TITLE, fill=(255, 255, 255))
    size = 100
    font = ImageFont.truetype(FONT, size)
    cap = font.getbbox("M")[3] - font.getbbox("M")[1]
    font = ImageFont.truetype(FONT, round(size * TITLE_CAP / cap))
    draw.text((TITLE_LEFT, TITLE_BASE), "MacMusic User’s Guide", font=font,
              fill=(20, 20, 20), anchor="ls")

    # The screen: MacMusic's Now Playing, in the cover's pale blue.
    screen = Image.open(os.path.join(FIGURES, "c0-cover-screen.png")).convert("RGB")
    x0, y0, x1, y1 = SCREEN
    screen = screen.resize((x1 - x0, y1 - y0), Image.NEAREST)
    im.paste(screen, (x0, y0))

    out = os.path.join(PRIVATE, "cover-macmusic.jpg")
    im.resize((2700, round(2700 * im.height / im.width)), Image.LANCZOS).save(out, quality=90)
    print("wrote", out)


icon()
cover()
