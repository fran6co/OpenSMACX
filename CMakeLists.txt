cmake_minimum_required(VERSION 3.20)

project(OpenSMACX VERSION 0.2.3 LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR
        "The current OpenSMACX target is a Windows DLL. "
        "Use the i686 MinGW toolchain when building from macOS or Linux.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR
        "OpenSMACX is injected into the 32-bit terranx.exe process and must be built for x86.")
endif()

set(OPENSMACX_SOURCES
    src/alpha.cpp
    src/autosound.cpp
    src/base.cpp
    src/basebutton.cpp
    src/basepop.cpp
    src/buffer.cpp
    src/buttongroup.cpp
    src/council.cpp
    src/dialog.cpp
    src/dllmain.cpp
    src/faction.cpp
    src/filemap.cpp
    src/flatbutton.cpp
    src/flic.cpp
    src/font.cpp
    src/game.cpp
    src/general.cpp
    src/graphicwin.cpp
    src/heap.cpp
    src/log.cpp
    src/maininterface.cpp
    src/map.cpp
    src/menu.cpp
    src/palette.cpp
    src/path.cpp
    src/probe.cpp
    src/random.cpp
    src/scroll.cpp
    src/spot.cpp
    src/sprite.cpp
    src/stdafx.cpp
    src/stringbox.cpp
    src/strings.cpp
    src/technology.cpp
    src/temp.cpp
    src/terraforming.cpp
    src/text.cpp
    src/textindex.cpp
    src/time.cpp
    src/veh.cpp
    src/win.cpp
)

add_library(OpenSMACX SHARED ${OPENSMACX_SOURCES})
add_library(OpenSMACX::OpenSMACX ALIAS OpenSMACX)

target_compile_features(OpenSMACX PRIVATE cxx_std_11)
target_compile_definitions(OpenSMACX PRIVATE
    BUILD_DLL
    $<$<CONFIG:Debug>:BUILD_DEBUG>
)
target_precompile_headers(OpenSMACX PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/stdafx.h")
target_link_libraries(OpenSMACX PRIVATE user32 gdi32 winmm)

set_target_properties(OpenSMACX PROPERTIES
    OUTPUT_NAME OpenSMACX
    PREFIX ""
    CXX_EXTENSIONS OFF
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
)

if(MINGW)
    target_sources(OpenSMACX PRIVATE src/OpenSMACX.def)
    target_compile_options(OpenSMACX PRIVATE
        -pedantic
        -Wall
        -Wextra
        -Wshadow
        -Wcast-align
        -Wfloat-conversion
        -Wuseless-cast
        -Wno-comment
        -Wno-parentheses
        -Wno-unknown-pragmas
        -fkeep-inline-functions
        $<$<CONFIG:Debug>:-Og>
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:Release>:-fno-delete-null-pointer-checks>
        $<$<CONFIG:Release>:-fno-strict-aliasing>
    )
    target_link_options(OpenSMACX PRIVATE
        $<$<CONFIG:Release>:-s>
        $<$<CONFIG:Release>:-static>
        $<$<CONFIG:Release>:-Wl,--allow-multiple-definition>
    )
elseif(MSVC)
    target_compile_options(OpenSMACX PRIVATE /W4)
endif()

install(TARGETS OpenSMACX
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
    ARCHIVE DESTINATION lib
)
