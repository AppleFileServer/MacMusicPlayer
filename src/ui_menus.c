#include "ui_menus.h"
#include "ui_lists.h"
#include "settings.h"

/* Main Menu: items and order Documented/Observed (manual p. 13 and its
   LCD screenshot). The last item, Now Playing, is listed only while a
   song is playing or paused (PROVISIONAL N1; see UIList_Build). */
static const MenuItemDef kMainItems[] = {
    { "\pMusic",            kActMenu, kMenuMusic },
    { "\pExtras",           kActStub, 0 },  /* PROVISIONAL (D3): contents unknown */
    { "\pSettings",         kActMenu, kMenuSettings },  /* Stage 9a */
    { "\pShuffle Songs",    kActShuffle, 0 },  /* manual p. 32 */
    { "\pBacklight",        kActBacklight, 0 },  /* S-9: a novelty */
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

/* Settings (Stage 9a): the specification decided in Stage 8b. The order
   is PROVISIONAL (the 4G guide gives no complete list). About and Legal
   come in 9b. */
static const MenuItemDef kSettingsItems[] = {
    { "\pAbout",              kActStub, 0 },
    { "\pShuffle",            kActSetting, kSettingShuffle },
    { "\pRepeat",             kActSetting, kSettingRepeat },
    { "\pClicker",            kActSetting, kSettingClicker },
    { "\pLegal",              kActStub, 0 },
    { "\pReset All Settings", kActMenu, kMenuReset }
};

/* Reset All Settings: Cancel first, so a stray Select doesn't reset
   (guide p. 52: "Settings > Reset All Settings > Reset"). */
static const MenuItemDef kResetItems[] = {
    { "\pCancel",             kActBack, 0 },
    { "\pReset",              kActReset, 0 }
};

static const MenuDef kMenus[kMenuCount] = {
    { "\piPod",  sizeof(kMainItems) / sizeof(kMainItems[0]),   kMainItems  },
    { "\pMusic", sizeof(kMusicItems) / sizeof(kMusicItems[0]), kMusicItems },
    { "\pSettings", sizeof(kSettingsItems) / sizeof(kSettingsItems[0]),
      kSettingsItems },
    { "\pReset All", sizeof(kResetItems) / sizeof(kResetItems[0]),
      kResetItems }
};

const MenuDef *UIMenus_Get(short menuID)
{
    return &kMenus[menuID];
}
