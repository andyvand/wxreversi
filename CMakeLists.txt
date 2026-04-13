#
# Cross-platform build for wxReversi.
#
# Requires wxWidgets 3.0 or newer (3.2+ recommended).
#
#   Linux  :  sudo apt install libwxgtk3.2-dev    (or equivalent)
#   macOS  :  brew install wxwidgets
#   Windows:  build/install wxWidgets from source, set wxWidgets_ROOT_DIR
#
# Configure + build:
#   cmake -S . -B build
#   cmake --build build --config Release
#

cmake_minimum_required(VERSION 3.16)
project(wxreversi C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build bin2c host tool (preserved for non-XRC binary embedding tasks).
set(BIN2C ${CMAKE_BINARY_DIR}/bin2c)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(HostCompile)
host_compile(${CMAKE_CURRENT_SOURCE_DIR}/bin2c.c ${BIN2C})

# wxWidgets core is enough: we only use wxApp, wxFrame, wxPanel, wxDC, wxMenu.
# xrc + xml are needed to load reversi.xrc (pattern bitmaps) at runtime.
find_package(wxWidgets REQUIRED COMPONENTS xrc xml html core base)

if(NOT WIN32)
    include(${wxWidgets_USE_FILE})
endif()

set(SOURCES
    reversi_wx.cpp
    reversi_engine.cpp
    reversi_engine.h
)

# --- XRC -> .xrs -> embedded C header --------------------------------------
#
# Compile reversi.xrc (+ REVPAT1.bmp, REVPAT8.bmp) into a single reversi.xrs
# archive with wxrc, then embed that archive as a C byte array via bin2c so
# the resources ship inside the executable itself. No runtime files needed.
find_program(WXRC_EXECUTABLE
    NAMES wxrc wxrc-3.2 wxrc-3.1 wxrc-3.0
    HINTS
        ${wxWidgets_ROOT_DIR}/bin
        ${wxWidgets_LIB_DIR}/../bin
)
if(NOT WXRC_EXECUTABLE)
    message(FATAL_ERROR
        "wxrc (wxWidgets resource compiler) not found; install wxWidgets "
        "development package or set WXRC_EXECUTABLE.")
endif()

set(REVERSI_XRC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/reversi.xrc
    ${CMAKE_CURRENT_SOURCE_DIR}/REVPAT1.bmp
    ${CMAKE_CURRENT_SOURCE_DIR}/REVPAT8.bmp
    ${CMAKE_CURRENT_SOURCE_DIR}/wxreversi.png
)
set(REVERSI_XRS        ${CMAKE_CURRENT_BINARY_DIR}/reversi.xrs)
set(REVERSI_XRS_HEADER ${CMAKE_CURRENT_BINARY_DIR}/reversi_xrs.h)

add_custom_command(
    OUTPUT  ${REVERSI_XRS}
    COMMAND ${WXRC_EXECUTABLE} -o ${REVERSI_XRS} reversi.xrc
    DEPENDS ${REVERSI_XRC_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "wxrc: compiling XRC resources -> reversi.xrs"
    VERBATIM
)

if(TARGET bin2c)
    set(_bin2c_cmd $<TARGET_FILE:bin2c>)
    set(_bin2c_dep bin2c)
else()
    set(_bin2c_cmd ${BIN2C})
    set(_bin2c_dep ${BIN2C})
endif()

add_custom_command(
    OUTPUT  ${REVERSI_XRS_HEADER}
    COMMAND ${_bin2c_cmd} ${REVERSI_XRS} ${REVERSI_XRS_HEADER} reversi_xrs
    DEPENDS ${REVERSI_XRS} ${_bin2c_dep}
    COMMENT "bin2c: embedding reversi.xrs -> reversi_xrs.h"
    VERBATIM
)

list(APPEND SOURCES ${REVERSI_XRS_HEADER})
# ---------------------------------------------------------------------------

if(WIN32)
    # GUI subsystem on Windows (no console window).
    add_executable(wxreversi WIN32 ${SOURCES} reversi.rc)
elseif(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE reversi.icns)
    set(REVERSI_ICON ${CMAKE_CURRENT_SOURCE_DIR}/reversi.icns)
    set_source_files_properties(${REVERSI_ICON} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
    add_executable(wxreversi MACOSX_BUNDLE
        ${SOURCES}
        ${REVERSI_ICON}
    )
    set_target_properties(wxreversi PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME        "Reversi"
        MACOSX_BUNDLE_GUI_IDENTIFIER     "org.classic.reversi"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        MACOSX_BUNDLE_BUNDLE_VERSION     "1.0"
        MACOSX_BUNDLE_ICON_FILE          "reversi.icns"
    )
elseif(UNIX)
    add_executable(wxreversi ${SOURCES})
    install(TARGETS wxreversi RUNTIME DESTINATION bin)
    install(FILES     ${CMAKE_CURRENT_SOURCE_DIR}/wxreversi.desktop      DESTINATION share/applications)
    install(FILES     ${CMAKE_CURRENT_SOURCE_DIR}/wxreversi.metainfo.xml DESTINATION share/metainfo)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sizes/                 DESTINATION share/icons/hicolor)
else()
    add_executable(wxreversi ${SOURCES})
endif()

target_include_directories(wxreversi PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(wxreversi PRIVATE ${wxWidgets_LIBRARIES})

# Nice warning defaults.
if(MSVC)
    target_compile_options(wxreversi PRIVATE /W4)
else()
    target_compile_options(wxreversi PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()
