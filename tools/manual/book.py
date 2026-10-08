"""The parts every chapter of the MacMusic User's Guide shares: the
draft's title page, the opener spread (SG10), pictures, and the HTML
page around a chapter for Paged.js (docs/manual/manual.css)."""

import os, re, struct

BOOK = "MacMusic User&rsquo;s Guide"

FIGURES = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                       "..", "..", "docs", "manual", "figures", "png")


def png_size(name):
    """A figure's width and height in pixels, from its PNG header."""
    with open(os.path.join(FIGURES, name + ".png"), "rb") as f:
        return struct.unpack(">II", f.read(24)[16:24])


def anchor(title):
    """An id from a heading: lower case, entities and punctuation dropped."""
    t = re.sub(r"&#?[a-z0-9]+;", "", title.lower())
    return re.sub(r"[^a-z0-9]+", "-", re.sub(r"<[^>]+>", "", t)).strip("-")


def img(name, width, cls="screen"):
    """A figure from docs/manual/figures/png, `width` points wide. Its
    height is given too, so Paged.js lays out the page before the
    picture has loaded."""
    w, h = png_size(name)
    return ('<img class="%s" src="figures/%s.png" style="width:%spt; height:%.1fpt">'
            % (cls, name, width, float(width) * h / w))


def draft_title(chapter, note):
    """The page in front of a chapter printed on its own for review."""
    return ('<div class="draft-title">\n'
            ' <p class="draft-book">%s</p>\n'
            ' <p class="draft-what">%s</p>\n'
            ' <p class="draft-note">%s</p>\n'
            '</div>\n' % (BOOK, chapter, note))


def opener(number, name, art):
    """The chapter's opener spread: MacMusic's screen in grey across both
    pages (figures `art`-l and `art`-r), the chapter's name in the
    right-hand page's margin column. It also names the chapter for the
    running feet."""
    return ('<div class="opener opener-left"><img src="figures/%s-l.png"></div>\n'
            '<div class="opener opener-right"><img src="figures/%s-r.png">\n'
            ' <div class="opener-title" data-foot="Chapter %d: %s">'
            '<p class="ch">Chapter %d</p><p class="name">%s</p></div>\n'
            '</div>\n' % (art, art, number, name, number, name))


def page(title, body):
    """The HTML file for Paged.js."""
    return """<!doctype html>
<html><head><meta charset="utf-8">
<title>%s</title>
<link rel="stylesheet" href="manual.css">
<script>window.PagedConfig = { after: () => { window.__pagedDone = true; } };</script>
<script src="paged.polyfill.js"></script>
</head><body>
%s
</body></html>
""" % (title, body)
