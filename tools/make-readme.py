#!/usr/bin/env python3
"""Makes the TeachText "MacMusic Read Me" from docs/user-guide.md (1.0,
decision V9): plain text in Mac Roman with CR line ends, one line per
paragraph (TeachText wraps them), headings in capitals or underlined,
bullets as Mac Roman bullets, tables as one line per row, and the
Markdown marks removed. TeachText opens at most 32,000 bytes; the tool
stops if the text is longer.

Usage: tools/make-readme.py [output file]
    (default: build/MacMusic Read Me.txt; tools/make-release.sh puts it
    on the release disk as a TEXT/ttxt file)."""

import os, re, sys

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
source = os.path.join(root, "docs", "user-guide.md")
target = sys.argv[1] if len(sys.argv) > 1 else \
    os.path.join(root, "build", "MacMusic Read Me.txt")
LIMIT = 32000


def plain(text):
    """Inline Markdown removed: links, bold, code."""
    text = re.sub(r"\[([^\]]+)\]\([^)]+\)", r"\1", text)
    text = text.replace("**", "").replace("`", "")
    return text


lines = open(source, encoding="utf-8").read().splitlines()
out = []                        # paragraphs
para = []                       # the lines of the paragraph being read
table = []                      # the cells of the table being read


def end_paragraph():
    if para:
        out.append(" ".join(para))
        para.clear()


def end_table():
    """A table as one line per row: "Heading: cell; heading: cell"."""
    if table:
        head = table[0]
        for row in table[1:]:
            out.append("• " + row[0] + ": " + "; ".join(
                "%s: %s" % (h.lower(), c) for h, c in zip(head[1:], row[1:])))
        out.append("")
        table.clear()


for raw in lines + [""]:
    line = raw.rstrip()
    stripped = line.strip()
    if stripped.startswith("|"):
        end_paragraph()
        cells = [plain(c.strip()) for c in stripped.strip("|").split("|")]
        if not all(re.fullmatch(r":?-+:?", c) for c in cells):
            table.append(cells)
        continue
    end_table()
    if not stripped:
        end_paragraph()
        if out and out[-1] != "":
            out.append("")
        continue
    heading = re.match(r"^(#+)\s+(.*)", stripped)
    item = re.match(r"^(\s*)(?:[-*]|\d+\.)\s+(.*)", line)
    if heading:
        end_paragraph()
        title = plain(heading.group(2))
        level = len(heading.group(1))
        if out and out[-1] != "":
            out.append("")
        if level <= 2:
            out += [title.upper(), ""]
        else:
            out += [title, "-" * len(title), ""]
    elif item:
        end_paragraph()
        indent = "    " if item.group(1) else ""
        number = re.match(r"^\s*(\d+)\.", line)
        mark = number.group(1) + "." if number else "•"
        para.append(indent + mark + " " + plain(item.group(2)))
    else:
        para.append(plain(stripped))

while out and out[-1] == "":
    out.pop()
text = "\r".join(out) + "\r"
data = text.encode("mac_roman")
if len(data) > LIMIT:
    sys.exit("the Read Me is %d bytes: TeachText opens at most %d"
             % (len(data), LIMIT))
os.makedirs(os.path.dirname(target), exist_ok=True)
with open(target, "wb") as f:
    f.write(data)
print("wrote", target, len(data), "bytes")
