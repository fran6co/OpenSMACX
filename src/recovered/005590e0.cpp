// ORIGINAL: 0x005590E0 BYTE_EXACT
// name      ?get_adjective_leader@@YAPADH@Z
// size      26 bytes
// spans     0x005590E0-0x005590FA
// prototype int8* (__cdecl ?get_adjective_leader@@YAPADH@Z)(int factionID)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// 0x005590E0  ?get_adjective_leader@@YAPADH@Z  ->  ?get_adjective_leader@@YAPADH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

char* __cdecl get_adjective_leader(int a1) {
    return reinterpret_cast<char *>(g_00946ab4) + a1 * 1436;
}
