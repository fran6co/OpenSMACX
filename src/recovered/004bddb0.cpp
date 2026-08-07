// 0x004BDDB0  ?on_move@TutWin@@QAEXHH@Z  ->  ?on_move@TutWin@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Tell the attached object the tutorial window moved.
Original Offset: 004BDDB0
Return Value: n/a
Status: Complete
*/
void TutWin::on_move(int a1, int a2) {
    // field_53D4_ is typed int32_t by the scaffolding but holds a pointer to
    // a vtable-bearing object. Both arguments are dead.
    if (field_53D4_ != 0) {
        reinterpret_cast<VCall *>(field_53D4_)->slot063();
    }
}
