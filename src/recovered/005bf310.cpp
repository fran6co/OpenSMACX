// ORIGINAL: 0x005BF310 BYTE_EXACT
// name      ?X_pop@@YAHPBDP6AHXZ@Z
// size      32 bytes
// spans     0x005BF310-0x005BF330
// prototype 
// callers   70   call targets   1
// 0x005BF310  ?X_pop@@YAHPBDP6AHXZ@Z  ->  ?X_pop@@YAHPBDP6AHXZ@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

int __cdecl X_pop(int8* a1, int (__cdecl *a2)()) {
    return X_pop(reinterpret_cast<int8 *>(g_009b8aa8), a1, -1, 0, 0, a2);
}
