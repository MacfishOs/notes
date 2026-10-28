cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME cutefish-notes)
project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CTest)

list(APPEND CMAKE_PREFIX_PATH "/usr/lib/cmake/CutefishToolkit" "/usr/local/lib/cmake/CutefishToolkit" "${CMAKE_SOURCE_DIR}/../cutefish-toolkit/build")

find_package(CutefishToolkit REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-uc icu-i18n)

set(SRCS
    src/main.cpp
    src/model/note_item.cpp
    src/backend/note_store.cpp
    src/ui/window_controls.cpp
    src/ui/botanical_background.cpp
    src/ui/floating_toolbar.cpp
    src/ui/note_card_view.cpp
    src/ui/notes_grid_view.cpp
    src/ui/note_editor_view.cpp
    src/ui/main_window.cpp
)

set(HEADERS
    src/model/note_item.h
    src/backend/note_store.h
    src/ui/window_controls.h
    src/ui/botanical_background.h
    src/ui/floating_toolbar.h
    src/ui/note_card_view.h
    src/ui/notes_grid_view.h
    src/ui/note_editor_view.h
    src/ui/main_window.h
)

add_executable(${PROJECT_NAME} ${SRCS} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    ${CMAKE_SOURCE_DIR}/../cutefish-toolkit/src
    ${CMAKE_SOURCE_DIR}/../cutefish-toolkit/third_party/skia
)

find_library(SKIA_LIB NAMES skia PATHS "${CMAKE_SOURCE_DIR}/../cutefish-toolkit/build/skia")
pkg_check_modules(FONTCONFIG REQUIRED IMPORTED_TARGET fontconfig)
pkg_check_modules(FREETYPE REQUIRED IMPORTED_TARGET freetype2)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Cutefish::Toolkit
    PkgConfig::ICU
    ${SKIA_LIB}
    PkgConfig::FONTCONFIG
    PkgConfig::FREETYPE
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES cutefish-notes.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES cutefish-notes.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
install(FILES cutefish-notes.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/Cutefish/apps/scalable)
