cmake_minimum_required(VERSION 3.15)

project(MacMusic VERSION 1.0)

# 1.0 Part A: the version is set only above. CMake writes it into
# version.h (for the code: the About box and Settings > About) and
# version.r (the vers resources and the signature resource), both in the
# build folder. vers holds it in BCD: major, then minor and bug-fix.
set(patch "${PROJECT_VERSION_PATCH}")
if(patch STREQUAL "")
    set(patch 0)
endif()
math(EXPR VERS_MAJOR
     "(${PROJECT_VERSION_MAJOR} / 10) * 16 + ${PROJECT_VERSION_MAJOR} % 10"
     OUTPUT_FORMAT HEXADECIMAL)
math(EXPR VERS_MINOR "${PROJECT_VERSION_MINOR} * 16 + ${patch}"
     OUTPUT_FORMAT HEXADECIMAL)
configure_file(src/version.h.in ${CMAKE_BINARY_DIR}/version.h @ONLY)
configure_file(resources/version.r.in ${CMAKE_BINARY_DIR}/version.r @ONLY)

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
    src/libcache.c
    src/print.c
    src/printlayout.c
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
    ${CMAKE_BINARY_DIR}/version.r
)
target_include_directories(MacMusic PRIVATE ${CMAKE_BINARY_DIR})

# 1.1 (N11-24): printing in a code segment of its own.
target_link_options(MacMusic PRIVATE
    "-Wl,--mac-segments,${CMAKE_SOURCE_DIR}/resources/segments.map")

# 1.0 Part B2: the emulator smoke test (tests/run-smoke.sh). With this on,
# the build runs a scripted test instead of MacMusic; off, the default,
# the build is the released app, unchanged.
option(MACMUSIC_SMOKE_TEST "Build the emulator smoke test" OFF)
if(MACMUSIC_SMOKE_TEST)
    target_sources(MacMusic PRIVATE tests/smoke/smoke.c)
    target_include_directories(MacMusic PRIVATE tests/smoke src)
    target_compile_definitions(MacMusic PRIVATE MACMUSIC_SMOKE_TEST)
endif()

# Fix 1 (K2): the Finder uses MacMusic's icons only if the application's
# "has bundle" flag is set; Retro68's Rez can't set it, so it is set in
# the MacBinary file after each build.
add_custom_command(TARGET MacMusic_APPL POST_BUILD
    COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/set-bundle-bit.py
            ${CMAKE_BINARY_DIR}/MacMusic.bin)
