// 0x004E1F30  ?editor_redo@Console@@QAEXXZ  ->  ?editor_redo@Console@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Console::editor_redo() {
    load_undo(-1);
}
