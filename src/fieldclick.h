#ifndef FIELDCLICK_H
#define FIELDCLICK_H

#include <MacTypes.h>

/* 1.2 (N12-1): a click on the margin of a text field's box. The Dialog
   Manager draws an EditText item's frame 3 pixels outside the item's
   rectangle but takes clicks only inside the rectangle, so a click
   between the frame and the text went nowhere: the field being typed in
   kept the typing (the owner, Genre clicked after Composer). */

#define kFieldFrameMargin   3   /* the frame, outside the item's rectangle */

/* If pt is inside one of the fields' drawn boxes (its rectangle outset by
   kFieldFrameMargin) but outside the rectangle itself, moves pt to the
   nearest point inside that rectangle and returns true; otherwise leaves
   pt alone and returns false. The rectangles are in pt's coordinates. */
Boolean FieldClick_Adjust(const Rect *fields, short count, Point *pt);

#endif
