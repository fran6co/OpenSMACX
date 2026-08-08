// 0x00629DF0  ?close@Flic@@QAEXXZ  ->  ?close@Flic@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

void Flic::close() {
    char *self = reinterpret_cast<char *>(this);
    int fp = *reinterpret_cast<int *>(self + 0x5b8);
    *reinterpret_cast<int *>(self + 0x5ac) = 0;
    *reinterpret_cast<int *>(self + 0x5b0) = 0;
    *reinterpret_cast<int *>(self + 0x5b4) = 0;
    *reinterpret_cast<int *>(self + 0xa10) = 0;
    *reinterpret_cast<int *>(self + 0x5a8) = 0;
    *self = 0;
    *reinterpret_cast<int *>(self + 0x5a4) = 0;
    *reinterpret_cast<int *>(self + 0x59c) = 0;
    *reinterpret_cast<int *>(self + 0x5a0) = 0;
    if (fp != 0) {
        // The catalogued declaration for this callee is `fclose()` (zero
        // args); the call goes through a function-pointer cast to the real
        // one-argument prototype instead of redeclaring the callee.
        reinterpret_cast<int (__cdecl *)(int)>(fclose)(fp);
        *reinterpret_cast<int *>(self + 0x5b8) = 0;
    }

}
