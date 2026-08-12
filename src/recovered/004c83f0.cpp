// ORIGINAL: 0x004C83F0 BYTE_EXACT
// name      sub_4c83f0
// size      31 bytes
// spans     0x004C83F0-0x004C840F
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x004C83F0  sub_4c83f0  ->  _sub_4c83f0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl sub_4c83f0() {
    VCall *p = *reinterpret_cast<VCall **>(g_0090db20);
    while (p) {
        p->slot005();
        p = *reinterpret_cast<VCall **>(g_0090db20);
    }
    return 0xb;
}
