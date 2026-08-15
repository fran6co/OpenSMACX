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

# AND THE FORMAT THOSE FLAGS PRODUCE, which CMake 4.x requires to be stated.
# Normally it is discovered along with the compiler; forcing the compiler id
# above skips that discovery, and CMake 4.4 turns the omission into a hard
# "required internal CMake variable not set" at GENERATE time - after
# "Configuring done", which makes it read like a CMake installation fault
# rather than a missing setting. Nothing could be built here with a 4.x cmake
# until this line existed.
#
# `gcc` is the format the wrapper actually writes: cl cannot emit a depfile at
# all, so tools/vc6_cl.py parses `/showIncludes` and writes a GNU one itself,
# which is the whole reason the flags above are its own spelling.
set(CMAKE_CXX_DEPFILE_FORMAT gcc)
set(CMAKE_C_DEPFILE_FORMAT gcc)

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
target_link_libraries(OpenSMACX PRIVATE user32 gdi32 winmm)

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
# Everything that is not the executable.
#
# Above this line is the whole of "compile src/ into a binary": the toolchain,
# the source list, the target, and its install rule. Below it, in
# tests/CMakeLists.txt, is everything a developer runs and a builder does not -
# the gates, the tool tests, the recovery harness, the staging and hybrid
# targets, and the artifact paths and Python interpreter they need.
#
# DEFAULTS ON, so `cmake -B build` does exactly what it did before this split
# and no existing invocation changes meaning. `-DOPENSMACX_BUILD_TOOLING=OFF`
# is the lean build: the executable and nothing else, with no Python, no
# binutils, no wine prefix and no game directory required.
#
# The two roots below exist because a subdirectory redefines
# CMAKE_CURRENT_SOURCE_DIR and CMAKE_CURRENT_BINARY_DIR to ITSELF. Forty-one
# paths in the moved half meant the repository root when they were written, and
# would have silently become tests/ - `tools/verify_cmake_paths_exist.py`
# checks that every literal ${...}/ path resolves, so the breakage would have
# been caught, but naming the roots explicitly means it never happens. It also
# fixes a latent fragility: the two functions moved below evaluate
# CMAKE_CURRENT_SOURCE_DIR in the CALLER's directory, so they were only ever
# correct while every caller lived at the top level.
# ---------------------------------------------------------------------------
set(OPENSMACX_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(OPENSMACX_BUILD_ROOT "${CMAKE_CURRENT_BINARY_DIR}")

option(OPENSMACX_BUILD_TOOLING
    "Build the recovery tooling, gates and test suite (off = executable only)"
    ON)

if(OPENSMACX_BUILD_TOOLING)
    add_subdirectory(tests)
endif()

