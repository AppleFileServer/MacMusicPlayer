#!/usr/bin/env python3
"""Makes narrowed static copies of EB Garamond for the MacMusic manual
(Phase 4, P2): Apple's manuals of 1984-85 were set in a condensed
Garamond; EB Garamond (SIL Open Font License) is the nearest free face,
but normal width. Each copy is an instance of the variable font at one
weight, with every outline, advance width, side bearing, kerning and
mark-position value scaled horizontally, and the family renamed
("MacMusic Garamond"), as the licence asks of modified versions.

Usage (with fontTools; see docs/environment.md):
    condense-font.py <EBGaramond[wght].ttf> <EBGaramond-Italic[wght].ttf>
                     <out folder> <scale, e.g. 0.86>
Writes MacMusicGaramond-<scale>-Regular.ttf, -Italic.ttf and -Bold.ttf."""

import os, sys
from fontTools.ttLib import TTFont
from fontTools.varLib.instancer import instantiateVariableFont

roman, italic, out, scale = sys.argv[1], sys.argv[2], sys.argv[3], float(sys.argv[4])
X_FIELDS = {"XAdvance", "XPlacement", "XCoordinate"}


def scale_x(obj, seen):
    """Scales every horizontal positioning value in a GPOS subtree."""
    if id(obj) in seen or obj is None or isinstance(obj, (int, float, str, bytes)):
        return
    seen.add(id(obj))
    if isinstance(obj, (list, tuple)):
        for item in obj:
            scale_x(item, seen)
        return
    if isinstance(obj, dict):
        for item in obj.values():
            scale_x(item, seen)
        return
    for name, value in list(vars(obj).items()):
        if name in X_FIELDS and isinstance(value, (int, float)):
            setattr(obj, name, round(value * scale))
        else:
            scale_x(value, seen)


def make(source, weight, style, out_name):
    font = TTFont(source)
    font = instantiateVariableFont(font, {"wght": weight})
    glyf, hmtx = font["glyf"], font["hmtx"]
    for name in font.getGlyphOrder():
        g = glyf[name]
        if g.isComposite():
            for c in g.components:
                c.x = round(c.x * scale)
        elif g.numberOfContours > 0:
            g.coordinates.scale((scale, 1))
            g.coordinates.toInt()
        advance, lsb = hmtx[name]
        hmtx[name] = (round(advance * scale), round(lsb * scale))
    for name in font.getGlyphOrder():
        glyf[name].recalcBounds(glyf)
    if "GPOS" in font:
        scale_x(font["GPOS"].table, set())
    font["OS/2"].xAvgCharWidth = round(font["OS/2"].xAvgCharWidth * scale)
    family = "MacMusic Garamond %d" % round(scale * 100)
    names = font["name"]
    for rec in list(names.names):
        if rec.nameID in (1, 3, 4, 6, 16, 17, 21, 22, 25):
            names.removeNames(nameID=rec.nameID)
    full = "%s %s" % (family, style)
    for nid, text in ((1, family), (2, style), (3, full + "; condensed from EB Garamond"),
                      (4, full), (6, full.replace(" ", "")), (16, family), (17, style)):
        names.setName(text, nid, 3, 1, 0x409)
    names.setName("Condensed by the MacMusic project from EB Garamond "
                  "(SIL Open Font License 1.1)", 5, 3, 1, 0x409)
    font["OS/2"].usWeightClass = weight
    font.save(os.path.join(out, out_name))


os.makedirs(out, exist_ok=True)
tag = "%d" % round(scale * 100)
make(roman, 400, "Regular", "MacMusicGaramond%s-Regular.ttf" % tag)
make(italic, 400, "Italic", "MacMusicGaramond%s-Italic.ttf" % tag)
make(roman, 700, "Bold", "MacMusicGaramond%s-Bold.ttf" % tag)
print("wrote MacMusic Garamond %s (scale %.2f) to %s" % (tag, scale, out))
