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
# Files whose original members also left header-inline copies in game objects
# (net_class, basepop, dialog, dialogs, win, ...) still live in src/ until they
# are split.
add_library(jackal STATIC
    jackal.cpp palette.cpp font.cpp graphics_language.cpp heap.cpp
    time.cpp
    checkbutton.cpp radiobutton.cpp checkbox.cpp filewin.cpp
    autosound.cpp buffer.cpp buttongroup.cpp caviar.cpp cursor.cpp
    ddinit.cpp directdraw.cpp editbox.cpp editgroup.cpp filebox.cpp
    filefind.cpp filemap.cpp imagebutton.cpp listbox.cpp log.cpp
    mci_video.cpp menu.cpp net_fifo.cpp net_session_picker.cpp
    number_parsing.cpp parse.cpp polygon.cpp popup_api.cpp pulldown.cpp
    pushbutton.cpp random.cpp spot.cpp sprite.cpp spritebox.cpp
    string_table.cpp text.cpp texheap.cpp texture.cpp timer_messages.cpp
    trig.cpp vector.cpp video.cpp video_queue.cpp win_coordinates.cpp
    win_input.cpp
    dialog.cpp
    dialog_style.cpp
    stringlist.cpp flic.cpp basebutton.cpp graphicwin.cpp
    flatbutton.cpp
    scroll.cpp
    stringbox.cpp
    dialogs.cpp
    basepop.cpp)
target_include_directories(jackal PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(jackal PRIVATE $<$<CONFIG:Debug>:BUILD_DEBUG>)
target_compile_options(jackal PRIVATE /W4 "--repro-root=${PROJECT_SOURCE_DIR}")
# Firaxis's Caviar wrapper (caviar.cpp) drives the AnimaTek library and
# includes its headers.
target_link_libraries(jackal PRIVATE caviar)

# Native member-reference verification needs these owners' actual CodeView types.
# Qualified /Z7 controls preserve their runtime code, data, EH and FPO content.
set_property(SOURCE checkbox.cpp radiobutton.cpp filewin.cpp palette.cpp
    string_table.cpp buffer.cpp caviar.cpp log.cpp sprite.cpp texheap.cpp
    ddinit.cpp filefind.cpp parse.cpp trig.cpp menu.cpp
    APPEND PROPERTY COMPILE_OPTIONS "/Z7")
# Buffer's original hline5E1B94 and init5D77C3 call the CRT memset.
# Keep memory-call selection at TU scope; this is an evidence-led build policy,
# not proof of the original command line. Source byte loops retain inline fills.
set_property(SOURCE buffer.cpp APPEND PROPERTY COMPILE_OPTIONS "/Oi-")
# Preserve original out-of-line close/bit-helper calls in this owner.
# This optimized matching exception must not override Debug's /Ob0.
set_property(SOURCE pulldown.cpp
    APPEND PROPERTY COMPILE_OPTIONS "$<$<NOT:$<CONFIG:Debug>>:/Ob1>")
