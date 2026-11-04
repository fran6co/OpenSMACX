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

# `C` AS WELL AS `CXX`, for zlib. zlib 1.0.2 uses K&R parameter definitions,
# which no C++ compiler will parse - `byte_match.compile_unit` already writes
# each vendored unit under its own `.c` suffix for exactly this reason. The
# toolchain has defined `CMAKE_C_COMPILER` all along; only this line was
# keeping the language out.
project(OpenSMACX VERSION 0.2.3 LANGUAGES CXX C)

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
# to them rather than the other way round.
target_compile_options(zlib PRIVATE /W0)

set(OPENSMACX_SOURCES
    src/alpha.cpp
    src/alphasave.cpp
    src/faction_strategy.cpp
    src/factionprofilewin.cpp
    src/main.cpp
    src/multiwin.cpp
    src/midi.cpp
    src/matrix3.cpp
    src/filebox.cpp
    src/word_state.cpp
    src/table_storage.cpp
    src/span_math.cpp
    src/scale_adjustment.cpp
    src/technology_queries.cpp
    src/byte_utilities.cpp
    src/shared_word.cpp
    src/fx.cpp
    src/alphanet.cpp
    src/alphanet_records.cpp
    src/ambience.cpp
    src/autosound.cpp
    src/base.cpp
    src/base_compute.cpp
    src/basebutton.cpp
    src/basepop.cpp
    src/buffer.cpp
    src/buffer_mask.cpp
    src/polygon.cpp
    src/buttongroup.cpp
    src/caviar_objects.cpp
    src/caviar.cpp
    src/caviar_chain.cpp
    src/caviar_record.cpp
    src/caviar_io.cpp
    src/caviar_error.cpp
    src/caviar_geometry.cpp
    src/caviar_cpu.cpp
    src/council.cpp
    src/dialog.cpp
    src/dialogs.cpp
    src/faction.cpp
    src/faction_turn.cpp
    src/faction_treaty.cpp
    src/faction_diplomacy.cpp
    src/faction_dialogue.cpp
    src/faction_network.cpp
    src/famewin.cpp
    src/filemap.cpp
    src/flatbutton.cpp
    src/flic.cpp
    src/font_queue.cpp
    src/interface_fonts.cpp
    src/game.cpp
    src/game_desktop.cpp
    src/game_calendar.cpp
    src/faction_monuments.cpp
    src/newgame.cpp
    src/application_math.cpp
    src/application_text.cpp
    src/general.cpp
    src/danger.cpp
    src/directdraw.cpp
    src/number_parsing.cpp
    src/scenario_undo.cpp
    src/header_io.cpp
    src/crt_random.cpp
    src/graphicwin.cpp
    src/graphicwin_events.cpp
    src/log.cpp
    src/maininterface.cpp
    src/map.cpp
    src/movement_queries.cpp
    src/map_sites.cpp
    src/territory.cpp
    src/world.cpp
    src/map_altitude.cpp
    src/mapwin.cpp
    src/mapwin_terrain.cpp
    src/basewin.cpp
    src/video.cpp
    src/video_queue.cpp
    src/datalink.cpp
    src/datalink_entries.cpp
    src/netwin.cpp
    src/socialwin.cpp
    src/social.cpp
    src/designwin.cpp
    src/worldwin.cpp
    src/diplowin.cpp
    src/alphamovie.cpp
    src/mci_video.cpp
    src/movie_media.cpp
    src/movie_media_surface.cpp
    src/movie_media_decoder.cpp
    src/movie_decoder_frame.cpp
    src/movie_decoder_huffman.cpp
    src/movie_decoder_idct.cpp
    src/cursor.cpp
    src/imagebutton.cpp
    src/infowin.cpp
    src/interlude.cpp
    src/statuswin.cpp
    src/wave.cpp
    src/battlewin.cpp
    src/councwin.cpp
    src/credits.cpp
    src/diplopop.cpp
    src/gamma.cpp
    src/pushbutton.cpp
    src/messagewin.cpp
    src/newtechwin.cpp
    src/picktech.cpp
    src/monuwin.cpp
    src/pickwin.cpp
    src/prodpicker.cpp
    src/reportwin.cpp
    src/setupwin.cpp
    src/multidebug.cpp
    src/tutwin.cpp
    src/popmenu.cpp
    src/popup.cpp
    src/popup_wrappers.cpp
    src/popup_api.cpp
    src/netmsg.cpp
    src/editbox.cpp
    src/editgroup.cpp
    src/xpops.cpp
    src/planwin.cpp
    src/playerlock.cpp
    src/netdaemon.cpp
    src/netdaemon_interface.cpp
    src/netdaemon_message_data.cpp
    src/netdaemon_spock.cpp
    src/prefwin.cpp
    src/prefwin_geometry.cpp
    src/quaylewin.cpp
    src/replaywin.cpp
    src/reportif.cpp
    src/console.cpp
    src/console_commands.cpp
    src/alphamenu.cpp
    src/uv2player.cpp
    src/dipedit.cpp
    src/sound.cpp
    src/sound_loader.cpp
    src/wave_group_list.cpp
    src/spritebox.cpp
    src/listbox.cpp
    src/net_class.cpp
    src/net_fifo.cpp
    src/net_session_picker.cpp
    src/node.cpp
    src/squarelock.cpp
    src/deletionlist.cpp
    src/lock.cpp
    src/texture.cpp
    src/texture_objects.cpp
    src/menu.cpp
    src/interface_palette.cpp
    src/path.cpp
    src/probe.cpp
    src/pulldown.cpp
    src/random.cpp
    src/scroll.cpp
    src/scenario.cpp
    src/sounddevice.cpp
    src/spot.cpp
    src/spying.cpp
    src/sprite.cpp
    src/texheap.cpp
    src/sprite_assets.cpp
    src/sprite_objects.cpp
    src/caviar_renderer_objects.cpp
    src/stdafx.cpp
    src/stringbox.cpp
    src/stringlist.cpp
    src/stringstruct.cpp
    src/name_table.cpp
    src/strings.cpp
    src/string_table.cpp
    src/string_formatting.cpp
    src/technology.cpp
    src/terraforming.cpp
    src/text.cpp
    src/time.cpp
    src/timer_messages.cpp
    src/enemy.cpp
    src/veh.cpp
    src/veh_queries.cpp
    src/vehicle_storage.cpp
    src/prototype_history.cpp
    src/vehicle_strategy.cpp
    src/vehicle_stats.cpp
    src/vehicle_orders.cpp
    src/vehicle_cost.cpp
    src/vector.cpp
    src/win.cpp
    src/win_coordinates.cpp
    src/win_input.cpp
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
#
# IT IS `OpenSMACX.exe` SINCE 2026-08-15, not `opensmacx-link-check.exe`. The
# link check was all it could be while `main()` returned 0 and nothing else was
# reachable; it now has the shipped entry point - the recovered `_WinMain@16`
# in src/main.cpp - and calls the game's own startup sequence from it. It is
# not yet a recovered or accepted game. But the artifact is the program
# being recovered rather than a probe, and its name should say so.
# The confirmed Jackal lifecycle/version owner is a distinct library. Its
# referenced entry points and generic palette/font/control owners are explicit
# archive inputs; there is no synthetic retention anchor. Other drawing
# subsystems remain direct application inputs until their membership is
# established. This is not a claim of complete JACKAL.LIB recovery.
add_subdirectory(src/jackal)
# These owners retain frame-based parameter accesses. Match their
# existing explicit recovery measurement profiles without changing the
# application's default compiler flags.
set_source_files_properties(
    src/map_altitude.cpp
    src/wave.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Keep the frame-based Wave_Device body on its measured
# translation-unit profile without changing application-wide compiler flags.
set_source_files_properties(
    src/wave_group_list.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# The Midi, device-wrapper and sound-loader bodies also retain frame-based
# argument accesses in the measured application release profile.
set_source_files_properties(
    src/midi.cpp
    src/sounddevice.cpp
    src/sound_loader.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve the frame-based argument accesses in these interface owners while
# keeping Win and the application-wide compiler defaults unchanged.
set_source_files_properties(
    src/datalink.cpp
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
    src/netdaemon_interface.cpp
    src/netdaemon_message_data.cpp
    src/netdaemon_spock.cpp
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
    src/movement_queries.cpp
    src/map_sites.cpp
    src/territory.cpp
    src/world.cpp
    src/faction.cpp
    src/faction_turn.cpp
    src/faction_treaty.cpp
    src/faction_diplomacy.cpp
    src/faction_dialogue.cpp
    src/faction_network.cpp
    src/veh.cpp
    src/veh_queries.cpp
    src/prototype_history.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve the mapped frame-based vehicle owners after their source split.
# Their original translation-unit names are unknown; other veh.cpp code stays
# on its existing owner profile.
set_source_files_properties(
    src/vehicle_cost.cpp
    src/vehicle_orders.cpp
    src/vehicle_strategy.cpp
    src/vehicle_stats.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve original frame-based accesses in the remaining application owners.
# Keep all other source recipes on their independently measured defaults.
# Newgame's original entries and the measured in-class Flic cleanup retain this
# source profile; this does not assert the original translation-unit boundary.
set_source_files_properties(
    src/game.cpp
    src/game_desktop.cpp
    src/game_calendar.cpp
    src/faction_monuments.cpp
    src/newgame.cpp
    src/reportwin.cpp
    src/netwin.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve measured frame-based accesses across these application window owners.
# Keep all other compiler and measurement policies unchanged.
set_source_files_properties(
    src/reportif.cpp
    src/statuswin.cpp
    src/prodpicker.cpp
    src/alphamenu.cpp
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
    src/dipedit.cpp
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
    src/name_table.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve measured frame-based accesses in the core application cohort.
# Mixed or non-improving owners retain their measured default recipes.
set_source_files_properties(
    src/ambience.cpp
    src/deletionlist.cpp
    src/factionprofilewin.cpp
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
# Preserve spying's original frame-based argument accesses.
set_source_files_properties(src/spying.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve the measured frame-based AlphaNet controller context. Its record
# and lifecycle owner remains on the default application profile.
set_source_files_properties(src/alphanet.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve measured frame-based accesses in the remaining application owners.
# Mixed generic/application owners retain their independently measured defaults.
set_source_files_properties(
    src/base.cpp
    src/base_compute.cpp
    src/multiwin.cpp
    src/socialwin.cpp
    src/social.cpp
    src/technology.cpp
    src/technology_queries.cpp
    src/uv2player.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Buffer's original hline5E1B94 and init5D77C3 call the CRT memset.
# Keep memory-call selection at TU scope; this is an evidence-led build policy,
# not proof of the original command line. Source byte loops retain inline fills.
set_property(SOURCE src/buffer.cpp APPEND PROPERTY COMPILE_OPTIONS "/Oi-")
# Preserve original out-of-line close/bit-helper calls in these owners.
# Append so DipEdit keeps its independently measured frame-based context.
# Video and MCI use separate implementation units under default app inlining.
# This optimized matching exception must not override Debug's /Ob0.
set_property(SOURCE
    src/pulldown.cpp
    src/dipedit.cpp
    APPEND PROPERTY COMPILE_OPTIONS "$<$<NOT:$<CONFIG:Debug>>:/Ob1>")
# Preserve the original frame-based AlphaMovie owner context without changing
# its independently measured /Ob2 inlining policy.
set_source_files_properties(src/alphamovie.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")
# Gamma's modal window keeps its measured frame context; the independent
# PushButton implementation retains its default frameless context.
set_source_files_properties(src/gamma.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")

# Preserve the measured frame-based context of the scenario-editor undo owner.
# General's unrelated helpers retain their independently measured default recipe.
set_source_files_properties(src/scenario_undo.cpp PROPERTIES COMPILE_OPTIONS "/Oy-")
# Retain standalone compiler type evidence for these source-owned global/member
# groups. This adds observability, not a different optimization/frame policy;
# reference checks consume each production object's own types and relocations.
set_property(SOURCE
    src/alpha.cpp
    src/buffer.cpp
    src/caviar.cpp
    src/caviar_chain.cpp
    src/credits.cpp
    src/game.cpp
    src/game_desktop.cpp
    src/game_calendar.cpp
    src/faction_monuments.cpp
    src/interface_palette.cpp
    src/maininterface.cpp
    src/map.cpp
    src/movement_queries.cpp
    src/map_sites.cpp
    src/mapwin.cpp
    src/mapwin_terrain.cpp
    src/netdaemon.cpp
    src/prefwin.cpp
    src/log.cpp
    src/newtechwin.cpp
    src/prototype_history.cpp
    src/console_commands.cpp
    src/net_class.cpp
    src/sprite.cpp
    src/texheap.cpp
    src/world.cpp
    src/win.cpp
    APPEND PROPERTY COMPILE_OPTIONS "/Z7")
# Keep these measured frame-based helper owners on their explicit source profile.
set_source_files_properties(
    src/byte_utilities.cpp
    src/shared_word.cpp
    src/span_math.cpp
    src/scale_adjustment.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# The application asset helpers retain the original EBP-based argument frame.
# Keep this profile local to the extracted asset owner; generic Sprite methods
# retain their measured default recipe. This does not assert the original TU.
set_source_files_properties(src/sprite_assets.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Application timer integration retains the pinned argument frame. The generic
# Time implementation is compiled separately in Jackal with its existing profile.
set_source_files_properties(src/time.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Preserve the measured frame-based contexts of the shared stream wrappers.
# General's unrelated helpers retain their independently measured default recipe.
set_source_files_properties(src/header_io.cpp src/crt_random.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Application arithmetic and text utilities use the established frame profile.
# The remaining General helpers retain their existing source recipe.
set_source_files_properties(src/application_math.cpp src/application_text.cpp
    PROPERTIES COMPILE_OPTIONS "/Oy-")
# Terraforming's three pinned functions retain EBP-based argument frames.
set_property(SOURCE src/terraforming.cpp APPEND PROPERTY COMPILE_OPTIONS "/Oy-")
# Only the CPU register probe uses the explicitly registered intrinsic frontend.
# The rest of Caviar and the application continue to use their VC6 recipes.
set_source_files_properties(src/caviar_cpu.cpp
    PROPERTIES COMPILE_OPTIONS "--frontend=intrinsics-i386-no-debug")
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
    "${CMAKE_CURRENT_SOURCE_DIR}/src/mapwin_terrain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/map.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/movie_media_decoder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/graphicwin.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/recovery/terrain_data.py")
target_include_directories(OpenSMACX PRIVATE "${OPENSMACX_TERRAIN_DIRECTORY}")

target_compile_definitions(OpenSMACX PRIVATE
    $<$<CONFIG:Debug>:BUILD_DEBUG>
)
# Stable logical TU identity and guarded /Brepro are shared with recovery
# recipes. VC6 otherwise time-seeds anonymous names and can reorder global
# storage between identical compilations. The wrapper refuses macro changes.
# PCH reuse cannot establish the fresh preprocessing evidence this requires.
target_compile_options(OpenSMACX PRIVATE "--repro-root=${CMAKE_CURRENT_SOURCE_DIR}")
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
target_link_libraries(OpenSMACX PRIVATE jackal zlib user32 gdi32 winmm dxguid ddraw vfw32 dsound comdlg32)

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
target_compile_options(OpenSMACX PRIVATE /W4)



install(TARGETS OpenSMACX
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
    ARCHIVE DESTINATION lib
)

# CMake builds the application. The installed recovery package owns verification:
# `uv run recover check --image <original>`; build success alone is not acceptance.
