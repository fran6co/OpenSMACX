cmake_minimum_required(VERSION 3.20)

project(OpenSMACX VERSION 0.2.3 LANGUAGES CXX)

include(CTest)

set(OPENSMACX_GAME_INSTALLER "" CACHE FILEPATH "Path to the locally owned GOG installer")
set(OPENSMACX_GAME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.opensmacx/game" CACHE PATH
    "Directory used for extracted proprietary game data")
set(OPENSMACX_PYTHON "" CACHE FILEPATH "Python interpreter with tools/requirements.txt installed")
set(OPENSMACX_WINE "" CACHE FILEPATH "Wine executable used to run Windows tools")
set(OPENSMACX_WINE_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/wineprefix"
    CACHE PATH "Dedicated owned Wine prefix used only by this build's runtime tests")
set(OPENSMACX_HYBRID_SMOKE_SECONDS 20 CACHE STRING
    "Seconds to observe a staged hybrid smoke launch")
set(OPENSMACX_GAMEPLAY_SCENARIO_SAVE "" CACHE STRING
    "Local fixture path relative to OPENSMACX_GAME_DIR")
set(OPENSMACX_GAMEPLAY_SCENARIO_VEHICLE "" CACHE STRING
    "Vehicle ID used by the deterministic gameplay scenario")
set(OPENSMACX_GAMEPLAY_SCENARIO_X "" CACHE STRING
    "Destination X coordinate used by the deterministic gameplay scenario")
set(OPENSMACX_GAMEPLAY_SCENARIO_Y "" CACHE STRING
    "Destination Y coordinate used by the deterministic gameplay scenario")
set(OPENSMACX_GAMEPLAY_SCENARIO_TIMEOUT 60 CACHE STRING
    "Seconds to wait for a deterministic gameplay scenario result")
set(OPENSMACX_LEGACY_EXE "${OPENSMACX_GAME_DIR}/terranx.exe" CACHE FILEPATH
    "User-owned executable used to prepare the local hybrid image")
set(OPENSMACX_HYBRID_DIR "${CMAKE_CURRENT_BINARY_DIR}/hybrid-image" CACHE PATH
    "Output directory for the local-only hybrid image pack")
set(OPENSMACX_HYBRID_EXE "${CMAKE_CURRENT_BINARY_DIR}/hybrid/terranx_pracx_legacy.exe"
    CACHE FILEPATH "Assembled executable path for the local hybrid image")
set(OPENSMACX_STAGED_GAME_DIR "${CMAKE_CURRENT_BINARY_DIR}/staged-game" CACHE PATH
    "Per-preset mirrored game directory used for staging and runtime tests")
set(OPENSMACX_LEGACY_LEAF_DIR "${CMAKE_CURRENT_BINARY_DIR}/legacy-leaves"
    CACHE PATH "Output directory for generated local legacy leaf assembly")
set(OPENSMACX_LEGACY_LEAF_EXE "${OPENSMACX_GAME_DIR}/terranx_original.exe"
    CACHE FILEPATH "Independently analyzed executable used to extract local legacy leaves")
set(OPENSMACX_RECOVERY_IDB
    "${CMAKE_CURRENT_SOURCE_DIR}/.opensmacx/analysis/terranx_ORIG_200_v3_7.5.SP3.idb"
    CACHE FILEPATH "Ignored hash-pinned canonical IDB used to regenerate recovery metadata")
set(OPENSMACX_LEGACY_LEAF_OBJECT
    "${CMAKE_CURRENT_BINARY_DIR}/legacy-leaves.obj"
    CACHE FILEPATH "Generated local legacy leaf COFF object")
