cmake_minimum_required(VERSION 3.20)

# ---------------------------------------------------------------------------
# The toolchain. There is exactly one: cl 12.00.8168, the compiler that built
# the original image.
#
# This is a matching decompilation, so VC6 is not one option among several -
# it is the only compiler whose output carries information. Anything else can
# tell you the code is valid; only this one can tell you it is RIGHT. It is
# set here rather than in a toolchain file behind a preset because with one
# compiler there is no choice to express, and a preset that must always be
# passed is a way to get the build wrong by forgetting it. `cmake -B build`
# is the whole invocation.
#
# It is legal this early because nothing here needs a probe. The compiler id
# is FORCED, which skips both the identification and the ABI try_compile -
# CMake identifies a compiler by building a probe and reading strings out of
# the object, and its MSVC probe assumes a native toolchain, so letting it try
# means it falls back to GNU rules and passes flags VC6 has never heard of.
#
# It runs under wine through tools/vc6_cl.py, which exists because CMake
# drives a compiler with POSIX paths and MSVC reads a leading `/` as the start
# of an option.
# ---------------------------------------------------------------------------
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)
set(CMAKE_SYSTEM_VERSION 4.0)

if(NOT DEFINED VC6_ROOT)
    if(DEFINED ENV{VC6_ROOT})
        set(VC6_ROOT "$ENV{VC6_ROOT}")
    else()
        set(VC6_ROOT "$ENV{HOME}/opt/vc6")
    endif()
endif()

set(_vc6_tools "${CMAKE_CURRENT_LIST_DIR}/tools")
set(CMAKE_CXX_COMPILER "${_vc6_tools}/vc6-cl" CACHE FILEPATH "VC6 C++ compiler under wine")
set(CMAKE_C_COMPILER   "${_vc6_tools}/vc6-cl" CACHE FILEPATH "VC6 C compiler under wine")
set(CMAKE_LINKER       "${_vc6_tools}/vc6-link" CACHE FILEPATH "VC6 linker under wine")
set(CMAKE_AR           "${_vc6_tools}/vc6-lib" CACHE FILEPATH "VC6 librarian under wine")

set(CMAKE_CXX_COMPILER_ID MSVC)
set(CMAKE_C_COMPILER_ID MSVC)
set(CMAKE_CXX_COMPILER_VERSION 12.00.8168)
set(CMAKE_C_COMPILER_VERSION 12.00.8168)
set(CMAKE_CXX_SIMULATE_ID "")
set(CMAKE_CXX_COMPILER_FRONTEND_VARIANT MSVC)
set(CMAKE_C_COMPILER_FRONTEND_VARIANT MSVC)
set(CMAKE_CXX_COMPILER_FORCED TRUE)
set(CMAKE_C_COMPILER_FORCED TRUE)

# Forcing the compiler id also skips CMake's ABI probe, which is what
# normally discovers these. VC6 is 32-bit x86 and nothing else, so they are
# constants rather than something to detect.
set(CMAKE_SIZEOF_VOID_P 4)
set(CMAKE_CXX_SIZEOF_DATA_PTR 4)
set(CMAKE_C_SIZEOF_DATA_PTR 4)

# VC6 predates every /std: switch and has no notion of a language standard.
# The tree is C++98 by necessity and src/vc6_compat.h supplies the C++11
# SPELLINGS it uses - static_assert, constexpr, override, final - as macros.
# So there is no standard to request and nothing that could honour it, which
# is why no target here asks for one.
set(CMAKE_CXX_STANDARD_DEFAULT "")
set(CMAKE_CXX98_STANDARD_COMPILE_OPTION "")
set(CMAKE_CXX98_EXTENSION_COMPILE_OPTION "")

# HEADER DEPENDENCIES. Without this the build is silently WRONG on every
# incremental run. CMake's Ninja generator emits `deps = gcc` and
# `depfile = $DEP_FILE` for this compiler but passes no flag that would
# produce one, so no depfile was ever written, ninja recorded `#deps 0` for
# all 137 objects, and editing a header rebuilt NOTHING. That is not a slow
# build, it is a build that reports success over stale objects - three
# measurements in one session were taken against them before it surfaced.
#
# cl cannot write a GNU depfile, so tools/vc6_cl.py consumes these flags
# itself, adds `/showIncludes`, and converts the `Z:\...` paths cl reports
# back to the POSIX ones ninja matches against. A path in the wrong alphabet
# is not a mismatch ninja reports; it is a dependency it silently lacks.
#
# The spelling is deliberately NOT the GNU one. `-MD` is MSVC's
# multithreaded-DLL runtime switch and CMake already passes it; a wrapper that
# consumed `-MD` as "write a depfile" would silently strip the runtime model
# from every compile. `-MT` collides with the static-runtime switch the same
# way. These two are the wrapper's own and can collide with nothing.
set(CMAKE_DEPFILE_FLAGS_CXX "--deptarget=<DEP_TARGET> --depfile=<DEP_FILE>")
set(CMAKE_DEPFILE_FLAGS_C "--deptarget=<DEP_TARGET> --depfile=<DEP_FILE>")

