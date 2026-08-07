// 0x00629D20  ?on_scrolled@StringBox@@QAEXHH@Z  ->  ?on_scrolled@StringBox@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Record the scroll position and repaint through virtual slot 62.
Original Offset: 00629D20
Return Value: n/a
Status: Complete
*/
void StringBox::on_scrolled(int a1, int a2) {
    // a1 is dead: the disassembly never reads [esp+4]. Only the second
    // argument is stored.
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xa1c) = a2;
    reinterpret_cast<VCall *>(this)->slot062();
}
