// ORIGINAL: 0x004412A0 BYTE_EXACT
// name      ?string_callback@DiploWin@@QAAXHH@Z
// size      48 bytes
// spans     0x004412A0-0x004412D0
// prototype 
// callers   0   call targets   1
// 0x004412A0  ?string_callback@DiploWin@@QAAXHH@Z  ->  ?string_callback@DiploWin@@QAAXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

extern "C" void mz_diplo_guard_00745060();

void __cdecl DiploWin::string_callback(int a1, int a2) {
    Buffer *buf;
    if (mz_diplo_guard_00745060) {
        buf = (Buffer *)g_007454a4;
    } else {
        buf = (Buffer *)0;
    }
    buf->set_text_color(g_0068fa30[(int)this], g_0068fa50[(int)this], 1, 1);
}
