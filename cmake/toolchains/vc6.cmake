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

set(CMAKE_FIND_ROOT_PATH "${VC6_ROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
