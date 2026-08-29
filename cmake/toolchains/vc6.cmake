# ---------------------------------------------------------------------------
# The toolchain. There is exactly one: cl 12.00.8168, the compiler that built
# the original image.
#
# This is a matching decompilation, so VC6 is not one option among several -
# it is the only compiler whose output carries information. Anything else can
# tell you the code is valid; only this one can tell you it is RIGHT.
#
# It lived hardcoded in the root CMakeLists.txt until 2026-08-15, on the
# theory that one compiler is no choice to express. It moved here because a
# toolchain that only CMake's own runtime ever sees is a toolchain the rest of
# the world cannot: CLion could not resolve <windows.h> because the include
# paths reached cl through the wrapper's environment and never CMake's model,
# and every IDE or tool that reads CMAKE_TOOLCHAIN_FILE to learn "what am I
# looking at" learned nothing. The root CMakeLists.txt defaults this file in,
# so `cmake -B build` is still the whole invocation; pass
# -DCMAKE_TOOLCHAIN_FILE to override it and the default stands aside.
# CMakePresets.json stays retired (AGENTS.md, 2026-08-12).
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

# Fail HERE rather than at the first compile: a missing VC6 install used to
# surface as wine errors from inside the wrapper, which read like a build bug
# rather than a missing prerequisite.
if(NOT IS_DIRECTORY "${VC6_ROOT}/BIN" OR NOT IS_DIRECTORY "${VC6_ROOT}/INCLUDE")
    message(FATAL_ERROR
        "VC6_ROOT (${VC6_ROOT}) does not look like a VC6 install - it needs "
        "BIN/ and INCLUDE/. Point $ENV{VC6_ROOT} or -DVC6_ROOT= at one.")
endif()

set(VC6_INCLUDE_DIR "${VC6_ROOT}/INCLUDE" CACHE PATH
    "The Win32 + CRT headers cl 12.00.8168 compiles against")

set(_vc6_tools "${CMAKE_CURRENT_LIST_DIR}/../../tools")
# Normalized: CMake canonicalizes the COMPILER path but records the linker as
# given, and an unnormalized `.../../../tools/vc6-link` ended up spelled that
# way in every link line.
get_filename_component(_vc6_tools "${_vc6_tools}" ABSOLUTE)
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
set(CMAKE_DEPFILE_FORMAT gcc)
set(CMAKE_C_DEPFILE_FORMAT gcc)

# `/Z7`, NOT `/Zi`. VC6 writes debug info to a single `vc60.pdb` per output
# directory and takes no lock on it, so a parallel build fails with
# `C1033: cannot open program database` on whichever objects lose the race -
# 16 of 19 on the first run here. `/Z7` puts the debug info in the object
# file, where there is no shared state and no race. CMake's MSVC defaults
# assume a compiler that can share a PDB; this one cannot.
# /Oy- keeps frame pointers: without them (/O2 implies /Oy) gdb and winedbg
# cannot walk a backtrace, which is the whole point of the debug profile.
set(CMAKE_CXX_FLAGS_DEBUG "/Z7 /Ob0 /Od /Oy- /GZ" CACHE STRING "" FORCE)
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
# It has to be undone through the make-rules override rather than with a
# substitution here: the override file is included AFTER Windows-MSVC.cmake
# appends its `/Zm1000` and before `CMAKE_CXX_FLAGS` reaches the cache, which
# is the one window where the flag exists and is still editable. A toolchain
# file is read before either, so this is the right - and the only - place for
# the variable that names it.
set(CMAKE_USER_MAKE_RULES_OVERRIDE
    "${CMAKE_CURRENT_LIST_DIR}/../vc6-flag-overrides.cmake")

set(CMAKE_FIND_ROOT_PATH "${VC6_ROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# THE TOOLCHAIN'S SYSTEM HEADERS, EXPRESSED TO CMAKE. The build always worked
# without this - the wrapper feeds cl the same directory through the INCLUDE
# environment variable - but that variable exists only inside cl's process, so
# nothing outside the compile invocation could see the headers: CLion could
# not resolve <windows.h>, and compile_commands.json carried no include path.
# These variables are CMake's own spelling of the same fact - "the
# compiler's standard include directories" - appended as system includes on
# every target, read by every consumer of the project model.
#
# The install spells its headers in UPPERCASE (`WINDOWS.H`) while the source
# writes lowercase `#include <windows.h>`. cl under wine is case-insensitive
# and never notices; a case-sensitive host resolver might, so if an IDE still
# fails to open a header this is the place to look first.
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES "${VC6_INCLUDE_DIR}")
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES "${VC6_INCLUDE_DIR}")

# And the compile database, for the tools that read it directly.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
