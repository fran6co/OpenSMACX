# Reconstructed Jackal code remains in recovery scope, not the vendor exclusion.
# Only the confirmed lifecycle/version unit is assigned here so far.
add_library(jackal STATIC jackal.cpp)
target_include_directories(jackal PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(jackal PRIVATE $<$<CONFIG:Debug>:BUILD_DEBUG>)
target_compile_options(jackal PRIVATE /W4 "--repro-root=${PROJECT_SOURCE_DIR}")
