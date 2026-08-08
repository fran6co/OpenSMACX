// 0x005FD1C0  ?flush_keyboard@@YAXXZ  ->  ?flush_keyboard@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__stdcall *PeekMessageAProc)(int *, void *, unsigned int, unsigned int, unsigned int);

void __cdecl flush_keyboard() {
    int msg[7];
    PeekMessageAProc peek = reinterpret_cast<PeekMessageAProc>(*g_00669358);
    while (peek(msg, 0, 0x100, 0x108, 1)) {
    }
    *g_009b7acc = 0;
    *g_009b7ad0 = 0;
    check_net();
}
