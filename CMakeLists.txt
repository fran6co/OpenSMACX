cmake_minimum_required(VERSION 3.20)

# ---------------------------------------------------------------------------
# THE TOOLCHAIN IS DEFAULTED IN HERE, not passed on the command line.
#
# Everything VC6-specific - the cross system, the wrappers, the forced
# compiler id, the depfile and /Z7 and /Zm corrections - lives in
# cmake/toolchains/vc6.cmake, where anyone reading CMAKE_TOOLCHAIN_FILE can
# find it: CLion resolving <windows.h>, a future second toolchain, a human.
# It was hardcoded in this file until 2026-08-15, and as long as it was, the
# include search paths reached the compiler only through the wrapper's
# environment and no tool outside the build could see them.
#
# The default is a default, not a cage: -DCMAKE_TOOLCHAIN_FILE=... overrides
# it, and the guard stands aside for anything the user passes. CMakePresets
# stay retired (AGENTS.md, 2026-08-12) - this default is what they were
# rejected for providing, provided without one more file to forget.
# `cmake -B build` is still the whole invocation.
# ---------------------------------------------------------------------------
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/toolchains/vc6.cmake"
        CACHE FILEPATH "VC6 under wine is the only toolchain this tree builds with")
endif()

# THE BUILD'S OWN FLAGS, WRITTEN DOWN WHERE A TOOL CAN READ THEM.
#
# `recover measure` groups mapped bodies by their source translation unit
# and obtains compiler recipes through the configured environment. It must
# compile that file the way the build compiles it. Older tools approximated: copy the
# file's own directory beside it and hope. That resolves `#include "win.h"`
# and nothing else, so `src/basewin.cpp` died on
# `C1083: Cannot open include file: 'cstdint'` - which is supplied by
# `cmake/vc6-include`, added below, along with a forced include of
# `src/vc6_compat.h`.
#
# Mirroring those two into the tool would be a second copy of this file's
# decisions, and the copy goes stale the first time an include path is added.
# CMake already knows; this makes it say so.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL
    "write compiler recipes for the configured match-recovery environment"
    FORCE)

# RELEASE BY DEFAULT. The matching milestone is the linked executable, and
# `recover measure`/`record` and `recover prepare` all use the Release recipe
# (recovery/prepare.json, recovery/build.json). Left unset, CMake's
# Windows-MSVC platform module initializes CMAKE_BUILD_TYPE to Debug during
# project(), so a plain `cmake -B build` produced `/Od /Ob0 /GZ` objects
# whose claimed functions did not match (2026-11-08: 4,561 of 4,824
# measured-exact claims differed; the Release tree differed in none). A
# Debug tree also rewrote build/compile_commands.json with Debug recipes
# (-DBUILD_DEBUG, no /DNDEBUG) that measurement then read. Debug stays
# available for winedbg/gdb with an explicit -DCMAKE_BUILD_TYPE=Debug in a
# separate directory; it no longer exports a recipe database (after
# project(), where the toolchain turns the export on).
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Release reproduces the measured matching recipe; Debug is for debugging only")
endif()

# `C` AS WELL AS `CXX`, for zlib. zlib 1.0.2 uses K&R parameter definitions,
# which no C++ compiler will parse - `byte_match.compile_unit` already writes
# each vendored unit under its own `.c` suffix for exactly this reason. The
# toolchain has defined `CMAKE_C_COMPILER` all along; only this line was
# keeping the language out.
project(OpenSMACX VERSION 0.2.3 LANGUAGES CXX C)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Debug recipes are not matching recipes; never replace the measured database.
    set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
    set(CMAKE_EXPORT_COMPILE_COMMANDS OFF CACHE BOOL
        "Debug recipes are not matching recipes" FORCE)
endif()

# ASM_MASM for the routines the original wrote as MASM modules and VC6 inline
# asm cannot express (AGENTS.md, "Inline assembly"; user decision 2026-11-07).
# The toolchain points it at ML 6.11d under the same wine wrapper as cl.
# C/C++-only options below are scoped with COMPILE_LANGUAGE so they never
# reach it.
enable_language(ASM_MASM)

# The original allocator includes the static multithreaded CRT's lock and
# small-block-heap code (src/vendor/vc6-runtime.recovery.h). CMake's default
# DLL runtime instead links import thunks, even when application bodies match.
# Keep this runtime in every configuration; debug code generation must not
# silently select a different allocator. This project decision applies to zlib
# as well as C++ and is exported in the compilation database for measurements.
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

