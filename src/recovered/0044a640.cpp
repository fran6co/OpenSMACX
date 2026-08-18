// ORIGINAL: 0x0044A640 ?on_group_clicked@FameWin@@QAEXHHH@Z 0x0044A640-0x0044A655 BYTE_EXACT
// size      21 bytes
// prototype void (__thiscall ?on_group_clicked@FameWin@@QAEXHHH@Z)(FameWin* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0044A64C
// 0x0044A640  ?on_group_clicked@FameWin@@QAEXHHH@Z  ->  ?on_group_clicked@FameWin@@QAEXHHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Clear the group selection and repaint through virtual slot 62.
Original Offset: 0044A640
Return Value: n/a
Status: Complete
*/
void FameWin::on_group_clicked(int a1, int a2, int a3) {
    // All three arguments are dead; `ret 0xc` still pops them.
    *reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 0xa918) = 0;
    reinterpret_cast<VCall *>(this)->slot062();
}
