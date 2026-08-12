// ORIGINAL: 0x005BFD50 BYTE_EXACT
// name      ?X_pop_ask_number@@YAHPBDP6AHXZ@Z
// size      28 bytes
// spans     0x005BFD50-0x005BFD6C
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BFD70
// 0x005BFD50  ?X_pop_ask_number@@YAHPBDP6AHXZ@Z  ->  ?X_pop_ask_number@@YAHPBDP6AHXZ@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl X_pop_ask_number(const char* a1, int (__cdecl *a2)()) {
    return X_pop_ask_number(reinterpret_cast<char*>(g_009b8aa8), a1, 0, a2);
}
