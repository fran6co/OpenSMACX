// Generic rectangle factory, recovered as a separately callable public API.
// This semantic owner file does not assert the original translation unit.
#include <windows.h>
#include "../rectangle_operations.h"

/*
Purpose: Build a rectangle from an origin and dimensions using wrapping coordinates.
// ORIGINAL: 0x005F86C0 sub_5f86c0 0x005F86C0-0x005F86E6 BYTE_EXACT
// symbol    ?make_rect@@YAPAUtagRECT@@PAU1@HHHH@Z
// size      38 bytes
// prototype 
// callers   2   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
RECT *__cdecl make_rect(RECT *rect, int x, int y, int width, int height) {
    return set_rectangle_dimensions(rect, x, y, width, height);
}
