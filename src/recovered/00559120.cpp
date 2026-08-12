// ORIGINAL: 0x00559120 BYTE_EXACT
// name      ?get_adjective_faction@@YAPADH@Z
// size      26 bytes
// spans     0x00559120-0x0055913A
// prototype int8* (__cdecl ?get_adjective_faction@@YAPADH@Z)(int factionID)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// 0x00559120  ?get_adjective_faction@@YAPADH@Z  ->  ?get_adjective_faction@@YAPADH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

char* __cdecl get_adjective_faction(int a1) {
    return reinterpret_cast<char *>(g_00946bb4) + a1 * 1436;
}
