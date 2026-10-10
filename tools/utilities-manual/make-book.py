#!/usr/bin/env python3
"""Builds the whole of *MacMusic Utilities* (Phase 8; outline.md), as the
User's Guide's tools/manual/make-book.py builds it: the cover, the front
matter (make-front.py), Chapters 1 to 4 and the appendixes (from their
generators), the glossary, the index and the back covers, as
docs/utilities-manual/book.html, printed to
build-utilities/MacMusic-Utilities.pdf.

Each chapter's ids are made its own (prefixed c1- ... c4-), since every
chapter has an "intro", "l1" and so on. The contents and the index give
page numbers, so the book is laid out twice: the first time to find
which page each part, task and command fell on, the second with the
numbers filled in.

The own-use edition (UB1) uses the private cover and the rainbow logo
when the private assets hold them (make-cover.py makes the cover);
otherwise the plain edition is built.

Usage: make-book.py"""
import importlib.util, json, os, re, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from ub import BOOK, img, page

ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
OUT = os.path.join(ROOT, "docs", "utilities-manual", "book.html")
ANCHORS = os.path.join(ROOT, "build-utilities", "anchors.json")
PRIVATE = os.environ.get("UTILITIES_ASSETS", os.path.expanduser(
    "~/MacPlusProjects/manual-assets-private/utilities"))
OWN_USE = os.path.exists(os.path.join(PRIVATE, "cover-utilities.jpg"))
PDF = os.path.join(ROOT, "build-utilities", "MacMusic-Utilities%s.pdf" % ("" if OWN_USE else "-plain"))


def module(name):
    spec = importlib.util.spec_from_file_location(name.replace("-", "_"),
                                                  os.path.join(HERE, "make-%s.py" % name))
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


def own_ids(text, prefix):
    """A chapter's ids and its links to them, made its own."""
    text = re.sub(r'\bid="([^"]+)"', lambda m: 'id="%s%s"' % (prefix, m.group(1)), text)
    return re.sub(r'href="#([^"]+)"', lambda m: 'href="#%s%s"' % (prefix, m.group(1)), text)


def own_contents(text):
    """A chapter's contents page, with page numbers from the layout."""
    return re.sub(r'<ul class="toc">.*?</ul>', lambda m: re.sub(
        r'<a href="#([^"]+)">(.*?)</a>',
        r'<span class="toc-t">\2</span><span class="toc-pg">{{\1}}</span>', m.group(0)),
        text, flags=re.S)


def chapter(n):
    text = own_ids(module("chapter%d" % n).PART, "c%d-" % n)
    text = text.replace('<div class="opener opener-left">',
                        '<div class="opener opener-left" id="chapter-%d">' % n, 1)
    return own_contents(text)


# ---- Front matter -----------------------------------------------------------

def cover():
    if OWN_USE:
        return '<div class="cover"><img src="private/cover-utilities.jpg"></div>\n'
    return ('<div class="cover plain"><p class="cover-title"><i>%s</i></p>%s</div>\n'
            % (BOOK, img("ub-cover-screen", 400)))


CHAPTERS = [(1, "Getting Started"), (2, "Converting Songs"),
            (3, "Moving Songs to the Macintosh"), (4, "Backing Up Your Macintosh")]
APPENDIXES = [("a", "Messages"), ("b", "About the Files"), ("c", "Troubleshooting"),
              ("d", "Specifications and Licences")]


def book_contents(sections):
    rows = [("about", "About This Manual", "part")]
    for n, name in CHAPTERS:
        rows.append(("chapter-%d" % n, "Chapter %d: %s" % (n, name), "ch"))
        rows += [(a, t, "sec") for a, t in sections[n]]
    rows += [("appendix-%s" % l, "Appendix %s: %s" % (l.upper(), name), "part")
             for l, name in APPENDIXES]
    rows += [("glossary", "Glossary", "part"), ("index", "Index", "part")]
    # (Row classes are prefixed: a class "chapter" would start a page.)
    items = "".join('<li class="toc-%s"><span class="t">%s</span><span class="pg">%s</span></li>'
                    % (cls, t, "{{%s}}" % a) for a, t, cls in rows)
    return ('<div class="front"><div class="flow">'
            '<div class="margin"><h2 class="contents-head" id="book-contents">Contents</h2></div>'
            '<ul class="toc book-toc">%s</ul></div></div>\n' % items)