# The forced cross-toolchain skips normal compiler viability checks. If its
# identification command cannot run, CMake can still succeed with generic
# recipes (including -o instead of /Fo). Those recipes are unusable for
# matching; refuse them before preparation can publish a replacement database.
if(NOT CMAKE_CXX_COMPILER_ID OR NOT CMAKE_C_COMPILER_ID)
    message(FATAL_ERROR
        "Compiler identification failed; refusing to generate recovery recipes. "
        "Check compiler access and the configured toolchain environment "
        "(VC6/Wine and VC6_PREFIX for the default toolchain).")
endif()

if(NOT WIN32)
    message(FATAL_ERROR
        "The current OpenSMACX target is a Windows application. "
        "Use cmake/toolchains/vc6.cmake, which drives cl 12.00.8168 under "
        "wine, when building from macOS or Linux.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR
        "OpenSMACX recovers a 32-bit x86 application and must be built for x86.")
endif()

# ZLIB IS A DEPENDENCY, NOT A RECOVERY. `terranx.exe` statically links zlib
# 1.0.2 and says so in its own strings, and `src/vendor/zlib-1.0.2/` holds
# that release unmodified - see PROVENANCE.md there. Thirty-nine functions
# in the image are its code, and for a while each carried an annotation
# under that directory claiming a byte-exact RECOVERY. They were never
# recoveries: nobody here wrote them, and the same argument
# `docs/EXCLUSIONS.md` section 1 makes about the CRT applies - code that can
# be compiled or linked is not code to rewrite.
#
# So it is built, as its own target with its own flags, and DECLARED BEFORE
# the `include_directories` and `add_compile_options` below. Those are
# directory-scope and reach every target created after them, including the
# `/FI src/vc6_compat.h` that pulls in `<new>` - which is how a C target
# ends up on `eh.h(32): fatal error C1189: "eh.h is only for C++!"`. zlib
# 1.0.2 must compile as C: it uses K&R parameter definitions that no C++
# compiler will parse.
add_library(zlib STATIC
    src/vendor/zlib-1.0.2/adler32.c
    src/vendor/zlib-1.0.2/compress.c
    src/vendor/zlib-1.0.2/deflate.c
    src/vendor/zlib-1.0.2/infblock.c
    src/vendor/zlib-1.0.2/infcodes.c
    src/vendor/zlib-1.0.2/inffast.c
    src/vendor/zlib-1.0.2/inflate.c
    src/vendor/zlib-1.0.2/inftrees.c
    src/vendor/zlib-1.0.2/infutil.c
    src/vendor/zlib-1.0.2/trees.c
    src/vendor/zlib-1.0.2/uncompr.c
    src/vendor/zlib-1.0.2/zutil.c
)
target_include_directories(zlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/zlib-1.0.2")
# `/W0` and no forced include: PROVENANCE.md's whole point is that these
# files are the release and not a copy somebody tidied, so the build adapts
# to them rather than the other way round. The original was compiled without
# /Ob2: its static helpers (putShortMSB, flush_pending, bi_*, ...) stay out of
# line, so /Ob1 overrides CMake's default C release flags.
target_compile_options(zlib PRIVATE /W0 $<$<NOT:$<CONFIG:Debug>>:/Ob1>)

set(OPENSMACX_SOURCES
    src/alphamenu.cpp
    src/console_menu_policy.cpp
    src/alpha.cpp
    src/alphasave.cpp
    src/main.cpp
    src/multiwin.cpp
    src/matrix3.cpp
    src/span_math.cpp
    src/scale_adjustment.cpp
    src/technology_queries.cpp
    src/fx.cpp
    src/alphanet.cpp
    src/alphanet_records.cpp
    src/base.cpp
    src/base_build.cpp
    src/score.cpp
    src/basebutton.cpp
    src/basepop.cpp
    src/council.cpp
    src/dialog.cpp
    src/dialogs.cpp
    src/faction.cpp
    src/faction_treaty.cpp
    src/aggression.cpp
    src/famewin.cpp
    src/flatbutton.cpp
    src/flic.cpp
    src/font_queue.cpp
    src/interface_fonts.cpp
    src/interface_assets.cpp
    src/game.cpp
    src/turn.cpp
    src/setup.cpp
    src/colortables.cpp
    src/game_calendar.cpp
    src/newgame.cpp
    src/application_math.cpp
    src/application_text.cpp
    src/general.cpp
    src/danger.cpp
    src/crt_random.cpp
    src/graphicwin.cpp
    src/graphicwin_events.cpp
    src/maininterface.cpp
    src/worldwin.cpp
    src/map.cpp
    src/map_grid.cpp
    src/world.cpp
    src/mapwin.cpp
    src/mapwin_terrain.cpp
    src/basewin.cpp
    src/datalink.cpp
    src/help.cpp
    src/datalink_entries.cpp
    src/netwin.cpp
    src/socialwin.cpp
    src/designwin.cpp
    src/diplowin.cpp
    src/alphamovie.cpp
    src/movie_decoder_idct.cpp
    src/infowin.cpp
    src/interlude.cpp
    src/statuswin.cpp
    src/wave.cpp
    src/battlewin.cpp
    src/councwin.cpp
    src/credits.cpp
    src/diplopop.cpp
    src/gamma.cpp
    src/messagewin.cpp
    src/newtechwin.cpp
    src/picktech.cpp
    src/monuwin.cpp
    src/pickwin.cpp
    src/prodpicker.cpp
    src/setupwin.cpp
    src/multidebug.cpp
    src/tutwin.cpp
    src/popmenu.cpp
    src/popup.cpp
    src/popup_wrappers.cpp
    src/netmsg.cpp
    src/xpops.cpp
    src/planwin.cpp
    src/playerlock.cpp
    src/netdaemon.cpp
    src/netdaemon_message_data.cpp
    src/prefwin.cpp
    src/prefwin_geometry.cpp
    src/quaylewin.cpp
    src/replaywin.cpp
    src/reportif.cpp
    src/console.cpp
    src/console_commands.cpp
    src/uv2player.cpp
    src/sound.cpp
    src/net_class.cpp
    src/node.cpp
    src/squarelock.cpp
    src/lock.cpp
    src/interface_palette.cpp
    src/path.cpp
    src/probe.cpp
    src/scroll.cpp
    src/scenario.cpp
    src/sounddevice.cpp
    src/sprite_objects.cpp
    src/stdafx.cpp
    src/stringbox.cpp
    src/stringlist.cpp
    src/stringstruct.cpp
    src/strings.cpp
    src/string_formatting.cpp
    src/technology.cpp
    src/terraforming_actions.cpp
    src/time.cpp
    src/enemy.cpp
    src/veh.cpp
    src/orders.cpp
    src/battle.cpp
    src/veh_stack.cpp
    src/vehicle_render.cpp
    src/veh_queries.cpp
    src/vehicle_storage.cpp
    src/vehicle_cost.cpp
    src/win.cpp
    src/winnodelist.cpp
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
# did - the since-retired `recovery-gameplay-tests`, which failed under the vc6
# preset with `C1083: Cannot open include file: 'cstdint'`. Directory scope is
# kept because the next target to be added will spell it too. Declared before
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
    # sources a no-op, and `recovery/environment/wine.py` already translates `/FI`.
    add_compile_options("$<$<COMPILE_LANGUAGE:C,CXX>:/FI${CMAKE_CURRENT_SOURCE_DIR}/src/vc6_compat.h>")
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
#
# IT IS `OpenSMACX.exe` SINCE 2026-08-15, not `opensmacx-link-check.exe`. The
# link check was all it could be while `main()` returned 0 and nothing else was
# reachable; it now has the shipped entry point - the recovered `_WinMain@16`
# in src/main.cpp - and calls the game's own startup sequence from it. It is
# not yet a recovered or accepted game. But the artifact is the program
# being recovered rather than a probe, and its name should say so.
# JACKAL.LIB (Firaxis's toolkit) is a distinct library. Its whole-file members
# are archive inputs, with no synthetic retention anchor; src/jackal/CMakeLists.txt
# records the image evidence for its membership. Files that still mix
# library members with game-side inline copies remain application inputs until
# they are split. This is not a claim of complete JACKAL.LIB recovery.
add_subdirectory(src/jackal)
# The AnimaTek Caviar/VOX library is its own archive with its own (VC5)
# compiler recipe; Firaxis's Caviar wrapper is Jackal's src/jackal/caviar.cpp.
add_subdirectory(src/caviar)
# EA UK's uV/uV2 movie player ("Play uV/uV2 Version 1.50, Martin Griffiths,
# Electronic Arts UK 1997"): a separate third-party archive in the original,
# linked after the game objects and before ddraw.lib's thunk and JACKAL.LIB.
add_subdirectory(src/uv)
# These owners retain frame-based parameter accesses. Match their
# existing explicit recovery measurement profiles without changing the
# application's default compiler flags.
set_source_files_properties(
    src/wave.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# The Midi, device-wrapper and sound-loader bodies also retain frame-based
# argument accesses in the measured application release profile.
set_source_files_properties(
    src/sounddevice.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve the frame-based argument accesses in these interface owners while
# keeping Win and the application-wide compiler defaults unchanged.
set_source_files_properties(
    src/datalink.cpp
    src/help.cpp
    src/datalink_entries.cpp
    src/maininterface.cpp
    src/console.cpp
    src/console_commands.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve original frame-based arguments in the map and base interface owners.
set_source_files_properties(
    src/mapwin.cpp
    src/mapwin_terrain.cpp
    src/basewin.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Keep the controlled diplomacy/network/overview owners on their measured
# frame-preserving source profiles; application defaults and link settings stay fixed.
set_source_files_properties(
    src/netdaemon.cpp
    src/netdaemon_message_data.cpp
    src/diplowin.cpp
    src/diplopop.cpp
    src/worldwin.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve the original frame-based stack accesses in these mapped owners;
# leave the global compiler defaults unchanged.
set_source_files_properties(
    src/designwin.cpp
    src/tutwin.cpp
    src/xpops.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Original interface palette initialization and font lookup retain a frame
# pointer; generic library bodies have independently measured profiles.
set_source_files_properties(src/interface_palette.cpp src/interface_fonts.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# The co-located Sound-family lifetimes retain the original frame-based
# cleanup/deleting entries. Release already supplies /Ob2 for base inlining.
set_source_files_properties(src/sound.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve original frame-based argument and local-slot accesses in these core
# owners while keeping application-wide compiler defaults unchanged.
# World generation keeps its existing map.cpp profile after source separation;
# this does not establish the historical translation-unit boundary.
set_source_files_properties(
    src/map.cpp
    src/map_grid.cpp
    src/world.cpp
    src/faction.cpp
    src/faction_treaty.cpp
    src/aggression.cpp
    src/veh.cpp
    src/orders.cpp
    src/battle.cpp
    src/veh_stack.cpp
    src/vehicle_render.cpp
    src/veh_queries.cpp
    src/enemy.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve the mapped frame-based vehicle owners after their source split.
# Their original translation-unit names are unknown; other veh.cpp code stays
# on its existing owner profile.
set_source_files_properties(
    src/vehicle_cost.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve original frame-based accesses in the remaining application owners.
# Keep all other source recipes on their independently measured defaults.
# Newgame's original entries and the measured in-class Flic cleanup retain this
# source profile; this does not assert the original translation-unit boundary.
set_source_files_properties(
    src/game.cpp
    src/turn.cpp
    src/setup.cpp
    src/colortables.cpp
    src/interface_assets.cpp
    src/game_calendar.cpp
    src/newgame.cpp
    src/netwin.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve measured frame-based accesses across these application window owners.
# Keep all other compiler and measurement policies unchanged.
set_source_files_properties(
    src/reportif.cpp
    src/statuswin.cpp
    src/prodpicker.cpp
    src/alphamenu.cpp
    src/console_menu_policy.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve measured frame-based accesses in the qualifying owner cohort.
# Generic Sprite implementation and networking retain their measured default recipes.
set_source_files_properties(
    src/stringstruct.cpp
    src/councwin.cpp
    src/planwin.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Keep the application Popup on its measured frame-based profile while the
# generic modal API remains on the default frameless profile.
set_source_files_properties(
    src/popup.cpp
    src/popup_wrappers.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve the measured frame-based contexts for the PopMenu and SquareLock
# owners without changing their other compiler options.
set_source_files_properties(
    src/popmenu.cpp
    src/squarelock.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Application formatters retain calls into the separately compiled string table.
# The Strings implementation keeps its independently measured default recipe.
set_source_files_properties(src/string_formatting.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve measured frame-based accesses across the application owner batch.
# AlphaNet record/lifecycle code has its own default-profile owner below.
set_source_files_properties(
    src/infowin.cpp
    src/alphasave.cpp
    src/famewin.cpp
    src/messagewin.cpp
    src/setupwin.cpp
    src/path.cpp
    src/picktech.cpp
    src/pickwin.cpp
    src/council.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve measured frame-based accesses in the remaining owner cohort.
# Credits retains its default TU optimization/frame policy.
set_source_files_properties(
    src/prefwin.cpp
    src/fx.cpp
    src/replaywin.cpp
    src/multidebug.cpp
    src/newtechwin.cpp
    src/quaylewin.cpp
    src/battlewin.cpp
    src/lock.cpp
    src/monuwin.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve measured frame-based accesses in the core application cohort.
# Mixed or non-improving owners retain their measured default recipes.
set_source_files_properties(
    src/interlude.cpp
    src/netmsg.cpp
    src/playerlock.cpp
    src/probe.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve Alpha's measured frame-based application context.
# Shared graphics language state and its setter have a separate library owner.
set_source_files_properties(src/alpha.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")
# The original diagnostic wrapper retains EBP-based parameter accesses.
# Keep its calls out of line through a separate owner, with the existing frame profile.
set_source_files_properties(src/danger.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve the measured frame-based AlphaNet controller context. Its record
# and lifecycle owner remains on the default application profile.
set_source_files_properties(src/alphanet.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve measured frame-based accesses in the remaining application owners.
# Mixed generic/application owners retain their independently measured defaults.
set_source_files_properties(
    src/base.cpp
    src/base_build.cpp
    src/score.cpp
    src/multiwin.cpp
    src/socialwin.cpp
    src/technology.cpp
    src/technology_queries.cpp
    src/uv2player.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Danger's measurement recipe is plain /O2 (VC6: /Ob1); keep the build equal.
# This optimized matching exception must not override Debug's /Ob0.
set_property(SOURCE
    src/danger.cpp
    APPEND PROPERTY COMPILE_OPTIONS "$<$<NOT:$<CONFIG:Debug>>:/Ob1>")
# Preserve the original frame-based AlphaMovie owner context without changing
# its independently measured /Ob2 inlining policy.
set_source_files_properties(src/alphamovie.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")
# Gamma's modal window keeps its measured frame context; the independent
# PushButton implementation retains its default frameless context.
set_source_files_properties(src/gamma.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")
# Retain standalone compiler type evidence for these source-owned global/member
# groups. This adds observability, not a different optimization/frame policy;
# reference checks consume each production object's own types and relocations.
set_property(SOURCE
    src/alpha.cpp
    src/base.cpp
    src/base_build.cpp
    src/faction.cpp
    src/aggression.cpp
    src/score.cpp
    src/tutwin.cpp
    src/veh.cpp
    src/orders.cpp
    src/battle.cpp
    src/veh_stack.cpp
    src/enemy.cpp
    src/strings.cpp
    src/sounddevice.cpp
    src/fx.cpp
    src/sound.cpp
    src/netmsg.cpp
    src/reportif.cpp
    src/console.cpp
    src/statuswin.cpp
    src/vehicle_storage.cpp
    src/credits.cpp
    src/game.cpp
    src/turn.cpp
    src/setup.cpp
    src/colortables.cpp
    src/game_calendar.cpp
    src/interface_palette.cpp
    src/interface_assets.cpp
    src/maininterface.cpp
    src/map.cpp
    src/map_grid.cpp
    src/mapwin.cpp
    src/mapwin_terrain.cpp
    src/netdaemon.cpp
    src/prefwin.cpp
    src/newtechwin.cpp
    src/console_commands.cpp
    src/net_class.cpp
    src/terraforming_actions.cpp
    src/world.cpp
    src/win.cpp
    src/datalink.cpp
    src/designwin.cpp
    src/diplowin.cpp
    src/general.cpp
    src/infowin.cpp
    src/messagewin.cpp
    src/netwin.cpp
    src/newgame.cpp
    src/socialwin.cpp
    src/vehicle_render.cpp
    APPEND PROPERTY COMPILE_OPTIONS "/Z7")
# Keep these measured frame-based helper owners on their explicit source profile.
set_source_files_properties(
    src/span_math.cpp
    src/scale_adjustment.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Application timer integration retains the pinned argument frame. The generic
# Time implementation is compiled separately in Jackal with its existing profile.
set_source_files_properties(src/time.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve the measured frame-based contexts of the shared stream wrappers.
# General's unrelated helpers retain their independently measured default recipe.
set_source_files_properties(src/crt_random.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Application arithmetic and text utilities use the established frame profile.
# The remaining General helpers retain their existing source recipe.
set_source_files_properties(src/application_math.cpp src/application_text.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Terraforming providers and actions retain their pinned EBP-based frames.
set_property(SOURCE src/terraforming_actions.cpp APPEND PROPERTY COMPILE_OPTIONS "/Oy-")
add_executable(OpenSMACX ${OPENSMACX_SOURCES})

# Terrain, media decoder and GraphicWin lookup data remain local derivatives of
# the pinned original. Source owns bounded objects or explicit accessed windows;
# each generated include lives in
# ignored preparation context, also for IDE builds outside the build/ tree.
find_program(OPENSMACX_UV_EXECUTABLE NAMES uv REQUIRED)
if(NOT DEFINED OPENSMACX_ORIGINAL_IMAGE)
    if(DEFINED ENV{OPENSMACX_IMAGE})
        set(OPENSMACX_ORIGINAL_IMAGE "$ENV{OPENSMACX_IMAGE}" CACHE FILEPATH "Pinned original for terrain data")
    else()
        set(OPENSMACX_ORIGINAL_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/.opensmacx/game/terranx_original.exe"
            CACHE FILEPATH "Pinned original for terrain data")
    endif()
endif()
set(OPENSMACX_TERRAIN_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/recovery/preparation/terrain-data")
execute_process(
    COMMAND "${OPENSMACX_UV_EXECUTABLE}" run --project "${CMAKE_CURRENT_SOURCE_DIR}" python -c
        "import sys; from pathlib import Path; from runpy import run_path; run_path(sys.argv[1])['project_initializers'](Path(sys.argv[2]), Path(sys.argv[3]), Path(sys.argv[4]))"
        "${CMAKE_CURRENT_SOURCE_DIR}/recovery/terrain_data.py" "${CMAKE_CURRENT_SOURCE_DIR}"
        "${OPENSMACX_ORIGINAL_IMAGE}" "${OPENSMACX_TERRAIN_DIRECTORY}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE OPENSMACX_TERRAIN_RESULT ERROR_VARIABLE OPENSMACX_TERRAIN_ERROR)
if(NOT OPENSMACX_TERRAIN_RESULT EQUAL 0)
    message(FATAL_ERROR "Pinned terrain data generation failed: ${OPENSMACX_TERRAIN_ERROR}")
endif()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${OPENSMACX_ORIGINAL_IMAGE}" "${CMAKE_CURRENT_SOURCE_DIR}/src/IMAGE_PIN"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/mapwin.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/interface_assets.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/mapwin_terrain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/map.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/map_grid.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/uv/movie_media_decoder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/graphicwin.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/recovery/terrain_data.py")
target_include_directories(OpenSMACX PRIVATE "${OPENSMACX_TERRAIN_DIRECTORY}")
# The Firaxis Caviar wrapper (Jackal) and the uV decoder read generated tables too.
target_include_directories(jackal PRIVATE "${OPENSMACX_TERRAIN_DIRECTORY}")
target_include_directories(uv PRIVATE "${OPENSMACX_TERRAIN_DIRECTORY}")

target_compile_definitions(OpenSMACX PRIVATE
    $<$<CONFIG:Debug>:BUILD_DEBUG>
)
# Stable logical TU identity and guarded /Brepro are shared with recovery
# recipes. VC6 otherwise time-seeds anonymous names and can reorder global
# storage between identical compilations. The wrapper refuses macro changes.
# PCH reuse cannot establish the fresh preprocessing evidence this requires.
target_compile_options(OpenSMACX PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:--repro-root=${CMAKE_CURRENT_SOURCE_DIR}>")
# The assembler recipe: COFF objects, MASM 6 syntax. `recovery/project.json`
# pins the same switches for measurement of each assembler source.
target_compile_options(OpenSMACX PRIVATE "$<$<COMPILE_LANGUAGE:ASM_MASM>:-nologo;-coff>")
# `dxguid` carries the IIDs `Buffer::init` names - without it the link fails
# with `unresolved external symbol _IID_IDirectDrawSurface`, measured. It used
# to be a `#pragma comment(lib, "dxguid.lib")` in buffer.cpp, "so the
# dependency sits beside the code that needs it"; the Winmm pragma that
# comment cited as precedent turned out to be redundant with this line, so
# the precedent was one entry that did nothing.
# `ddraw` carries `DirectDrawCreate`, which `DDInit::init` names. It was a
# `#pragma comment(lib, "ddraw.lib")` in win.cpp for the same stated reason
# dxguid's was in buffer.cpp - "so the dependency sits beside the code that
# needs it" - and it belongs here for the same reason that one moved: a link
# input is the build's business, and a pragma hides it from anyone reading
# the build.

# FileBox owns the open/save dialog imports from comdlg32.
target_link_libraries(OpenSMACX PRIVATE uv jackal caviar zlib user32 gdi32 winmm dxguid ddraw vfw32 dsound comdlg32)

# `/MAP /MAPINFO:LINES` is the only debug format in this toolchain that gdb can
# be taught to read. VC6's own three cannot be: the PDB is 2.0 (`JG`, which
# LLVM's reader refuses), the CodeView in `.debug$S` is signature 2 (LLVM
# handles 4), and `/debugtype:coff` segfaults gdb 15.1 on `file` alone -
# measured, all three. The map carries every public with its final address and,
# with `/MAPINFO:LINES`, a line table per source, which is what
# `recover debug-sidecar` turns into an ELF gdb loads beside a live winedbg.
# It is a side file: the linked bytes are the same with it and without.
target_link_options(OpenSMACX PRIVATE
    "/MAP:${CMAKE_CURRENT_BINARY_DIR}/OpenSMACX.map" "/MAPINFO:LINES"
    # The original retains absolute-pointer fixups. VC6's nonincremental
    # Release link otherwise drops them even when instruction operands agree.
    "/FIXED:NO"
    # The Caviar renderers patch their own immediates (src/caviar/caviar_part_*.asm,
    # src/caviar/caviar_shadow_*.asm): the original's _SELFMOD section is execute,
    # read and write (0xE0000020).
    "/SECTION:_SELFMOD,ERW"
    # Capture final-pass input labels separately from the debugger map. The
    # compiler wrapper owns this output; the recovery package parses it.
    "/VERBOSE" "--link-diagnostics=${CMAKE_CURRENT_BINARY_DIR}/OpenSMACX.link.log")

# The sidecar is DERIVED FROM THE LINK, so it is built by it. Left as a command
# to remember, it goes stale the moment anything moves, and a stale one is
# worse than none: gdb reads it without complaint and reports names for the
# addresses of an older build. It reads the executable, map and objects from
# this build. Source path mapping and frame-base guarantees are toolchain policy,
# not assumptions in the portable package; only Debug explicitly retains EBP.
add_custom_command(TARGET OpenSMACX POST_BUILD
    COMMAND "${OPENSMACX_UV_EXECUTABLE}" run --project "${CMAKE_CURRENT_SOURCE_DIR}"
            recover debug-sidecar
            --exe "$<TARGET_FILE:OpenSMACX>"
            --map "${CMAKE_CURRENT_BINARY_DIR}/OpenSMACX.map"
            --out "${CMAKE_CURRENT_BINARY_DIR}/OpenSMACX.sym"
            --build-root "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles"
            --comp-dir "${CMAKE_CURRENT_SOURCE_DIR}"
            --path-map "z:/=/"
            --frame-base "$<IF:$<CONFIG:Debug>,ebp,none>"
            --summary
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/OpenSMACX.sym"
    COMMENT "Building the gdb sidecar"
    VERBATIM)

# `/subsystem:windows`, because the entry point is `WinMain` and not `main`.
# Left at the default the linker looks for `main`, finds only `_WinMain@16` and
# fails with LNK2001 - the property is what selects which of the two CRT entry
# stubs is pulled in.
#
# NO `CXX_STANDARD`. It said 11, which was never true of anything here: cl
# 12.00.8168 predates every `/std:` switch, this file already sets
# CMAKE_CXX_STANDARD_DEFAULT to nothing for that reason, and the C++11
# spellings the tree uses arrive as macros from src/vc6_compat.h. CMake could
# only honour it by passing a flag VC6 has never heard of, so with
# `CXX_STANDARD_REQUIRED YES` beside it the setting was one CMake upgrade away
# from failing the configure over a standard nothing was asking for.
set_target_properties(OpenSMACX PROPERTIES
    WIN32_EXECUTABLE ON
)

# VC6 is the only compiler, so its warning level is unconditional rather than
# a branch. The GNU warning set that used to sit here went with the toolchain.
#
# There is no module definition file any more. src/OpenSMACX.def was never
# linked here - every line in it was an ALIAS bridging an Itanium-mangled GCC
# symbol to the MSVC name the patched image imported, and cl 12.00.8168 emits
# the MSVC name directly - so it described symbols this build does not produce.
# It went with the DLL it served on 2026-08-13 (docs/RETIRED_ROUTES.md). Exports
# rest on __declspec(dllexport) alone and the export SET is not pinned; nothing
# in this build needs it pinned, because nothing imports by name.
target_compile_options(OpenSMACX PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W4>)

# Optimized C++ profile, per source. The original was built without string
# pooling (/Gf, implied by /O2) almost everywhere: identical literals sit at
# separate addresses per use ("{"/"}" five times at 0x6868E0-0x686904 for
# netwin's draw_session_info, net_class's "Oh no!! Send failed!" at 0x697700
# and 0x697738, terraforming_actions' "space_sm.pcx" once per popp arm), and
# literal addresses are almost never shared between units, which link-wide
# merged /Gf COMDATs would be. A 2026-11-11 scan of every mapped function's
# literal pushes found unpooled evidence in 46 units, too few repeats to tell
# in 59, and pooled evidence only in the AnimaTek Caviar library
# (caviar_loader pushes "CPU not supported" from two functions and "No room
# for color table" twice in one through a single address each), so every unit
# gets /O2's expansion minus /Gf and only the Caviar library keeps /O2. VC6
# has no /Gf-, so the expansion is spelled out. /Oy and /Oi are part of it
# except in units whose own options disable them (/Oy-, /Oi-): both spellings
# together make VC6 warn D4025, which fails the reproducibility pass. The profile is prepended so each unit's own
# options (/Oy-, /Oi-, /Ob1, /Z7) still follow it; /Ob2 follows the profile
# because /O2 alone means /Ob1.
function(opensmacx_apply_optimization target)
    cmake_parse_arguments(PARSE_ARGV 1 arg "POOLED" "" "")
    get_target_property(target_sources ${target} SOURCES)
    get_target_property(target_dir ${target} SOURCE_DIR)
    set(optimized "$<CONFIG:Release,RelWithDebInfo>")
    foreach(source IN LISTS target_sources)
        if(NOT source MATCHES "\\.cpp$")
            continue()
        endif()
        cmake_path(ABSOLUTE_PATH source BASE_DIRECTORY "${target_dir}" OUTPUT_VARIABLE absolute)
        get_property(existing SOURCE "${absolute}" TARGET_DIRECTORY ${target} PROPERTY COMPILE_OPTIONS)
        if(arg_POOLED)
            set(profile /O2 /Ob2)
        else()
            set(profile /Og)
            if(NOT existing MATCHES "/Oi-")
                list(APPEND profile /Oi)
            endif()
            list(APPEND profile /Ot)
            if(NOT existing MATCHES "/Oy-")
                list(APPEND profile /Oy)
            endif()
            list(APPEND profile /Gs /Gy /Ob2)
        endif()
        set(options)
        foreach(flag IN LISTS profile)
            list(APPEND options "$<${optimized}:${flag}>")
        endforeach()
        set_property(SOURCE "${absolute}" TARGET_DIRECTORY ${target}
            PROPERTY COMPILE_OPTIONS ${options} ${existing})
    endforeach()
endfunction()
opensmacx_apply_optimization(OpenSMACX)
opensmacx_apply_optimization(jackal)
opensmacx_apply_optimization(uv)
opensmacx_apply_optimization(caviar POOLED)



install(TARGETS OpenSMACX
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
    ARCHIVE DESTINATION lib
)

# CMake builds the application. The installed recovery package owns verification:
# `uv run recover check --image <original>`; build success alone is not acceptance.
