# Included by CMakeCommonLanguageInclude after Windows-MSVC.cmake has built
# the default flags and before they are written to the cache - the only point
# at which `/Zm1000` both exists and can still be changed. See the block in
# CMakeLists.txt that sets CMAKE_USER_MAKE_RULES_OVERRIDE for why 1000 is the
# thing that produces `C1060: compiler is out of heap space` rather than the
# thing that prevents it.
#
# This file must stay a FILE: CMAKE_USER_MAKE_RULES_OVERRIDE takes a path, so
# unlike the rest of the toolchain it cannot be folded into CMakeLists.txt.

string(REPLACE "/Zm1000" "/Zm200" CMAKE_C_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")
string(REPLACE "/Zm1000" "/Zm200" CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT}")
