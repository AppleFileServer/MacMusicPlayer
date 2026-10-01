#include "ui_menus.h"

/* Main Menu: items and order Documented/Observed (manual p. 13 and its
   LCD screenshot). The conditional "Now Playing" item is not included. */
static const MenuItemDef kMainItems[] = {
    { "\pMusic",            kMenuMusic   },
    { "\pExtras",           kSubmenuStub },  /* PROVISIONAL (D3): contents unknown */
    { "\pSettings",         kSubmenuStub },  /* PROVISIONAL (D3): order unknown */
    { "\pShuffle Songs",    kNoSubmenu   },  /* playback: later stage */
    { "\pBacklight",        kNoSubmenu   }   /* backlight: later stage */
};

/* Music: first six items Observed in Apple's Music screenshot.
   PROVISIONAL (D4): "Audiobooks" is Inferred from the scroll bar showing a
   seventh item; included to reproduce the reference screenshot. */
static const MenuItemDef kMusicItems[] = {
    { "\pPlaylists",        kSubmenuStub },
    { "\pArtists",          kSubmenuStub },
    { "\pAlbums",           kSubmenuStub },
    { "\pSongs",            kSubmenuStub },
    { "\pGenres",           kSubmenuStub },
    { "\pComposers",        kSubmenuStub },
    { "\pAudiobooks",       kSubmenuStub }
};

static const MenuDef kMenus[kMenuCount] = {
    { "\piPod",  sizeof(kMainItems) / sizeof(kMainItems[0]),   kMainItems  },
    { "\pMusic", sizeof(kMusicItems) / sizeof(kMusicItems[0]), kMusicItems }
};

const MenuDef *UIMenus_Get(short menuID)
{
    return &kMenus[menuID];
}
