// 0x004E0FA0  ?editor_climate@Console@@QAEXXZ  ->  ?editor_climate@Console@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Console::editor_climate() {
    auto_undo();
    if (!custom_planet(0, 0)) {
        world_climate();
        draw_map(1);
    }
}
