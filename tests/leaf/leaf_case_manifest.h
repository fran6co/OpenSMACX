// How many cases each family file is expected to register.
//
// main() checks these at RUN TIME.  A family whose file drops out of the
// CMake source list, or whose static initialisers are discarded, registers
// fewer cases than its line here claims and the run fails loudly instead of
// quietly losing coverage.  This is the one line outside a family file that
// adding a test may touch.
//
// LEAF_FAMILY_COUNT(<family>, <cases>)
LEAF_FAMILY_COUNT(core_construction, 18)
LEAF_FAMILY_COUNT(leaf_recoveries, 5)
LEAF_FAMILY_COUNT(widget_accessors, 30)
LEAF_FAMILY_COUNT(text_and_resources, 15)
LEAF_FAMILY_COUNT(dialog_and_strings, 9)
LEAF_FAMILY_COUNT(win_and_graphic, 15)
LEAF_FAMILY_COUNT(destructors, 12)
LEAF_FAMILY_COUNT(buffer_and_font, 14)
LEAF_FAMILY_COUNT(constant_stubs, 5)
LEAF_FAMILY_COUNT(wave_and_sound, 22)
LEAF_FAMILY_COUNT(delegates, 11)
LEAF_FAMILY_COUNT(forwarders, 10)
LEAF_FAMILY_COUNT(stubs_and_locks, 12)
LEAF_FAMILY_COUNT(locks, 10)
LEAF_FAMILY_COUNT(device_forwarders, 10)
LEAF_FAMILY_COUNT(buffer_writers, 10)
LEAF_FAMILY_COUNT(pops_and_console, 13)
LEAF_FAMILY_COUNT(thunks, 8)
