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


# ---- A chapter's pieces (Chapters 2 and 4) --------------------------------

TEXT = 308              # the text column, in points
TASK = 260              # a task's picture


def fig(name, width=TEXT, caption=None):
    return ('<figure class="window">%s%s</figure>'
            % (img(name, width), "<figcaption>%s</figcaption>" % caption if caption else ""))


def steps(*parts):
    """"> " a step; "~ " a grey note; "OR"; "* " an item in a list of
    open squares (consecutive ones make one list); the rest plain."""
    out, items = [], []

    def flush():
        if items:
            out.append('<ul class="boxes">%s</ul>' % "".join("<li>%s</li>" % i for i in items))
            items.clear()
    for p in parts:
        if p.startswith("* "):
            items.append(p[2:])
            continue
        flush()
        if p == "OR":
            out.append('<p class="or">OR</p>')
        elif p.startswith("> "):
            out.append('<p class="step">%s</p>' % p[2:])
        elif p.startswith("~ "):
            out.append('<p class="grey">%s</p>' % p[2:])
        else:
            out.append("<p>%s</p>" % p)
    flush()
    return "".join(out)


class Parts:
    """A chapter's lessons and reference sections, as they're made, for
    its contents page."""
    def __init__(self):
        self.lessons = []
        self.sections = []

    def lesson(self, number, title):
        a = "l%d" % number
        self.lessons.append((number, title, a))
        return ('<div class="margin"><div class="lesson-head" id="%s">'
                '<span class="number">%d</span><span class="title">%s</span></div></div>'
                % (a, number, title))

    def section(self, title, ident=None):
        a = ident or "r-" + anchor(title)
        self.sections.append((title, a))
        return '<div class="margin"><h2 class="margin-head" id="%s">%s</h2></div>' % (a, title)


def head(title, ident):
    return '<div class="margin"><h2 class="margin-head" id="%s">%s</h2></div>' % (ident, title)


def sub(title):
    return '<div class="margin"><h3 class="margin-sub">%s</h3></div>' % title


def cmd(name, *paras):
    return ('<div class="entry"><div class="margin"><h4 class="cmd" id="c-%s">%s</h4></div>'
            % (anchor(name), name) + "".join("<p>%s</p>" % p for p in paras) + "</div>")


def table(head_, *rows, cls="ref"):
    h = "".join("<th>%s</th>" % c for c in head_)
    r = "".join("<tr>%s</tr>" % "".join("<td>%s</td>" % c for c in row) for row in rows)
    return '<table class="%s"><thead><tr>%s</tr></thead><tbody>%s</tbody></table>' % (cls, h, r)


class Topic:
    def __init__(self, title, intro, tasks):
        self.title, self.intro, self.tasks = title, intro, tasks

    def html(self):
        # The topic's head and introduction stay on the page with its first task.
        first, rest = self.tasks[0][1], "".join(t for _, t in self.tasks[1:])
        return ('<div class="topic-block">'
                '<div class="keep"><div class="margin"><h2 class="margin-head" id="%s">%s</h2></div>%s%s</div>%s</div>'
                % (anchor(self.title), self.title, self.intro, first, rest))


def T(title, body, *figures):
    """A task: its title in the margin, its steps and pictures beside it."""
    pics = "".join(fig(f, w) for f, w in ((f if isinstance(f, tuple) else (f, TASK)) for f in figures))
    return (title, '<div class="entry task"><div class="margin"><h3 class="task-head" id="%s">%s</h3></div>%s%s</div>'
            % (anchor(title), title, body, pics))


