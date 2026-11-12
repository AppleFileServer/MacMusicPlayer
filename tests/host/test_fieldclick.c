/* 1.2 (N12-1): a click on the margin of a text field's box is moved
   inside the field (fieldclick.c). The rectangles are the song window's
   (DITL 131 in resources/MacMusic.r), in the dialog's coordinates. */

#include "hosttest.h"
#include "fieldclick.h"

static const Rect kFields[] = {
    { 40, 96, 56, 342 },        /* Title */
    { 66, 96, 82, 342 },        /* Artist */
    { 92, 96, 108, 342 },       /* Album */
    { 118, 96, 134, 342 },      /* Genre */
    { 144, 96, 160, 342 },      /* Composer */
    { 170, 96, 186, 146 }       /* Track */
};
#define kCount ((short)(sizeof kFields / sizeof kFields[0]))

/* Adjusts (v, h); returns whether it moved, and where to. */
static Boolean Try(short v, short h, short *newV, short *newH)
{
    Point pt;
    Boolean moved;

    pt.v = v;
    pt.h = h;
    moved = FieldClick_Adjust(kFields, kCount, &pt);
    *newV = pt.v;
    *newH = pt.h;
    return moved;
}

int main(void)
{
    short v, h;

    Test("a click on the text is left alone");
    CHECK(!Try(126, 200, &v, &h));
    CHECK_EQ(v, 126);
    CHECK_EQ(h, 200);
    CHECK(!Try(118, 96, &v, &h));           /* the rectangle's top-left */
    CHECK(!Try(133, 341, &v, &h));          /* its bottom-right pixel */

    Test("the owner's clicks: Genre's margin, above and below the text");
    /* y=173 and y=191 on the screen; the window's top is at 56. */
    CHECK(Try(117, 200, &v, &h));
    CHECK_EQ(v, 118);
    CHECK_EQ(h, 200);
    CHECK(Try(135, 200, &v, &h));
    CHECK_EQ(v, 133);
    CHECK_EQ(h, 200);

    Test("each edge of the drawn box");
    CHECK(Try(115, 200, &v, &h));           /* the frame's top line */
    CHECK_EQ(v, 118);
    CHECK(Try(136, 200, &v, &h));           /* its bottom line */
    CHECK_EQ(v, 133);
    CHECK(Try(126, 93, &v, &h));            /* left */
    CHECK_EQ(v, 126);
    CHECK_EQ(h, 96);
    CHECK(Try(126, 344, &v, &h));           /* right */
    CHECK_EQ(h, 341);
    CHECK(Try(115, 93, &v, &h));            /* a corner */
    CHECK_EQ(v, 118);
    CHECK_EQ(h, 96);

    Test("outside every drawn box: left alone");
    CHECK(!Try(114, 200, &v, &h));          /* between Album and Genre */
    CHECK_EQ(v, 114);
    CHECK(!Try(137, 200, &v, &h));          /* between Genre and Composer */
    CHECK(!Try(126, 92, &v, &h));           /* the label column */
    CHECK(!Try(126, 345, &v, &h));
    CHECK(!Try(178, 160, &v, &h));          /* right of Track, before the
                                               Audiobook box */

    Test("the right field: Album's lower margin, Composer's upper");
    CHECK(Try(109, 120, &v, &h));
    CHECK_EQ(v, 107);                       /* Album, not Genre */
    CHECK(Try(142, 120, &v, &h));
    CHECK_EQ(v, 144);                       /* Composer */

    Test("Track, a short field");
    CHECK(Try(188, 148, &v, &h));
    CHECK_EQ(v, 185);
    CHECK_EQ(h, 145);

    Test("an empty rectangle is no field");
    {
        Rect none[1] = { { 10, 10, 10, 10 } };
        Point pt;

        pt.v = 9;
        pt.h = 9;
        CHECK(!FieldClick_Adjust(none, 1, &pt));
        CHECK_EQ(pt.v, 9);
    }

    return TestsDone("test_fieldclick");
}