# `/Z7`, NOT `/Zi`. VC6 writes debug info to a single `vc60.pdb` per output
# directory and takes no lock on it, so a parallel build fails with
# `C1033: cannot open program database` on whichever objects lose the race -
# 16 of 19 on the first run here. `/Z7` puts the debug info in the object
# file, where there is no shared state and no race. CMake's MSVC defaults
# assume a compiler that can share a PDB; this one cannot.
set(CMAKE_CXX_FLAGS_DEBUG "/Z7 /Ob0 /Od /GZ" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_DEBUG "/Z7 /Ob0 /Od /GZ" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/Z7 /O2" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELWITHDEBINFO "/Z7 /O2" CACHE STRING "" FORCE)

# `/Zm200`, NOT the `/Zm1000` CMake picks for us.
#
# Windows-MSVC.cmake appends `/Zm1000` unconditionally when MSVC_VERSION is
# below 1310, and on a real 1999 Windows box that is generous and harmless.
# Under wine it is neither. `/Zm` does not cap the heap, it RESERVES it up
# front, and the reservation has to find room in a 32-bit address space that
# already has the precompiled header mapped at a fixed base. At 1000 the
# reservation fails and the compiler reports the failure as
# `fatal error C1060: compiler is out of heap space` - so asking for ten times
# the memory is precisely what makes it run out.
#
# Measured on src/ambience.cpp, twice at each setting: /Zm1000 fails, /Zm400,
# /Zm200, /Zm100 and no /Zm at all all succeed. Measured again on the four
# largest units - dllmain.cpp at 10,657 lines, veh, map, init_thunks - where
# 100 through 600 behave identically, so nothing here is short of heap at 200.
# It is 2x the default for headroom and far below where the reservation
# breaks.
#
# THIS FLAG WAS THE WHOLE OF "C1060 ON AT LEAST ONE TRANSLATION UNIT". It was
# not one unit and it was not the source: 137 of 138 objects died on it. And
# because CMAKE_CXX_FLAGS is a CACHE variable, a build directory configured
# before this override landed keeps `/Zm1000` forever - reconfiguring never
# rewrites it. If every object fails with C1060, DELETE the build directory
# rather than reconfiguring it.
#
# It has to be undone through the make-rules override rather than here: this
# block is read BEFORE Windows-MSVC.cmake, which appends its `/Zm1000` to
# whatever was left behind, so a substitution at this point has nothing to
# substitute. The override file is included afterwards and still before
# `CMAKE_CXX_FLAGS` reaches the cache, which is the one window where the flag
# exists and is still editable.
set(CMAKE_USER_MAKE_RULES_OVERRIDE
    "${CMAKE_CURRENT_LIST_DIR}/cmake/vc6-flag-overrides.cmake")

set(CMAKE_FIND_ROOT_PATH "${VC6_ROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

project(OpenSMACX VERSION 0.2.3 LANGUAGES CXX)

include(CTest)

# ---------------------------------------------------------------------------
# GNU tools that are NOT the compiler.
#
# The DLL is built by VC6 and nothing else. Two things in this tree still want
# GNU binaries, and neither is a second compiler for the product:
#
#   * `nm` and `objdump` INSPECT objects. They read VC6's output natively -
#     `objdump -h` on a cl 12.00.8168 object reports `file format pe-i386` -
#     so the ABI checks that used to sit behind a second compiler's guard
#     work unchanged.
#     They were never gated on the compiler because they needed it; they were
#     gated on it because it happened to be what supplied them.
#
#   * a GNU C++ driver assembles `legacy-leaves.S` (`-x assembler-with-cpp`)
#     and compiles the static-recompilation pilot. Those are local
#     verification subsystems whose output is never committed and never linked
#     into OpenSMACX.dll, per docs/STATIC_RECOMPILATION.md (retired). VC6 cannot build
#     them and does not need to.
#
# Each is found separately so a host with binutils but no cross-compiler still
# gets the ABI gates, which is the pairing that matters. Absence is REPORTED,
# never silent - a gate that quietly stops existing is the failure mode
# tools/verify_checks_can_fail.py exists to prevent.
# ---------------------------------------------------------------------------
# Plain binutils, not a cross toolchain: `objdump -f` reports a cl 12.00.8168
# object as `pe-i386` on an ordinary Linux host, so the check needs the
# INSPECTORS and never needed a second compiler to have produced them.
find_program(OPENSMACX_NM NAMES nm)
find_program(OPENSMACX_OBJDUMP NAMES objdump)

if(OPENSMACX_NM AND OPENSMACX_OBJDUMP)
    set(OPENSMACX_HAVE_BINUTILS TRUE)
else()
    set(OPENSMACX_HAVE_BINUTILS FALSE)
    message(STATUS
        "binutils (nm/objdump) not found: the recovery-abi gates are SKIPPED. "
        "They read VC6 objects and are what checks that no recovered body "
        "carries exception unwind data the original does not have.")
endif()


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
        "Use cmake/toolchains/vc6.cmake, which drives cl 12.00.8168 under "
        "wine, when building from macOS or Linux.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR
        "OpenSMACX is injected into the 32-bit terranx.exe process and must be built for x86.")
endif()

set(OPENSMACX_SOURCES
    src/alpha.cpp
    src/main.cpp
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
    src/basepop.cpp
    src/basepop_font.cpp
    src/buffer.cpp
    src/buttongroup.cpp
    src/caviar.cpp
    src/council.cpp
    src/dialog.cpp
    src/dialogs.cpp
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
    src/scroll.cpp
    src/scenario.cpp
    src/sounddevice.cpp
    src/spot.cpp
    src/spot_recovery.cpp
    src/spying_recovery.cpp
    src/sprite.cpp
    src/stdafx.cpp
    src/stringbox.cpp
    src/stringstruct.cpp
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
)

# `<stdint.h>` and `<cstdint>` are C99 and C++11; cl 12.00.8168 ships neither,
# and thirteen `C1083: Cannot open include file` came from that alone. The
# shims are put on the include path rather than spliced into each includer so
# the source keeps saying the standard thing and there is nothing to undo when
# this compiler is retired. Guarded by version, so a modern build never
# shadows its own headers with these.
#
# DIRECTORY SCOPE, NOT PER TARGET. This was attached to `OpenSMACX` alone, which
# is a hand-maintained list of one: every target added afterwards that spelled
# `<cstdint>` broke in exactly the same way with exactly the same error, and one
# did. `recovery-gameplay-tests` compiles `tests/recovery_fixtures.h`, whose
# line 35 is `#include <cstdint>`, and it has been failing to build under the
# vc6 preset with `C1083: Cannot open include file: 'cstdint'`. Declared before
# the first target so every target in this directory inherits it.
#
# `src` is not optional: the two shims include "vc6_compat.h" by quoted name and
# no copy of it lives beside them.
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/cmake/vc6-include"
                        "${CMAKE_CURRENT_SOURCE_DIR}/src")
    # The C++11 spellings the shim supplies - `nullptr`, `constexpr`,
    # `static_assert`, the `namespace std` using-declarations - reach the DLL
    # through `src/stdafx.h`, which every one of its sources includes. The test
    # targets have no such header, so `src/globals_diff.h` and
    # `tests/globals_diff_tests.cpp` failed on `'std' : is not a class or
    # namespace name` and `'nullptr' : undeclared identifier`. Force-including
    # it here delivers the same shim to every target without a per-file edit,
    # which is the rule this directory already follows for the include path.
    # `#pragma once` in the header makes the double delivery to `stdafx.h`
    # sources a no-op, and `tools/vc6_cl.py` already translates `/FI`.
    add_compile_options("/FI${CMAKE_CURRENT_SOURCE_DIR}/src/vc6_compat.h")
