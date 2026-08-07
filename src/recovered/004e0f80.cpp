// 0x004E0F80  ?editor_polar@Console@@QAEXXZ  ->  ?editor_polar@Console@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Console::editor_polar() {
    auto_undo();
    world_polar_caps();
    world_climate();
    draw_map(1);
}
