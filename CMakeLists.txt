cmake_minimum_required(VERSION 3.15)

project(MacMusic)

add_application(MacMusic
    CREATOR "iPdM"
    src/main.c
    src/ipod_shell.c
    src/ipod_lcd.c
    src/lcd_draw.c
    src/prefs.c
    src/settings.c
    src/shuffle.c
    src/legal.c
    src/volume.c
    src/libfolder.c
    src/libsetup.c
    src/library.c
    src/message.c
    src/modal.c
    src/wav.c
    src/songfile.c
    src/fileops.c
    src/import.c
    src/songdialog.c
    src/editsong.c
    src/export.c
    src/playlists.c
    src/playback.c
    src/audio.c
    src/ui_lists.c
    src/ui_menus.c
    src/ui_nav.c
    src/input.c
    resources/MacMusic.r
    resources/legal.r
    resources/finder-icons.r
)

# Fix 1 (K2): the Finder uses MacMusic's icons only if the application's
# "has bundle" flag is set; Retro68's Rez can't set it, so it is set in
# the MacBinary file after each build.
add_custom_command(TARGET MacMusic_APPL POST_BUILD
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/set-bundle-bit.py
            ${CMAKE_BINARY_DIR}/MacMusic.bin)
