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
# `tools/decomp_status.py` measures a body in its real translation unit
# whenever the scaffold cannot build one, and to do that it has to compile
# that file the way the build compiles it. It used to approximate: copy the
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
    "write build/compile_commands.json; tools/vc6_build_flags.py reads it"
    FORCE)

# `C` AS WELL AS `CXX`, for zlib. zlib 1.0.2 uses K&R parameter definitions,
# which no C++ compiler will parse - `byte_match.compile_unit` already writes
# each vendored unit under its own `.c` suffix for exactly this reason. The
# toolchain has defined `CMAKE_C_COMPILER` all along; only this line was
# keeping the language out.
project(OpenSMACX VERSION 0.2.3 LANGUAGES CXX C)

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
    src/vendor/zlib-1.0.2/deflate.c
    src/vendor/zlib-1.0.2/infblock.c
    src/vendor/zlib-1.0.2/infcodes.c
    src/vendor/zlib-1.0.2/inffast.c
    src/vendor/zlib-1.0.2/inflate.c
    src/vendor/zlib-1.0.2/inftrees.c
    src/vendor/zlib-1.0.2/trees.c
)
target_include_directories(zlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/zlib-1.0.2")
# `/W0` and no forced include: PROVENANCE.md's whole point is that these
# files are the release and not a copy somebody tidied, so the build adapts
# to them rather than the other way round.
target_compile_options(zlib PRIVATE /W0)

set(OPENSMACX_SOURCES
    src/alpha.cpp
    src/main.cpp
    src/pending_bodies.cpp
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
    src/imagebutton.cpp
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
    src/editbox.cpp
    src/editgroup.cpp
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
    src/spying.cpp
    src/sprite.cpp
    src/stdafx.cpp
    src/stringbox.cpp
    src/stringstruct.cpp
    src/strings.cpp
    src/technology.cpp
    src/temp.cpp
    src/terraforming.cpp
    src/text.cpp
    src/textindex.cpp
    src/time.cpp
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
#
# IT IS `OpenSMACX.exe` SINCE 2026-08-15, not `opensmacx-link-check.exe`. The
# link check was all it could be while `main()` returned 0 and nothing else was
# reachable; it now has the shipped entry point - the recovered `_WinMain@16`
# in src/main.cpp - and calls the game's own startup sequence from it. It is
# not yet a game: six of WinMain's callees are still reached through
# src/pending_bodies.cpp. But the artifact is the program being recovered
# rather than a probe, and its name should say so.
add_executable(OpenSMACX ${OPENSMACX_SOURCES})

target_compile_definitions(OpenSMACX PRIVATE
    $<$<CONFIG:Debug>:BUILD_DEBUG>
)
target_precompile_headers(OpenSMACX PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/stdafx.h")
# `dxguid` carries the IIDs `Buffer::init` names - without it the link fails
# with `unresolved external symbol _IID_IDirectDrawSurface`, measured. It used
# to be a `#pragma comment(lib, "dxguid.lib")` in buffer.cpp, "so the
# dependency sits beside the code that needs it"; the Winmm pragma that
# comment cited as precedent turned out to be redundant with this line, so
# the precedent was one entry that did nothing.
target_link_libraries(OpenSMACX PRIVATE zlib user32 gdi32 winmm dxguid)

# `/MAP /MAPINFO:LINES` is the only debug format in this toolchain that gdb can
# be taught to read. VC6's own three cannot be: the PDB is 2.0 (`JG`, which
# LLVM's reader refuses), the CodeView in `.debug$S` is signature 2 (LLVM
# handles 4), and `/debugtype:coff` segfaults gdb 15.1 on `file` alone -
# measured, all three. The map carries every public with its final address and,
# with `/MAPINFO:LINES`, a line table per source, which is what
# `tools/gdb_sidecar.py` turns into an ELF gdb loads beside a live winedbg.
# It is a side file: the linked bytes are the same with it and without.
target_link_options(OpenSMACX PRIVATE
    "/MAP:${CMAKE_CURRENT_BINARY_DIR}/OpenSMACX.map" "/MAPINFO:LINES")

# The sidecar is DERIVED FROM THE LINK, so it is built by it. Left as a command
# to remember, it goes stale the moment anything moves, and a stale one is
# worse than none: gdb reads it without complaint and reports names for the
# addresses of an older build. It costs under a second and reads only what the
# link just wrote - the executable, the map, and the objects' CodeView.
add_custom_command(TARGET OpenSMACX POST_BUILD
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/gdb_sidecar.py"
            --exe "$<TARGET_FILE:OpenSMACX>"
            --map "${CMAKE_CURRENT_BINARY_DIR}/OpenSMACX.map"
            --out "${CMAKE_CURRENT_BINARY_DIR}/OpenSMACX.sym"
            --build-root "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles"
            --comp-dir "${CMAKE_CURRENT_SOURCE_DIR}"
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

# ---------------------------------------------------------------------------
# THE BUILD IS THE EXECUTABLE. Nothing else.
#
# `tests/` held 95 ctest registrations - the tool tests, the ABI, vtable and
# layout gates, the annotation-integrity checks and a C++ fixture target -
# and it was RETIRED on 2026-08-15 because nothing consumed its results.
#
# The finding that settled it: the suite reported 21% passing, 75 failures,
# every one of them `ModuleNotFoundError: No module named 'pefile'`, because
# a plain `cmake -S . -B build` had reconfigured the tree onto
# /usr/bin/python3 instead of the venv every tool runs on. That state is not
# something a person sees and tolerates - it is what an unrun suite looks
# like. A second check had been failing since the very commit that
# introduced its bound, and stayed red through every commit after.
#
# The gate is `tools/decomp_status.py --check`: it compiles every BYTE_EXACT
# claim in `src/` against the shipped image and fails if one no longer
# reproduces. That one is run every batch, its result is read, and it is the
# only check in this tree that was ever load-bearing.
#
# `OPENSMACX_ROOT` and `OPENSMACX_BUILD_ROOT` outlive the split that needed
# them: `tools/` still reads them out of the cache.
# ---------------------------------------------------------------------------
set(OPENSMACX_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(OPENSMACX_BUILD_ROOT "${CMAKE_CURRENT_BINARY_DIR}")

