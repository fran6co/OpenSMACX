// ORIGINAL: 0x004832D0 BYTE_EXACT
// name      ?string_callback@NetWin@@QAAXHH@Z
// size      50 bytes
// spans     0x004832D0-0x00483302
// prototype 
// callers   0   call targets   2
// 0x004832D0  ?string_callback@NetWin@@QAAXHH@Z  ->  ?string_callback@NetWin@@QAAXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

extern "C" void mz_netwin_guard_80b438();

void __cdecl NetWin::string_callback(int a1, int a2) {
    Buffer *buf;
    if (mz_netwin_guard_80b438) {
        buf = (Buffer *)g_0080b87c;
    } else {
        buf = (Buffer *)0;
    }
    buf->set_text_color(((NetWin *)g_0080a6f8)->get_player_color((int)this), 0, 1, 1);
}
