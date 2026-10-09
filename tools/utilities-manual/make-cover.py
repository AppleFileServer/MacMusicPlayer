#!/usr/bin/env python3
"""Makes *MacMusic Utilities*' cover (Phase 8; decisions UC1-UC5), as the
User's Guide's is made (tools/manual/make-cover.py):

- docs/utilities-manual/figures/png/ub-cover-screen.png: the Macintosh
  Plus's screen, recovered from the owner's Mini vMac screenshot (the
  MacMusic Utilities disk's window, with the three stand-in programs'
  icons), tinted the User's Guide cover's pale blue; for the plain
  edition's cover, and for the own edition's below.
- The own edition's cover, only when the private assets are there:
  MacDraw's cover with its binding holes painted out, its title replaced
  by "MacMusic Utilities" and its screen by the one above, written as
  utilities/cover-utilities.jpg in the private assets, never in the
  repository (it holds Apple's artwork).

The screenshot is kept privately, in
~/MacPlusProjects/manual-assets-private/utilities-screenshots/cover/.

Usage (with Pillow): make-cover.py"""
import os, subprocess, sys, tempfile
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..", "..")
FIGURES = os.path.join(ROOT, "docs", "utilities-manual", "figures", "png")
PRIVATE = os.environ.get("MANUAL_ASSETS",
                         os.path.expanduser("~/MacPlusProjects/manual-assets-private"))
TOOLS = os.environ.get("MANUAL_TOOLS", os.path.expanduser("~/MacPlusProjects/manual-tools"))
FONT = os.path.join(TOOLS, "fonts", "built", "MacMusicGaramond100-Regular.ttf")
SHOT = os.path.join(PRIVATE, "utilities-screenshots", "cover", "cover-screen-2026-10-09-171212.png")

# As the User's Guide's cover (tools/manual/make-cover.py): where things
# are on the MacDraw cover scan (4489 x 5360), and the screen's tint.
SCREEN = (1503, 704, 4406, 2652)
TITLE = (1420, 380, 2420, 632)
TITLE_LEFT, TITLE_BASE = 1436, 600
TITLE_CAP = 168
HOLES = 205
TINT = "zoom=6 ink=1e2832 paper=bfdae1 nopointer"


def screen():
    """The Plus's screen from the screenshot, by the User's Guide's figure
    tool (Mini vMac's screen at 384, 198, four times enlarged)."""
    if not os.path.exists(SHOT):
        print("no cover screenshot: the cover screen is left as it is")
        return
    with tempfile.TemporaryDirectory() as work:
        spec = os.path.join(work, "figures.txt")
        open(spec, "w").write("screen 384 198 4\nub-cover-screen %s full %s\n" % (SHOT, TINT))
        subprocess.run([sys.executable, os.path.join(ROOT, "tools", "manual", "make-figures.py"),
                        spec, FIGURES], check=True)


def cover():
    scan = os.path.join(PRIVATE, "macdraw-cover-4489.png")
    if not os.path.exists(scan):
        print("no private assets: the plain edition's cover will be used")
        return
    im = Image.open(scan).convert("RGB")
    draw = ImageDraw.Draw(im)
    strip = im.crop((HOLES, 0, HOLES + 20, im.height)).resize((1, 1), Image.BOX)
    draw.rectangle((0, 0, HOLES, im.height), fill=strip.getpixel((0, 0)))
    draw.rectangle(TITLE, fill=(255, 255, 255))
    font = ImageFont.truetype(FONT, 100)
    cap = font.getbbox("M")[3] - font.getbbox("M")[1]
    font = ImageFont.truetype(FONT, round(100 * TITLE_CAP / cap))
    draw.text((TITLE_LEFT, TITLE_BASE), "MacMusic Utilities", font=font,
              fill=(20, 20, 20), anchor="ls")
    shot = Image.open(os.path.join(FIGURES, "ub-cover-screen.png")).convert("RGB")
    x0, y0, x1, y1 = SCREEN
    im.paste(shot.resize((x1 - x0, y1 - y0), Image.NEAREST), (x0, y0))
    out = os.path.join(PRIVATE, "utilities", "cover-utilities.jpg")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    im.resize((2700, round(2700 * im.height / im.width)), Image.LANCZOS).save(out, quality=90)
    print("wrote", out)


screen()
cover()
