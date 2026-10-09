"""The parts every chapter of *MacMusic Utilities* shares (UB3): its
draft title page, pictures, and the HTML page around a chapter for
Paged.js. The opener spread and the anchors are the User's Guide's
(tools/manual/book.py); the styles are the User's Guide's
(docs/manual/manual.css) with this book's few differences after them
(docs/utilities-manual/utilities.css)."""

import os, struct, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "manual"))
from book import anchor, opener          # noqa: E402,F401  (shared with the User's Guide)

BOOK = "MacMusic Utilities"
FIGURES = os.path.join(HERE, "..", "..", "docs", "utilities-manual", "figures", "png")


def png_size(name):
    with open(os.path.join(FIGURES, name + ".png"), "rb") as f:
        return struct.unpack(">II", f.read(24)[16:24])


def img(name, width, cls="screen"):
    """A figure, `width` points wide, its height given so Paged.js can lay
    out the page before the picture loads."""
    w, h = png_size(name)
    return ('<img class="%s" src="figures/%s.png" style="width:%spt; height:%.1fpt">'
            % (cls, name, width, float(width) * h / w))


def draft_title(chapter, note):
    return ('<div class="draft-title">\n'
            ' <p class="draft-book"><i>%s</i></p>\n'
            ' <p class="draft-what">%s</p>\n'
            ' <p class="draft-note">%s</p>\n'
            '</div>\n' % (BOOK, chapter, note))


def page(title, body):
    return """<!doctype html>
<html><head><meta charset="utf-8">
<title>%s</title>
<link rel="stylesheet" href="manual.css">
<link rel="stylesheet" href="utilities.css">
<script>window.PagedConfig = { after: () => { window.__pagedDone = true; } };</script>
<script src="paged.polyfill.js"></script>
</head><body>
%s
</body></html>
""" % (title, body)
