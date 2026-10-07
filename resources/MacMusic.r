#include "Menus.r"
#include "Dialogs.r"
#include "Processes.r"

/* Stage 6a-2: replaces Retro68's default SIZE resource (its build lists
   our resources after Retro68APPL.r, so this one wins). Changes from the
   default: the app can run in the background under MultiFinder, so the
   audio ring keeps being refilled with another program in front (S2),
   and it accepts suspend/resume events (handled from Stage 6b-1). Window
   activation stays as before (needsActivateOnFGSwitch: MultiFinder sends
   activate events). Partition: about 240 KB is in use while playing a
   small library (measured); 512 KB preferred leaves room for a large
   library, the import buffer and Standard File; 384 KB minimum. */
resource 'SIZE' (-1) {
    reserved,
    acceptSuspendResumeEvents,
    reserved,
    canBackground,
    needsActivateOnFGSwitch,
    backgroundAndForeground,
    dontGetFrontClicks,
    ignoreChildDiedEvents,
    is32BitCompatible,
    notHighLevelEventAware,
    onlyLocalHLEvents,
    notStationeryAware,
    dontUseTextEditServices,
    reserved,
    reserved,
    reserved,
    512 * 1024,
    384 * 1024
};

/* Item 2 (the divider) is disabled: mask 0x7FFFFFFD clears its bit. */
resource 'MENU' (128) {
    128, textMenuProc;
    0x7FFFFFFD, enabled;
    apple;
    {
        "About MacMusic\0xC9", noIcon, noKey, noMark, plain;
        "-", noIcon, noKey, noMark, plain;
    }
};

/* Items 1 (Import Song...), 2 (Export Song...) and 4 (divider) start
   disabled: mask 0x7FFFFFF4 clears their bits. Import is enabled at run
   time once a library is set; Export while the highlight is on a song. */
resource 'MENU' (129) {
    129, textMenuProc;
    0x7FFFFFF4, enabled;
    "File";
    {
        "Import Song\0xC9", noIcon, noKey, noMark, plain;
        "Export Song\0xC9", noIcon, noKey, noMark, plain;
        "Choose Library\0xC9", noIcon, noKey, noMark, plain;
        "-", noIcon, noKey, noMark, plain;
        "Quit", noIcon, "Q", noMark, plain;
    }
};

/* Items 1-6 are for desk accessories. Stage 8c: Edit Song... (item 8)
   is enabled at run time while a song is highlighted or playing on Now
   Playing. */
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
        "-", noIcon, noKey, noMark, plain;
        "Edit Song\0xC9", noIcon, noKey, noMark, plain;
    }
};

resource 'MBAR' (128) {
    { 128, 129, 130, 131 };
};

/* Playlists (Stage 3C, P3/P6). The playlist items after the divider are
   added and enabled at run time. */