set(OPENSMACX_RECOVERY_ORACLE_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/recovery-oracles")
set(OPENSMACX_RECOVERY_ORACLE_OBJECT
    "${CMAKE_CURRENT_BINARY_DIR}/recovery-oracles.obj")

function(opensmacx_require_local_artifact_path variable)
    get_filename_component(candidate "${${variable}}" ABSOLUTE
        BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    get_filename_component(local_root "${CMAKE_CURRENT_SOURCE_DIR}/.opensmacx" ABSOLUTE)
    get_filename_component(build_root "${CMAKE_CURRENT_SOURCE_DIR}/build" ABSOLUTE)
    set(component "${candidate}")
    while(TRUE)
        if(IS_SYMLINK "${component}")
            message(FATAL_ERROR
                "${variable} must not contain symlink components: ${component}")
        endif()
        get_filename_component(parent "${component}" DIRECTORY)
        if(parent STREQUAL component)
            break()
        endif()
        set(component "${parent}")
    endwhile()
    if(candidate STREQUAL local_root OR candidate STREQUAL build_root)
        message(FATAL_ERROR
            "${variable} must be below the ignored .opensmacx or build directory")
    endif()
    string(FIND "${candidate}/" "${local_root}/" local_prefix)
    string(FIND "${candidate}/" "${build_root}/" build_prefix)
    if(NOT local_prefix EQUAL 0 AND NOT build_prefix EQUAL 0)
        message(FATAL_ERROR
            "${variable} must be inside the ignored .opensmacx or build directory: ${candidate}")
    endif()
    set(${variable} "${candidate}" PARENT_SCOPE)
endfunction()

foreach(variable IN ITEMS
        OPENSMACX_GAME_DIR
        OPENSMACX_STAGED_GAME_DIR
        OPENSMACX_WINE_PREFIX
        OPENSMACX_HYBRID_DIR
        OPENSMACX_HYBRID_EXE
        OPENSMACX_LEGACY_LEAF_DIR
        OPENSMACX_LEGACY_LEAF_OBJECT
        OPENSMACX_RECOVERY_ORACLE_DIR
        OPENSMACX_RECOVERY_ORACLE_OBJECT)
    opensmacx_require_local_artifact_path(${variable})
endforeach()

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
    src/atexit_thunks.cpp
    src/fx.cpp
    src/vector_teardown.cpp
    src/alphanet.cpp
    src/ambience.cpp
    src/autosound.cpp
    src/base.cpp
    src/basebutton.cpp
    src/basebutton_oracle.cpp
    src/basepop.cpp
    src/basepop_font.cpp
    src/buffer.cpp
    src/buffer_oracle.cpp
    src/buttongroup.cpp
    src/caviar.cpp
    src/constructor_oracle.cpp
    src/council.cpp
    src/dialog.cpp
    src/dialogs.cpp
    src/dllmain.cpp
    src/faction.cpp
    src/filemap.cpp
    src/filemap_recovery.cpp
    src/flatbutton.cpp
    src/flic.cpp
    src/font.cpp
    src/font_recovery.cpp
    src/game.cpp
    src/general.cpp
    src/graphicwin.cpp
    src/graphicwin_oracle.cpp
    src/heap.cpp
    src/log.cpp
    src/maininterface.cpp
    src/map.cpp
    src/mapwin.cpp
    src/basewin.cpp
    src/datalink.cpp
    src/netwin.cpp
    src/socialwin.cpp
    src/designwin.cpp
    src/worldwin.cpp
    src/diplowin.cpp
    src/alphamovie.cpp
    src/checkbutton.cpp
    src/cursor.cpp
    src/infowin.cpp
    src/statuswin.cpp
    src/filewin.cpp
    src/wave.cpp
    src/battlewin.cpp
    src/councwin.cpp
    src/credits.cpp
    src/diplopop.cpp
    src/gamma.cpp
    src/messagewin.cpp
    src/newtechwin.cpp
    src/pickwin.cpp
    src/prodpicker.cpp
    src/reportwin.cpp
    src/setupwin.cpp
    src/multidebug.cpp
    src/tutwin.cpp
    src/popmenu.cpp
    src/popup.cpp
    src/netmsg.cpp
    src/radiobutton.cpp
    src/checkbox.cpp
    src/editgroup.cpp
    src/dialogs_recovery.cpp
    src/xpops.cpp
    src/planwin.cpp
    src/playerlock.cpp
    src/netdaemon.cpp
    src/console.cpp
    src/replaywin.cpp
    src/reportif.cpp
    src/dipedit.cpp
    src/sound.cpp
    src/wave_device.cpp
    src/spritebox.cpp
    src/listbox.cpp
    src/net_class.cpp
    src/squarelock.cpp
    src/deletionlist.cpp
    src/lock.cpp
    src/texture.cpp
    src/menu.cpp
    src/palette.cpp
    src/path.cpp
    src/probe.cpp
    src/pulldown.cpp
    src/random.cpp
    src/runtime_oracle.cpp
    src/scroll.cpp
    src/scroll_oracle.cpp
    src/scenario.cpp
    src/sounddevice.cpp
    src/spot.cpp
    src/spot_recovery.cpp
    src/spying_recovery.cpp
    src/sprite.cpp
    src/sprite_oracle.cpp
    src/stdafx.cpp
    src/stringbox.cpp
    src/stringstruct.cpp
    src/stringstruct_oracle.cpp
    src/text_recovery.cpp
    src/strings.cpp
    src/technology.cpp
    src/temp.cpp
    src/terraforming.cpp
    src/text.cpp
    src/textindex.cpp
    src/textindex_recovery.cpp
    src/time.cpp
    src/time_recovery.cpp
    src/veh.cpp
    src/vector.cpp
    src/win.cpp
    src/win_oracle.cpp
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
        # The canonical executable contains no C++ throw entry point and its
        # operator new returns null rather than raising, so no recovered body
        # may introduce an exception the original could not have produced.
        -fno-exceptions
        $<$<CONFIG:Debug>:-Og>
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:Release>:-fno-delete-null-pointer-checks>
        $<$<CONFIG:Release>:-fno-strict-aliasing>
    )
    target_link_options(OpenSMACX PRIVATE
        $<$<CONFIG:Release>:-s>
        -static
        -Wl,--allow-multiple-definition
    )

    set(OPENSMACX_ALPHANET_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/alphanet.cpp.obj")
    set(OPENSMACX_AUTOSOUND_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/autosound.cpp.obj")
    set(OPENSMACX_SCENARIO_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/scenario.cpp.obj")
    set(OPENSMACX_BUTTON_GROUP_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/buttongroup.cpp.obj")
    set(OPENSMACX_BASEPOP_FONT_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/basepop_font.cpp.obj")
    set(OPENSMACX_BUFFER_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/buffer.cpp.obj")
    set(OPENSMACX_BASE_BUTTON_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/basebutton.cpp.obj")
    set(OPENSMACX_BASE_BUTTON_ORACLE_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/basebutton_oracle.cpp.obj")
    set(OPENSMACX_DIALOG_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/dialog.cpp.obj")
    set(OPENSMACX_CONSTRUCTOR_ORACLE_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/constructor_oracle.cpp.obj")
    set(OPENSMACX_FONT_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/font_recovery.cpp.obj")
    set(OPENSMACX_FILEMAP_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/filemap_recovery.cpp.obj")
    set(OPENSMACX_FLAT_BUTTON_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/flatbutton.cpp.obj")
    set(OPENSMACX_HEAP_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/heap.cpp.obj")
    set(OPENSMACX_GRAPHICWIN_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/graphicwin.cpp.obj")
    set(OPENSMACX_GRAPHICWIN_ORACLE_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/graphicwin_oracle.cpp.obj")
    set(OPENSMACX_LOG_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/log.cpp.obj")
    set(OPENSMACX_MENU_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/menu.cpp.obj")
    set(OPENSMACX_PALETTE_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/palette.cpp.obj")
    set(OPENSMACX_PULLDOWN_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/pulldown.cpp.obj")
    set(OPENSMACX_RANDOM_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/random.cpp.obj")
    set(OPENSMACX_SCROLL_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/scroll.cpp.obj")
    set(OPENSMACX_SCROLL_ORACLE_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/scroll_oracle.cpp.obj")
    set(OPENSMACX_RUNTIME_ORACLE_OBJECT_FILE
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/runtime_oracle.cpp.obj")
    set(OPENSMACX_STRING_STRUCT_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/stringstruct.cpp.obj")
    set(OPENSMACX_SPOT_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/spot_recovery.cpp.obj")
    set(OPENSMACX_STRINGS_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/strings.cpp.obj")
    set(OPENSMACX_TEXT_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/text_recovery.cpp.obj")
    set(OPENSMACX_TEXT_INDEX_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/textindex_recovery.cpp.obj")
    set(OPENSMACX_TIME_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/time_recovery.cpp.obj")
    set(OPENSMACX_WIN_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/win.cpp.obj")
    set(OPENSMACX_WIN_ORACLE_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/win_oracle.cpp.obj")
    set(OPENSMACX_VECTOR_OBJECT
        "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/vector.cpp.obj")
    add_custom_target(verify-recovery-abi
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_recovery_abi.py"
            --nm "${CMAKE_NM}"
            --objdump "${CMAKE_OBJDUMP}"
            --object "${OPENSMACX_ALPHANET_OBJECT}"
            --autosound-object "${OPENSMACX_AUTOSOUND_OBJECT}"
            --scenario-object "${OPENSMACX_SCENARIO_OBJECT}"
            --button-group-object "${OPENSMACX_BUTTON_GROUP_OBJECT}"
            --basepop-font-object "${OPENSMACX_BASEPOP_FONT_OBJECT}"
            --buffer-object "${OPENSMACX_BUFFER_OBJECT}"
            --base-button-object "${OPENSMACX_BASE_BUTTON_OBJECT}"
            --base-button-oracle-object "${OPENSMACX_BASE_BUTTON_ORACLE_OBJECT}"
            --dialog-object "${OPENSMACX_DIALOG_OBJECT}"
            --constructor-oracle-object "${OPENSMACX_CONSTRUCTOR_ORACLE_OBJECT}"
            --font-object "${OPENSMACX_FONT_OBJECT}"
            --filemap-object "${OPENSMACX_FILEMAP_OBJECT}"
            --flat-button-object "${OPENSMACX_FLAT_BUTTON_OBJECT}"
            --heap-object "${OPENSMACX_HEAP_OBJECT}"
            --graphicwin-object "${OPENSMACX_GRAPHICWIN_OBJECT}"
            --graphicwin-oracle-object "${OPENSMACX_GRAPHICWIN_ORACLE_OBJECT}"
            --log-object "${OPENSMACX_LOG_OBJECT}"
            --menu-object "${OPENSMACX_MENU_OBJECT}"
            --palette-object "${OPENSMACX_PALETTE_OBJECT}"
            --pulldown-object "${OPENSMACX_PULLDOWN_OBJECT}"
            --random-object "${OPENSMACX_RANDOM_OBJECT}"
            --scroll-object "${OPENSMACX_SCROLL_OBJECT}"
            --scroll-oracle-object "${OPENSMACX_SCROLL_ORACLE_OBJECT}"
            --runtime-oracle-object "${OPENSMACX_RUNTIME_ORACLE_OBJECT_FILE}"
            --string-struct-object "${OPENSMACX_STRING_STRUCT_OBJECT}"
            --spot-object "${OPENSMACX_SPOT_OBJECT}"
            --strings-object "${OPENSMACX_STRINGS_OBJECT}"
            --text-object "${OPENSMACX_TEXT_OBJECT}"
            --text-index-object "${OPENSMACX_TEXT_INDEX_OBJECT}"
            --time-object "${OPENSMACX_TIME_OBJECT}"
            --vector-object "${OPENSMACX_VECTOR_OBJECT}"
            --win-object "${OPENSMACX_WIN_OBJECT}"
            --win-oracle-object "${OPENSMACX_WIN_ORACLE_OBJECT}"
        DEPENDS OpenSMACX
        USES_TERMINAL
        VERBATIM
    )
elseif(MSVC)
    target_compile_options(OpenSMACX PRIVATE /W4)
endif()

if(BUILD_TESTING)
    add_executable(recovery-leaf-tests
        tests/recovery_leaf_tests.cpp
        src/atexit_thunks.cpp
        src/fx.cpp
        src/vector_teardown.cpp
        src/alphanet.cpp
        src/basepop.cpp
        src/ambience.cpp
        src/autosound.cpp
        src/basebutton.cpp
        src/basepop_font.cpp
        src/buffer.cpp
        src/buttongroup.cpp
        src/dialog.cpp
        src/filemap_recovery.cpp
        src/flatbutton.cpp
        src/flic.cpp
        src/font_recovery.cpp
        src/graphicwin.cpp
        src/log.cpp
        src/maininterface.cpp
        src/mapwin.cpp
        src/basewin.cpp
        src/datalink.cpp
        src/netwin.cpp
        src/socialwin.cpp
        src/designwin.cpp
        src/worldwin.cpp
        src/diplowin.cpp
        src/alphamovie.cpp
        src/checkbutton.cpp
        src/cursor.cpp
        src/infowin.cpp
        src/statuswin.cpp
        src/filewin.cpp
        src/wave.cpp
        src/battlewin.cpp
        src/councwin.cpp
        src/credits.cpp
        src/diplopop.cpp
        src/gamma.cpp
        src/messagewin.cpp
        src/newtechwin.cpp
        src/pickwin.cpp
        src/prodpicker.cpp
        src/reportwin.cpp
        src/setupwin.cpp
        src/menu.cpp
        src/palette.cpp
        src/pulldown.cpp
        src/random.cpp
        src/scroll.cpp
        src/sounddevice.cpp
        src/spot_recovery.cpp
        src/spying_recovery.cpp
        src/sprite.cpp
        src/stringbox.cpp
        src/stringstruct.cpp
        src/text_recovery.cpp
        src/textindex_recovery.cpp
        src/time_recovery.cpp
        src/vector.cpp
        src/caviar.cpp
        src/multidebug.cpp
        src/tutwin.cpp
            src/popmenu.cpp
        src/popup.cpp
        src/netmsg.cpp
        src/radiobutton.cpp
        src/checkbox.cpp
        src/editgroup.cpp
        src/dialogs_recovery.cpp
        src/xpops.cpp
        src/planwin.cpp
        src/playerlock.cpp
        src/netdaemon.cpp
        src/console.cpp
        src/replaywin.cpp
        src/reportif.cpp
        src/dipedit.cpp
        src/sound.cpp
        src/wave_device.cpp
        src/spritebox.cpp
        src/listbox.cpp
        src/net_class.cpp
        src/squarelock.cpp
        src/deletionlist.cpp
        src/lock.cpp
        src/texture.cpp
        src/win.cpp
    )
    target_compile_features(recovery-leaf-tests PRIVATE cxx_std_11)
    target_compile_options(recovery-leaf-tests PRIVATE -fno-exceptions)
    target_compile_definitions(recovery-leaf-tests PRIVATE
        BUILD_DLL OPENSMACX_SPOT_MINIMAL_INIT)
    set_target_properties(recovery-leaf-tests PROPERTIES
        CXX_EXTENSIONS OFF
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
    )

    if(MINGW)
        target_compile_options(recovery-leaf-tests PRIVATE
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
            $<$<CONFIG:Debug>:-Og>
            $<$<CONFIG:Release>:-O2>
        )
        target_link_options(recovery-leaf-tests PRIVATE
            -static
            -Wl,--wrap=atexit
            -Wl,--wrap=fclose
            -Wl,--wrap=free)
    endif()

    if(CMAKE_CROSSCOMPILING)
        set(OPENSMACX_RECOVERY_TEST_COMMAND
            "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/run_windows_test.py"
            --executable "$<TARGET_FILE:recovery-leaf-tests>"
            --wine-prefix "${OPENSMACX_WINE_PREFIX}"
        )
        if(OPENSMACX_WINE)
            list(APPEND OPENSMACX_RECOVERY_TEST_COMMAND
                --wine "${OPENSMACX_WINE}")
        endif()
        add_test(NAME recovery-leaf-tests
            COMMAND ${OPENSMACX_RECOVERY_TEST_COMMAND}
        )
    else()
        add_test(NAME recovery-leaf-tests COMMAND recovery-leaf-tests)
    endif()

    # Direct-source tests for the gameplay core.
    #
    # These cannot live in recovery-leaf-tests: that binary defines file-scope
    # stand-ins for map.cpp and general.cpp symbols - a scriptable mem_get,
    # call-recording lock_map/unlock_map, xrange, purge_spaces - which its own
    # fixtures assert against, so linking the real translation units there
    # collides with every one of those observers. Splitting by link closure
    # rather than by subject gives the ~1,800 gameplay free functions a home
    # without disturbing the existing suite.
    #
    # The closure below is self-satisfying: the support classes are present
    # because the gameplay tree calls into them, and the only symbols it does
    # not resolve internally are Win32 imports (gdi32, winmm) and the deferred
    # oracle entry point, which the test file stubs.
    add_executable(recovery-gameplay-tests
        tests/recovery_gameplay_tests.cpp
        src/alpha.cpp
        src/vector_teardown.cpp
        src/base.cpp
        src/basebutton.cpp
        src/buffer.cpp
        src/buttongroup.cpp
        src/council.cpp
        src/faction.cpp
        src/filemap.cpp
        src/filemap_recovery.cpp
        src/font.cpp
        src/font_recovery.cpp
        src/game.cpp
        src/general.cpp
        src/graphicwin.cpp
        src/heap.cpp
        src/log.cpp
        src/maininterface.cpp
        src/map.cpp
        src/palette.cpp
        src/path.cpp
        src/probe.cpp
        src/random.cpp
        src/scenario.cpp
        src/scroll.cpp
        src/spot.cpp
        src/spot_recovery.cpp
        src/sprite.cpp
        src/strings.cpp
        src/technology.cpp
        src/temp.cpp
        src/terraforming.cpp
        src/text.cpp
        src/text_recovery.cpp
        src/textindex.cpp
        src/textindex_recovery.cpp
        src/time.cpp
        src/time_recovery.cpp
        src/veh.cpp
        src/win.cpp
        src/autosound.cpp
    )
    target_compile_features(recovery-gameplay-tests PRIVATE cxx_std_11)
    target_compile_options(recovery-gameplay-tests PRIVATE -fno-exceptions)
    target_compile_definitions(recovery-gameplay-tests PRIVATE BUILD_DLL)
    set_target_properties(recovery-gameplay-tests PROPERTIES
        CXX_EXTENSIONS OFF
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
    )

    if(MINGW)
        target_compile_options(recovery-gameplay-tests PRIVATE
            -Wall
            -Wextra
            -Wno-comment
            -Wno-parentheses
            -Wno-unknown-pragmas
            $<$<CONFIG:Debug>:-Og>
            $<$<CONFIG:Release>:-O2>
        )
        target_link_options(recovery-gameplay-tests PRIVATE -static)
        target_link_libraries(recovery-gameplay-tests PRIVATE gdi32 winmm)
    endif()

    if(CMAKE_CROSSCOMPILING)
        set(OPENSMACX_GAMEPLAY_TEST_COMMAND
            "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/run_windows_test.py"
            --executable "$<TARGET_FILE:recovery-gameplay-tests>"
            --wine-prefix "${OPENSMACX_WINE_PREFIX}"
        )
        if(OPENSMACX_WINE)
            list(APPEND OPENSMACX_GAMEPLAY_TEST_COMMAND
                --wine "${OPENSMACX_WINE}")
        endif()
        add_test(NAME recovery-gameplay-tests
            COMMAND ${OPENSMACX_GAMEPLAY_TEST_COMMAND}
        )
    else()
        add_test(NAME recovery-gameplay-tests COMMAND recovery-gameplay-tests)
    endif()

    if(MINGW)
        add_test(NAME recovery-abi
            COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_recovery_abi.py"
                --nm "${CMAKE_NM}"
                --objdump "${CMAKE_OBJDUMP}"
                --object "${OPENSMACX_ALPHANET_OBJECT}"
                --autosound-object "${OPENSMACX_AUTOSOUND_OBJECT}"
                --scenario-object "${OPENSMACX_SCENARIO_OBJECT}"
                --button-group-object "${OPENSMACX_BUTTON_GROUP_OBJECT}"
                --basepop-font-object "${OPENSMACX_BASEPOP_FONT_OBJECT}"
                --buffer-object "${OPENSMACX_BUFFER_OBJECT}"
                --base-button-object "${OPENSMACX_BASE_BUTTON_OBJECT}"
                --base-button-oracle-object "${OPENSMACX_BASE_BUTTON_ORACLE_OBJECT}"
                --dialog-object "${OPENSMACX_DIALOG_OBJECT}"
                --constructor-oracle-object "${OPENSMACX_CONSTRUCTOR_ORACLE_OBJECT}"
                --font-object "${OPENSMACX_FONT_OBJECT}"
                --filemap-object "${OPENSMACX_FILEMAP_OBJECT}"
                --flat-button-object "${OPENSMACX_FLAT_BUTTON_OBJECT}"
                --heap-object "${OPENSMACX_HEAP_OBJECT}"
                --graphicwin-object "${OPENSMACX_GRAPHICWIN_OBJECT}"
                --graphicwin-oracle-object "${OPENSMACX_GRAPHICWIN_ORACLE_OBJECT}"
                --log-object "${OPENSMACX_LOG_OBJECT}"
                --menu-object "${OPENSMACX_MENU_OBJECT}"
                --palette-object "${OPENSMACX_PALETTE_OBJECT}"
                --pulldown-object "${OPENSMACX_PULLDOWN_OBJECT}"
                --random-object "${OPENSMACX_RANDOM_OBJECT}"
                --scroll-object "${OPENSMACX_SCROLL_OBJECT}"
                --scroll-oracle-object "${OPENSMACX_SCROLL_ORACLE_OBJECT}"
                --runtime-oracle-object "${OPENSMACX_RUNTIME_ORACLE_OBJECT_FILE}"
                --string-struct-object "${OPENSMACX_STRING_STRUCT_OBJECT}"
                --spot-object "${OPENSMACX_SPOT_OBJECT}"
                --strings-object "${OPENSMACX_STRINGS_OBJECT}"
                --text-object "${OPENSMACX_TEXT_OBJECT}"
                --text-index-object "${OPENSMACX_TEXT_INDEX_OBJECT}"
                --time-object "${OPENSMACX_TIME_OBJECT}"
                --vector-object "${OPENSMACX_VECTOR_OBJECT}"
                --win-object "${OPENSMACX_WIN_OBJECT}"
                --win-oracle-object "${OPENSMACX_WIN_ORACLE_OBJECT}"
        )
    endif()

    add_test(NAME hybrid-smoke-tool-tests
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_smoke_hybrid_game.py"
    )

    add_test(NAME gameplay-scenario-tool-tests
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_run_gameplay_scenario.py"
    )

    add_test(NAME external-analysis-tool-tests
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_external_analysis.py"
    )

    add_test(NAME recovery-metadata-tool-tests
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_verify_recovery_metadata.py"
    )

    add_test(NAME stage-preset-game-tool-tests
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_stage_preset_game_dir.py"
    )

    add_test(NAME redirect-signature-tool-tests
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_generate_redirect_signatures.py"
    )

    add_test(NAME batch-decompile-tool-tests
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_batch_decompile.py"
    )

    add_test(NAME mutation-harness-tool-tests
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_mutate_and_verify.py"
    )

    add_test(NAME shape-classifier-tool-tests
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_classify_recovery_shapes.py"
    )

    add_test(NAME add-redirect-tool-tests
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_add_redirect.py"
    )

    add_test(NAME bulk-recover-stubs-tool-tests
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_bulk_recover_stubs.py"
    )

    add_test(NAME constant-return-scan-tool-tests
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_find_constant_returns.py"
    )

    add_test(NAME class-layout-tool-tests
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_derive_class_layout.py"
    )

    add_test(NAME disassembly-tool-tests
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_disasm.py"
    )

    add_test(NAME delegate-analysis-tool-tests
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_analyze_delegates.py"
    )
endif()

install(TARGETS OpenSMACX
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
    ARCHIVE DESTINATION lib
)

set(OPENSMACX_METADATA_VERIFY_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/recovery-metadata-verify")
add_custom_target(verify-recovery-metadata
    COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_recovery_metadata.py"
        --idb "${OPENSMACX_RECOVERY_IDB}"
        --verify-dir "${OPENSMACX_METADATA_VERIFY_DIR}"
    DEPENDS
        "${OPENSMACX_RECOVERY_IDB}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_recovery_metadata.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/export_recovery_inventory.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/correlate_recovery_analyses.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery-overrides.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery-binding-classifications.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/ida9-functions.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/ghidra-functions.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/ghidra-interior-references.csv"
    USES_TERMINAL
    VERBATIM
)

# Every recovery invalidates the committed metadata by construction, so the
# unpromoted gate is guaranteed to fail its first pass on any batch that landed
# a body. Running this first regenerates, copies over docs/recovery/, and still
# performs the byte comparison, which turns the standing two-pass cycle into
# one. It is deliberately NOT a dependency of verify-recovery-batch: the gate
# must keep failing on drift for anyone who has not consciously promoted.
add_custom_target(promote-recovery-metadata
    COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_recovery_metadata.py"
        --idb "${OPENSMACX_RECOVERY_IDB}"
        --verify-dir "${OPENSMACX_METADATA_VERIFY_DIR}"
        --promote
    DEPENDS
        "${OPENSMACX_RECOVERY_IDB}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_recovery_metadata.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/export_recovery_inventory.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/correlate_recovery_analyses.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery-overrides.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery-binding-classifications.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/ida9-functions.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/ghidra-functions.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/ghidra-interior-references.csv"
    USES_TERMINAL
    VERBATIM
)

add_custom_target(prepare-hybrid-image
    COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_prepare_hybrid_image.py"
    COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/prepare_hybrid_image.py"
        --exe "${OPENSMACX_LEGACY_EXE}"
        --output "${OPENSMACX_HYBRID_DIR}"
    DEPENDS
        verify-recovery-metadata
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/prepare_hybrid_image.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_prepare_hybrid_image.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/functions.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/priorities.csv"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/summary.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery-binding-classifications.csv"
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

add_custom_target(stage-preset-game
    COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_stage_preset_game_dir.py"
    COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/stage_preset_game_dir.py"
        --source "${OPENSMACX_GAME_DIR}"
        --destination "${OPENSMACX_STAGED_GAME_DIR}"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/local_artifact.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/stage_preset_game_dir.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_stage_preset_game_dir.py"
    USES_TERMINAL
    VERBATIM
)

set(OPENSMACX_HYBRID_STAGE_COMMAND
    "${OPENSMACX_PYTHON_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/opensmac_patcher.py"
    --exe "${OPENSMACX_HYBRID_EXE}"
    --dll "$<TARGET_FILE:OpenSMACX>"
    --output "${OPENSMACX_STAGED_GAME_DIR}"
    --exe-name terranx_hybrid.exe
    --wine-prefix "${OPENSMACX_WINE_PREFIX}"
)
if(OPENSMACX_WINE)
    list(APPEND OPENSMACX_HYBRID_STAGE_COMMAND --wine "${OPENSMACX_WINE}")
endif()

add_custom_target(stage-hybrid-game
    COMMAND ${OPENSMACX_HYBRID_STAGE_COMMAND}
    DEPENDS
        OpenSMACX
        assemble-hybrid-executable
        stage-preset-game
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/local_artifact.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/opensmac_patcher.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/owned_wine_prefix.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/wine_runtime.py"
    USES_TERMINAL
    VERBATIM
)

set(OPENSMACX_HYBRID_SMOKE_COMMAND
    "${OPENSMACX_PYTHON_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/smoke_hybrid_game.py"
    --game-dir "${OPENSMACX_STAGED_GAME_DIR}"
    --executable terranx_hybrid.exe
    --wine-prefix "${OPENSMACX_WINE_PREFIX}"
    --duration "${OPENSMACX_HYBRID_SMOKE_SECONDS}"
    --log "${CMAKE_CURRENT_BINARY_DIR}/hybrid-smoke.log"
    --result "${CMAKE_CURRENT_BINARY_DIR}/hybrid-smoke-result.json"
    --oracle-result "${CMAKE_CURRENT_BINARY_DIR}/hybrid-runtime-oracle-result.txt"
)
if(OPENSMACX_WINE)
    list(APPEND OPENSMACX_HYBRID_SMOKE_COMMAND --wine "${OPENSMACX_WINE}")
endif()

add_custom_target(smoke-hybrid-game
    COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_smoke_hybrid_game.py"
    COMMAND ${OPENSMACX_HYBRID_SMOKE_COMMAND}
    DEPENDS
        stage-hybrid-game
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/smoke_hybrid_game.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_smoke_hybrid_game.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/owned_wine_prefix.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/runtime_process.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/run_game.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/setup_game.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/wine_runtime.py"
    USES_TERMINAL
    VERBATIM
)

set(OPENSMACX_GAMEPLAY_SCENARIO_COMMAND
    "${OPENSMACX_PYTHON_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/run_gameplay_scenario.py"
    --game-dir "${OPENSMACX_STAGED_GAME_DIR}"
    --executable terranx_hybrid.exe
    --save "${OPENSMACX_GAMEPLAY_SCENARIO_SAVE}"
    --result "${CMAKE_CURRENT_BINARY_DIR}/gameplay-scenario-result.json"
    --log "${CMAKE_CURRENT_BINARY_DIR}/gameplay-scenario.log"
    --wine-prefix "${OPENSMACX_WINE_PREFIX}"
    --timeout "${OPENSMACX_GAMEPLAY_SCENARIO_TIMEOUT}"
)
if(OPENSMACX_WINE)
    list(APPEND OPENSMACX_GAMEPLAY_SCENARIO_COMMAND --wine "${OPENSMACX_WINE}")
endif()

add_custom_target(inspect-gameplay-scenario
    COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_run_gameplay_scenario.py"
    COMMAND ${OPENSMACX_GAMEPLAY_SCENARIO_COMMAND} --inspect
    DEPENDS
        stage-hybrid-game
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/run_gameplay_scenario.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_run_gameplay_scenario.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/owned_wine_prefix.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/runtime_process.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/wine_runtime.py"
    USES_TERMINAL
    VERBATIM
)

add_custom_target(run-gameplay-scenario
    COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_run_gameplay_scenario.py"
    COMMAND ${OPENSMACX_GAMEPLAY_SCENARIO_COMMAND}
        --vehicle "${OPENSMACX_GAMEPLAY_SCENARIO_VEHICLE}"
        --x "${OPENSMACX_GAMEPLAY_SCENARIO_X}"
        --y "${OPENSMACX_GAMEPLAY_SCENARIO_Y}"
    DEPENDS
        stage-hybrid-game
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/run_gameplay_scenario.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_run_gameplay_scenario.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/owned_wine_prefix.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/runtime_process.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/wine_runtime.py"
    USES_TERMINAL
    VERBATIM
)

add_custom_target(run-gameplay-resolution
    COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_run_gameplay_scenario.py"
    COMMAND ${OPENSMACX_GAMEPLAY_SCENARIO_COMMAND}
        --vehicle "${OPENSMACX_GAMEPLAY_SCENARIO_VEHICLE}"
        --x "${OPENSMACX_GAMEPLAY_SCENARIO_X}"
        --y "${OPENSMACX_GAMEPLAY_SCENARIO_Y}"
        --resolve
    DEPENDS
        stage-hybrid-game
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/run_gameplay_scenario.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_run_gameplay_scenario.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/owned_wine_prefix.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/runtime_process.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/wine_runtime.py"
    USES_TERMINAL
    VERBATIM
)

add_custom_target(run-gameplay-advancement
    COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_run_gameplay_scenario.py"
    COMMAND ${OPENSMACX_GAMEPLAY_SCENARIO_COMMAND}
        --vehicle "${OPENSMACX_GAMEPLAY_SCENARIO_VEHICLE}"
        --x "${OPENSMACX_GAMEPLAY_SCENARIO_X}"
        --y "${OPENSMACX_GAMEPLAY_SCENARIO_Y}"
        --advance
    DEPENDS
        stage-hybrid-game
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/run_gameplay_scenario.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_run_gameplay_scenario.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/owned_wine_prefix.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/runtime_process.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/wine_runtime.py"
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
        --exe "${OPENSMACX_LEGACY_LEAF_EXE}"
        --output "${OPENSMACX_LEGACY_LEAF_DIR}"
    DEPENDS
        "${OPENSMACX_LEGACY_LEAF_EXE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/extract_legacy_leaves.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/local_artifact.py"
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

    if(BUILD_TESTING)
        set(OPENSMACX_RECOVERY_ORACLE_ASSEMBLY
            "${OPENSMACX_RECOVERY_ORACLE_DIR}/legacy-leaves.S")
        set(OPENSMACX_RECOVERY_ORACLE_MANIFEST
            "${OPENSMACX_RECOVERY_ORACLE_DIR}/manifest.json")
        set(OPENSMACX_RECOVERY_ORACLE_SYMBOLS
            "${OPENSMACX_RECOVERY_ORACLE_DIR}/symbols.csv")
        add_custom_command(
            OUTPUT
                "${OPENSMACX_RECOVERY_ORACLE_ASSEMBLY}"
                "${OPENSMACX_RECOVERY_ORACLE_MANIFEST}"
                "${OPENSMACX_RECOVERY_ORACLE_SYMBOLS}"
            COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_extract_legacy_leaves.py"
            COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/extract_legacy_leaves.py"
                --exe "${OPENSMACX_LEGACY_LEAF_EXE}"
                --output "${OPENSMACX_RECOVERY_ORACLE_DIR}"
                --max-size 1024
                --address 0x004E2610
                --address 0x004E2660
                --address 0x004E26B0
                --address 0x004BA830
                --address 0x00625750
                --address 0x00625770
                --address 0x005ED7D0
                --address 0x005EE0F0
                --address 0x005EE110
                --address 0x00605B10
                --address 0x00605BE0
                --address 0x00605C30
                --address 0x00606C50
                --address 0x00606F00
                --address 0x005F8CB0
                --address 0x005F8D20
                --address 0x005F8D90
                --address 0x005F8DF0
                --address 0x005F9040
                --address 0x005F90A0
                --address 0x005F9F40
                --address 0x005FB820
                --address 0x005FB990
                --address 0x005F86C0
                --address 0x005FA7A0
                --address 0x006343C0
                --address 0x006343D0
                --address 0x00634430
                --address 0x00634480
                --address 0x006344B0
                --address 0x00634670
                --address 0x00628150
            DEPENDS
                "${OPENSMACX_LEGACY_LEAF_EXE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/extract_legacy_leaves.py"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/local_artifact.py"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_extract_legacy_leaves.py"
                "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/analysis-correlation.csv"
                "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/analysis-summary.json"
                "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/functions.csv"
                "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/ghidra-functions.csv"
                "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/ghidra-interior-references.csv"
            USES_TERMINAL
            VERBATIM
        )
        add_custom_command(
            OUTPUT "${OPENSMACX_RECOVERY_ORACLE_OBJECT}"
            COMMAND "${CMAKE_CXX_COMPILER}"
                -x assembler-with-cpp
                -c "${OPENSMACX_RECOVERY_ORACLE_ASSEMBLY}"
                -o "${OPENSMACX_RECOVERY_ORACLE_OBJECT}"
            DEPENDS "${OPENSMACX_RECOVERY_ORACLE_ASSEMBLY}"
            USES_TERMINAL
            VERBATIM
        )
        set_source_files_properties("${OPENSMACX_RECOVERY_ORACLE_OBJECT}"
            PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)
        add_executable(recovery-oracle-tests EXCLUDE_FROM_ALL
            tests/recovery_oracle_tests.cpp
            src/alphanet.cpp
            src/menu.cpp
            src/pulldown.cpp
            src/random.cpp
            src/scroll.cpp
            src/vector.cpp
            src/win.cpp
            # The PullDown destructor delegates to the GraphicWin destructor,
            # which pulls in the Buffer teardown it forwards to, which in turn
            # needs the Spot and Font objects Buffer owns.
            src/graphicwin.cpp
            src/buffer.cpp
            src/spot_recovery.cpp
            src/font_recovery.cpp
            # FontQueue's teardown dispatches through the vector iterator seam.
            src/vector_teardown.cpp
            # The recovered constructors extend that closure: Win::construct
            # builds its AutoSound member and Buffer::construct resolves its
            # default colours through Palette::get_rgbquad.
            src/autosound.cpp
            src/palette.cpp
            "${OPENSMACX_RECOVERY_ORACLE_OBJECT}"
        )
        target_compile_features(recovery-oracle-tests PRIVATE cxx_std_11)
        target_compile_options(recovery-oracle-tests PRIVATE -fno-exceptions)
        target_compile_definitions(recovery-oracle-tests PRIVATE
            BUILD_DLL OPENSMACX_SPOT_MINIMAL_INIT)
        target_compile_options(recovery-oracle-tests PRIVATE
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
            $<$<CONFIG:Debug>:-Og>
            $<$<CONFIG:Release>:-O2>
        )
        target_link_options(recovery-oracle-tests PRIVATE -static)

        set(OPENSMACX_RECOVERY_ORACLE_COMMAND
            "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/run_windows_test.py"
            --executable "$<TARGET_FILE:recovery-oracle-tests>"
            --wine-prefix "${OPENSMACX_WINE_PREFIX}"
        )
        if(OPENSMACX_WINE)
            list(APPEND OPENSMACX_RECOVERY_ORACLE_COMMAND
                --wine "${OPENSMACX_WINE}")
        endif()
        add_custom_target(verify-recovery-oracles
            COMMAND ${OPENSMACX_RECOVERY_ORACLE_COMMAND}
            DEPENDS recovery-oracle-tests
            USES_TERMINAL
            VERBATIM
        )
        # Also registered as a CTest so tools that drive suites through ctest
        # - the mutation harness in particular - can measure this suite's
        # coverage instead of arguing about it. ctest does not build, so
        # harness runs pair it with `--target recovery-oracle-tests`.
        add_test(NAME recovery-oracle COMMAND ${OPENSMACX_RECOVERY_ORACLE_COMMAND})

        set(OPENSMACX_STATIC_RECOMPILE_DIR
            "${CMAKE_CURRENT_BINARY_DIR}/static-recompile-pilot")
        set(OPENSMACX_STATIC_RECOMPILE_ORIGINAL_DIR
            "${OPENSMACX_STATIC_RECOMPILE_DIR}/original")
        set(OPENSMACX_STATIC_RECOMPILE_GENERATED_DIR
            "${OPENSMACX_STATIC_RECOMPILE_DIR}/generated")
        set(OPENSMACX_STATIC_RECOMPILE_ORIGINAL_ASSEMBLY
            "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_DIR}/legacy-leaves.S")
        set(OPENSMACX_STATIC_RECOMPILE_ORIGINAL_MANIFEST
            "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_DIR}/manifest.json")
        set(OPENSMACX_STATIC_RECOMPILE_ORIGINAL_SYMBOLS
            "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_DIR}/symbols.csv")
        set(OPENSMACX_STATIC_RECOMPILE_ORIGINAL_OBJECT
            "${OPENSMACX_STATIC_RECOMPILE_DIR}/original.obj")
        set(OPENSMACX_STATIC_RECOMPILE_GENERATED_SOURCE
            "${OPENSMACX_STATIC_RECOMPILE_GENERATED_DIR}/static-recompiled-004C5460.cpp")
        set(OPENSMACX_STATIC_RECOMPILE_MANIFEST
            "${OPENSMACX_STATIC_RECOMPILE_GENERATED_DIR}/manifest.json")

        add_custom_command(
            OUTPUT
                "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_ASSEMBLY}"
                "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_MANIFEST}"
                "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_SYMBOLS}"
            COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/extract_legacy_leaves.py"
                --exe "${OPENSMACX_LEGACY_LEAF_EXE}"
                --output "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_DIR}"
                --max-size 64
                --address 0x004C5460
            DEPENDS
                "${OPENSMACX_LEGACY_LEAF_EXE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/extract_legacy_leaves.py"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/local_artifact.py"
                "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/analysis-correlation.csv"
                "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/analysis-summary.json"
                "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/functions.csv"
                "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/ghidra-functions.csv"
                "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/ghidra-interior-references.csv"
            USES_TERMINAL
            VERBATIM
        )
        add_custom_command(
            OUTPUT "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_OBJECT}"
            COMMAND "${CMAKE_CXX_COMPILER}"
                -x assembler-with-cpp
                -c "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_ASSEMBLY}"
                -o "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_OBJECT}"
            DEPENDS "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_ASSEMBLY}"
            USES_TERMINAL
            VERBATIM
        )
        add_custom_command(
            OUTPUT
                "${OPENSMACX_STATIC_RECOMPILE_GENERATED_SOURCE}"
                "${OPENSMACX_STATIC_RECOMPILE_MANIFEST}"
            COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_static_recompile_pilot.py"
            COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/static_recompile_pilot.py"
                --exe "${OPENSMACX_LEGACY_LEAF_EXE}"
                --oracle-manifest "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_MANIFEST}"
                --output "${OPENSMACX_STATIC_RECOMPILE_GENERATED_DIR}"
            DEPENDS
                "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_MANIFEST}"
                "${OPENSMACX_LEGACY_LEAF_EXE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/local_artifact.py"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/static_recompile_pilot.py"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/static_recompile_runtime.h"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_static_recompile_pilot.py"
            USES_TERMINAL
            VERBATIM
        )
        add_custom_target(generate-static-recompile-pilot
            DEPENDS
                "${OPENSMACX_STATIC_RECOMPILE_GENERATED_SOURCE}"
                "${OPENSMACX_STATIC_RECOMPILE_MANIFEST}"
        )

        set_source_files_properties(
            "${OPENSMACX_STATIC_RECOMPILE_GENERATED_SOURCE}"
            PROPERTIES GENERATED TRUE)
        add_library(static-recompile-pilot-generated OBJECT EXCLUDE_FROM_ALL
            "${OPENSMACX_STATIC_RECOMPILE_GENERATED_SOURCE}")
        target_compile_features(static-recompile-pilot-generated PRIVATE cxx_std_11)
        target_include_directories(static-recompile-pilot-generated PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/tools")
        target_compile_options(static-recompile-pilot-generated PRIVATE
            -pedantic
            -Wall
            -Wextra
            -Wshadow
            -Wcast-align
            -Wfloat-conversion
            -Wuseless-cast
            -Wno-attributes
            $<$<CONFIG:Debug>:-Og>
            $<$<CONFIG:Release>:-O2>
        )

        set_source_files_properties("${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_OBJECT}"
            PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)
        add_executable(static-recompile-pilot-standalone-tests EXCLUDE_FROM_ALL
            tests/static_recompile_pilot_tests.cpp
            $<TARGET_OBJECTS:static-recompile-pilot-generated>)
        target_compile_features(
            static-recompile-pilot-standalone-tests PRIVATE cxx_std_11)
        target_compile_definitions(static-recompile-pilot-standalone-tests PRIVATE
            OPENSMACX_STATIC_RECOMPILE_STANDALONE=1)
        target_include_directories(static-recompile-pilot-standalone-tests PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/tools")
        target_compile_options(static-recompile-pilot-standalone-tests PRIVATE
            -pedantic -Wall -Wextra -Wshadow -Wcast-align -Wfloat-conversion
            -Wuseless-cast -Wno-attributes
            $<$<CONFIG:Debug>:-Og> $<$<CONFIG:Release>:-O2>)
        target_link_options(static-recompile-pilot-standalone-tests PRIVATE -static)

        add_executable(static-recompile-pilot-tests EXCLUDE_FROM_ALL
            tests/static_recompile_pilot_tests.cpp
            "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_OBJECT}"
            $<TARGET_OBJECTS:static-recompile-pilot-generated>)
        target_compile_features(static-recompile-pilot-tests PRIVATE cxx_std_11)
        target_include_directories(static-recompile-pilot-tests PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/tools")
        target_compile_options(static-recompile-pilot-tests PRIVATE
            -pedantic -Wall -Wextra -Wshadow -Wcast-align -Wfloat-conversion
            -Wuseless-cast -Wno-attributes
            $<$<CONFIG:Debug>:-Og> $<$<CONFIG:Release>:-O2>)
        target_link_options(static-recompile-pilot-tests PRIVATE -static)

        set(OPENSMACX_STATIC_RECOMPILE_STANDALONE_COMMAND
            "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/run_windows_test.py"
            --executable "$<TARGET_FILE:static-recompile-pilot-standalone-tests>"
            --wine-prefix "${OPENSMACX_WINE_PREFIX}")
        set(OPENSMACX_STATIC_RECOMPILE_DIFFERENTIAL_COMMAND
            "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/run_windows_test.py"
            --executable "$<TARGET_FILE:static-recompile-pilot-tests>"
            --wine-prefix "${OPENSMACX_WINE_PREFIX}")
        if(OPENSMACX_WINE)
            list(APPEND OPENSMACX_STATIC_RECOMPILE_STANDALONE_COMMAND
                --wine "${OPENSMACX_WINE}")
            list(APPEND OPENSMACX_STATIC_RECOMPILE_DIFFERENTIAL_COMMAND
                --wine "${OPENSMACX_WINE}")
        endif()
        add_custom_target(verify-static-recompile-pilot
            COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_static_recompile_pilot.py"
                --nm "${CMAKE_NM}"
                --objdump "${CMAKE_OBJDUMP}"
                --compiler "${CMAKE_CXX_COMPILER}"
                --object "$<TARGET_OBJECTS:static-recompile-pilot-generated>"
                --original-object "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_OBJECT}"
                --exe "${OPENSMACX_LEGACY_LEAF_EXE}"
                --oracle-manifest "${OPENSMACX_STATIC_RECOMPILE_ORIGINAL_MANIFEST}"
                --manifest "${OPENSMACX_STATIC_RECOMPILE_MANIFEST}"
            COMMAND ${OPENSMACX_STATIC_RECOMPILE_STANDALONE_COMMAND}
            COMMAND ${OPENSMACX_STATIC_RECOMPILE_DIFFERENTIAL_COMMAND}
            DEPENDS
                static-recompile-pilot-standalone-tests
                static-recompile-pilot-tests
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_static_recompile_pilot.py"
            USES_TERMINAL
            VERBATIM
        )
    endif()
endif()

if(OPENSMACX_GAME_INSTALLER)
    set(OPENSMACX_STAGE_COMMAND
        "${OPENSMACX_PYTHON_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/setup_game.py"
        --installer "${OPENSMACX_GAME_INSTALLER}"
        --dll "$<TARGET_FILE:OpenSMACX>"
        --output "${OPENSMACX_GAME_DIR}"
        --wine-prefix "${OPENSMACX_WINE_PREFIX}"
        --reuse
    )
    if(OPENSMACX_WINE)
        list(APPEND OPENSMACX_STAGE_COMMAND --wine "${OPENSMACX_WINE}")
    endif()

    add_custom_target(stage-game
        COMMAND ${OPENSMACX_STAGE_COMMAND}
        DEPENDS
            OpenSMACX
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/local_artifact.py"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/opensmac_patcher.py"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/owned_wine_prefix.py"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/setup_game.py"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/wine_runtime.py"
        USES_TERMINAL
        VERBATIM
    )
endif()

if(MINGW AND BUILD_TESTING)
    add_custom_target(verify-redirect-signatures
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_generate_redirect_signatures.py"
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_redirect_signatures.py"
            --exe "${OPENSMACX_LEGACY_LEAF_EXE}"
            --runtime-exe "${OPENSMACX_LEGACY_EXE}"
            --output "${CMAKE_CURRENT_BINARY_DIR}/redirect_signatures.h"
        COMMAND "${CMAKE_COMMAND}" -E compare_files
            "${CMAKE_CURRENT_BINARY_DIR}/redirect_signatures.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/redirect_signatures.h"
        DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_redirect_signatures.py"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/test_generate_redirect_signatures.py"
            "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery-redirects.csv"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/redirect_signatures.h"
        USES_TERMINAL
        VERBATIM
    )

    add_custom_target(verify-recovery-batch
        COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure
        DEPENDS
            verify-recovery-metadata
            verify-recovery-abi
            verify-recovery-oracles
            verify-redirect-signatures
            recovery-leaf-tests
            recovery-gameplay-tests
            assemble-legacy-leaves
            smoke-hybrid-game
        USES_TERMINAL
        VERBATIM
    )
endif()
