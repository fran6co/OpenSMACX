# Reconstructed Jackal code remains in recovery scope, not the vendor exclusion.
# Generic palette, font and checkbox/radio controls are archived separately
# from application instances and interface initialization. Other generic
# collaborators retain their current homes until their boundaries are recovered.
add_library(jackal STATIC
    jackal.cpp palette.cpp font.cpp
    checkbutton.cpp radiobutton.cpp checkbox.cpp filewin.cpp)
target_include_directories(jackal PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(jackal PRIVATE $<$<CONFIG:Debug>:BUILD_DEBUG>)
target_compile_options(jackal PRIVATE /W4 "--repro-root=${PROJECT_SOURCE_DIR}")
