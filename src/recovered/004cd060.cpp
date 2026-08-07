// 0x004CD060  ?action_feature@@YAXXZ  ->  ?action_feature@@YAXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void __cdecl action_feature() {
    if (*g_0093f660) {
        reinterpret_cast<NetMsg *>(g_00805338)->pop(
            reinterpret_cast<int8 *>(g_00687fdc), 0x1388, 0, 0);
    }
}
