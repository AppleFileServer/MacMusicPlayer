#!/usr/bin/env python3
"""Writes docs/manual/sample/sample.html, the Phase 4 sample spread
(Chapter 1 lesson 7 facing the Chapter 2 task page "Moving Through a
Song"), in MacMusic Garamond 100, the normal width the owner chose (P3).
spread("86") gives the narrowed width it was compared with."""
import os
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "..", "docs", "manual", "sample", "sample.html")

S = 200 / 174          # points per Mac pixel in the labelled LCD figure
X0 = 140               # the image's left edge: the text column
def leader(label, ly, tx, ty, side="left"):
    """A label at height ly, its red leader running to Mac pixel (tx, ty)
    of the LCD crop: across, then angled (MJ 14). Left labels sit in the
    margin column; right ones beside the picture (MW 122). A label may
    have two lines, split by "|"."""
    x, y = X0 + tx * S, ty * S
    lines = label.split("|")
    if side == "left":
        tx_, a, b, anchor = 0, 112, 126, "start"
    else:
        right = X0 + 174 * S
        tx_, a, b, anchor = right + 22, right + 18, right + 6, "start"
    text = "".join('<tspan x="%d" dy="%s">%s</tspan>' % (tx_, "0" if i == 0 else "10", t)
                   for i, t in enumerate(lines))
    return ('<text x="%d" y="%.1f" text-anchor="%s">%s</text>'
            '<polyline class="leader" points="%d,%.1f %d,%.1f %.1f,%.1f"/>'
            % (tx_, ly + 3, anchor, text, a, ly, b, ly, x, y))

LABELS = [("Play icon", 16, 13, 15), ("Place in the list", 38, 22, 34),
          ("Title, artist and album", 58, 52, 49),
          ("Progress bar", 127, 60, 110), ("Time played", 143, 22, 124),
          ("Shuffle and|Repeat icons", 38, 150, 33, "right"),
          ("Time left", 143, 154, 124, "right")]
CALLOUTS = ('<svg class="callouts" width="450pt" height="168pt" viewBox="0 0 450 168" overflow="visible">'
            '<image href="figures/fig-1-6.png" x="%d" y="0" width="200" height="%.1f" '
            'style="image-rendering: pixelated"/>' % (X0, 142 * S)
            + "".join(leader(*l) for l in LABELS) + "</svg>")

def spread(v):
    return f'''
<div class="lesson{v} {'w100' if v == '100' else ''}">
 <div class="flow">
  <div class="margin">
   <div class="lesson-head"><span class="number">7</span><span>Playing a Song</span></div>
  </div>
   <p>You can move around MacMusic&rsquo;s menus now, so it&rsquo;s time to
   listen to something. Every song in your library is listed by title
   under Songs, in the Music menu.</p>
   <p class="step">Choose Music from the main menu, and then choose Songs.</p>
   <p>Turn the click wheel until the item is highlighted, then click the
   centre button.</p>
   <figure class="beside"><img class="screen" src="figures/fig-1-5.png" style="width:96pt"></figure>
   <p>The Songs screen lists your songs in alphabetical order.</p>
   <p class="step">Highlight a song and click the centre button.</p>
   <p>In a few seconds you&rsquo;ll hear the song through the
   Macintosh&rsquo;s speaker, and the <b class="term">Now Playing</b>
   screen shows what&rsquo;s playing:</p>
  <figure class="wide">{CALLOUTS}</figure>
   <p>When the song ends, the next one in the list follows it, with no
   pause between them.</p>
   <p class="margin margin-note grey">You can also press the Space bar
   to pause and to play.</p>
   <p class="step">Click the bottom of the click wheel to pause.</p>
   <p>The play icon changes to two bars. Click again to carry on.</p>
 </div>
</div>

<div class="task{v} page-break {'w100' if v == '100' else ''}">
 <div class="task-page">
  <div class="column">
   <h2 class="topic">Moving Through a Song</h2>
   <p>While a song plays, the Now Playing screen lets you skip ahead,
   go back, or jump to any place in it, and set how loud it is.</p>
   <ul class="boxes">
    <li>fast-forward and rewind</li>
    <li>the scrubber</li>
    <li>the volume</li>
   </ul>
   <p class="grey">The music keeps playing while you do any of these.</p>
  </div>
  <div class="column">
   <h3>To Fast-Forward or Rewind</h3>
   <p class="tstep">Position the pointer on the right side of the click
   wheel and hold down the mouse button. To rewind, use the left side.</p>
   <p>After half a second the play icon changes to two arrows, and the
   song moves on at four times its normal speed; after two seconds,
   sixteen times.</p>
   <p class="tstep">Release the mouse button.</p>
   <p>The song carries on from the new place.</p>
   <p class="or">OR</p>
   <p>Hold down Option and the Right Arrow or Left Arrow key.</p>
   <div class="foot"><img class="screen" src="figures/fig-2-8.png"></div>
  </div>
  <div class="column">
   <h3>To Use the Scrubber</h3>
   <p class="tstep">On Now Playing, click the centre button.</p>
   <p>A diamond, the <b class="term">scrubber</b>, appears on the
   progress bar.</p>
   <p class="tstep">Turn the click wheel.</p>
   <p>One full turn moves through the whole song. When you stop turning,
   the song jumps to the new place.</p>
   <p class="grey">The scrubber goes away by itself after five seconds.</p>
   <div class="foot"><img class="screen" src="figures/fig-2-9.png"></div>
  </div>
  <div class="column">
   <h3>To Change the Volume</h3>
   <p class="tstep">On Now Playing, turn the click wheel: clockwise for
   louder, counterclockwise for softer.</p>
   <p>The volume bar takes the place of the progress bar. One full turn
   goes from the softest setting to the loudest.</p>
   <p class="or">OR</p>
   <p>Press the Down Arrow key for louder, the Up Arrow key for softer,
   one step at a time.</p>
   <p class="grey">MacMusic remembers the volume for next time.</p>
   <div class="foot"><img class="screen" src="figures/fig-2-12a.png"></div>
  </div>
 </div>
</div>
'''

html = f'''<!doctype html>
<html><head><meta charset="utf-8">
<title>MacMusic manual: sample spread</title>
<link rel="stylesheet" href="manual.css">
<script>window.PagedConfig = {{ after: () => {{ window.__pagedDone = true; }} }};</script>
<script src="paged.polyfill.js"></script>
</head><body>
{spread("100")}
</body></html>
'''
os.makedirs(os.path.dirname(OUT), exist_ok=True)
open(OUT, "w").write(html)
print("wrote", os.path.normpath(OUT))
