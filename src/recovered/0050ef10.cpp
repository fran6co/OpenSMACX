// ORIGINAL: 0x0050EF10 BYTE_EXACT
// name      ?turn_timer@@YAXH@Z
// size      50 bytes
// spans     0x0050EF10-0x0050EF42
// prototype 
// callers   0   call targets   2
// 0x0050EF10  ?turn_timer@@YAXH@Z  ->  ?turn_timer@@YAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl turn_timer(int a1) {
    if (*g_0093f660 != 0) {
        if (*g_00915620 == 0) {
            if (*g_00703de0 != 0) {
                reinterpret_cast<GraphicWin *>(g_006fec80)->redraw();
            }
            reinterpret_cast<MultiWin *>(g_007fd648)->draw(1);
        }
    }
}
