# JACKAL.LIB, Firaxis's GUI/system toolkit: reconstructed code that stays in
# recovery scope, not under the vendor exclusion.
#
# Membership is image evidence, not filenames:
# - Library members follow every game object in .text. Jackal starts with
#   mem_get at 0x005D4510, after the uV library and ddraw.lib's
#   DirectDrawCreate thunk, and ends before vfw32.lib's thunks and the CRT at
#   0x00644DC0.
# - The data and bss contributions follow the same order.
#
# The original archive also held the AnimaTek Caviar (VC5) and zlib 1.0.2
# objects. In the image their members interleave with Jackal's in
# dependency-resolution waves, which LINK produces only within one archive.
# This build keeps them as their own targets (src/caviar, zlib) by decision;
# grouping does not change per-function bytes.
#
# The member list is in image order (each unit's main .text run, tumap.py);
# LINK takes archive members by need, so the order records the evidence and
# does not reproduce the layout.
#
# Where a member's header-inline copies landed in game objects, the copies stay
# measured in game units: a compile_context fact, or a small src/ residual for
# bodies that are still out of line.
add_library(jackal STATIC
    mem_get.cpp heap.cpp net_lists.cpp graphicwin.cpp buffer.cpp sprite.cpp
    win.cpp pulldown.cpp win_coordinates.cpp spot.cpp menu.cpp win_input.cpp
    timer_messages.cpp text.cpp palette.cpp mci_video.cpp filefind.cpp
    basepop.cpp dialog_style.cpp scroll.cpp basebutton.cpp flatbutton.cpp
    stringlist.cpp dialog.cpp listbox.cpp radiobutton.cpp checkbox.cpp
    spritebox.cpp editgroup.cpp dialogs.cpp filewin.cpp editbox.cpp time.cpp
    string_table.cpp caviar.cpp font.cpp texture.cpp imagebutton.cpp
    random.cpp parse.cpp log.cpp polygon.cpp graphics_language.cpp
    popup_api.cpp filemap.cpp number_parsing.cpp hsv.cpp stringbox.cpp
    flic.cpp buttongroup.cpp autosound.cpp pushbutton.cpp directdraw.cpp
    jackal.cpp net_class.cpp checkbutton.cpp net_fifo.cpp vector.cpp
    env_open.cpp filebox.cpp texheap.cpp ddinit.cpp video.cpp cursor.cpp
    trig.cpp net_session_picker.cpp video_queue.cpp)
target_include_directories(jackal PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
# JACKAL.LIB was built with the VC6 SP3 back end (build 8447), the game with
# RTM (8168): the image's Rich header has 66 C++ objects at 8447 beside 126 at
# 8168, and a paired whole-tree screen (2026-11-21) lost no Jackal function
# under SP3 and gained JackalVoiceRx::add_player 0x00633180, while the
# application lost 30. The wrapper's explicit recipe option selects the SP3
# BIN (recovery/environment/wine.py, ~/opt/vc6sp3/PROVENANCE.md).
get_target_property(_jackal_sources jackal SOURCES)
set_property(SOURCE ${_jackal_sources} APPEND PROPERTY COMPILE_OPTIONS "--compiler=vc6sp3")
target_compile_definitions(jackal PRIVATE $<$<CONFIG:Debug>:BUILD_DEBUG>)
target_compile_options(jackal PRIVATE /W4 "--repro-root=${PROJECT_SOURCE_DIR}")
# Firaxis's Caviar wrapper (caviar.cpp) drives the AnimaTek library and
# includes its headers.
target_link_libraries(jackal PRIVATE caviar)

# Native member-reference verification needs these owners' actual CodeView types.
# Qualified /Z7 controls preserve their runtime code, data, EH and FPO content.
set_property(SOURCE checkbox.cpp radiobutton.cpp filewin.cpp palette.cpp
    string_table.cpp buffer.cpp caviar.cpp log.cpp sprite.cpp texheap.cpp
    ddinit.cpp filefind.cpp parse.cpp trig.cpp menu.cpp win.cpp mem_get.cpp
    env_open.cpp hsv.cpp net_class.cpp net_lists.cpp
    APPEND PROPERTY COMPILE_OPTIONS "/Z7")
# Buffer's original hline5E1B94 and init5D77C3 call the CRT memset.
# Keep memory-call selection at TU scope; this is an evidence-led build policy,
# not proof of the original command line. Source byte loops retain inline fills.
set_property(SOURCE buffer.cpp APPEND PROPERTY COMPILE_OPTIONS "/Oi-")
# Preserve original out-of-line close/bit-helper calls in this owner.
# This optimized matching exception must not override Debug's /Ob0.
set_property(SOURCE pulldown.cpp
    APPEND PROPERTY COMPILE_OPTIONS "$<$<NOT:$<CONFIG:Debug>>:/Ob1>")
