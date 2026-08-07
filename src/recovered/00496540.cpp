// 0x00496540  ?on_selected@QuayleWin@@QAEXHH@Z  ->  ?on_selected@QuayleWin@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Record the selected item and repaint through virtual slot 62.
Original Offset: 00496540
Return Value: n/a
Status: Complete
*/
void QuayleWin::on_selected(int a1, int a2) {
    // a2 is unused; only the first argument is stored.
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0x2144) = a1;
    reinterpret_cast<VCall *>(this)->slot062();
}
