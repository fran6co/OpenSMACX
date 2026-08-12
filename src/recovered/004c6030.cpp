// ORIGINAL: 0x004C6030 BYTE_EXACT
// name      sub_4c6030
// size      34 bytes
// spans     0x004C6030-0x004C6052
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C6047
// 0x004C6030  sub_4c6030  ->  _sub_4c6030
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

typedef int (__cdecl *ApiFn1)(int);

int __cdecl sub_4c6030(int a1) {
    if (!*g_0090db7c) {
        return 1;
    }
    return (*reinterpret_cast<ApiFn1 *>(g_0090db28))(a1);
}
