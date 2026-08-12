// ORIGINAL: 0x00477F00 BYTE_EXACT
// name      ?on_button_clicked@MultiWin@@QAEXH@Z
// size      48 bytes
// spans     0x00477F00-0x00477F30
// prototype void (__thiscall ?on_button_clicked@MultiWin@@QAEXH@Z)(MultiWin* this, int)
// callers   2   call targets   2
// 0x00477F00  ?on_button_clicked@MultiWin@@QAEXH@Z  ->  ?on_button_clicked@MultiWin@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void MultiWin::on_button_clicked(int a1) {
    switch (a1) {
    case 0:
        reinterpret_cast<Console *>(g_009156b0)->chat(0);
        break;
    case 1:
        call_council(*g_00939284);
        break;
    }
}
