# EA UK's uV/uV2 movie player library ("Play uV/uV2 Version 1.50, Martin
# Griffiths, Electronic Arts UK 1997", at 0x00691F08 just before its data).
# Image boundary: 0x005C9EC0..0x005D44FB, directly after the last game object.
# It references only itself and the CRT, owns the UVA_DATA and unnamed data
# sections, and is complete before ddraw.lib's DirectDrawCreate thunk at
# 0x005D44FC and JACKAL.LIB. That places it as its own archive, or as the last
# game objects; it is third-party code either way. The game's uv2player.cpp
# is its only caller and stays in the application.
add_library(uv STATIC
    movie_media.cpp
    movie_media_decoder.cpp
    movie_media_decoder_g6.cpp
    movie_media_decoder_uv.asm)
target_include_directories(uv PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(uv PRIVATE $<$<CONFIG:Debug>:BUILD_DEBUG>)
target_compile_options(uv PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:/W4;--repro-root=${PROJECT_SOURCE_DIR}>"
    "$<$<COMPILE_LANGUAGE:ASM_MASM>:-nologo;-coff>")

# Native member-reference verification needs this owner's CodeView types.
set_property(SOURCE movie_media.cpp APPEND PROPERTY COMPILE_OPTIONS "/Z7")
# The decoder's C owners 0x005D3980..0x005D4231 carry Pentium Pro instruction
# scheduling; 0x005D2E00, 0x005D2E70 and 0x005D35B0 measure closer without it
# (all three live in movie_media_decoder.cpp). The split unit is an inferred
# boundary, not a historical filename.
set_source_files_properties(movie_media_decoder_g6.cpp PROPERTIES COMPILE_OPTIONS "/G6")
