#include "ui_menus.h"
#include "ui_lists.h"

/* Main Menu: items and order Documented/Observed (manual p. 13 and its
   LCD screenshot). The last item, Now Playing, is listed only while a
   song is playing or paused (PROVISIONAL N1; see UIList_Build). */
static const MenuItemDef kMainItems[] = {
    { "\pMusic",            kActMenu, kMenuMusic },
    { "\pExtras",           kActStub, 0 },  /* PROVISIONAL (D3): contents unknown */
    { "\pSettings",         kActStub, 0 },  /* PROVISIONAL (D3): order unknown */
    { "\pShuffle Songs",    kActShuffle, 0 },  /* manual p. 32 */
    { "\pBacklight",        kActNone, 0 },  /* backlight: later stage */
    { "\pNow Playing",      kActNowPlaying, 0 }
};

/* Music: first six items Observed in Apple's Music screenshot.
   PROVISIONAL (D4): "Audiobooks" is Inferred from the scroll bar showing a
   seventh item; included to reproduce the reference screenshot. */
static const MenuItemDef kMusicItems[] = {
    { "\pPlaylists",        kActScreen, kScreenPlaylists },
    { "\pArtists",          kActScreen, kScreenArtists },
    { "\pAlbums",           kActScreen, kScreenAlbums },
    { "\pSongs",            kActScreen, kScreenSongs },
    { "\pGenres",           kActScreen, kScreenGenres },
    { "\pComposers",        kActScreen, kScreenComposers },
    { "\pAudiobooks",       kActScreen, kScreenAudiobooks }
};

static const MenuDef kMenus[kMenuCount] = {
    { "\piPod",  sizeof(kMainItems) / sizeof(kMainItems[0]),   kMainItems  },
    { "\pMusic", sizeof(kMusicItems) / sizeof(kMusicItems[0]), kMusicItems }
};

const MenuDef *UIMenus_Get(short menuID)
{
    return &kMenus[menuID];
}