# ---- The index ------------------------------------------------------------------
# Each subject, and the ids it points to (its pages come from the first
# layout); a "see" entry points to another subject instead.
INDEX = [
    ("Add Files", ["c3-l1", "c3-to-add-songs", "c3-c-files"]),
    ("Advanced (Converter)", ["c2-to-change-the-normalise-level", "c2-r-advanced"]),
    ("Advanced (Transfer Utility)", ["c3-r-advanced"]),
    ("&ldquo;already there&rdquo;", ["c2-if-a-file-is-already-there", "c3-if-an-image-is-already-there",
                                     "c4-to-restore-files-to-a-folder"]),
    ("Apple silicon", ["c1-need", "a-what-you-need"]),
    ("Back Up Now", ["c4-l1", "c4-to-back-up-the-card", "c4-c-toolbar"]),
    ("backing up", ["c4-intro", "c4-l1", "c4-backing-up", "c4-r-what-happens-when-you-back-up",
                    "a-backing-up", "a-backups"]),
    ("backup folder", ["c4-intro", "c4-to-change-the-settings", "c4-r-where-the-backups-are",
                       "a-the-backup-folder"]),
    ("Backup Utility", "Macintosh Backup Utility"),
    ("BlueSCSI", ["c1-intro", "c3-intro", "a-disk-images", "c4-intro"]),
    ("Both (MP3 and WAV)", ["c2-l2", "c2-to-make-a-macmusic-wav-an-mp3-or-both"]),
    ("Browse Backups", ["c4-to-see-what-a-snapshot-holds", "c4-c-toolbar"]),
    ("Build Transfer Image", ["c3-l1", "c3-to-build-the-disk-image", "c3-c-build-transfer-image"]),
    ("Cancel", ["c2-to-cancel-a-conversion", "c3-to-cancel", "c4-to-cancel-a-backup"]),
    ("card", "SD card"),
    ("check-sums", ["c4-intro", "a-the-backup-folder"]),
    ("checking a snapshot", ["c4-to-check-a-snapshot", "a-the-history"]),
    ("clean shutdown", ["c4-if-a-disk-wasnt-shut-down-cleanly", "a-backing-up",
                        "t-a-disk-on-the-plus-has-gone-wrong"]),
    ("Clear All", ["c3-to-remove-songs"]),
    ("Clear (incomplete jobs)", ["c4-to-clear-incomplete-jobs"]),
    ("Converter", "MacMusic Converter Utility"),
    ("converting", ["c2-l1", "c2-l2", "c2-r-what-happens-when-you-convert"]),
    ("Copy Summary", ["c3-to-see-what-was-checked", "c3-c-result"]),
    ("Delete (a snapshot)", ["c4-to-delete-a-snapshot", "a-the-history"]),
    ("Deno", ["c2-c-ffmpeg-yt-dlp-and-deno", "a-software-inside-the-utilities"]),
    ("Details", ["appendix-a", "c2-if-a-video-cant-be-used"]),
    ("disk image", ["c1-l2", "c3-intro", "a-disk-images"]),
    ("disk template", ["c3-c-disk-template", "a-disk-images"]),
    ("Edit menu", ["c2-c-edit-menu"]),
    ("Eject", ["c4-l1", "c4-to-eject-the-card"]),
    ("exFAT", ["c4-to-restore-onto-a-new-card", "a-restoring-onto-a-new-card"]),
    ("FFmpeg", ["c2-c-ffmpeg-yt-dlp-and-deno", "a-software-inside-the-utilities"]),
    ("file names", ["c2-to-set-the-file-name", "c3-if-a-name-is-changed", "a-names-on-the-macintosh",
                    "t-a-file-name-has-changed"]),
    ("HD60_512 Transfer.hda", ["c3-intro", "a-disk-images"]),
    ("History", ["c4-the-history", "c4-c-history"]),
    ("ID 0", ["c3-to-choose-the-scsi-id", "a-building-the-disk-image"]),
    ("importing songs on the Plus", ["c1-l2", "c3-to-use-the-disk-on-the-plus", "t-a-song-wont-import"]),
    ("incomplete jobs", ["c4-to-clear-incomplete-jobs", "a-the-history"]),
    ("installing the utilities", ["c1-l1"]),
    ("Keep (Never Thin)", ["c4-to-keep-a-snapshot"]),
    ("Keep Both", ["c2-if-a-file-is-already-there", "c3-if-an-image-is-already-there",
                   "c4-to-restore-files-to-a-folder"]),
    ("Licences", ["c2-c-version", "a-software-inside-the-utilities"]),
    ("Local File", ["c2-l1", "c2-to-convert-a-sound-file", "c2-c-local-file-and-youtube"]),
    ("Look Up", ["c2-l2", "c2-to-convert-a-youtube-video"]),
    ("Macintosh Backup Utility", ["c1-intro", "chapter-4", "p-macintosh-backup-utility",
                                  "a-macintosh-backup-utility-1-0"]),
    ("MacMusic Converter Utility", ["c1-intro", "chapter-2", "p-macmusic-converter-utility",
                                    "a-macmusic-converter-utility-1-0"]),
    ("MacMusic Transfer Utility", ["c1-intro", "chapter-3", "p-macmusic-transfer-utility",
                                   "a-macmusic-transfer-utility-1-0"]),
    ("MacMusic WAV files", ["c1-intro", "c2-intro", "a-macmusic-wav-files"]),
    ("<i>MacMusic User&rsquo;s Guide</i>", ["about", "c1-l2", "c3-to-use-the-disk-on-the-plus"]),
    ("manifest", ["c4-intro", "a-the-backup-folder"]),
    ("menus (Converter)", ["c2-r-the-menus"]),
    ("messages", ["appendix-a"]),
    ("MP3 files", ["c2-to-make-a-macmusic-wav-an-mp3-or-both", "c2-if-the-song-is-already-an-mp3",
                   "a-mp3-files"]),
    ("names on the Macintosh", "file names"),
    ("new card, restoring onto", ["c4-to-restore-onto-a-new-card", "a-restoring-onto-a-new-card",
                                  "t-the-card-itself-has-failed"]),
    ("Normalise", ["c2-to-even-out-the-loudness", "c2-to-change-the-normalise-level",
                   "c2-c-normalise-to", "t-songs-arent-equally-loud"]),
    ("Open Anyway", ["c1-l1", "t-a-program-wont-open-the-first-time"]),
    ("power cut", "clean shutdown"),
    ("progress", ["c2-while-it-works", "c3-l1", "c4-l1"]),
    ("quitting while it works", ["c2-if-you-quit-while-it-works", "c3-to-cancel"]),
    ("re-encoding", ["c2-if-the-song-is-already-an-mp3", "a-converting"]),
    ("Register", ["c4-l1", "c4-to-register-a-card", "a-your-card"]),
    ("Remove", ["c3-to-remove-songs"]),
    ("Restore", ["c4-l2", "c4-restoring", "a-restoring"]),
    ("Restore to a New Card", "new card, restoring onto"),
    ("Result (Transfer Utility)", ["c3-l1", "c3-c-result"]),
    ("Results (Converter)", ["c2-l1", "c2-to-find-the-files", "c2-c-results", "a-in-the-results"]),
    ("Save To", ["c2-to-choose-where-files-are-saved", "c3-to-choose-where-its-saved"]),
    ("Saves as", ["c2-l1", "c2-to-set-the-file-name"]),
    ("SCSI ID", ["c3-intro", "c3-to-choose-the-scsi-id", "a-disk-images", "t-the-disk-doesnt-appear"]),
    ("SD card", ["c1-l2", "c3-to-copy-the-disk-image-to-the-card", "c4-intro", "c4-your-card"]),
    ("Settings (Backup Utility)", ["c4-to-change-the-settings"]),
    ("Show in Finder", ["c2-to-find-the-files", "c3-l1"]),
    ("Shut Down", ["c1-l2", "c3-to-copy-the-disk-image-to-the-card", "c4-l1"]),
    ("size of the disk", ["c3-how-the-size-is-worked-out", "a-disk-images"]),
    ("snapshots", ["c4-intro", "c4-the-history", "a-the-backup-folder"]),
    ("specifications", ["appendix-d"]),
    ("thinning", ["c4-how-older-snapshots-are-thinned", "c4-to-change-the-settings",
                  "t-the-backups-take-too-much-room"]),
    ("Title", ["c2-to-set-the-title-macmusic-shows"]),
    ("Transfer disk", ["c3-intro", "c3-l1", "c3-to-use-the-disk-on-the-plus", "t-the-disk-doesnt-appear"]),
    ("Trim silence at the ends", ["c2-to-trim-silence-at-the-ends"]),
    ("troubleshooting", ["appendix-c"]),
    ("USB drive", ["c1-usb", "about"]),
    ("Verify Backup", ["c4-to-check-a-snapshot", "c4-c-toolbar"]),
    ("WAV Files (folder)", ["c3-intro", "c3-l1", "c3-to-use-the-disk-on-the-plus"]),
    ("&ldquo;What was checked&rdquo;", ["c3-to-see-what-was-checked"]),
    ("YouTube", ["c2-l2", "c2-to-convert-a-youtube-video", "c2-if-a-video-cant-be-used",
                 "a-choosing-a-youtube-video", "t-youtube-stops-working"]),
    ("yt-dlp", ["c2-c-ffmpeg-yt-dlp-and-deno", "t-youtube-stops-working",
                "a-software-inside-the-utilities"]),
]


