// ORIGINAL: 0x004B4CD0 BYTE_EXACT
// 0x004B4CD0  ?UNK2@StatusWin@@QAEXXZ  ->  ?UNK2@StatusWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Clear the status sentinel and hide the shared window.
Original Offset: 004B4CD0
Return Value: n/a
Status: Complete
*/
// `Win::hide` is reached by a tail `jmp`, which the call graph does not
// record, so it is declared here with the same calling shape.
class Win { public: void hide(); };

void StatusWin::UNK2() {
    *g_006874c4 = -1;
    reinterpret_cast<Win *>(g_00937118)->hide();
}
