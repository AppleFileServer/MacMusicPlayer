#ifndef LEGAL_H
#define LEGAL_H

#include <MacTypes.h>

/* Stage 9b: the Legal screen's text ('TEXT' 128, made from
   docs/legal/legal-statement.md by tools/make-legal-r.py), wrapped in the
   small font (Geneva 9) to the LCD's width the first time it is needed.
   kSmallLines lines show at a time (lcd_layout.h). */

short Legal_LineCount(void);

/* Line n (0-based) of the wrapped text; an empty line between paragraphs. */
void Legal_Line(short n, Str255 out);

#endif
