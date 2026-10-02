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

/* Items 1 (Import Song..., until Stage 3b) and 3 (divider) are disabled:
   mask 0x7FFFFFFA clears their bits. */
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
