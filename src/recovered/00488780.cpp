// 0x00488780  ?list_was_dblclicked@@YAXH@Z  ->  ?list_was_dblclicked@@YAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl list_was_dblclicked(int a1) {
    if (a1 >= 1 && a1 <= 7) {
        reinterpret_cast<PickWin *>(g_00822718)->on_button_clicked(a1);
    }
}
