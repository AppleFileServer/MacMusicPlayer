cmake_minimum_required(VERSION 3.15)

project(MacMusicPlayer)

add_application(MacMusicPlayer
    CREATOR "iPdM"
    src/main.c
    src/ipod_shell.c
    src/ipod_lcd.c
    src/lcd_draw.c
    src/prefs.c
    src/libfolder.c
    src/libsetup.c
    src/library.c
    src/message.c
    src/wav.c
    src/import.c
    src/playlists.c
    src/playback.c
    src/audio.c
    src/ui_lists.c
    src/ui_menus.c
    src/ui_nav.c
    src/input.c
    resources/MacMusicPlayer.r
)
