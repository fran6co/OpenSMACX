// 0x00559100  ?get_adjective_insult_leader@@YAPADH@Z  ->  ?get_adjective_insult_leader@@YAPADH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

char * __cdecl get_adjective_insult_leader(int a1) {
    return reinterpret_cast<char *>(g_00946b34) + a1 * 1436;
}
