// ORIGINAL: 0x0050B8F0 BYTE_EXACT
// name      sub_50b8f0
// size      26 bytes
// spans     0x0050B8F0-0x0050B90A
// prototype 
// callers   2   call targets   0
// 0x0050B8F0  sub_50b8f0  ->  _sub_50b8f0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl sub_50b8f0(int a1) {
    if (a1 < 0) {
        return a1;
    }
    return *reinterpret_cast<short *>(reinterpret_cast<char *>(g_00952858) + a1 * 0x34);
}