def pages_text(ids, at):
    """The pages for some ids: sorted, each once, runs joined (12&ndash;13)."""
    nums = sorted({int(page_number(at[i], at)) for i in ids if i in at
                   and page_number(at[i], at).isdigit()})
    out, i = [], 0
    while i < len(nums):
        j = i
        while j + 1 < len(nums) and nums[j + 1] == nums[j] + 1:
            j += 1
        out.append(str(nums[i]) if i == j else "%d&ndash;%d" % (nums[i], nums[j]))
        i = j + 1
    return ", ".join(out) or "00"


def plain(term):
    return re.sub(r"<[^>]+>|&[a-z]+;", "", term)


def index(at):
    groups = {}
    for term, refs in INDEX:
        groups.setdefault(plain(term)[0].upper(), []).append((term, refs))
    html = []
    for letter in sorted(groups):
        html.append('<h3 class="index-letter">%s</h3>' % letter)
        for term, refs in sorted(groups[letter], key=lambda e: plain(e[0]).lower()):
            if isinstance(refs, str):
                html.append('<p class="ix">%s. <i>See</i> %s</p>' % (term, refs))
            else:
                html.append('<p class="ix">%s, %s</p>' % (term, pages_text(refs, at)))
    return ('<div class="chapter page-break"><div class="appendix-title" id="index" '
            'data-foot="Index"><h1 class="name">Index</h1></div>'
            '<div class="index-columns">%s</div></div>\n' % "".join(html))


