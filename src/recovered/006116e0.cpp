// ORIGINAL: 0x006116E0 BYTE_EXACT
// name      sub_6116e0
// size      72 bytes
// spans     0x006116E0-0x00611728
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00611730 0x00644EF2
// 0x006116E0  sub_6116e0  ->  _sub_6116e0
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

// reads [ecx-0x28] with no stack access before any call - the receiver is
// `this`, so this is a __thiscall member, not a free function.
class Handle { public:
    void prep();  // sub_611730, thiscall on the same object
    int detach(char a1);
};

int Handle::detach(char a1) {
    // `mov al, byte ptr [esp+0xc]` reads a single byte off the stack, not a
    // full dword - the parameter is a byte-sized type, not `int`.
    char *outer = reinterpret_cast<char *>(this) - 0x28;
    Handle *self_ptr = reinterpret_cast<Handle *>(outer + 0x28);
    self_ptr->prep();
    int *self = reinterpret_cast<int *>(self_ptr);
    self[0] = reinterpret_cast<int>(g_006693ac);
    int val = self[1];
    *g_009b3374 = val;
    if (a1 & 1) {
        if (outer != 0) {
            if (val == 0) {
                free(outer);
            }
            *g_009b3374 = 0;
        }
    }
    return reinterpret_cast<int>(outer);
}
