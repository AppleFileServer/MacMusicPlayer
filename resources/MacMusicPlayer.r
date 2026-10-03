#include "Menus.r"
#include "Dialogs.r"

/* Item 2 (the divider) is disabled: mask 0x7FFFFFFD clears its bit. */
resource 'MENU' (128) {
    128, textMenuProc;
    0x7FFFFFFD, enabled;
    apple;
    {
        "About MacMusicPlayer\0xC9", noIcon, noKey, noMark, plain;
        "-", noIcon, noKey, noMark, plain;
    }
};

/* Items 1 (Import Song...) and 3 (divider) start disabled: mask 0x7FFFFFFA
   clears their bits. Import is enabled at run time once a library is set. */
resource 'MENU' (129) {
    129, textMenuProc;
    0x7FFFFFFA, enabled;
    "File";
    {
        "Import Song\0xC9", noIcon, noKey, noMark, plain;
        "Choose Library\0xC9", noIcon, noKey, noMark, plain;
        "-", noIcon, noKey, noMark, plain;
        "Quit", noIcon, "Q", noMark, plain;
    }
};

resource 'MENU' (130) {
    130, textMenuProc;
    0, enabled;
    "Edit";
    {
        "Undo", noIcon, "Z", noMark, plain;
        "-", noIcon, noKey, noMark, plain;
        "Cut", noIcon, "X", noMark, plain;
        "Copy", noIcon, "C", noMark, plain;
        "Paste", noIcon, "V", noMark, plain;
        "Clear", noIcon, noKey, noMark, plain;
    }
};

resource 'MBAR' (128) {
    { 128, 129, 130 };
};

/* About box. System 6 ignores the position field, so the rect is
   centred by hand for the 512x342 Macintosh Plus screen. */
resource 'ALRT' (128) {
    { 100, 106, 220, 406 },
    128,
    {
        OK, visible, silent;
        OK, visible, silent;
        OK, visible, silent;
        OK, visible, silent;
    },
    noAutoCenter
};

resource 'DITL' (128) {
    {
        { 88, 214, 108, 284 },
        Button { enabled, "OK" };

        { 12, 16, 30, 284 },
        StaticText { disabled, "MacMusicPlayer" };

        { 36, 16, 80, 284 },
        StaticText { disabled, "A music player for the Macintosh Plus.\rVersion 0.1" };
    }
};

/* Library alerts. Rects are centred by hand for the 512x342 screen, as
   System 6 ignores the position field. */

/* No library, or it can't be found: ^0 explains which. */
resource 'ALRT' (129) {
    { 96, 76, 226, 436 },
    129,
    {
        OK, visible, silent;
        OK, visible, silent;
        OK, visible, silent;
        OK, visible, silent;
    },
    noAutoCenter
};

resource 'DITL' (129) {
    {
        { 98, 250, 118, 350 },
        Button { enabled, "Choose\0xC9" };

        { 98, 136, 118, 236 },
        Button { enabled, "Not Now" };

        { 12, 16, 86, 350 },
        StaticText { disabled, "^0" };
    }
};

/* General message: ^0 is the text, ^1 an optional detail. */
resource 'ALRT' (130) {
    { 110, 96, 210, 416 },
    130,
    {
        OK, visible, sound1;
        OK, visible, sound1;
        OK, visible, sound1;
        OK, visible, sound1;
    },
    noAutoCenter
};

resource 'DITL' (130) {
    {
        { 68, 230, 88, 310 },
        Button { enabled, "OK" };

        { 12, 16, 60, 310 },
        StaticText { disabled, "^0^1" };
    }
};

/* Import Song window. Edit fields 3-8 are in tab order. ^0 = source file
   name, ^1 = duration. Item 17 is a user item that draws the outline
   round the default Import button. Created invisible, shown when filled. */
resource 'DLOG' (131) {
    { 56, 76, 300, 436 },
    dBoxProc,
    invisible,
    noGoAway,
    0,
    131,
    "",
    noAutoCenter
};

resource 'DITL' (131) {
    {
        { 210, 262, 230, 342 },
        Button { enabled, "Import" };

        { 210, 166, 230, 246 },
        Button { enabled, "Cancel" };

        { 40, 96, 56, 342 },
        EditText { enabled, "" };

        { 66, 96, 82, 342 },
        EditText { enabled, "" };

        { 92, 96, 108, 342 },
        EditText { enabled, "" };

        { 118, 96, 134, 342 },
        EditText { enabled, "" };

        { 144, 96, 160, 342 },
        EditText { enabled, "" };

        { 170, 96, 186, 146 },
        EditText { enabled, "" };

        { 169, 180, 187, 342 },
        CheckBox { enabled, "Audiobook" };

        { 40, 16, 56, 90 },
        StaticText { disabled, "Title:" };

        { 66, 16, 82, 90 },
        StaticText { disabled, "Artist:" };

        { 92, 16, 108, 90 },
        StaticText { disabled, "Album:" };

        { 118, 16, 134, 90 },
        StaticText { disabled, "Genre:" };

        { 144, 16, 160, 90 },
        StaticText { disabled, "Composer:" };

        { 170, 16, 186, 90 },
        StaticText { disabled, "Track:" };

        { 10, 16, 28, 342 },
        StaticText { disabled, "^0  (^1)" };

        { 206, 258, 234, 346 },
        UserItem { disabled };
    }
};

/* The chosen WAV's audio is already in the library as song ^0.
   Cancel is the default; Import Anyway goes ahead (D31). */
resource 'ALRT' (132) {
    { 106, 86, 226, 426 },
    132,
    {
        OK, visible, sound1;
        OK, visible, sound1;
        OK, visible, sound1;
        OK, visible, sound1;
    },
    noAutoCenter
};

resource 'DITL' (132) {
    {
        { 88, 236, 108, 326 },
        Button { enabled, "Cancel" };

        { 88, 120, 108, 226 },
        Button { enabled, "Import Anyway" };

        { 12, 16, 78, 326 },
        StaticText { disabled, "This WAV file\0xD5s audio is already in the "
                               "iPod Library as \0xD2^0\0xD3. Import it again?" };
    }
};
