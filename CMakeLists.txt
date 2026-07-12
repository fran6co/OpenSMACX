cmake_minimum_required(VERSION 3.20)

project(OpenSMACX VERSION 0.2.3 LANGUAGES CXX)

set(OPENSMACX_GAME_INSTALLER "" CACHE FILEPATH "Path to the locally owned GOG installer")
set(OPENSMACX_GAME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.opensmacx/game" CACHE PATH
    "Directory used for extracted proprietary game data")
set(OPENSMACX_PYTHON "" CACHE FILEPATH "Python interpreter with tools/requirements.txt installed")
set(OPENSMACX_WINE "" CACHE FILEPATH "Wine executable used to run Windows tools")
set(OPENSMACX_LEGACY_EXE "${OPENSMACX_GAME_DIR}/terranx_original.exe" CACHE FILEPATH
    "User-owned executable used to prepare the local hybrid image")
set(OPENSMACX_HYBRID_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.opensmacx/hybrid-image" CACHE PATH
    "Output directory for the local-only hybrid image pack")
set(OPENSMACX_HYBRID_EXE "${CMAKE_CURRENT_SOURCE_DIR}/.opensmacx/hybrid/terranx_legacy.exe"
    CACHE FILEPATH "Assembled executable path for the local hybrid image")
set(OPENSMACX_LEGACY_LEAF_DIR "${CMAKE_CURRENT_BINARY_DIR}/legacy-leaves"
    CACHE PATH "Output directory for generated local legacy leaf assembly")
set(OPENSMACX_LEGACY_LEAF_OBJECT
    "${CMAKE_CURRENT_BINARY_DIR}/legacy-leaves.obj"
    CACHE FILEPATH "Generated local legacy leaf COFF object")

if(OPENSMACX_PYTHON)
    set(OPENSMACX_PYTHON_EXECUTABLE "${OPENSMACX_PYTHON}")
else()
    find_program(OPENSMACX_PYTHON_EXECUTABLE NAMES python3 python)
    if(NOT OPENSMACX_PYTHON_EXECUTABLE)
        if(CMAKE_HOST_WIN32)
            set(OPENSMACX_PYTHON_EXECUTABLE python)
        else()
            set(OPENSMACX_PYTHON_EXECUTABLE python3)
        endif()
    endif()
endif()

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
    src/caviar.cpp
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

add_custom_target(prepare-hybrid-image
    COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/prepare_hybrid_image.py"
        --exe "${OPENSMACX_LEGACY_EXE}"
        --output "${OPENSMACX_HYBRID_DIR}"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/prepare_hybrid_image.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/functions.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/priorities.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/summary.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/analysis-summary.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/analysis-correlation.csv"
    USES_TERMINAL
    VERBATIM
)

add_custom_target(assemble-hybrid-executable
    COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/assemble_hybrid_image.py"
        --image "${OPENSMACX_HYBRID_DIR}"
        --output "${OPENSMACX_HYBRID_EXE}"
    DEPENDS
        prepare-hybrid-image
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/assemble_hybrid_image.py"
    USES_TERMINAL
    VERBATIM
)

set(OPENSMACX_HYBRID_STAGE_COMMAND
    "${OPENSMACX_PYTHON_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/opensmac_patcher.py"
    --exe "${OPENSMACX_HYBRID_EXE}"
    --dll "$<TARGET_FILE:OpenSMACX>"
    --output "${OPENSMACX_GAME_DIR}"
    --exe-name terranx_hybrid.exe
)
if(OPENSMACX_WINE)
    list(APPEND OPENSMACX_HYBRID_STAGE_COMMAND --wine "${OPENSMACX_WINE}")
endif()

add_custom_target(stage-hybrid-game
    COMMAND ${OPENSMACX_HYBRID_STAGE_COMMAND}
    DEPENDS OpenSMACX assemble-hybrid-executable
    USES_TERMINAL
    VERBATIM
)

set(OPENSMACX_LEGACY_LEAF_ASSEMBLY
    "${OPENSMACX_LEGACY_LEAF_DIR}/legacy-leaves.S")
set(OPENSMACX_LEGACY_LEAF_MANIFEST
    "${OPENSMACX_LEGACY_LEAF_DIR}/manifest.json")
set(OPENSMACX_LEGACY_LEAF_SYMBOLS
    "${OPENSMACX_LEGACY_LEAF_DIR}/symbols.csv")

add_custom_command(
    OUTPUT
        "${OPENSMACX_LEGACY_LEAF_ASSEMBLY}"
        "${OPENSMACX_LEGACY_LEAF_MANIFEST}"
        "${OPENSMACX_LEGACY_LEAF_SYMBOLS}"
    COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_extract_legacy_leaves.py"
    COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/extract_legacy_leaves.py"
        --exe "${OPENSMACX_LEGACY_EXE}"
        --output "${OPENSMACX_LEGACY_LEAF_DIR}"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/extract_legacy_leaves.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_extract_legacy_leaves.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/analysis-correlation.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/analysis-summary.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/functions.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/ghidra-functions.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/ghidra-interior-references.csv"
    USES_TERMINAL
    VERBATIM
)
add_custom_target(extract-legacy-leaves
    DEPENDS
        "${OPENSMACX_LEGACY_LEAF_ASSEMBLY}"
        "${OPENSMACX_LEGACY_LEAF_MANIFEST}"
        "${OPENSMACX_LEGACY_LEAF_SYMBOLS}"
)

if(MINGW)
    get_filename_component(OPENSMACX_LEGACY_LEAF_OBJECT_DIR
        "${OPENSMACX_LEGACY_LEAF_OBJECT}" DIRECTORY)
    add_custom_command(
        OUTPUT "${OPENSMACX_LEGACY_LEAF_OBJECT}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory
            "${OPENSMACX_LEGACY_LEAF_OBJECT_DIR}"
        COMMAND "${CMAKE_CXX_COMPILER}"
            -x assembler-with-cpp
            -c "${OPENSMACX_LEGACY_LEAF_ASSEMBLY}"
            -o "${OPENSMACX_LEGACY_LEAF_OBJECT}"
        DEPENDS "${OPENSMACX_LEGACY_LEAF_ASSEMBLY}"
        USES_TERMINAL
        VERBATIM
    )
    add_custom_target(assemble-legacy-leaves
        DEPENDS "${OPENSMACX_LEGACY_LEAF_OBJECT}"
    )
endif()

if(OPENSMACX_GAME_INSTALLER)
    set(OPENSMACX_STAGE_COMMAND
        "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/setup_game.py"
        --installer "${OPENSMACX_GAME_INSTALLER}"
        --dll "$<TARGET_FILE:OpenSMACX>"
        --output "${OPENSMACX_GAME_DIR}"
        --reuse
    )
    if(OPENSMACX_WINE)
        list(APPEND OPENSMACX_STAGE_COMMAND --wine "${OPENSMACX_WINE}")
    endif()

    add_custom_target(stage-game
        COMMAND ${OPENSMACX_STAGE_COMMAND}
        DEPENDS OpenSMACX
        USES_TERMINAL
        VERBATIM
    )
endif()
