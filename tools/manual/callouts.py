"""Labelled screen pictures for the MacMusic manual (style guide §8.2):
a figure in the text column with labels in the margin column to its left
(and beside it on the right), each joined to its part by a thin red
leader, across and then angled (MW 12, MJ 14).

Coordinates are in points from the text block's left edge, so a figure
placed with class "wide" lines up with the margin column."""

TEXT_COLUMN = 140       # the margin column (118pt) and the gutter (22pt)


def leader(label, ly, tx, ty, side, x0, scale, width_px):
    """A label at height ly (points), its leader running to Mac pixel
    (tx, ty) of the picture. A label may have two lines, split by "|"."""
    x, y = x0 + tx * scale, ty * scale
    if side == "left":
        lx, a, b = 0, 112, 126
    else:
        right = x0 + width_px * scale
        lx, a, b = right + 22, right + 18, right + 6
    text = "".join('<tspan x="%d" dy="%s">%s</tspan>' % (lx, "0" if i == 0 else "10", t)
                   for i, t in enumerate(label.split("|")))
    return ('<text x="%d" y="%.1f">%s</text>'
            '<polyline class="leader" points="%d,%.1f %d,%.1f %.1f,%.1f"/>'
            % (lx, ly + 3, text, a, ly, b, ly, x, y))


def figure(src, width_px, height_px, scale, labels, x0=TEXT_COLUMN, width=448):
    """An SVG of the picture at `scale` points per Mac pixel, placed x0
    points in, with its labels: (text, label y, pixel x, pixel y[, side])."""
    height = height_px * scale
    parts = ['<svg class="callouts" width="%dpt" height="%.0fpt" viewBox="0 0 %d %.0f" '
             'overflow="visible">' % (width, height, width, height),
             '<image href="%s" x="%d" y="0" width="%.1f" height="%.1f" '
             'style="image-rendering: pixelated"/>' % (src, x0, width_px * scale, height)]
    for l in labels:
        text, ly, tx, ty = l[:4]
        side = l[4] if len(l) > 4 else "left"
        parts.append(leader(text, ly, tx, ty, side, x0, scale, width_px))
    parts.append("</svg>")
    return "".join(parts)
