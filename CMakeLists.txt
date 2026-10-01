cmake_minimum_required(VERSION 3.15)

project(MacMusicPlayer)

add_application(MacMusicPlayer
    src/main.c
    src/ipod_shell.c
    src/ipod_lcd.c
    resources/MacMusicPlayer.r
)
