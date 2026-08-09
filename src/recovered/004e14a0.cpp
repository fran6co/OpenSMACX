// ORIGINAL: 0x004E14A0
// 0x004E14A0  ?editor_rockiness@Console@@QAEXXZ  ->  ?editor_rockiness@Console@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Console::editor_rockiness() {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);
    auto_undo();
    if (X_pop(reinterpret_cast<const char *>(g_00689174), 0x40, 0) != 0) {
        world_rocky();
        draw_map(1);
    }
}
