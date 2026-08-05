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
# The gate's suite is 58 tests of which 51 are host-side and independent; the
# seven Wine-backed ones share the single prefix above and are serialised
# against each other by a RESOURCE_LOCK. 8, not the core count: with the Wine
# tests on a lock they are the critical path, so more lanes buy nothing.
set(OPENSMACX_CTEST_JOBS 8 CACHE STRING
    "Parallel CTest lanes used by verify-recovery-batch")
# 60, measured, not guessed. At 20 s the game never reached the
# scenario_opening_movie call site that triggers the DEFERRED oracle phase, so
# four suites - including the 108 generated ones - silently did not run while the
# gate reported success. What costs the time is the game booting to that call
# site: the generated suite itself is 8 ms per function, 0.1 s for the seventeen
# that get to run, so suite cost was never the constraint and the "180 s run" the
# oracle notes carried was never measured. 60 s reaches the deferred phase with
# margin; a longer window buys nothing.
set(OPENSMACX_HYBRID_SMOKE_SECONDS 60 CACHE STRING
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

# Nearly every tools/test_*.py registration is the same three lines; one
# definition keeps them identical.
function(opensmacx_add_python_tool_test name test_file)
    add_test(NAME ${name}
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/${test_file}"
    )
    # Recorded so build-freshness can be required by everything EXCEPT these.
    # They run interpreted from the source tree and have no build product that
    # can go stale, so a stale build directory does not invalidate them.
    set_property(GLOBAL APPEND PROPERTY OPENSMACX_PYTHON_TOOL_TESTS ${name})
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
    src/init_thunks.cpp
    src/adjustor_thunks.cpp
    src/deleting_thunks.cpp
    src/delegation_thunks.cpp
    src/nullsub_thunks.cpp
    src/field_accessors.cpp
    src/leaf_recoveries.cpp
    src/global_arith.cpp
    src/guarded_teardowns.cpp
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
    src/game_recovery.cpp
    src/general.cpp
    src/graphicwin.cpp
    src/generated_signature_oracle.cpp
    src/oracle_fault_guard.cpp
    src/graphicwin_oracle.cpp
    src/heap.cpp
    src/log.cpp
    src/maininterface.cpp
    src/map.cpp
    src/map_color.cpp
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
    src/alphamenu.cpp
    src/uv2player.cpp
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
        # -fno-exceptions ALONE DOES NOT REMOVE THE UNWIND TABLES, and on this
        # toolchain it never did - it just was not visible before. i686-w64-
        # mingw32 g++ 13 defaults to asynchronous unwind tables, so
        # autosound.cpp.obj carried 1,081 .eh_frame sections - one per
        # std::ctype instantiation reached through <sstream> and <string> in
        # stdafx.h, plus a 0x6d4-byte .eh_frame of its own - and
        # verify-recovery-abi failed exactly as it is designed to.
        #
        # The check is right and stays as it is. What was missing is the rest
        # of the flag: an object with no exceptions and full unwind tables
        # still carries the machinery the original does not have, and MSVC 6
        # emitted none of it. Removing the tables is what makes the object
        # match the claim; relaxing the check would only have stopped it being
        # reported.
        -fno-asynchronous-unwind-tables
        -fno-unwind-tables
        $<$<CONFIG:Debug>:-Og>
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:Release>:-fno-delete-null-pointer-checks>
        $<$<CONFIG:Release>:-fno-strict-aliasing>
    )
    target_link_options(OpenSMACX PRIVATE
        $<$<CONFIG:Release>:-s>
        -static
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
            --generated-signature-oracle-object
                "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/generated_signature_oracle.cpp.obj"
            --oracle-fault-guard-object
                "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/oracle_fault_guard.cpp.obj"
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
    # The src closure the leaf suite links.  Explicit list, never a glob: a
    # glob that misses a file drops coverage exactly the way an unregistered
    # test does.
    set(OPENSMACX_LEAF_SRC_CLOSURE
        src/atexit_thunks.cpp
        src/init_thunks.cpp
        src/adjustor_thunks.cpp
        src/deleting_thunks.cpp
        src/delegation_thunks.cpp
        src/nullsub_thunks.cpp
        src/field_accessors.cpp
        src/leaf_recoveries.cpp
        src/global_arith.cpp
        src/guarded_teardowns.cpp
        src/fx.cpp
        src/vector_teardown.cpp
        src/alphanet.cpp
        src/basepop.cpp
        src/ambience.cpp
        src/autosound.cpp
        src/basebutton.cpp
        src/basepop_font.cpp
        src/map_color.cpp
        src/buffer.cpp
        src/buttongroup.cpp
        src/dialog.cpp
        src/dialogs.cpp
        src/filemap_recovery.cpp
        src/flatbutton.cpp
        src/flic.cpp
        src/font_recovery.cpp
        src/graphicwin.cpp
        src/log.cpp
        src/maininterface.cpp
        src/mapwin.cpp
        src/game_recovery.cpp
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
        src/alphamenu.cpp
        src/uv2player.cpp
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

    # The split suite.  Direct sources, not a static library, so no static
    # initialiser can be discarded for want of --whole-archive; and one
    # explicit line per family, so a missing family is visible here as well as
    # at run time.
    set(OPENSMACX_LEAF_FAMILY_SRCS
        tests/leaf/core_construction_tests.cpp
        tests/leaf/leaf_recoveries_tests.cpp
        tests/leaf/widget_accessors_tests.cpp
        tests/leaf/text_and_resources_tests.cpp
        tests/leaf/dialog_and_strings_tests.cpp
        tests/leaf/win_and_graphic_tests.cpp
        tests/leaf/destructors_tests.cpp
        tests/leaf/buffer_and_font_tests.cpp
        tests/leaf/constant_stubs_tests.cpp
        tests/leaf/wave_and_sound_tests.cpp
        tests/leaf/delegates_tests.cpp
        tests/leaf/forwarders_tests.cpp
        tests/leaf/stubs_and_locks_tests.cpp
        tests/leaf/locks_tests.cpp
        tests/leaf/device_forwarders_tests.cpp
        tests/leaf/buffer_writers_tests.cpp
        tests/leaf/pops_and_console_tests.cpp
        tests/leaf/thunks_tests.cpp
    )
    set(OPENSMACX_LEAF_TEST_SRCS
        tests/leaf/leaf_harness.cpp
        tests/leaf/leaf_stubs.cpp
        ${OPENSMACX_LEAF_FAMILY_SRCS}
    )

    # The case manifest is derived, never edited: it is the file that two
    # independent recoveries in different families would otherwise both have to
    # touch.
    #
    # It must NOT be derived from OPENSMACX_LEAF_FAMILY_SRCS, which is the list
    # it exists to police.  Deriving it from that list would mean a family
    # dropping out of the build also drops out of the manifest: both sides of
    # the run-time count check would shrink together, the counts would still
    # agree, and the guard would be blind exactly when it is needed.  So the
    # counts come from an INDEPENDENT glob of the directory, and this configure
    # step fails if the glob and the explicit list disagree in either
    # direction.  The build still uses the explicit list; the glob exists only
    # to notice divergence and to keep a dropped family present in the manifest
    # so the binary refuses to run.
    file(GLOB OPENSMACX_LEAF_FAMILY_GLOB CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/leaf/*_tests.cpp")
    list(SORT OPENSMACX_LEAF_FAMILY_GLOB)
    set(_leaf_listed_abs "")
    foreach(_leaf_family_src IN LISTS OPENSMACX_LEAF_FAMILY_SRCS)
        list(APPEND _leaf_listed_abs
            "${CMAKE_CURRENT_SOURCE_DIR}/${_leaf_family_src}")
    endforeach()
    list(SORT _leaf_listed_abs)
    if(NOT _leaf_listed_abs STREQUAL OPENSMACX_LEAF_FAMILY_GLOB)
        set(_leaf_unlisted "")
        foreach(_leaf_path IN LISTS OPENSMACX_LEAF_FAMILY_GLOB)
            if(NOT _leaf_path IN_LIST _leaf_listed_abs)
                string(APPEND _leaf_unlisted "\n    ${_leaf_path}")
            endif()
        endforeach()
        set(_leaf_missing "")
        foreach(_leaf_path IN LISTS _leaf_listed_abs)
            if(NOT _leaf_path IN_LIST OPENSMACX_LEAF_FAMILY_GLOB)
                string(APPEND _leaf_missing "\n    ${_leaf_path}")
            endif()
        endforeach()
        message(FATAL_ERROR
            "tests/leaf family files and OPENSMACX_LEAF_FAMILY_SRCS disagree.\n"
            "On disk but not built (their cases would silently stop running):"
            "${_leaf_unlisted}\n"
            "Listed in CMake but not on disk:${_leaf_missing}\n"
            "Every tests/leaf/*_tests.cpp must appear in "
            "OPENSMACX_LEAF_FAMILY_SRCS and vice versa.")
    endif()

    set(OPENSMACX_LEAF_MANIFEST
        "${CMAKE_CURRENT_BINARY_DIR}/generated/leaf/leaf_generated_manifest.h")
    add_custom_command(
        OUTPUT "${OPENSMACX_LEAF_MANIFEST}"
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_leaf_manifest.py"
            --output "${OPENSMACX_LEAF_MANIFEST}"
            ${OPENSMACX_LEAF_FAMILY_GLOB}
        DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_leaf_manifest.py"
            ${OPENSMACX_LEAF_FAMILY_GLOB}
        COMMENT "Deriving the leaf case manifest from tests/leaf/*_tests.cpp"
        VERBATIM
    )

    add_executable(recovery-leaf-tests
        ${OPENSMACX_LEAF_TEST_SRCS}
        ${OPENSMACX_LEAF_MANIFEST}
        ${OPENSMACX_LEAF_SRC_CLOSURE}
    )
    target_include_directories(recovery-leaf-tests PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/generated/leaf")

    foreach(_leaf_target recovery-leaf-tests)
        target_compile_features(${_leaf_target} PRIVATE cxx_std_11)
        target_compile_options(${_leaf_target} PRIVATE -fno-exceptions)
        target_compile_definitions(${_leaf_target} PRIVATE
            BUILD_DLL OPENSMACX_SPOT_MINIMAL_INIT)
        set_target_properties(${_leaf_target} PROPERTIES
            CXX_EXTENSIONS OFF
            CXX_STANDARD 11
            CXX_STANDARD_REQUIRED YES
        )

        if(MINGW)
            target_compile_options(${_leaf_target} PRIVATE
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
            target_link_options(${_leaf_target} PRIVATE
                -static
                -Wl,--wrap=atexit
                -Wl,--wrap=fclose
                -Wl,--wrap=free)
        endif()
    endforeach()

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
        # The reversed run is the regression test for order dependence between
        # leaf cases - a case that borrows a pointer another case left behind
        # passes forward and faults reversed. It was documented as such while
        # nothing ran it; a guarantee nobody executes is a comment.
        add_test(NAME recovery-leaf-tests-reversed
            COMMAND ${OPENSMACX_RECOVERY_TEST_COMMAND} -- --reverse
        )
    else()
        add_test(NAME recovery-leaf-tests COMMAND recovery-leaf-tests)
        add_test(NAME recovery-leaf-tests-reversed
            COMMAND recovery-leaf-tests --reverse)
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
        src/flatbutton.cpp
        src/font.cpp
        src/font_recovery.cpp
        src/game.cpp
        src/game_recovery.cpp
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

    # The differential oracle's per-function verdict state machine.
    #
    # This is the one piece of the oracle that decides every number the project
    # reports: sixteen per-case verdicts fold into the single row the report
    # prints and the headline counts. It used to be inline in the driver's case
    # loop, so exercising it meant running the whole harness under Wine against
    # the user's own terranx.exe - and a false PASS lived there long enough to
    # reach committed numbers. tools/lifted_oracle_fold.h is the extraction; it
    # is pure, so this target links nothing but the test file itself and needs
    # no image, no Wine prefix and no lifted objects to be meaningful.
    # The bisect that names WHICH global moved. It reports an address in a FAIL
    # verdict, so an off-by-one in it produces a plausible wrong answer rather
    # than a visible failure - which is why it is a hand-written header with a
    # behavioural test instead of emitted text.
    add_executable(globals-diff-tests
        tests/globals_diff_tests.cpp
    )
    target_compile_features(globals-diff-tests PRIVATE cxx_std_11)
    set_target_properties(globals-diff-tests PROPERTIES
        CXX_EXTENSIONS OFF
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
    )
    if(MINGW)
        target_compile_options(globals-diff-tests PRIVATE
            -pedantic -Wall -Wextra -Wshadow)
        # Self-contained, as every other test executable here is: without it the
        # binary wants libstdc++-6.dll and libgcc_s_dw2-1.dll from the Wine
        # prefix and fails to load with c0000135.
        target_link_options(globals-diff-tests PRIVATE -static)
    endif()
    if(CMAKE_CROSSCOMPILING)
        set(OPENSMACX_GLOBALS_DIFF_TEST_COMMAND
            "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/run_windows_test.py"
            --executable "$<TARGET_FILE:globals-diff-tests>"
            --wine-prefix "${OPENSMACX_WINE_PREFIX}"
        )
        if(OPENSMACX_WINE)
            list(APPEND OPENSMACX_GLOBALS_DIFF_TEST_COMMAND
                --wine "${OPENSMACX_WINE}")
        endif()
        add_test(NAME globals-diff-tests
            COMMAND ${OPENSMACX_GLOBALS_DIFF_TEST_COMMAND}
        )
    else()
        add_test(NAME globals-diff-tests COMMAND globals-diff-tests)
    endif()

    # The guard rewrites a faulting thread's EIP and ESP from a process-global
    # handler. What keeps that safe is a closed exception-code list and a thread
    # gate, neither of which is visible by reading the generated oracle.
    add_executable(oracle-fault-guard-tests
        tests/oracle_fault_guard_tests.cpp
        src/oracle_fault_guard.cpp
    )
    target_compile_features(oracle-fault-guard-tests PRIVATE cxx_std_11)
    target_include_directories(oracle-fault-guard-tests PRIVATE src)
    set_target_properties(oracle-fault-guard-tests PROPERTIES
        CXX_EXTENSIONS OFF
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
    )
    if(MINGW)
        target_compile_options(oracle-fault-guard-tests PRIVATE
            -pedantic -Wall -Wextra -Wno-attributes)
        target_link_options(oracle-fault-guard-tests PRIVATE -static)
    endif()
    if(CMAKE_CROSSCOMPILING)
        set(OPENSMACX_FAULT_GUARD_TEST_COMMAND
            "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/run_windows_test.py"
            --executable "$<TARGET_FILE:oracle-fault-guard-tests>"
            --wine-prefix "${OPENSMACX_WINE_PREFIX}"
        )
        if(OPENSMACX_WINE)
            list(APPEND OPENSMACX_FAULT_GUARD_TEST_COMMAND
                --wine "${OPENSMACX_WINE}")
        endif()
        add_test(NAME oracle-fault-guard-tests
            COMMAND ${OPENSMACX_FAULT_GUARD_TEST_COMMAND}
        )
    else()
        add_test(NAME oracle-fault-guard-tests
            COMMAND oracle-fault-guard-tests)
    endif()

    add_executable(lifted-oracle-fold-tests
        tests/lifted_oracle_fold_tests.cpp
    )
    target_compile_features(lifted-oracle-fold-tests PRIVATE cxx_std_11)
    set_target_properties(lifted-oracle-fold-tests PROPERTIES
        CXX_EXTENSIONS OFF
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
    )
    if(MINGW)
        target_compile_options(lifted-oracle-fold-tests PRIVATE
            -pedantic
            -Wall
            -Wextra
            -Wshadow
            $<$<CONFIG:Debug>:-Og>
            $<$<CONFIG:Release>:-O2>
        )
        target_link_options(lifted-oracle-fold-tests PRIVATE -static)
    endif()

    if(CMAKE_CROSSCOMPILING)
        set(OPENSMACX_FOLD_TEST_COMMAND
            "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/run_windows_test.py"
            --executable "$<TARGET_FILE:lifted-oracle-fold-tests>"
            --wine-prefix "${OPENSMACX_WINE_PREFIX}"
        )
        if(OPENSMACX_WINE)
            list(APPEND OPENSMACX_FOLD_TEST_COMMAND
                --wine "${OPENSMACX_WINE}")
        endif()
        add_test(NAME lifted-oracle-fold-tests
            COMMAND ${OPENSMACX_FOLD_TEST_COMMAND}
        )
    else()
        add_test(NAME lifted-oracle-fold-tests
            COMMAND lifted-oracle-fold-tests)
    endif()

    if(MINGW)
        add_test(NAME recovery-abi
            COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_recovery_abi.py"
                --nm "${CMAKE_NM}"
                --objdump "${CMAKE_OBJDUMP}"
                --object "${OPENSMACX_ALPHANET_OBJECT}"
                # Sweep what the DLL actually built. The --*-object flags below
                # are a hand-kept list of 35; the DLL compiles 137, so 102
                # translation units could lose -fno-exceptions unnoticed, and
                # adding a source file never added it to the list.
                --object-dir
                "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src"
                --generated-signature-oracle-object
                "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/generated_signature_oracle.cpp.obj"
            --oracle-fault-guard-object
                "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/OpenSMACX.dir/src/oracle_fault_guard.cpp.obj"
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

    opensmacx_add_python_tool_test(hybrid-smoke-tool-tests test_smoke_hybrid_game.py)

    opensmacx_add_python_tool_test(gameplay-scenario-tool-tests test_run_gameplay_scenario.py)

    opensmacx_add_python_tool_test(external-analysis-tool-tests test_external_analysis.py)

    opensmacx_add_python_tool_test(recovery-metadata-tool-tests test_verify_recovery_metadata.py)

    opensmacx_add_python_tool_test(stage-preset-game-tool-tests test_stage_preset_game_dir.py)

    opensmacx_add_python_tool_test(batch-decompile-tool-tests test_batch_decompile.py)

    opensmacx_add_python_tool_test(mutation-harness-tool-tests test_mutate_and_verify.py)

    opensmacx_add_python_tool_test(gate-runner-tests test_run_gate.py)

    # The runner that every Wine-backed test goes through. Its one job beyond
    # invoking wine is to stop the owned prefix afterwards, and skipping that
    # to "save teardown" cost 1.05 s per CTest invocation because the retained
    # session holds CTest's output pipe open.
    opensmacx_add_python_tool_test(windows-test-runner-tests test_run_windows_test.py)

    # Runs on the stock interpreter on purpose. The doctor's job is to diagnose
    # a machine that has none of the dependencies yet, so a test that needed
    # them would not be testing the thing that matters.
    opensmacx_add_python_tool_test(host-doctor-tests test_host_doctor.py)

    opensmacx_add_python_tool_test(shape-classifier-tool-tests test_classify_recovery_shapes.py)

    opensmacx_add_python_tool_test(add-redirect-tool-tests test_add_redirect.py)

    opensmacx_add_python_tool_test(bulk-recover-stubs-tool-tests test_bulk_recover_stubs.py)

    opensmacx_add_python_tool_test(constant-return-scan-tool-tests test_find_constant_returns.py)

    opensmacx_add_python_tool_test(class-layout-tool-tests test_derive_class_layout.py)

    opensmacx_add_python_tool_test(disassembly-tool-tests test_disasm.py)

    # The experiment itself needs Visual C++ 6.0 under Wine and skips cleanly
    # without it, so only its offline half is registered: the COFF reader, the
    # normalisation, and the comparator. Those are the parts that could turn a
    # mismatch into a match, and they run everywhere.
    opensmacx_add_python_tool_test(msvc6-byte-match-tests test_msvc6_byte_match.py)

    # The general oracle the experiment was extracted into. Same reasoning, and
    # one addition: it pins that the SHAPE_EXACT tier can actually FIRE. Its
    # first draft kept non-relocated immediates, which made it nearly redundant
    # with BYTE_EXACT and therefore almost unable to fire at all - a tier that
    # never fires is indistinguishable from one that cannot.
    opensmacx_add_python_tool_test(byte-match-tests test_byte_match.py)

    # Exact matches are kept in a committed ledger and may rise but never fall.
    # This check is compiler-free: it reads the ledger and the checked-in floor,
    # so CI can enforce the result even though the VC6 recompile oracle itself
    # is local-only. Similarity tiers are diagnostics and do not advance it.
    add_test(NAME byte-match-ratchet
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/byte_match_fanout.py" --check)

    # The COFF target emitter and the callee-declaration repair, both of which
    # objdiff pairs symbols through: a target object with the wrong symbol name
    # produces a diff against nothing, and a callee VC6 re-mangles differently
    # shows up as a `call` mismatch the agent cannot fix from the call site.
    opensmacx_add_python_tool_test(target-object-emitter-tests
                                   test_emit_target_object.py)
    opensmacx_add_python_tool_test(mizuchi-declfix-tests test_mizuchi_declfix.py)

    # The writeback is the only tool in this tree that edits src/ from a line
    # number, and every way its splice can go wrong leaves a file that still
    # looks right in review: a swallowed doc comment, an orphaned brace, a
    # deleted `Status: Complete`. These run without Wine because the half they
    # pin - the editing, the catalogue line shift, the restore-on-refusal - is
    # the half that mutates the tree.
    opensmacx_add_python_tool_test(mizuchi-writeback-tests
                                   test_mizuchi_writeback.py)

    # The scaffolding emitter. Its coverage went 38% -> 98% over the fan-out
    # population and every step was a defect, including a repair regex pinned
    # to wording CL does not use - which matched nothing and returned a
    # byte-identical compile rate. These pin the repaired behaviour.
    opensmacx_add_python_tool_test(emit-translation-unit-tests
                                   test_emit_translation_unit.py)

    opensmacx_add_python_tool_test(generator-support-tests test_generator_support.py)

    opensmacx_add_python_tool_test(delegate-analysis-tool-tests test_analyze_delegates.py)

    opensmacx_add_python_tool_test(delegation-thunk-generator-tests test_generate_delegation_thunks.py)

    opensmacx_add_python_tool_test(nullsub-thunk-generator-tests test_generate_nullsub_thunks.py)

    opensmacx_add_python_tool_test(whole-image-lifter-tests test_lift_whole_image.py)

    opensmacx_add_python_tool_test(lifted-crt-tests test_lifted_crt.py)

    opensmacx_add_python_tool_test(find-leaf-testable-tests test_find_leaf_testable.py)

    opensmacx_add_python_tool_test(leaf-manifest-generator-tests
        test_generate_leaf_manifest.py)

    opensmacx_add_python_tool_test(x86-lowerer-tests test_x86_lower.py)

    # The mutation runner is the only evidence that an oracle PASS means
    # anything, so its own bugs turn into confident wrong numbers everywhere
    # else. Two of its three known defects were classification, not plumbing:
    # the two kinds of INCONCLUSIVE mean opposite things and were scored the
    # same, and a mutation whose regex matched nothing vanished from the run
    # instead of being reported as an untested property.
    opensmacx_add_python_tool_test(oracle-mutation-runner-tests test_lifted_oracle_mutate.py)

    # The one module that decides what every published number MEANS: which
    # rows are in scope (5,673 of 6,000 - external_library is excluded by
    # written policy and the exclusion is published with the figure), how a row
    # is priced (the sum of body spans, never the address extent), and which
    # direction a number moves. Two callers compute the same block from
    # different row shapes - the exporter's in-memory dicts and the reused
    # export's CSV strings - so a difference between them would publish two
    # numbers for one tree.
    opensmacx_add_python_tool_test(recovery-metrics-tests test_recovery_metrics.py)

    # The shape catalogue behind unproven_by_shape. A misclassified body is
    # invisible in the total, so the decisions are tested on synthetic bytes.
    opensmacx_add_python_tool_test(recovered-shape-classifier-tests test_classify_recovered_shapes.py)

    # Every population docs/EXCLUSIONS.md declares out of scope. An exclusion
    # is a promise about a number, and the two numbers this file re-derives
    # were both wrong when they were prose: `push -1; push imm32` counted as an
    # SEH prologue invented ~685 `_except_handler3` frames that do not exist,
    # and a linear sweep decoding a switch table reported thirteen port-I/O
    # bodies where there is one. The synthetic fixtures pin both refusals, and
    # where the pinned executable is present the last case re-measures the
    # image and fails on any figure the document has let drift.
    opensmacx_add_python_tool_test(exclusion-census-tests
        test_measure_exclusions.py)

    # The upper-bound catalogue that lets a class with no pinned sizeof stage an
    # oracle receiver. The safety argument is directional - a bound above the
    # true size wastes a zero-filled buffer, one below it lets a method write
    # past its receiver - so these tests are about the refusals and about
    # `bounded` never being spelled the same as `pinned`. Deriving the
    # catalogue needs the uncommitted executable and stays a local step, as it
    # does for every other catalogue here.
    opensmacx_add_python_tool_test(class-size-bounds-tests
        test_derive_class_size_bounds.py)

    opensmacx_add_python_tool_test(boot-loop-tests test_lifted_boot_loop.py)

    # The whole-image summary: every headline number the project publishes.
    # The byte-weighted verdict table, "compared and agreed", the evidence
    # tiers under it, the three numbers that must go DOWN and the seed
    # histogram are all computed here and nowhere else, so an arithmetic slip
    # is not caught downstream - it is printed and believed. Byte weighting in
    # particular has already been got wrong once in this project, by reading a
    # field that is a SUM of body spans as if it were a contiguous address
    # extent. The fixtures are synthetic reports in a temp dir; nothing here
    # reads build/oracle.
    opensmacx_add_python_tool_test(lifted-oracle-summary-tests test_lifted_oracle_summary.py)

    opensmacx_add_python_tool_test(lifted-oracle-compare-tests test_lifted_oracle_compare.py)

    # Reads WHY a nearly-full-strength function fell short. Two of its guards
    # exist because the first real run tripped on them: `%#010x` of zero prints
    # no `0x`, so the pattern silently dropped exactly the null-dereference
    # lines that are the whole diagnosis; and a function that is N seeds short
    # owes N reasons, which is the only check that noticed. Fixtures are
    # captured oracle text; nothing here runs the oracle.
    opensmacx_add_python_tool_test(lifted-oracle-why-not-full-tests
        test_lifted_oracle_why_not_full.py)

    # Canonical address -> the bytes our compiler produced, which is one half of
    # the SMT equivalence spike's input. Its failure mode is not finding
    # nothing, it is finding the WRONG body and looking certain: the tests pin
    # that a body ends at the next symbol IN ITS OWN SECTION, that an ambiguous
    # name is refused, and that i686 fastcall decoration is undone in the one
    # direction c++filt accepts. Fixtures are synthetic COFF; no build needed.
    opensmacx_add_python_tool_test(compiled-recovery-extractor-tests
        test_extract_compiled_recovery.py)

    # The Z3 x86 encoding. These tests do NOT establish that it is correct.
    # tools/validate_x86_smt.py is what does that, by driving the encoding and
    # lifted_x86.h with the same states and requiring bit-for-bit agreement;
    # six deliberately damaged encodings were all caught by it.
    #
    # BUT NOTHING RUNS IT, INCLUDING THIS FILE. It is not registered as a test
    # and cannot be: its reference harness includes build/lifted/
    # lifted_runtime.h, which tools/lift_whole_image.py generates and which is
    # gitignored, so a gate lane has no way to build it. On a tree without that
    # header it now says so instead of emitting a compiler error that reads
    # like an encoding fault. The agreement above is therefore a RECORDED PAST
    # RESULT, not a standing guarantee, and this comment used to claim
    # otherwise in the present tense.
    #
    # What IS pinned here is what validation cannot reach: that the formulas
    # are symbolic, since a prover never sees a concrete state.
    opensmacx_add_python_tool_test(x86-smt-encoding-tests test_x86_smt.py)

    # The only place in this repository where a function's ABI is written down
    # mechanically rather than guessed: the 4,821 MSVC-mangled names. The parse
    # has one trap that reads as correct - on an INSTANCE member the character
    # after the kind char is the `this` cv-qualifier, not the calling
    # convention, and reading it as the convention reports 2,905 of 4,026
    # functions as __cdecl and looks entirely plausible. So the tests pin the
    # parse against undname per row, pin the argument layout, and pin the
    # body-versus-name arbitration that decides where `this` really arrives.
    # This file was delivered without a registration and ctest never ran it.
    opensmacx_add_python_tool_test(calling-convention-tests test_recover_conventions.py)

    # The same mangled names, read for their FULL prototype rather than just
    # their convention: 1,605 catalogued rows have a mangled name and no
    # prototype, and the linker already wrote every one of those prototypes
    # down. The dangerous failure is not a missing row, it is a WRONG one -
    # every consumer of the `prototype` column reads it as certain, so a
    # mis-rendered argument list silently changes which oracles are generated
    # and which proofs compare EAX. The generator gates itself on a five-axis
    # agreement control against the 3,213 rows that carry both a name and an
    # IDA recording (convention and stack layout agree on all 3,213), and these
    # tests pin the rendered shape byte-for-byte, pin the refusals, and hold
    # the weak IDA-argument-count file separate from the certain one - it
    # carries no convention, because IDA's agrees only 30.3% of the time.
    #
    # That control's population is the rows that ALREADY have a recording, so
    # it is by construction disjoint from the rows being published, and it
    # never asks the machine code anything. Nine published rows turned out to
    # be contradicted by the `ret imm` their own body executes. A purge gate
    # now refuses those, and docs/recovery/callee-purge.csv carries its answer
    # so the gate replays here with no executable present: these tests rebuild
    # the implied purge from the PUBLISHED columns and fail if any row
    # disagrees with the committed observation, which is what stops a
    # regenerated catalogue from quietly re-admitting one.
    #
    # That gate swept each body linearly and filed "no ret" as no evidence. It
    # was one instruction short: 345 of those bodies end in a DIRECT JMP - they
    # are tail-call thunks and their `ret` is in the target. Following the jump
    # ONE hop (never two; a chain is unknown) reaches 324 of them, and 277
    # AGREE with the published prototype while 47 CONTRADICT it. The 277 are
    # what makes the hop a method rather than a way of manufacturing refusals,
    # and the run refuses to write if that agreement collapses. The 47 are
    # every `??3<Class>@@SAXPAXI@Z` row in the image - `sub ecx, <imm> ; jmp
    # <??_G...>` adjustor thunks published as purging 0 while the body they
    # jump to executes `ret 4`, so a caller pushed 8 and got 4 popped. They
    # were also 47 of the 47 `static`-kind rows, a whole kind no control had
    # ever reached. All 47 are refused, and these tests pin that.
    #
    # A hopped number is NOT a direct observation and must never read as one:
    # callee-purge.csv carries a `source` column and the published `purge`
    # column distinguishes `verified-via-tail-jump` from `verified`. The tests
    # pin both, and pin that a pre-hop three-column catalogue is REFUSED rather
    # than having its sourceless numbers rounded up into direct observations.
    opensmacx_add_python_tool_test(derived-prototype-tests
        test_derive_prototypes_from_names.py)

    # The plan decides what the oracle is allowed to compare, so every mistake
    # in it can only LOSE coverage - and losing coverage is silent: the
    # function stops failing instead of starting to. The `undef=<hex>` mask is
    # the sharpest case, because a mask one bit too wide stops comparing a flag
    # that is perfectly well defined and nothing downstream can tell that from
    # agreement. The tests assemble instruction bodies as bytes and pin the
    # mask in both directions, per flag, including the redefinition rule that
    # is the whole risk and the second body span this repo has read wrong
    # before.
    opensmacx_add_python_tool_test(lifted-oracle-plan-tests test_lifted_oracle_plan.py)

    # The join between the oracle's trap CLASSIFIER and the four files that
    # write the trap reasons it matches. The classifier decides whether a trap
    # is a mnemonic waiting to be lowered (SKIP-trap, a phase-4 work item) or a
    # construct the lift cannot model (SKIP-reached-blocked, the runtime proof
    # that a static blocking flag describes a path the program takes). Measured
    # over the 470 extcall-only functions that split was 79 to 0.
    #
    # It matches STRINGS, and the strings live in lift_whole_image.py,
    # generate_imports.py, lifted_tls.h and lifted_imports.h. Reword one and
    # everything still compiles, every C++ test stays green, and the verdict
    # silently degrades - the two halves only ever meet at runtime under Wine
    # against the user's own executable. This test is where they meet instead.
    opensmacx_add_python_tool_test(lifted-oracle-trap-reason-tests test_lifted_oracle_trap_reasons.py)

    # The import boundary's derivation: which import lives at which IAT slot,
    # and how many bytes of argument each one pops. Both are recovered rather
    # than written down, and a wrong slot binds every call in the image to a
    # neighbouring function while every ABI check still passes - so the
    # derivation is cross-checked here against arithmetic on the PE's own
    # FirstThunk, independent of the pefile attribute the generator reads.
    opensmacx_add_python_tool_test(import-shim-tool-tests test_generate_imports.py)

    opensmacx_add_python_tool_test(jump-table-tests test_jump_tables.py)

    opensmacx_add_python_tool_test(deleting-thunk-generator-tests test_generate_deleting_thunks.py)

    opensmacx_add_python_tool_test(adjustor-thunk-generator-tests test_generate_adjustor_thunks.py)

    opensmacx_add_python_tool_test(atexit-thunk-generator-tests test_generate_atexit_thunks.py)

    opensmacx_add_python_tool_test(field-accessor-generator-tests test_generate_field_accessors.py)

    opensmacx_add_python_tool_test(init-thunk-generator-tests test_generate_init_thunks.py)

    opensmacx_add_python_tool_test(redirect-signature-generator-tests test_generate_redirect_signatures.py)

    opensmacx_add_python_tool_test(redirect-patch-fit-tool-tests test_verify_redirect_patch_fit.py)

    # The inventory behind `unproven_recovered`. Its dangerous failure is
    # publishing a proof that is not one, so it is gated like the metric it
    # feeds.
    opensmacx_add_python_tool_test(proven-functions-tool-tests test_export_proven_functions.py)

    # The generator that emits those proofs. Same dangerous failure, one step
    # earlier: an oracle for a function the hybrid cannot execute both forms of
    # would carry a marker that lowers the metric while proving nothing.
    opensmacx_add_python_tool_test(signature-oracle-generator-tests test_generate_signature_oracles.py)

    # A defect class no oracle here can see: the original divides signed and
    # the recovery declared unsigned, which agree on every non-negative input.
    # `bitmask` was found by byte-matching, not by any test. This compares the
    # catalogue's decoration against the GCC symbol each .def alias maps to,
    # and refuses to let the candidate population grow.
    opensmacx_add_python_tool_test(export-signedness-audit-tests test_audit_export_signedness.py)

    # The callgraph records only direct `call rel32`, so a function that
    # dispatches through a vtable slot reads as a perfect leaf. Three were
    # picked as recovery targets on that basis.
    opensmacx_add_python_tool_test(indirect-call-site-tests test_indirect_call_sites.py)

    # CTest never builds, so a green run can be a green run of the last build.
    opensmacx_add_python_tool_test(build-freshness-tests test_verify_build_freshness.py)
    # --exe is passed explicitly rather than left to the tool's default. Ranking
    # a disagreement as signed_divide means disassembling the original, and with
    # no executable every finding stays `unranked` so that half of the ratchet
    # compares 0 against 44 and can never fail. The tool now returns 2 rather
    # than printing that as a pass, and naming the path here is what makes the
    # dependency visible at the point of registration.
    add_test(NAME export-signedness-audit
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/audit_export_signedness.py"
            --check
            --exe "${OPENSMACX_LEGACY_LEAF_EXE}"
    )

    # THE REST OF THE DERIVED ARTIFACTS, wired 2026-08-01. Each of these four
    # tools had exactly one mention in this file and it was the TEST file, not
    # the tool - so the tests exercised the tool's logic while nothing ever
    # asked whether the committed output was still current. The comment below
    # diagnosed precisely this and then wired two of six.
    #
    # Measured before wiring, because a check that cannot fail is worse than no
    # check: each was damaged and each refused. proven.csv with a row removed,
    # generated_signature_oracle.cpp with a comment edited, class-size-bounds.csv
    # with a row removed, and EXCLUSIONS.md with one number in the ```measured
    # fence changed - all four exit 1. Costs 0.05 s, 0.09 s, 0.24 s and 1.92 s,
    # none of them near the 12.3 s critical path.
    #
    # These WILL go red when a generator's committed output lags a source edit -
    # that is the entire point, and it has already happened once today: adding
    # two #include lines to game.cpp moved a function two lines and staled the
    # generated provenance comment. Regenerate and commit the output with the
    # change that caused it.
    add_test(NAME proven-functions-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/export_proven_functions.py"
            --check
    )
    add_test(NAME signature-oracles-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_signature_oracles.py"
            --check
    )
    add_test(NAME class-size-bounds-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/derive_class_size_bounds.py"
            --check
            --exe "${OPENSMACX_LEGACY_LEAF_EXE}"
    )
    # --exe named explicitly for the same reason as the signedness audit: with
    # no image this compares the document against nothing, and it now returns 2
    # rather than reporting that as agreement.
    add_test(NAME exclusions-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/measure_exclusions.py"
            --check
            --exe "${OPENSMACX_LEGACY_LEAF_EXE}"
    )

    # Four artifacts under docs/recovery/ are DERIVED, and promotion regenerates
    # none of them. Each has a `--check`, but only the signedness one above was
    # reachable from any build path: the tests registered for the other two
    # exercise the tool's logic, not whether the COMMITTED file is still current.
    # summary.json's shape split is computed from recovered-shapes.csv, so a
    # stale one publishes a wrong split under a green gate. 0.43 s and 0.65 s.
    # AGENTS.md's "Current recovery state" line is typed by hand and had
    # drifted again by 2026-08-01. Its previous defence was a sentence warning
    # the reader it might be wrong, which is not a defence.
    # recovery_gameplay_tests.cpp self-registers its cases, so the way to lose
    # a test is now to write it and forget the GAMEPLAY_CASE line. Nothing else
    # can see that: it compiles, and every remaining assertion still passes.
    add_test(NAME test-registration
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_test_registration.py"
    )

    opensmacx_add_python_tool_test(documented-counts-tests test_verify_documented_counts.py)

    # A tools/test_*.py runs only because this file names it, and nothing about
    # an unregistered one looks wrong - it imports, it passes by hand, and the
    # gate reports a clean sweep without it. Four sat unregistered before anyone
    # noticed. The check distinguishes execution from mere reference: a test
    # named only in a DEPENDS list is rebuilt when it changes and never run.
    add_test(NAME tool-test-registration
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_tool_test_registration.py"
    )
    opensmacx_add_python_tool_test(tool-test-registration-tests
                                   test_verify_tool_test_registration.py)

    # THE META-CHECK. Damages real repository content and requires each gate
    # check to refuse it. Three of the thirteen checks added on 2026-07-31 could
    # not fail, and every one of them passed its own unit tests - the controls
    # each exercised the single input shape their author had in mind. A unit
    # test asks whether a function returns the right value for a synthetic
    # input; this asks whether the shipped check refuses real content that has
    # been damaged.
    #
    # It also requires every check in its COVERED_CHECKS set to have a damage
    # case that actually ran, so a new check ships with a proof it can fail
    # rather than a promise. 10 cases across 8 checks in under 5 s.
    add_test(NAME checks-can-fail
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_checks_can_fail.py"
    )
    opensmacx_add_python_tool_test(checks-can-fail-tests
                                   test_verify_checks_can_fail.py)

    # A RECOVERY RUN, CODIFIED. Recovering a function means asking the tools a
    # fixed sequence of questions - body spans, callees, indirect sites, the
    # bytes purged on return, seam resolution - whose answers are deterministic
    # functions of the pinned image and the committed catalogue. The authoring
    # in the middle is not automatable; everything around it is, and nothing
    # checked that a tool change left those answers alone.
    #
    # Eight functions, each on the slate for a shape that has broken something
    # here before: a split body whose cold span holds 2,102 B, a body that
    # dispatches through a vtable and reads as a perfect leaf, a thiscall whose
    # purge excludes `this`, and recovered/unrecovered pairs. Damaging
    # parse_body_ranges to return one span is caught as
    # `span_bytes 16430 -> 14328`.
    add_test(NAME recovery-pipeline
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_recovery_pipeline.py"
            --exe "${OPENSMACX_LEGACY_LEAF_EXE}"
    )
    opensmacx_add_python_tool_test(recovery-pipeline-tests
                                   test_verify_recovery_pipeline.py)

    # Two gate checks that had no unit tests at all. checks-can-fail covers
    # them end-to-end against the generated CTestTestfile and the real suite;
    # these pin the branches those inputs do not happen to contain - the
    # unquoted RESOURCE_LOCK spelling, a call before main(), the non-vacuity
    # floor, and the absent-input refusals.
    opensmacx_add_python_tool_test(wine-test-lock-tests
                                   test_verify_wine_test_locks.py)
    opensmacx_add_python_tool_test(test-registration-tests
                                   test_verify_test_registration.py)

    # command_runs_scenario_executable decides which PIDs stop_executable_
    # processes signals. A false positive there does not fail a test - it kills
    # something outside the build, which is what the never-issue-a-global-Wine-
    # shutdown rule exists to prevent. The negative cases are the point: a
    # command holding `<staged>.log`, a different token, a non-wine launcher.
    opensmacx_add_python_tool_test(runtime-process-tests test_runtime_process.py)

    # measure_observability.py answers "does anything watch this function" for
    # one build per function instead of ~152, which is what made asking it over
    # the whole corpus possible at all. Deliberately NOT registered as a gate
    # test: a full run is ~30 minutes against a 107 s lane. Only its unit tests
    # run here; the corpus figure belongs in docs/RECOVERY_ECONOMICS.md, where
    # it is dated, because it moves as fixtures are written.
    opensmacx_add_python_tool_test(observability-tests
                                   test_measure_observability.py)

    # The ratchet. 141 of 193 recovered functions of >=200 B are observed by
    # nothing; writing 131 retroactive fixtures would halt the objective, so the
    # debt is published and capped rather than paid off. A new recovery absent
    # from the census fails, and the counts may fall but never rise. Compares
    # two committed files and builds nothing.
    add_test(NAME observability-ratchet
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_observability_ratchet.py"
    )
    opensmacx_add_python_tool_test(observability-ratchet-tests
                                   test_verify_observability_ratchet.py)

    # recovery-abi prefetches its objdump/nm calls in parallel, which is a
    # speedup applied to a verifier. These cover the two properties that keep
    # that safe - an unplanned command is still executed, and a command that
    # fails while being speculatively prefetched is not cached - neither of
    # which a passing gate would reveal.
    opensmacx_add_python_tool_test(recovery-abi-tool-tests test_verify_recovery_abi.py)

    # The staged hybrid imports 462 names from OpenSMACX.dll and its import
    # table is frozen, so a removed or renamed .def alias breaks the game. That
    # is already enforced - but only by stage-hybrid-game, after a DLL build,
    # ImportAdder under Wine and an assembled image, which is why 62/62 once
    # passed against a DLL the game could not load. Seconds, from git, here.
    opensmacx_add_python_tool_test(def-append-only-tests test_verify_def_append_only.py)
    add_test(NAME def-append-only
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_def_append_only.py"
    )
    add_test(NAME documented-counts
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_documented_counts.py"
    )

    # A fixed game address dereferenced at load time faults before main() in
    # the host test binaries once the image grows past the mapped range - so
    # merely ADDING TESTS crashes an unrelated binary, and the fault is
    # attributed to whichever commit grew it.
    opensmacx_add_python_tool_test(load-time-address-tests test_verify_no_load_time_addresses.py)
    add_test(NAME load-time-addresses
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_no_load_time_addresses.py"
    )

    add_test(NAME recovered-shapes-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/classify_recovered_shapes.py"
            --check
    )
    add_test(NAME derived-prototypes-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/derive_prototypes_from_names.py"
            --check
    )

    # The check itself, against the real table: `install_redirect` writes five
    # bytes with no regard for how long the function it replaces is, and 280 of
    # the wired redirects are shorter than that.
    add_test(NAME redirect-patch-fit
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_redirect_patch_fit.py"
            --dllmain "${CMAKE_CURRENT_SOURCE_DIR}/src/dllmain.cpp"
            --functions "${CMAKE_CURRENT_SOURCE_DIR}/docs/recovery/functions.csv"
    )
endif()

install(TARGETS OpenSMACX
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
    ARCHIVE DESTINATION lib
)

set(OPENSMACX_METADATA_VERIFY_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/recovery-metadata-verify")
# The two metadata-gate targets run the same command over the same inputs;
# promote adds --promote. One definition keeps their dependency lists from
# drifting apart.
function(opensmacx_add_metadata_gate_target name)
    add_custom_target(${name}
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_recovery_metadata.py"
            --idb "${OPENSMACX_RECOVERY_IDB}"
            --verify-dir "${OPENSMACX_METADATA_VERIFY_DIR}"
            ${ARGN}
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
endfunction()

opensmacx_add_metadata_gate_target(verify-recovery-metadata)

# Every recovery invalidates the committed metadata by construction, so the
# unpromoted gate is guaranteed to fail its first pass on any batch that landed
# a body. Running this first regenerates, copies over docs/recovery/, and still
# performs the byte comparison, which turns the standing two-pass cycle into
# one. It is deliberately NOT a dependency of verify-recovery-batch: the gate
# must keep failing on drift for anyone who has not consciously promoted.
opensmacx_add_metadata_gate_target(promote-recovery-metadata --promote)

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
    # Every suite src/runtime_oracle.cpp registers, named so the smoke fails on
    # one that never reports rather than accepting the terminator alone. The
    # last green gate accepted a result file with generated-signatures missing
    # and zero verdict lines in the log; a named set is what catches that.
    # tools/test_smoke_hybrid_game.py pins this list against the DLL's own.
    --expect-oracle-suite=constructor
    --expect-oracle-suite=basebutton
    --expect-oracle-suite=scroll
    --expect-oracle-suite=win
    --expect-oracle-suite=sprite
    --expect-oracle-suite=buffer
    --expect-oracle-suite=graphicwin
    --expect-oracle-suite=stringstruct
    --expect-oracle-suite=spying
    --expect-oracle-suite=basebutton-release
    --expect-oracle-suite=sprite-release
    --expect-oracle-suite=buffer-release
    --expect-oracle-suite=generated-signatures
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
            # Scroll::destroy composes the embedded FlatButton destructors,
            # which chain through the BaseButton teardown; their Time, CRT
            # free, and allocator externals are link-only stubs in the suite.
            src/flatbutton.cpp
            src/basebutton.cpp
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

    # Every test that goes through tools/run_windows_test.py drives the ONE
    # marker-protected prefix at ${OPENSMACX_WINE_PREFIX}, and that runner
    # stops the prefix when it finishes. Two of them at once therefore means
    # one killing the other's wineserver mid-test: measured, plain `ctest -j8`
    # failed 1 of 58 (recovery-leaf-tests) where the serial run passed 58/58,
    # and an earlier lane failed 3. RESOURCE_LOCK is what lets -j be honest -
    # CTest never schedules two holders of the same lock together.
    #
    # if(TEST) rather than a bare set_tests_properties, because these tests
    # exist only when cross-compiling; and the list is checked mechanically by
    # the wine-test-lock-check test below, because a wine-backed test MISSING
    # from it does not fail, it flakes.
    set(OPENSMACX_WINE_LOCKED_TESTS
            recovery-leaf-tests
            recovery-leaf-tests-reversed
            recovery-gameplay-tests
            globals-diff-tests
            oracle-fault-guard-tests
            lifted-oracle-fold-tests
            recovery-oracle)
    list(LENGTH OPENSMACX_WINE_LOCKED_TESTS OPENSMACX_WINE_LOCKED_TEST_COUNT)
    foreach(_wine_locked_test IN LISTS OPENSMACX_WINE_LOCKED_TESTS)
        if(TEST ${_wine_locked_test})
            set_tests_properties(${_wine_locked_test} PROPERTIES
                RESOURCE_LOCK wineprefix)
        endif()
    endforeach()

    # Reads the generated CTestTestfile.cmake, so it checks what CTest will
    # actually run rather than what this file appears to say.
    add_test(NAME wine-test-lock-check
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_wine_test_locks.py"
            --ctest-file "${CMAKE_CURRENT_BINARY_DIR}/CTestTestfile.cmake"
            # Derived from the list above rather than typed, so the expectation
            # cannot drift from it. Without this the check printed "verified
            # NOTHING" and returned 0 whenever it matched no Wine-backed test -
            # which is what a rename of the runner or of --wine-prefix looks
            # like, and it is indistinguishable from a healthy native build.
            --expect-at-least "${OPENSMACX_WINE_LOCKED_TEST_COUNT}"
    )

    # CTest NEVER BUILDS. Twice this project reported "62/62 passed" against
    # binaries a failed build had not replaced - once against a DLL that had
    # failed to link - because the operator ran ctest after a build error and
    # every executable from the previous build was still on disk.
    #
    # Per-binary freshness would not have caught it: no test executable links
    # OpenSMACX.dll (they are all static), so on that day every test binary was
    # legitimately current. The DLL is passed explicitly for that reason.
    add_test(NAME build-freshness
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_build_freshness.py"
            --build-dir "${CMAKE_CURRENT_BINARY_DIR}"
            --ctest-file "${CMAKE_CURRENT_BINARY_DIR}/CTestTestfile.cmake"
            --ninja "${CMAKE_MAKE_PROGRAM}"
            --also "$<TARGET_FILE:OpenSMACX>"
    )
    set_tests_properties(build-freshness PROPERTIES FIXTURES_SETUP build_fresh)

    # A FIXTURES_SETUP, so a stale tree turns these into `***Not Run` instead of
    # into passes - a stale PASS is the whole failure mode. Membership is
    # derived by EXCLUSION so a new binary-backed test is guarded without anyone
    # remembering to add it; the python tool tests are exempt because they run
    # interpreted from the source tree and have no build product to go stale.
    get_property(_opensmacx_all_tests DIRECTORY PROPERTY TESTS)
    get_property(_opensmacx_tool_tests GLOBAL PROPERTY OPENSMACX_PYTHON_TOOL_TESTS)
    list(REMOVE_ITEM _opensmacx_all_tests ${_opensmacx_tool_tests} build-freshness)
    if(_opensmacx_all_tests)
        set_tests_properties(${_opensmacx_all_tests}
            PROPERTIES FIXTURES_REQUIRED build_fresh)
    endif()

    add_custom_target(verify-recovery-batch
        COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure
            --parallel "${OPENSMACX_CTEST_JOBS}"
        DEPENDS
            verify-recovery-metadata
            verify-recovery-abi
            verify-recovery-oracles
            verify-redirect-signatures
            recovery-leaf-tests
            recovery-gameplay-tests
            lifted-oracle-fold-tests
            globals-diff-tests
            # The gate RAN this without building it. It is registered in
            # CTestTestfile.cmake and reachable from `all`, but it was in
            # neither the explicit nor the order-only inputs of this edge, so
            # the one path advertised as safe from stale binaries was launching
            # one it never rebuilt. recovery-oracle-tests needs no line here -
            # verify-recovery-oracles already pulls it in order-only.
            oracle-fault-guard-tests
            assemble-legacy-leaves
            smoke-hybrid-game
        USES_TERMINAL
        VERBATIM
    )
endif()
