"""The parts every chapter of the MacMusic User's Guide shares: the
draft's title page, the opener spread (SG10), pictures, and the HTML
page around a chapter for Paged.js (docs/manual/manual.css)."""

BOOK = "MacMusic User&rsquo;s Guide"


def img(name, width, cls="screen"):
    """A figure from docs/manual/figures/png, `width` points wide."""
    return '<img class="%s" src="figures/%s.png" style="width:%spt">' % (cls, name, width)


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
