# Build with the compiler that built the original image: cl 12.00.8168.
#
# This is a matching decompilation, so VC6 is not one option among several -
# it is the only compiler whose output carries information. Anything else can
# tell you the code is valid; only this one can tell you it is RIGHT.
#
# It runs under wine through tools/vc6_cl.py, which exists because CMake
# drives a compiler with POSIX paths and MSVC reads a leading `/` as the start
# of an option.
#
# The compiler id is FORCED rather than detected. CMake identifies a compiler
# by compiling a probe and reading strings out of the object, and its MSVC
# probe assumes a native toolchain; letting it try means it falls back to GNU
# rules and passes flags VC6 has never heard of.

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

set(_vc6_tools "${CMAKE_CURRENT_LIST_DIR}/../../tools")
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
set(CMAKE_CXX_STANDARD_DEFAULT "")
set(CMAKE_CXX98_STANDARD_COMPILE_OPTION "")
set(CMAKE_CXX98_EXTENSION_COMPILE_OPTION "")

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
# This was the whole of "C1060 on at least one translation unit": it was not
# one unit and it was not the source. It was this flag.
#
# It has to be undone through the make-rules override rather than here: this
# file is read BEFORE Windows-MSVC.cmake, which appends its `/Zm1000` to
# whatever the toolchain left behind, so a substitution at this point has
# nothing to substitute. The override file is included afterwards and still
# before `CMAKE_CXX_FLAGS` reaches the cache, which is the one window where
# the flag exists and is still editable.
set(CMAKE_USER_MAKE_RULES_OVERRIDE
    "${CMAKE_CURRENT_LIST_DIR}/vc6-flag-overrides.cmake")

set(CMAKE_FIND_ROOT_PATH "${VC6_ROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