resource 'MENU' (131) {
    131, textMenuProc;
    0x7FFFFFFD, enabled;
    "Playlists";
    {
        "New Playlist\0xC9", noIcon, noKey, noMark, plain;
        "-", noIcon, noKey, noMark, plain;
    }
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
        StaticText { disabled, "MacMusic" };

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

/* 5b-3 (L1): Find Existing... is the default. */
resource 'DITL' (129) {
    {
        { 98, 230, 118, 350 },
        Button { enabled, "Find Existing\0xC9" };

        { 98, 116, 118, 220 },
        Button { enabled, "Create New\0xC9" };

        { 98, 16, 118, 106 },
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

/* New Playlist window: name field (item 3) and the default-button
   outline user item (5). Created invisible, shown when filled. */
resource 'DLOG' (133) {
    { 120, 96, 222, 416 },
    dBoxProc,
    invisible,
    noGoAway,
    0,
    133,
    "",
    noAutoCenter
};

resource 'DITL' (133) {
    {
        { 70, 222, 90, 302 },
        Button { enabled, "Create" };

        { 70, 126, 90, 206 },
        Button { enabled, "Cancel" };

        { 38, 18, 54, 302 },
        EditText { enabled, "" };

        { 10, 16, 28, 302 },
        StaticText { disabled, "Name of the new playlist:" };

        { 66, 218, 94, 306 },
        UserItem { disabled };
    }
};

/* Song ^0 is already in playlist ^1 (P5). Cancel is the default. */
resource 'ALRT' (134) {
    { 116, 96, 226, 416 },
    134,
    {
        OK, visible, sound1;
        OK, visible, sound1;
        OK, visible, sound1;
        OK, visible, sound1;
    },
    noAutoCenter
};

resource 'DITL' (134) {
    {
        { 78, 216, 98, 306 },
        Button { enabled, "Cancel" };

        { 78, 112, 98, 206 },
        Button { enabled, "Add Again" };

        { 12, 16, 68, 306 },
        StaticText { disabled, "\0xD2^0\0xD3 is already in the playlist "
                               "\0xD2^1\0xD3. Add it again?" };
    }
};

/* Stage 5b-2 import. The chosen file is on a different disk from the
   iPod Library, so it must be copied. Import is the default. */
resource 'ALRT' (135) {
    { 110, 86, 216, 426 },
    135,
    {
        OK, visible, sound1;
        OK, visible, sound1;
        OK, visible, sound1;
        OK, visible, sound1;
    },
    noAutoCenter
};

resource 'DITL' (135) {
    {
        { 74, 236, 94, 326 },
        Button { enabled, "Import" };

        { 74, 136, 94, 226 },
        Button { enabled, "Cancel" };

        { 12, 16, 64, 326 },
        StaticText { disabled, "Importing from another disk will take "
                               "some time. Do you still want to import?" };
    }
};

/* Move (OK, the default) or copy the chosen file into the library. */
resource 'ALRT' (136) {
    { 104, 86, 226, 426 },
    136,
    {
        OK, visible, silent;
        OK, visible, silent;
        OK, visible, silent;
        OK, visible, silent;
    },
    noAutoCenter
};

resource 'DITL' (136) {
    {
        { 90, 236, 110, 326 },
        Button { enabled, "OK" };

        { 90, 136, 110, 226 },
        Button { enabled, "Just Copy" };

        { 12, 16, 80, 326 },
        StaticText { disabled, "The file will be moved into the iPod "
                               "Library. Choose Just Copy to leave the "
                               "original where it is (copying takes "
                               "longer)." };
    }
};

/* "Importing..." / "Exporting...": shown, without buttons, while the
   work is done. ^0 is the text. */
resource 'DLOG' (137) {
    { 146, 156, 196, 356 },
    dBoxProc,
    invisible,
    noGoAway,
    0,
    137,
    "",
    noAutoCenter
};

resource 'DITL' (137) {
    {
        { 16, 16, 34, 184 },
        StaticText { disabled, "^0" };
    }
};

/* E4: the file could not be moved (^0 says why). Copy is the default. */
resource 'ALRT' (138) {
    { 106, 86, 226, 426 },
    138,
    {
        OK, visible, sound1;
        OK, visible, sound1;
        OK, visible, sound1;
        OK, visible, sound1;
    },
    noAutoCenter
};

resource 'DITL' (138) {
    {
        { 88, 236, 108, 326 },
        Button { enabled, "Copy" };

        { 88, 136, 108, 226 },
        Button { enabled, "Cancel" };

        { 12, 16, 78, 326 },
        StaticText { disabled, "^0 Copy it into the iPod Library "
                               "instead?" };
    }
};

/* Stage 5b-3 (L1). File > Choose Library...: use an existing library or
   create a new one. Use Existing is the default. */
resource 'ALRT' (139) {
    { 104, 41, 222, 471 },
    139,
    {
        OK, visible, silent;
        OK, visible, silent;
        OK, visible, silent;
        OK, visible, silent;
    },
    noAutoCenter
};

resource 'DITL' (139) {
    {
        { 86, 254, 106, 414 },
        Button { enabled, "Use Existing Library\0xC9" };

        { 86, 100, 106, 244 },
        Button { enabled, "Create New Library\0xC9" };

        { 86, 16, 106, 90 },
        Button { enabled, "Cancel" };

        { 12, 16, 76, 414 },
        StaticText { disabled, "Use an existing iPod Library, or create a "
                               "new one? To use an existing library, open "
                               "its \0xD2iPod Library Info\0xD3 file or one "
                               "of its songs." };
    }
};

/* 5b-3 (L3): Save in Create New while inside a library. Use It is the
   default. */
resource 'ALRT' (140) {
    { 110, 96, 216, 416 },
    140,
    {
        OK, visible, sound1;
        OK, visible, sound1;
        OK, visible, sound1;
        OK, visible, sound1;
    },
    noAutoCenter
};

resource 'DITL' (140) {
    {
        { 74, 230, 94, 310 },
        Button { enabled, "Use It" };

        { 74, 136, 94, 216 },
        Button { enabled, "Cancel" };

        { 12, 16, 64, 310 },
        StaticText { disabled, "This folder is already an iPod Library. "
                               "Use it, instead of creating a new library "
                               "inside it?" };
    }
};