endif()

# AN EXECUTABLE, NOT A DLL, SINCE 2026-08-12. The shared library existed to be
# injected into the shipped executable through 2,049 fixed-address redirects;
# that route is retired while the recovery is finished by byte-matching, which
# runs nothing (docs/RETIRED_ROUTES.md). What the build still has to answer is
# "does the recovered tree compile, and does every symbol it names resolve" -
# and an executable answers it BETTER, because every object is linked directly
# rather than pulled from an archive on demand, so an undefined symbol anywhere
# is a link error. The DLL gave that guarantee only for the 488 names its `.def`
# exported.
add_executable(OpenSMACX ${OPENSMACX_SOURCES})

target_compile_definitions(OpenSMACX PRIVATE
    $<$<CONFIG:Debug>:BUILD_DEBUG>
)
target_precompile_headers(OpenSMACX PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/stdafx.h")
target_link_libraries(OpenSMACX PRIVATE user32 gdi32 winmm)

set_target_properties(OpenSMACX PROPERTIES
    OUTPUT_NAME opensmacx-link-check
    PREFIX ""
    CXX_EXTENSIONS OFF
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
)

# VC6 is the only compiler, so its warning level is unconditional rather than
# a branch. The GNU warning set that used to sit here went with the toolchain.
#
# src/OpenSMACX.def is NOT linked. That file is generated by
# a retired generator, and every line in it is an ALIAS bridging an
# Itanium-mangled GCC symbol to the MSVC name the patched image imports.
# cl 12.00.8168 emits the MSVC name directly, so the aliases describe symbols
# this build does not produce. generate_vc6_exports (retired) is the VC6
# counterpart and is written but not yet wired in here; until it is, exports
# rest on __declspec(dllexport) alone and the export SET is not pinned.
target_compile_options(OpenSMACX PRIVATE /W4)

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

# Gated on the INSPECTORS, not on a compiler: objdump reads a cl
# 12.00.8168 object as pe-i386, so this check is compiler-agnostic and
# only ever needed nm and objdump to exist.
if(OPENSMACX_HAVE_BINUTILS)
    add_custom_target(verify-recovery-abi
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_recovery_abi.py"
            --nm "${OPENSMACX_NM}"
            --objdump "${OPENSMACX_OBJDUMP}"
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
endif()

if(BUILD_TESTING)
    # Direct-source tests for the gameplay core, and now the only behavioural
    # suite. It exists separately because the retired leaf binary defined
    # file-scope stand-ins for map.cpp and general.cpp symbols that its own
    # fixtures asserted against, so the real translation units could not be
    # linked beside them; splitting by link closure rather than by subject gave
    # the ~1,800 gameplay free functions a home. See docs/RETIRED_ROUTES.md.
    #
    # IT CARRIES THE EVIDENCE NOW. Of the 63 recovered functions of >=200 B
    # that any behavioural test observes - and byte-matching proves nothing
    # about a single one of them - this suite is the sole observer for 37 and a
    # joint observer for 17 more.
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
    target_compile_options(recovery-gameplay-tests PRIVATE -fno-exceptions)
    target_compile_definitions(recovery-gameplay-tests PRIVATE BUILD_DLL)
    set_target_properties(recovery-gameplay-tests PROPERTIES
        CXX_EXTENSIONS OFF
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
    )


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
    # The inspectors, not the compiler - see OPENSMACX_HAVE_BINUTILS.
    if(OPENSMACX_HAVE_BINUTILS)
        add_test(NAME recovery-abi
            COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_recovery_abi.py"
                --nm "${OPENSMACX_NM}"
                --objdump "${OPENSMACX_OBJDUMP}"
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









    # The runner that every Wine-backed test goes through. Its one job beyond
    # invoking wine is to stop the owned prefix afterwards, and skipping that
    # to "save teardown" cost 1.05 s per CTest invocation because the retained
    # session holds CTest's output pipe open.
    opensmacx_add_python_tool_test(windows-test-runner-tests test_run_windows_test.py)

    # Runs on the stock interpreter on purpose. The doctor's job is to diagnose
    # a machine that has none of the dependencies yet, so a test that needed
    # them would not be testing the thing that matters.





    opensmacx_add_python_tool_test(class-layout-tool-tests test_derive_class_layout.py)

    opensmacx_add_python_tool_test(disassembly-tool-tests test_disasm.py)

    # The experiment itself needs Visual C++ 6.0 under Wine and skips cleanly
    # without it, so only its offline half is registered: the COFF reader, the
    # normalisation, and the comparator. Those are the parts that could turn a
    # mismatch into a match, and they run everywhere.

    # The general oracle the experiment was extracted into. Same reasoning, and
    # one addition: it pins that the SHAPE_EXACT tier can actually FIRE. Its
    # first draft kept non-relocated immediates, which made it nearly redundant
    # with BYTE_EXACT and therefore almost unable to fire at all - a tier that
    # never fires is indistinguishable from one that cannot.
    opensmacx_add_python_tool_test(byte-match-tests test_byte_match.py)

    # The source-map grammar and the status tool built on it. Both are fully
    # offline: the scanner's modes, precedence and drift categories, and the
    # status tool's cache, ledger merge law (never truncate, never downgrade
    # BYTE_EXACT) and placeholder generator. The VC6 measurement itself is the
    # ratchet's job and the pilot's; these pin everything around it.
    opensmacx_add_python_tool_test(annotation-scan-tests
                                   test_annotation_scan.py)
    opensmacx_add_python_tool_test(decomp-status-tests test_decomp_status.py)

    # The preamble that measurement compiles a body in. This one is load
    # bearing in an unusual direction: a WRONG declaration here does not fail,
    # it silently changes what is being measured, so the tests pin the two
    # properties that keep it honest - nothing is emitted for a name the
    # scaffolding already declares (so a unit that compiles today is untouched,
    # text and cached verdict included), and a declaration it cannot make
    # standalone is refused rather than approximated.
    opensmacx_add_python_tool_test(src-declaration-tests
                                   test_src_declarations.py)

    # THE RATCHET, and it reads `src/` rather than a committed ledger. Every
    # piece proved to recompile to the shipped bytes carries the claim on its
    # own annotation - `// ORIGINAL: 0x... BYTE_EXACT` - so the floor is the
    # number of claims rather than a constant somebody remembers to bump, and
    # deleting a body deletes its claim. The previous form compared two
    # hardcoded totals against a 1.5 MB CSV, and counted one function
    # (0x0064F09C) whose body had been reset to `// BODY GOES HERE.`.
    #
    # It needs VC6 and the pinned image, which is not a new requirement: every
    # other measurement here already does, `tools/disasm.py` included, and the
    # image cannot be distributed. The old check's "compiler-free so CI can
    # enforce it" was protecting a CI that does not exist.
    add_test(NAME byte-match-ratchet
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/decomp_status.py" --check)

    # The COFF target emitter and the callee-declaration repair, both of which
    # objdiff pairs symbols through: a target object with the wrong symbol name
    # produces a diff against nothing, and a callee VC6 re-mangles differently
    # shows up as a `call` mismatch the agent cannot fix from the call site.
    opensmacx_add_python_tool_test(target-object-emitter-tests
                                   test_emit_target_object.py)

    # The symbol both objects have to carry. objdiff looks ONE name up on each
    # side, so a decoration rule off by an underscore or by four bytes of
    # argument count does not produce a wrong diff - it produces no diff at
    # all, on every affected function at once, which reads as "nothing has
    # matched yet". 1,179 catalogued functions were in exactly that state.
    opensmacx_add_python_tool_test(class-layout-tests test_class_layouts.py)
    opensmacx_add_python_tool_test(cast-classification-tests test_classify_casts.py)

    opensmacx_add_python_tool_test(idb-member-tests test_export_idb_members.py)

    opensmacx_add_python_tool_test(hypothesis-layout-tests
                                   test_emit_hypothesis_layouts.py)

    opensmacx_add_python_tool_test(thinker-layout-tests
                                   test_correlate_thinker_layouts.py)


    opensmacx_add_python_tool_test(access-bound-tests
                                   test_derive_access_bounds.py)


    opensmacx_add_python_tool_test(agent-brief-tests
                                   test_agent_brief.py)


    opensmacx_add_python_tool_test(vc6-wrapper-tests
                                   test_vc6_cl.py)




    opensmacx_add_python_tool_test(vtable-slot-tests
                                   test_vtable_slots.py)

    opensmacx_add_python_tool_test(no-duplicate-bodies-tests
                                   test_no_duplicate_bodies.py)

    # Fails when a line is inserted above a committed body and its
    # `source_locations` is not re-pointed. 1,592 had drifted before this
    # existed, and 619 of those far enough to score the wrong text entirely.
    opensmacx_add_python_tool_test(source-location-tests
                                   test_repair_source_locations.py)

    # Fails if a build artefact is ever scored for a row `src/` owns. Four
    # stale units resurrected NO_COMPILE rows as BYTE_EXACT and put them in
    # the ratchet floor.
    opensmacx_add_python_tool_test(collect-ownership-tests
                                   test_collect_ownership.py)

    opensmacx_add_python_tool_test(verify-function-tests
                                   test_verify_recovered_function.py)



    opensmacx_add_python_tool_test(yitzi-notes-tests
                                   test_correlate_yitzi_notes.py)


    opensmacx_add_python_tool_test(base-edge-tests
                                   test_derive_base_edges.py)

    opensmacx_add_python_tool_test(agreed-size-tests
                                   test_derive_agreed_sizes.py)

    opensmacx_add_python_tool_test(layout-verifier-tests
                                   test_verify_class_layouts.py)

    # The per-MEMBER layout check, as distinct from the per-CLASS one above.
    # `sizeof` cannot see a member split at the wrong boundary: drop a field,
    # append a pad of the same width, and every assertion in the tree still
    # passes while every offset after the hole is wrong. This compares what
    # src/ compiles to against the (offset, width) accesses the image's own
    # methods make, so it needs no recovered body to exercise a layout.
    opensmacx_add_python_tool_test(member-offset-tests
                                   test_verify_member_offsets.py)

    # The other half of the same evidence: an access inside a slab of unknown
    # storage proves a scalar starts there, so the slab can be split into named
    # members. The property these tests carry is EXTENT PRESERVATION - the
    # replacement must occupy exactly the bytes the slab did, or every member
    # after it moves. `--apply` re-probes and reverts on a size change, and
    # these are what make that revert rare rather than routine.

    # The third reading of a layout, and the only one that recovers TYPES. A
    # constructor's receiver IS the object it constructs, so `??0Palette` on
    # `this + 0x5BC` states that a Palette begins there - which is how
    # src/flic.h was found declaring a pointer where 0x454 bytes of object
    # live, and how 23 undeclared inheritance edges surfaced at offset zero.



    opensmacx_add_python_tool_test(recovery-symbol-tests
                                   test_recovery_symbols.py)
    opensmacx_add_python_tool_test(mizuchi-declfix-tests test_declfix.py)

    # The writeback is the only tool in this tree that edits src/ from a line
    # number, and every way its splice can go wrong leaves a file that still
    # looks right in review: a swallowed doc comment, an orphaned brace, a
    # deleted `Status: Complete`. These run without Wine because the half they
    # pin - the editing, the catalogue line shift, the restore-on-refusal - is
    # the half that mutates the tree.
    opensmacx_add_python_tool_test(mizuchi-writeback-tests
                                   test_writeback.py)

    # The scaffolding emitter. Its coverage went 38% -> 98% over the fan-out
    # population and every step was a defect, including a repair regex pinned
    # to wording CL does not use - which matched nothing and returned a
    # byte-identical compile rate. These pin the repaired behaviour.
    opensmacx_add_python_tool_test(emit-translation-unit-tests
                                   test_emit_translation_unit.py)

    opensmacx_add_python_tool_test(generator-support-tests test_generator_support.py)







    # The mutation runner is the only evidence that an oracle PASS means
    # anything, so its own bugs turn into confident wrong numbers everywhere
    # else. Two of its three known defects were classification, not plumbing:
    # the two kinds of INCONCLUSIVE mean opposite things and were scored the
    # same, and a mutation whose regex matched nothing vanished from the run
    # instead of being reported as an untested property.

    # The one module that decides what every published number MEANS: which
    # rows are in scope (5,673 of 6,000 - external_library is excluded by
    # written policy and the exclusion is published with the figure), how a row
    # is priced (the sum of body spans, never the address extent), and which
    # direction a number moves. Two callers compute the same block from
    # different row shapes - the exporter's in-memory dicts and the reused
    # export's CSV strings - so a difference between them would publish two
    # numbers for one tree.
    opensmacx_add_python_tool_test(recovery-metrics-tests test_recovery_metrics.py)

    # The frontier is what SELECTS work for a recovery batch, and it had no test
    # while it was printing a queue that was 45% already-proved functions -
    # `functions.csv` does not know about src/recovered/. These pin the join
    # against the source map in both directions: a proved address leaves the
    # queue, and it also resolves its callers so the frontier grows.
    opensmacx_add_python_tool_test(recovery-frontier-tests
                                   test_recovery_frontier.py)

    # The shape catalogue behind unproven_by_shape. A misclassified body is
    # invisible in the total, so the decisions are tested on synthetic bytes.

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


    # The whole-image summary: every headline number the project publishes.
    # The byte-weighted verdict table, "compared and agreed", the evidence
    # tiers under it, the three numbers that must go DOWN and the seed
    # histogram are all computed here and nowhere else, so an arithmetic slip
    # is not caught downstream - it is printed and believed. Byte weighting in
    # particular has already been got wrong once in this project, by reading a
    # field that is a SUM of body spans as if it were a contiguous address
    # extent. The fixtures are synthetic reports in a temp dir; nothing here
    # reads build/oracle.


    # Reads WHY a nearly-full-strength function fell short. Two of its guards
    # exist because the first real run tripped on them: `%#010x` of zero prints
    # no `0x`, so the pattern silently dropped exactly the null-dereference
    # lines that are the whole diagnosis; and a function that is N seeds short
    # owes N reasons, which is the only check that noticed. Fixtures are
    # captured oracle text; nothing here runs the oracle.

    # Canonical address -> the bytes our compiler produced, which is one half of
    # the SMT equivalence spike's input. Its failure mode is not finding
    # nothing, it is finding the WRONG body and looking certain: the tests pin
    # that a body ends at the next symbol IN ITS OWN SECTION, that an ambiguous
    # name is refused, and that i686 fastcall decoration is undone in the one
    # direction c++filt accepts. Fixtures are synthetic COFF; no build needed.

    # The Z3 x86 encoding. These tests do NOT establish that it is correct.
    # validate_x86_smt (retired) is what does that, by driving the encoding and
    # lifted_x86.h with the same states and requiring bit-for-bit agreement;
    # six deliberately damaged encodings were all caught by it.
    #
    # BUT NOTHING RUNS IT, INCLUDING THIS FILE. It is not registered as a test
    # and cannot be: its reference harness includes build/lifted/
    # lifted_runtime.h, which lift_whole_image (retired) generates and which is
    # gitignored, so a gate lane has no way to build it. On a tree without that
    # header it now says so instead of emitting a compiler error that reads
    # like an encoding fault. The agreement above is therefore a RECORDED PAST
    # RESULT, not a standing guarantee, and this comment used to claim
    # otherwise in the present tense.
    #
    # What IS pinned here is what validation cannot reach: that the formulas
    # are symbolic, since a prover never sees a concrete state.

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

    # The import boundary's derivation: which import lives at which IAT slot,
    # and how many bytes of argument each one pops. Both are recovered rather
    # than written down, and a wrong slot binds every call in the image to a
    # neighbouring function while every ABI check still passes - so the
    # derivation is cross-checked here against arithmetic on the PE's own
    # FirstThunk, independent of the pefile attribute the generator reads.









    # The inventory behind `unproven_recovered`. Its dangerous failure is
    # publishing a proof that is not one, so it is gated like the metric it
    # feeds.

    # The generator that emits those proofs. Same dangerous failure, one step
    # earlier: an oracle for a function the hybrid cannot execute both forms of
    # would carry a marker that lowers the metric while proving nothing.

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
    # The VCall shim the COMPILED tree uses. It is generated from the same
    # function the verification scaffolding uses, and a drift between the two
    # would let a body verify against one slot layout and call another.
    add_test(NAME vtable-shim-header-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/emit_vtable_shim_header.py"
            --check
    )
    add_test(NAME class-size-bounds-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/derive_class_size_bounds.py"
            --check
            --exe "${OPENSMACX_LEGACY_LEAF_EXE}"
    )
    # docs/recovery/verified-layouts.txt decides which classes are handed to an
    # agent with real members instead of an opaque shell, and it was the one
    # recovery artifact with no staleness gate while nine others had one.
    # test_class_layouts.py's VerificationGateTest only asserts
    # verified <= pinned: it catches a class that stopped being EXTRACTED and
    # is blind to one whose extracted SHAPE changed, which is the failure that
    # reaches an agent. This recompiles every probe.
    #
    # It drives ~/opt/vc6/.wineprefix, which is byte_match.py's own prefix and
    # not the ${OPENSMACX_WINE_PREFIX} that run_windows_test.py stops when it
    # finishes - so it needs no wineprefix RESOURCE_LOCK, and
    # verify_wine_test_locks.py correctly does not demand one. A SECOND
    # VC6-backed test would share this prefix and should take a lock of its own.
    # --check here also re-runs the control: a lower bound that exceeds a size
    # the tree already knows is a defect in the derivation, and gating
    # staleness without gating soundness would watch the wrong thing.
    add_test(NAME access-lower-bounds-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/derive_access_bounds.py"
            --check
            --exe "${OPENSMACX_LEGACY_LEAF_EXE}"
    )
    # No declared member boundary may be contradicted by the image. The
    # comparison is between two exact things - the offsets a VC6 probe reads
    # out of what src/ compiles to, and the (offset, width) accesses the
    # image's own thiscall methods make through `this` - so a straddle or an
    # overrun is a fact about the tree rather than a heuristic complaint.
    #
    # A REFUSED HEADER FAILS THIS TOO. A probe unit that does not compile
    # contributes no declared layout, every access into its classes then falls
    # through every test, and the run would print OK having measured nothing.
    # The damage case in test_verify_member_offsets.py caught exactly that on
    # its first run.
    #
    # Same VC6 prefix as class-layouts-current above, so the same reasoning
    # about RESOURCE_LOCK applies. Skips without the pinned image, reading the
    # published member-accesses.csv instead.
    add_test(NAME member-offsets-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_member_offsets.py"
            --check
            --exe "${OPENSMACX_LEGACY_LEAF_EXE}"
    )
    # Correlates an outside human's notes with the `// yitzi 0x134` markers on
    # the `static_assert`s, three ways: the note, the marker, and the size the
    # compiler pins. It checked a CSV against itself until 2026-08-12, which
    # could only fail if somebody hand-edited a generated file.
    #
    # Degrades rather than skipping: without the fetched, ignored note it still
    # refuses a marker that contradicts its own assertion, since that needs no
    # external source to detect. A stated size this tree declares no class for
    # (Faction) is reported and not failed - an open question is not a defect,
    # and failing on it would force somebody to invent a number to go green.
    add_test(NAME yitzi-sizes-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/correlate_yitzi_notes.py"
            --check
    )
    # Skips without the pinned image; every name here is read out of it.
    # Skips without the pinned image, which every verdict here depends on.
    add_test(NAME base-edges-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/derive_base_edges.py"
            --check
            --exe "${OPENSMACX_LEGACY_LEAF_EXE}"
    )
    # --check re-runs the control as well as comparing bytes. The agreement is
    # only admissible while it stays at zero wrong against the classes whose
    # size src/ already pins, and 30 of those pins now came FROM it.
    add_test(NAME agreed-class-sizes-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/derive_agreed_sizes.py"
            --check
    )
    # Skips cleanly without the ignored canonical IDB, which is the normal
    # state of a fresh checkout.
    add_test(NAME idb-members-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/export_idb_members.py"
            --check
    )
    # Both inputs - idb-members.csv and thinker-members.csv - are committed, so
    # this reproduces byte-for-byte on a clean checkout with no IDB, no wine
    # and no external analysis present.
    add_test(NAME hypothesis-layouts-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/emit_hypothesis_layouts.py"
            --check
    )
    add_test(NAME verified-layouts-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_class_layouts.py"
            --check
    )
    # Every `this`-relative cast in src/, resolved against the layout its
    # class declares. The committed CSV is the measurement; this refuses a
    # checkout where the code moved and the measurement did not.
    add_test(NAME cast-classification-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/classify_casts.py"
            --check
    )
    # The derivation's own regression gate. It existed and nothing ran it, so a
    # change that made the image-derived sizes answer WRONGLY - the one failure
    # mode that matters, as opposed to abstaining - would have been silent.
    add_test(NAME class-layout-derivation-pinned
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/derive_class_layout.py"
            --check-pinned
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
    # Every tool has an entry point, and every entry point names a tool that
    # exists. The obvious version of this audit - grep CMakeLists - is the one
    # that gets it wrong: it called mizuchi_context.py dead, and mizuchi.yaml
    # invokes it. docs/TOOLS.md is a root, so keeping a tool means placing it
    # there and deleting one means striking its line; neither can drift.
    add_test(NAME tool-reachability
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_tool_reachability.py"
    )
    opensmacx_add_python_tool_test(tool-reachability-tests
                                   test_verify_tool_reachability.py)

    # A retirement that deletes a tool leaves its CMake lines behind, and CMake
    # never objects: add_custom_target resolves nothing at configure time, and a
    # target outside `all` is never built. Twenty-three references survived that
    # way - COMMAND lines naming `prepare_hybrid_image (retired)`, a placeholder
    # spelling from rewriting tool names in place, and DEPENDS on
    # docs/recovery/functions.csv, deleted when the map moved into src/. A full
    # configure and a 173-step build passed over every one of them.
    add_test(NAME cmake-paths
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_cmake_paths_exist.py"
            --check
    )
    opensmacx_add_python_tool_test(cmake-paths-tests
                                   test_verify_cmake_paths_exist.py)

    # `unittest.main()` collects the classes defined SO FAR, so anything below
    # that block never runs - and the file still exits 0, so the gate invoking
    # it goes green over a suite it did not execute. test_emit_translation_unit
    # ran 57 of its 73 tests that way, with sixteen stranded below the block.
    # Neither test-registration nor tool-test-registration could see it: the
    # file WAS registered and CMake DID run it. `unittest discover` imports the
    # module and finds everything, so the defect is invisible from a discovery
    # run and appears only under the invocation the gate uses.
    add_test(NAME tests-all-run
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_tests_all_run.py"
    )
    opensmacx_add_python_tool_test(tests-all-run-tests
                                   test_verify_tests_all_run.py)

    add_test(NAME tool-test-registration
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_tool_test_registration.py"
    )
    opensmacx_add_python_tool_test(tool-test-registration-tests
                                   test_verify_tool_test_registration.py)

    # AGENTS.md has always said a test must never write into the source tree.
    # Nothing enforced it, and test_integrate_recovery.py was deleting
    # src/recovered/00401000.cpp - a BYTE_EXACT proof no other file carried -
    # and re-pointing 37 rows of functions.csv on EVERY ctest run. It also made
    # test_repair_source_locations pass: that test asserts the committed
    # catalogue has not drifted, and it was green because the offender sorts
    # first and had just silently repaired it.
    #
    # It runs the 111 tool tests a second time (134 s measured) because there
    # is no cheaper honest form: the property is dynamic, and running each file
    # in its own subprocess is what lets a failure NAME the offender instead of
    # reporting that the suite was dirty.
    add_test(NAME tests-do-not-write
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/verify_tests_do_not_write.py"
    )
    opensmacx_add_python_tool_test(tests-do-not-write-tests
                                   test_verify_tests_do_not_write.py)

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

    # `verify_check_tests_observe.py` is the other half of the same question:
    # checks-can-fail asks whether a check CAN refuse, this asks whether its
    # tests would NOTICE if it stopped. It is a diagnostic that is run
    # deliberately rather than a gate, so only its unit tests are registered -
    # but they must be, or the file is one CMake never executes.
    opensmacx_add_python_tool_test(check-tests-observe-tests
                                   test_verify_check_tests_observe.py)

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

    # measure_observability.py answers "does anything watch this function" for
    # one build per function instead of ~152, which is what made asking it over
    # the whole corpus possible at all. Deliberately NOT registered as a gate
    # test: a full run is ~30 minutes against a 107 s lane. Only its unit tests
    # run here; the corpus figure belongs in docs/RECOVERY_ECONOMICS.md, where
    # it is dated, because it moves as fixtures are written.

    # `observability-ratchet` was RETIRED as a gate on 2026-08-12 (see
    # docs/RETIRED_ROUTES.md): it made every new recovery of >=200 B fail until
    # someone ran measure_observability.py on it, a per-recovery tax payable only
    # in the currency of the route being retired. The TOOL stays and is still
    # runnable, so its unit tests still run - a surviving tool with an
    # unregistered test file is exactly what tool-test-registration refuses.

    # The ratchet. 141 of 193 recovered functions of >=200 B are observed by
    # nothing; writing 131 retroactive fixtures would halt the objective, so the
    # debt is published and capped rather than paid off. A new recovery absent
    # from the census fails, and the counts may fall but never rise. Compares
    # two committed files and builds nothing.

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

    add_test(NAME derived-prototypes-current
        COMMAND "${OPENSMACX_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/derive_prototypes_from_names.py"
            --check
    )

endif()

install(TARGETS OpenSMACX
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
    ARCHIVE DESTINATION lib
)

if(BUILD_TESTING)
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
            recovery-gameplay-tests
            globals-diff-tests
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

    # The two GNU-assembled entries are appended only when the driver that can
    # build them was found. They are the differential-oracle track: a 32-
    # function suite that runs a recovered body beside the original at its real
    # address, which is behavioural evidence byte-matching does not give. It is
    # worth keeping and it is not worth REQUIRING - its input lives under
    # .opensmacx/, which .gitignore excludes, so a checkout without the
    # user-owned executable never had it either. Listing them unconditionally
    # made a missing cross-assembler a configure error in a build whose product
    # does not use one: legacy-leaves.obj is not linked into OpenSMACX.dll.
    # Was a second compiler's probe gating `verify-recovery-oracles`, a
    # second compiler gating a target this tree no longer defines. Both went
    # with the oracle route; the variable is empty rather than deleted because
    # the DEPENDS below still interpolates it.
    set(OPENSMACX_GATE_ORACLE_DEPENDS)

    add_custom_target(verify-recovery-batch
        COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure
            --parallel "${OPENSMACX_CTEST_JOBS}"
        DEPENDS
            verify-recovery-abi
            ${OPENSMACX_GATE_ORACLE_DEPENDS}
            recovery-gameplay-tests
            globals-diff-tests
        USES_TERMINAL
        VERBATIM
    )
endif()
