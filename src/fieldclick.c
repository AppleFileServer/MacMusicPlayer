#include "fieldclick.h"

/* 1.2 (N12-1): see fieldclick.h. Written with MacTypes alone, so the host
   tests can check it. A point is inside a Rect when it is at least top and
   left and less than bottom and right, as PtInRect has it. */

static Boolean Inside(const Rect *r, Point pt, short outset)
{
    return pt.v >= r->top - outset && pt.v < r->bottom + outset
        && pt.h >= r->left - outset && pt.h < r->right + outset;
}

static short Clamp(short x, short low, short high)
{
    return x < low ? low : x > high ? high : x;
}

Boolean FieldClick_Adjust(const Rect *fields, short count, Point *pt)
{
    short i;
    const Rect *r;

    for (i = 0; i < count; i++)
    {
        r = &fields[i];
        if (r->bottom <= r->top || r->right <= r->left)
            continue;                   /* an empty rectangle: no field */
        if (Inside(r, *pt, 0))
            return false;               /* already in a field */
        if (Inside(r, *pt, kFieldFrameMargin))
        {
            pt->v = Clamp(pt->v, r->top, r->bottom - 1);
            pt->h = Clamp(pt->h, r->left, r->right - 1);
            return true;
        }
    }
    return false;
}
