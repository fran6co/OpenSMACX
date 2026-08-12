// ORIGINAL: 0x004D0450 BYTE_EXACT
// 0x004D0450  ?skip@Console@@QAEXH@Z  ->  ?skip@Console@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Console::skip(int a1) {
    // BODY GOES HERE.
    //
    // Reach fields by offset - the class is deliberately empty:
    //     char *self = reinterpret_cast<char *>(this);
    //     int v = *reinterpret_cast<int *>(self + 0x24);

    if (veh_ready(a1) != 0) {
        int result = not_my_turn();
        if (result == 0) {
            reinterpret_cast<NetDaemon *>(g_0093cd90)->order_veh(a1, -1, result);
            return;
        }
    }
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int *>(self + 0x23be4) = 0;
}
