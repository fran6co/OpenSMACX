// ORIGINAL: 0x00432010 BYTE_EXACT
// name      ?on_button_passover@Datalink@@QAEXHH@Z
// size      34 bytes
// spans     0x00432010-0x00432032
// prototype void (__thiscall ?on_button_passover@Datalink@@QAEXHH@Z)(Datalink* this, int, int)
// callers   0   call targets   1
// 0x00432010  ?on_button_passover@Datalink@@QAEXHH@Z  ->  ?on_button_passover@Datalink@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Datalink::on_button_passover(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x29dc) == 0xe) {
        reinterpret_cast<ReportWin *>(g_00876478)->on_button_passover(a1, a2);
    }
}
