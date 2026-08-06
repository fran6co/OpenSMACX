// 0x00401000  ??0StringStruct@@QAE@H@Z  ->  ??0StringStruct@@QAE@H@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

StringStruct::StringStruct(int a1) {
    int *p = reinterpret_cast<int *>(this);
    int zero = 0;
    int guard = zero;
    if (zero) {
        *g_009b3374 = reinterpret_cast<int>(&guard);
    }
    if (a1 != 0) {
        p[1] = reinterpret_cast<int>(g_006693b0);
        p[7] = reinterpret_cast<int>(g_006693ac);
        p[8] = *g_009b3374;
        *g_009b3374 = zero;
    }
    p[0] = reinterpret_cast<int>(g_006693a4);
    *reinterpret_cast<int *>(reinterpret_cast<char *>(p) + *reinterpret_cast<int *>(p[1] + 4) + 4) = reinterpret_cast<int>(g_006693a0);
    p[2] = zero;
    p[3] = zero;
    p[4] = zero;
    p[5] = zero;
    p[6] = zero;
}
