cmake_minimum_required(VERSION 3.15)

project(MacMusicPlayer)

add_application(MacMusicPlayer
    src/main.c
    src/ipod_shell.c
    src/ipod_lcd.c
    src/lcd_draw.c
    src/prefs.c
    src/libfolder.c
    src/libsetup.c
    src/library.c
    src/ui_lists.c
    src/ui_menus.c
    src/ui_nav.c
    src/input.c
    resources/MacMusicPlayer.r
)
