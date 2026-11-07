# The AnimaTek Caviar/VOX voxel library (0x00627D00..0x00640A42 and its
# _SELFMOD kernels at 0x00664000), archived separately from the game-side
# Caviar class (src/caviar.cpp, 0x00616AA0..0x00618E9E). The original was a
# separately built library: its C translation units were compiled by Visual
# C++ 5.0 (CL 11.00.7022) at /O2 - recovery/COMPILER_EVIDENCE.md, "Caviar
# library compiler" - while the hand-written MASM modules are assembled by ML
# 6.11d like the application's. The linker stays LINK 6.00.
#
# Each unit's compiler is a measured per-unit fact, selected by the wrapper's
# explicit `--compiler=vc5` recipe option (recovery/environment/wine.py); the
# units without it stay VC6. Not every Caviar range ends in VC5's nop padding.
# VC5 has no /GZ (D4002), so this directory's debug profile omits it.
set(CMAKE_CXX_FLAGS_DEBUG "/Z7 /Ob0 /Od /Oy-")
add_library(caviar STATIC
    caviar_geometry.cpp
    caviar_loader.cpp
    caviar_chain.cpp
    caviar_io.cpp
    caviar_record.cpp
    caviar_error.cpp
    caviar_scene.cpp
    caviar_cpu.cpp
    caviar_raster.cpp
    caviar_part_s1a_clipped.asm
    caviar_part_s1a_contained.asm
    caviar_part_s1o_clipped.asm
    caviar_part_s1o_contained.asm
    caviar_part_s2a_clipped.asm
    caviar_part_s2a_contained.asm
    caviar_shadow_clipped.asm
    caviar_shadow_contained.asm
    caviar_part_s2o_clipped.asm
    caviar_part_s2o_contained.asm)
target_include_directories(caviar PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(caviar PRIVATE $<$<CONFIG:Debug>:BUILD_DEBUG>)
target_compile_options(caviar PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:/W4;--repro-root=${PROJECT_SOURCE_DIR}>"
    "$<$<COMPILE_LANGUAGE:ASM_MASM>:-nologo;-coff>")

# No forced vc6_compat.h: every C++ unit here includes stdafx.h first, which
# includes it. VC5 keys `#pragma once` on the include spelling, so the forced
# absolute spelling plus stdafx.h's relative one would expand it twice under
# the reproducibility check's relative-source preprocessing pass.
get_target_property(_caviar_options caviar COMPILE_OPTIONS)
list(FILTER _caviar_options EXCLUDE REGEX "/FI")
set_property(TARGET caviar PROPERTY COMPILE_OPTIONS ${_caviar_options})

# Measured per unit (2026-11-07); see recovery/COMPILER_EVIDENCE.md.
set(CAVIAR_VC5_SOURCES
    caviar_geometry.cpp
    caviar_loader.cpp
    caviar_chain.cpp
    caviar_io.cpp
    caviar_record.cpp
    caviar_error.cpp
    caviar_raster.cpp caviar_scene.cpp)
set_property(SOURCE ${CAVIAR_VC5_SOURCES} APPEND PROPERTY COMPILE_OPTIONS "--compiler=vc5")

# Native member-reference verification needs these owners' CodeView types.
set_property(SOURCE caviar_chain.cpp caviar_scene.cpp caviar_raster.cpp caviar_loader.cpp
    APPEND PROPERTY COMPILE_OPTIONS "/Z7")
