cmake_minimum_required(VERSION 3.15)

project(MacMusicPlayer)

add_application(MacMusicPlayer
    src/main.c
    src/ipod_shell.c
    src/ipod_lcd.c
    src/lcd_draw.c
    src/ui_menus.c
    src/ui_nav.c
    src/input.c
    resources/MacMusicPlayer.r
)