def back_cover(logo):
    return ('<div class="inside-back"></div>\n'
            '<div class="back-cover"><p class="publisher">MacMusic</p>'
            '<div class="back-logo">%s</div></div>\n' % logo(30))


# ---- The book ---------------------------------------------------------------

def book(at):
    front = module("front")
    chapters = {n: chapter(n) for n, _ in CHAPTERS}
    sections = {n: [(a, t) for t, a in re.findall(
        r'<li class="part"><span class="toc-t">(.*?)</span><span class="toc-pg">\{\{([^}]+)\}\}</span>',
        re.search(r'<ul class="toc">(.*?)</ul>', chapters[n], flags=re.S).group(1))]
        for n in chapters}
    body = (cover() + front.COPYRIGHT + front.title_page()
            + book_contents(sections) + front.ABOUT
            + "".join(chapters[n] for n in sorted(chapters))
            + module("appendixes").PART
            + front.BACK + index(at) + back_cover(front.logo))
    return re.sub(r"\{\{([a-z0-9-]+)\}\}",
                  lambda m: page_number(at[m.group(1)], at) if m.group(1) in at else "00", body)


FOOT_NAMES = ([("chapter-%d" % n, "Chapter %d: %s" % (n, name)) for n, name in CHAPTERS]
              + [("appendix-%s" % l, "Appendix %s: %s" % (l.upper(), name)) for l, name in APPENDIXES]
              + [("glossary", "Glossary"), ("index", "Index")])


def page_number(p, at):
    """The number printed on the p'th page: roman from the title page,
    arabic from Chapter 1's opener, which is page 2 (as the User's
    Guide's)."""
    if p >= at.get("chapter-1", 10 ** 6):
        return str(p - at["chapter-1"] + 2)
    return roman(p - at.get("title-page", 1) + 1)


def feet(at):
    """Each page's number, written into its running foot's number box."""
    if not at:
        return ""
    parts = sorted((at[i], name) for i, name in FOOT_NAMES if i in at)
    rules = []
    for p in range(at.get("title-page", 1), 400):
        n = page_number(p, at)
        name = ([nm for start, nm in parts if start <= p] or [""])[-1]
        left = n + ("\\2003\\2003 " + name if name and p >= at.get("chapter-1", 10 ** 6) else "")
        for box, text in (("bottom-left", left), ("bottom-right", n)):
            rules.append('.pagedjs_page[data-page-number="%d"] .pagedjs_margin-%s '
                         '> .pagedjs_margin-content::after { content: "%s" !important; }'
                         % (p, box, text))
    return "<style>%s</style>\n" % "\n".join(rules)


def roman(n):
    out = ""
    for value, letters in ((10, "x"), (9, "ix"), (5, "v"), (4, "iv"), (1, "i")):
        while n >= value:
            out += letters
            n -= value
    return out


def build(at, anchors=None):
    html = page(BOOK, book(at)).replace("</head>", feet(at) + "</head>", 1)
    open(OUT, "w").write(html)
    cmd = [os.path.join(HERE, "build-pdf.sh"), OUT, PDF] + ([anchors] if anchors else [])
    subprocess.run(cmd, check=True, cwd=ROOT)


if __name__ == "__main__":
    os.makedirs(os.path.dirname(ANCHORS), exist_ok=True)
    build({}, ANCHORS)                      # the first layout: where everything fell
    at = json.load(open(ANCHORS))
    for ident in [i for term, refs in INDEX if not isinstance(refs, str) for i in refs]:
        if ident not in at:
            print("index: no such id:", ident)
    build(at)                               # the second, with the page numbers
    print("edition:", "own use (UB1)" if OWN_USE else "plain")
