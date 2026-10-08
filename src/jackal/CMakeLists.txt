# Reconstructed Jackal code remains in recovery scope, not the vendor exclusion.
# The generic Palette implementation is archived here separately from the
# application's palette objects and interface initialization.
add_library(jackal STATIC jackal.cpp palette.cpp font.cpp)
target_include_directories(jackal PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(jackal PRIVATE $<$<CONFIG:Debug>:BUILD_DEBUG>)
target_compile_options(jackal PRIVATE /W4 "--repro-root=${PROJECT_SOURCE_DIR}")
