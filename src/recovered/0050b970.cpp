// ORIGINAL: 0x0050B970 ?popp@@YAHPBDPBDP6AHXZ@Z 0x0050B970-0x0050B991 BYTE_EXACT
// size      33 bytes
// prototype 
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0048C0A0
// 0x0050B970  ?popp@@YAHPBDPBDP6AHXZ@Z  ->  ?popp@@YAHPBD0P6AHXZ@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl popp(const char *a1, const char *a2, int (__cdecl *a3)()) {
    return popp(*reinterpret_cast<char **>(g_00691b0c), a1, 0, a2, a3);
}
