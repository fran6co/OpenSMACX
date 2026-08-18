// ORIGINAL: 0x0050B9C0 sub_50b9c0 0x0050B9C0-0x0050B9DE BYTE_EXACT
// size      30 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890
// 0x0050B9C0  sub_50b9c0  ->  _sub_50b9c0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl sub_50b9c0(char *a1, char *a2) {
    reinterpret_cast<NetMsg *>(g_00805338)->pop(a1, 5000, 0, a2);
}
