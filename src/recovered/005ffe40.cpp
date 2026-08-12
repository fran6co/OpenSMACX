// ORIGINAL: 0x005FFE40 BYTE_EXACT
// name      sub_5ffe40
// size      56 bytes
// spans     0x005FFE40-0x005FFE78
// prototype 
// callers   0   call targets   0
// 0x005FFE40  sub_5ffe40  ->  _sub_5ffe40
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef long (__stdcall *DefWindowProcAProc)(void *, unsigned int, unsigned int, long);

void __cdecl sub_5ffe40(void *a1, unsigned int a2, int a3, unsigned short a4,
                        unsigned short a5) {
    if (a3 != 0) {
        DefWindowProcAProc pDefWindowProcA =
            *reinterpret_cast<DefWindowProcAProc *>(g_006692b0);
        pDefWindowProcA(reinterpret_cast<void *>(*g_009b8194), 0x100, a2,
                       (static_cast<long>(a5) << 16) | a4);
    }
}
