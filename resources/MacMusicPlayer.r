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

resource 'MENU' (129) {
    129, textMenuProc;
    allEnabled, enabled;
    "File";
    {
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
