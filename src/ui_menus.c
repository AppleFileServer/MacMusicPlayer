#include "ui_menus.h"
#include "ui_lists.h"
#include "settings.h"

/* Main Menu: items and order Documented/Observed (manual p. 13 and its
   LCD screenshot). The last item, Now Playing, is listed only while a
   song is playing or paused (PROVISIONAL N1; see UIList_Build). */
static const MenuItemDef kMainItems[] = {
    { "\pMusic",            kActMenu, kMenuMusic },
    { "\pExtras",           kActMenu, kMenuExtras },
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
   is PROVISIONAL (the 4G guide gives no complete list). About and Legal:
   Stage 9b. */
static const MenuItemDef kSettingsItems[] = {
    { "\pAbout",              kActScreen, kScreenAbout },
    { "\pShuffle",            kActSetting, kSettingShuffle },
    { "\pRepeat",             kActSetting, kSettingRepeat },
    { "\pLegal",              kActScreen, kScreenLegal },
    { "\pReset All Settings", kActMenu, kMenuReset }
};

/* Extras (D3, decided 2026-10-08): only the Clock. The 4G's Extras also
   held Contacts, Calendar, Notes and Games; MacMusic leaves them out. */
static const MenuItemDef kExtrasItems[] = {
    { "\pClock",              kActScreen, kScreenClock }
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
      kResetItems },
    { "\pExtras", sizeof(kExtrasItems) / sizeof(kExtrasItems[0]),
      kExtrasItems }
};

const MenuDef *UIMenus_Get(short menuID)
{
    return &kMenus[menuID];
}
