// ORIGINAL: 0x004C3910 BYTE_EXACT
// name      sub_4c3910
// size      49 bytes
// spans     0x004C3910-0x004C3941
// prototype 
// callers   0   call targets   2
// 0x004C3910  sub_4c3910  ->  _sub_4c3910
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl sub_4c3910() {
    reinterpret_cast<ButtonGroup *>(g_007cd12c)->set(0x3ee, 0);
    char *object = *reinterpret_cast<char **>(g_007d3c40);
    reinterpret_cast<VCall *>(
        object + (*reinterpret_cast<int **>(object))[1])->slot002();
    reinterpret_cast<Win *>(g_008e9f60)->show(0);
}
